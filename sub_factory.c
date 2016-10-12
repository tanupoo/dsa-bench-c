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
#include <dslink/requester.h>
#define LOG_TAG "dsbench"	// dslink/log.h requires...
#include <dslink/log.h>
#include <dslink/ws.h>
#include <dslink/utils.h>

#include "sub.h"
#include "sub_factory.h"

#include "smelt.h"
struct smelt *mt;

static int (*is_finished)(char *);

static void
sub_on_req_close(struct DSLink *link, ref_t *req_ref, json_t *resp)
{
	(void) link;
	(void) resp;
	(void) req_ref;
	json_t *rid = json_object_get(resp, "rid");
	log_info("Request %i closed.\n", (int) json_integer_value(rid));
}

static int
is_finished_by_message_counter(char *s_val)
{
	int counter;
	char *p, *bp;

	for (p = s_val; *p != '\0' && *p != ','; p++)
		;
	if (*p == '\0') {
		printf("invalid format. [%s]\n", s_val);
		return -1;
	}
	counter = strtol(s_val, &bp, 10);
	if (*bp != ',') {
		printf("invalid format. [%s]\n", s_val);
		return -1;
	}

	return counter < max_count;
}

static int
is_finished_by_number_of_received_messages(char *s_val)
{
	static int counter = 0;

	counter++;

	return counter < max_count;
}

static void
sub_on_value_update(struct DSLink *link, uint32_t sid, json_t *val,
		json_t *ts)
{
	char *s_time = (char *)json_string_value(ts);
	char *s_val = (char *)json_string_value(val);
	int ret;

	struct smelt_info mi;
	static int mid = 1;
	mi.mid = mid++;
	mi.len_hdr = 0;
	mi.len_sys = 0;

	if (f_debug)
		printf("Got sid=%d v=%s\n", sid, s_val);

	smelt_start_ts(mt, &mi, s_time); /* ts */
	if (f_debug)
		printf("  ts=%s\n", s_time);
#ifdef USE_SMELT_E_TS
{
	char buf[64];
	strftimeval_current(buf, sizeof(buf), NULL);
	smelt_end_ts(mt, &mi, buf);
	if (f_debug)
		printf("  recv=%s\n", buf);
}
#else
	smelt_end_tv(mt, &mi);
#endif

	ret = (*is_finished)(s_val);
	if (ret < -1) {
		dslink_close(link);
		exit(0);
	}
	if (ret)
		return;

	/* end of requester */
	dslink_close(link);
	smelt_print_result(mt);

	exit(0);
}

static void
sub_on_ready(DSLink *link)
{
	ref_t *ref;

	ref = dslink_requester_subscribe(link, node_name,
			sub_on_value_update, sub_qos);
	if (ref == NULL) {
		log_fatal("failed to subscribe to %s\n", node_name);
		return;
	}

	((RequestHolder *)ref->data)->close_cb = sub_on_req_close;
}

// Called to initialize your node structure.
static void
sub_on_init(DSLink *link)
{
//	if (factory_init(link, link->responder->super_root) < 0)
//		errx(1, "ERROR: factory_init() failed.");

	log_info("Initialized!\n");
}

// Called when the DSLink is connected.
static void
sub_on_connected(DSLink *link)
{
	(void) link;
	log_info("Connected!\n");
}

// Called when the DSLink is disconnected.
// If this was not initiated by dslink_close,
// then a reconnection attempt is made.
static void
sub_on_disconnected(DSLink *link)
{
	(void) link;
	log_info("Disconnected!\n");
}

// The main function.
int
requester_start(int argc, char **argv, char *my_name)
{
	DSLinkCallbacks cbs = {  // Create our callback struct.
	    sub_on_init,         // init_cb
	    sub_on_connected,    //on_connected_cb
	    sub_on_disconnected, // on_disconnected_cb
	    sub_on_ready         // on_requester_ready_cb
	};

	/*
	 * decision polity to finish the measurement.
	 * 1: the number of received messages reaches the max_counter.
	 * 0: counter in the message reaches the max_counter.
	 */
	if (f_end_policy)
		is_finished = is_finished_by_number_of_received_messages;
	else
		is_finished = is_finished_by_message_counter;

#ifdef USE_SMELT_E_TS
	mt = smelt_init(SMELT_MODE_S_TS|SMELT_MODE_E_TS, 100000, 0, 0, 0);
#else
	mt = smelt_init(SMELT_MODE_S_TS, 100000, 0, 0, 0);
#endif

	// Initializes a DSLink and handles reconnection.
	// Pass command line arguments, our dsId, requester or responder?
	// and a reference to our callbacks.
	return dslink_init(argc, argv, my_name, 1, 0, &cbs);
}
