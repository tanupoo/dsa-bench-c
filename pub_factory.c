/*
 * Copyright (C) 2000 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include <dslink/dslink.h>
#define LOG_TAG "dsbench"	// dslink/log.h requires...
#include <dslink/log.h>
#include <dslink/ws.h>
#include <dslink/utils.h>
#include <jansson.h>
#include <uv.h>

#include "pub.h"
#include "pub_factory.h"
#include "smelt.h"

struct value_holder {
	DSLink *link;
	DSNode *node;
	char *node_name;
	char *base_string;
	uint32_t counter;
};

static const char *
mk_val(struct value_holder *vh)
{
	char *bp;
	int len, rest_len;

	rest_len = value_size;
	bp = vh->base_string;
	len = snprintf(bp, rest_len, "%d,", vh->counter);
	if (len == rest_len)
		return vh->base_string;

	rest_len -= len;
	bp += len;
	len = strftimeval_current(bp, rest_len, NULL);

	rest_len -= len;
	bp += len;
	if (rest_len > 0) {
		while (rest_len--)
			*bp++ = 'x';
		*bp = '\0';
	}

	return vh->base_string;
}

static void
set_value(uv_timer_t *timer)
{
	struct value_holder *vh = (struct value_holder *)timer->data;

	vh->counter++;

	/*
	 * check the counter if it reaches the max.
	 */
	if (vh->counter > max_count) {
		/* shutdown */
		log_info("done %d\n", vh->counter);
		uv_timer_stop(timer);
		uv_close((uv_handle_t *)timer, (uv_close_cb)dslink_free);
		dslink_close(vh->link);
		return;
	}

	/*
	 * set value to topic node
	 */
	const char *val = mk_val(vh);
	json_t *j_val = json_string(val);

	if (dslink_node_set_value(vh->link, vh->node, j_val) != 0) {
		log_warn("failed to set the value of %s\n", vh->node_name);
		json_decref(j_val);
		dslink_node_tree_free(vh->link, vh->node);
		return;
	}

	if (f_debug) {
		printf("published %s\n", val);
	}
}

static char *
new_base_string(int value_size)
{
	char *buf = malloc(1 + value_size);
	int i;

	for (i = 0; i < value_size; i++)
		buf[i] = 'x';
	buf[value_size] = '\0';

	return buf;
}

static int
factory_init(DSLink *link, DSNode *super_root)
{
	/*
	 * create a test_node
	 */
	DSNode *test_node = dslink_node_create(super_root, node_name, "node");
	if (!test_node) {
		log_warn("failed to create a node, %s.\n", node_name);
		return -1;
	}

	json_t *j = json_string("string");
	if (dslink_node_set_meta(link, test_node, "$type", j) != 0) {
		log_warn("failed to set the type.\n");
		dslink_node_tree_free(link, test_node);
		json_decref(j);
		return -1;
	}
	json_decref(j);

	if (dslink_node_add_child(link, test_node) != 0) {
		log_warn("failed to add a node, %s\n", node_name);
		dslink_node_tree_free(link, test_node);
		return -1;
	}

	/*
	 * initialize params to be passed to a callback, i.e. set_value().
	 */
	struct value_holder *vh = malloc(sizeof(struct value_holder));
	if (vh == NULL) {
		log_err("failed to malloc(value_holder)\n");
		dslink_node_tree_free(link, test_node);
		return -1;
	}
	vh->link = link;
	vh->node = test_node;
	vh->node_name = node_name;
	vh->base_string = new_base_string(value_size);
	vh->counter = 0;

	/*
	 * set timer
	 */
	uv_timer_t *timer = malloc(sizeof(uv_timer_t));
	uv_timer_init(&link->loop, timer);
	timer->data = vh;
	uv_timer_start(timer, set_value, 0, update_interval);

	return 0;
}

static void
pub_on_init(DSLink *link)
{
	if (factory_init(link, link->responder->super_root) < 0)
		errx(1, "ERROR: factory_init() failed.");

	log_info("Initialized!\n");
}

static void
pub_on_connected(DSLink *link)
{
	log_info("Connected!\n");
}

static void
pub_on_disconnected(DSLink *link)
{
	log_info("Disconnected!\n");
}

int
responder_start(int argc, char **argv, char *my_name)
{
	DSLinkCallbacks cbs = {
	    pub_on_init,
	    pub_on_connected,
	    pub_on_disconnected,
	    NULL
	};

	return dslink_init(argc, argv, my_name, 0, 1, &cbs);
}
