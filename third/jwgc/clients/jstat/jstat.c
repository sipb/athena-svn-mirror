#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <libjwgc.h>

char *whoami;

void
usage()
{
	fprintf(stderr, "Usage: %s %s[-h]\n%s\
  -h       Display help\n\
",
	whoami,
#ifdef NODEBUG
	"",
	""
#else
	"[-d <flags>] ",
	"  -d       Enable/Disable debugging (leave <flags> blank for usage)\n"
#endif /* NODEBUG */
	);
	exit(1);
}

void
jstat_on_event_handler(jwgconn conn, jwgpacket packet)
{
	xode x, y;
	char *cdata;

	x = packet->x;
	cdata = xode_get_tagdata(x, "user");
	if (cdata) {
		printf("User:               %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "server");
	if (cdata) {
		printf("Server:             %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "resource");
	if (cdata) {
		printf("Resource:           %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "version");
	if (cdata) {
		printf("Jwgc Version:       %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "machinetype");
	if (cdata) {
		printf("Machine Type:       %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "localtime");
	if (cdata) {
		printf("Localhost Time:     %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "connecttime");
	if (cdata) {
		printf("Connection Time:    %.58s\n", cdata);
	}

	cdata = xode_get_tagdata(x, "connectstate");
	if (cdata) {
		printf("Connection State:   %.58s\n", cdata);
	}

	printf("\nServices Available:\n");
	y = xode_get_tag(x, "agents");
	y = xode_get_firstchild(y);
	while (y) {
		char *name, *service, *jid;
		xode z;

		name = xode_get_attrib(y, "name");
		service = xode_get_attrib(y, "service");
		jid = xode_get_attrib(y, "jid");
		printf("    %-30.30s %41.41s\n", name, jid);
		printf("          Provides: %s", service);
		z = xode_get_firstchild(y);
		while (z) {
			printf(" %s", xode_get_name(z));
			z = xode_get_nextsibling(z);
		}
		printf("\n");
		y = xode_get_nextsibling(y);
	}

	cdata = xode_get_tagdata(x, "bugreport");
	if (cdata) {
		printf("\nPlease report bugs to %s.\n", cdata);
	}
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int arg;
	xode x;
	jwgconn jwg;

	whoami = argv[0];
	dinit();

	arg = 1;

	for (; arg < argc; arg++) {
		switch (argv[arg][1]) {
#ifndef NODEBUG
			case 'd':
				arg++;
				if (arg >= argc) {
					dprinttypes();
				}
				dparseflags(argv[arg]);
				break;
#endif /* NODEBUG */
			case 'h':
				usage();
				break;
			default:
				fprintf(stderr, "Unknown option: %s\n",
					argv[arg]);
				usage();
		}
	}

	jwg = jwg_new();
	if (!jwg) {
		fprintf(stderr, "jstat: failed to initialize jwgc connection\n");
		exit(1);
	}
	jwg_event_handler(jwg, jstat_on_event_handler);
	jwg_start(jwg);
	if (jwg_getfd(jwg) < 0) {
		fprintf(stderr, "jstat: failed to create jwgc connection\n");
		exit(1);
	}
	x = xode_new("status");
	jwg_send(jwg, x);
	xode_free(x);

	jwg_poll(jwg, -1);
	jwg_stop(jwg);

	exit(0);
}
