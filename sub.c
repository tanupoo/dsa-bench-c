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
#include <getopt.h>
#include <err.h>

#include "sub.h"
#include "sub_factory.h"
#include "cbench.h"

char *my_name = DEFAULT_REQUESTER_NAME;
int max_count = DEFAULT_MAX_COUNTER;
int f_end_policy = DEFAULT_END_POLICY;
char *node_name = "/downstream/" DEFAULT_RESPONDER_NAME "/" DEFAULT_NODE_NAME;
int sub_qos = DEFAULT_DS_QOS;
int f_debug = 0;

char *prog_name = NULL;

static void
usage()
{
	printf("Usage: %s [OPTIONS]\n", prog_name);
	printf(
"    -B,--broker=url     set the broker URL. (required)\n"
"                        e.g. http://dsa-broker.example.com/conn\n"
"    -D,--dslink=string  set the dslink name of this. (default:%s)\n"
"    -l,--log=level      set the log level.  (info,none,..., default:%s)\n"
"    -m,--max-count=num  set the max value of the counter. (default:%d)\n"
"                        see -p option also.\n"
"    -p,--policy=num     set the policy to decide whether to finish\n"
"                        the measurement. (default:%d)\n"
"                          0: counter in the message reaches.\n"
"                          1: the number of received messages.\n"
"    -n,--node=string    set the node name to be subscribed.\n"
"                        (default:%s)\n"
"    -q,--qos=num        set the level of the QoS. (default:%d)\n"
"    -d,--debug          increase verbosity.\n"
"    -h,--help           show this help menu.\n"
	, DEFAULT_REQUESTER_NAME, DEFAULT_DS_LOG_LEVEL,
	DEFAULT_MAX_COUNTER, DEFAULT_END_POLICY, node_name,
	DEFAULT_DS_QOS);

	exit(0);
}

static void
run(char *my_name, char *broker_url, char *log_level)
{
	int argc = 5;
	char **argv;

	if (broker_url == NULL) {
		warnx("ERROR: broker_url must be set.");
		usage();
	}

	if (my_name == NULL) {
		warnx("ERROR: dslink name must be set.");
		usage();
	}

	if ((argv = calloc(5, sizeof(char *))) == NULL)
		err(1, "calloc(argv)");

	argv[0] = prog_name;
	argv[1] = "--broker";
	argv[2] = broker_url;
	argv[3] = "--log";
	argv[4] = log_level;

	requester_start(argc, argv, my_name);

	warnx("INFO: finished.");

	return;
}

int
main(int argc, char *argv[])
{
	prog_name = 1 + rindex(argv[0], '/');

	const char *shortopts = "D:B:l:m:p:n:q:dh";
	struct option longopts[] = {
	    {"dslink",    required_argument,  NULL,   'D'},
	    {"broker",    required_argument,  NULL,   'B'},
	    {"log",       required_argument,  NULL,   'l'},
	    {"max-count", required_argument,  NULL,   'm'},
	    {"policy",    required_argument,  NULL,   'p'},
	    {"node-name", required_argument,  NULL,   'n'},
	    {"qos",       required_argument,  NULL,   'q'},
	    {"debug",     no_argument,        NULL,   'd'},
	    {"help",      no_argument,        NULL,   'h'},
	    {NULL,        0,                  NULL,   0}
	};
	char *broker_url = NULL;
	char *log_level = DEFAULT_DS_LOG_LEVEL;

	while(1) {
		int ch = getopt_long(argc, argv, shortopts, longopts, NULL);
		if (ch == -1)
			break;
		switch (ch) {
		case 'D':
			my_name = strdup(optarg);
			break;
		case 'B':
			broker_url = strdup(optarg);
			break;
		case 'l':
			log_level = strdup(optarg);
			break;
		case 'm':
			max_count = atoi(optarg);
			break;
		case 'p':
			f_end_policy = atoi(optarg);
			break;
		case 'n':
			node_name = strdup(optarg);
			break;
		case 'q':
			sub_qos = atoi(optarg);
			break;
		case 'd':
			f_debug++;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	run(my_name, broker_url, log_level);

	return 0;
}
