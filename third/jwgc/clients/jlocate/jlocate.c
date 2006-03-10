#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <libjwgc.h>

int showall, groupmode, hideextra, swapjid, exactmatch;
char *target, *whoami;

void usage() {
	fprintf(stderr, "Usage: %s %s[-h] [-e] [-a] [-g] [-n] [-o] [-v] <account name>\n%s\
  -a   Show full contact list (normally unavailable is filtered)\n\
  -g   Show contacts by group\n\
  -e   Exact match on account name (no effect if none specified)\n\
  -h   Display help\n\
  -n   Show nickname on status line instead of jid\n\
  -o   Omit extra information (nickname or jid, depending on if -n is set)\n\
  -v   Verbose output\n\
\n\
  If no <account name> is provided, it returns your full contact list\n\
",
	whoami,
#ifdef NODEBUG
	"",
	""
#else
	"[-d <flags>] ",
	"  -d   Enable/Disable debugging (leave <flags> blank for usage)\n"
#endif /* NODEBUG */
	);
	exit(1);
}

void jlocate_standard_on_event_handler(jwgconn conn, jwgpacket packet) {
	xode x, r, rr;
	char *jid, *status, *nick, *resid;
	int cnt = 0;

	x = packet->x;
	x = xode_get_tag(x, "contact");
	for (;;) {
		if (!x) break;
		jid = xode_get_attrib(x, "jid");
		nick = xode_get_attrib(x, "nick");

		r = xode_get_tag(x, "resources");
		rr = xode_get_firstchild(r);
		if (!rr) {
			printf("%-49.49s %-15.15s %.16s\n",
				swapjid && nick ? nick : jid,
				"",
				"unavailable");
		}
		while (rr) {
			resid = xode_get_attrib(rr, "name");
			status = xode_get_attrib(rr, "status");
			printf("%-49.49s %-15.15s %.16s\n",
				swapjid && nick ? nick : jid,
				resid && strcmp(resid, "NULL") ? resid : "",
				status ? status : "unavailable");
			rr = xode_get_nextsibling(rr);
		}

		if (!hideextra) {
			if (swapjid && nick) {
				printf("    JID: %.69s\n", jid);
			}
			else if (!swapjid && nick) {
				printf("    Nickname: %.64s\n", nick);
			}
		}
		cnt++;
		x = xode_get_nextsibling(x);
	}

	printf("%d contacts found.\n", cnt);
}

void jlocate_grouped_on_event_handler(jwgconn conn, jwgpacket packet) {
	xode x, y, r, rr;
	char *jid, *status, *group, *nick, *resid;
	int cnt = 0;
	char *spacer;

	x = packet->x;
	x = xode_get_tag(x, "group");
	while (x) {
		group = xode_get_attrib(x, "name");
		if (group) {
			printf("%s:\n", strcmp(group,"") ? group : "Unknown");
			spacer = "    ";
		}
		else {
			spacer = "";
		}
		y = x;
		y = xode_get_tag(y, "contact");
		while (y) {
			jid = xode_get_attrib(y, "jid");
			nick = xode_get_attrib(y, "nick");

			r = xode_get_tag(y, "resources");
			rr = xode_get_firstchild(r);
			if (!rr) {
				printf("%s%-45.45s %-15.15s %.16s\n",
					spacer,
					swapjid && nick ? nick : jid,
					"",
					"unavailable");
			}
			while (rr) {
				resid = xode_get_attrib(rr, "name");
				status = xode_get_attrib(rr, "status");
				printf("%s%-45.45s %-15.15s %.16s\n",
					spacer,
					swapjid && nick ? nick : jid,
					resid && strcmp(resid, "NULL") ? resid : "",
					status ? status : "unavailable");
				rr = xode_get_nextsibling(rr);
			}

			if (!hideextra) {
				if (swapjid && nick) {
					printf("%s    JID: %.65s\n",
						spacer,
						jid);
				}
				else if (!swapjid && nick) {
					printf("%s    Nickname: %.60s\n",
						spacer,
						nick);
				}
			}
			cnt++;
			y = xode_get_nextsibling(y);
		}
		x = xode_get_nextsibling(x);
	}

	printf("%d contacts found.\n", cnt);
}

int main(argc, argv)
	int argc;
	char *argv[];
{
	int arg;
	xode x;
	jwgconn jwg;

	showall = hideextra = swapjid = groupmode = exactmatch = 0;
	target = NULL;

	whoami = argv[0];
	dinit();

	arg = 1;

	for (; arg < argc; arg++) {
		if (*argv[arg] != '-') {
			if (target) {
				usage();
			}
			target = argv[arg];
			showall = 1;
			continue;
		} 

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
			case 'a':
				showall = 1;
				break;
			case 'g':
				groupmode = 1;
				break;
			case 'h':
				usage();
				break;
			case 'e':
				exactmatch = 1;
				break;
			case 'n':
				swapjid = 1;
				break;
			case 'o':
				hideextra = 1;
				break;
			default:
				fprintf(stderr, "Unknown option: %s\n",
						argv[arg]);
				usage();
		}
	}

	jwg = jwg_new();
	if (!jwg) {
		fprintf(stderr, "jlocate: failed to initialize jwgc connection\n");
		exit(1);
	}
	if (groupmode) {
		jwg_event_handler(jwg, jlocate_grouped_on_event_handler);
	}
	else {
		jwg_event_handler(jwg, jlocate_standard_on_event_handler);
	}
	jwg_start(jwg);
	if (jwg_getfd(jwg) < 0) {
		fprintf(stderr, "jlocate: failed to create jwgc connection\n");
		exit(1);
	}
	x = xode_new("locate");
	if (target) {
		xode_put_attrib(x, "target", target);
		if (exactmatch) {
			xode_put_attrib(x, "match", "strict");
		}
		else {
			xode_put_attrib(x, "match", "loose");
		}
	}
	if (groupmode) {
		xode_put_attrib(x, "organization", "group");
	}
	if (showall) {
		xode_put_attrib(x, "showall", "yes");
	}
	jwg_send(jwg, x);
	xode_free(x);

	jwg_poll(jwg, -1);
	jwg_stop(jwg);

	exit(0);
}
