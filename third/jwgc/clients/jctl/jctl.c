#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <libjwgc.h>

char *whoami;
char *storedjid;
char *storedmode;
jwgconn jwga;
jwgconn jwgb;

#ifdef HAVE_USLEEP
#define SLEEP_CNT 40
#define SLEEP_TIME 25000
#define SLEEP_CALL usleep
#else
#define SLEEP_CNT 10
#define SLEEP_TIME 1
#define SLEEP_CALL sleep
#endif /* HAVE_USLEEP */

void jctl_do_sendrecv(jwgconn j, xode x);

void 
usage()
{
	fprintf(stderr, "Usage: %s %s[-h] command <args...>\n%s\
  -h       Display help\n\
\n\
  Commands:\n\
     help                      Display help\n\
     reread                    Reread description file (.jwgc.desc)\n\
     subscribe <jid>           Subscribe to <jid>'s presence\n\
     unsubscribe <jid>         Unsubscribe from <jid>'s presence\n\
     nickname <jid> <nick>     Sets a nickname <nick> on <jid>\n\
     group <jid> <group>       Sets a group <group> on <jid>\n\
     set <var> <setting>       Set variable <var> to <setting>\n\
     show <var>                Show variable setting of <var>\n\
     shutdown                  Log off of jabber server and shut down jwgc\n\
     join <chatroom jid>       Join a jabber groupchat room\n\
     leave <chatroom jid>      Leave a jabber groupchat room\n\
     register <jid>            Register with agent <jid>\n\
     search <jid>              Search with agent <jid>\n%s\
\n\
  <nick> in nickname and <group> in group can be left blank to unset a\n\
  nickname or group, respectively.\n\
",
	whoami,
#ifdef NODEBUG
	"",
	"",
	""
#else
	"[-d <flags>] ",
	"  -d       Enable/Disable debugging (leave <flags> blank for usage)\n",
	"     debug <debug flags>       Modify current debugging flags\n"
#endif /* NODEBUG */
	);

	exit(1);
}

void
jctl_on_event_handler(conn, packet)
	jwgconn conn;
	jwgpacket packet;
{
        xode x;
	char *cdata;

	x = packet->x;
	cdata = xode_get_data(x);

	if (!strcmp(xode_get_name(x), "iq")) {
		xode query, form, error, xtag;
		char *type;


		error = xode_get_tag(x, "error");
		if (error) {
			char *errormsg = xode_get_data(error);
			if (errormsg) {
				printf("ERROR: %s\n", errormsg);
			}
			return;
		}

		query = xode_get_tag(x, "query");
		if (query) {
			xtag = xode_get_tag(query, "x");
			if (xtag) {
				type = xode_get_attrib(xtag, "type");
				if (type) {
					if (!strcmp(type, "result")) {
						JDisplayForm(query);
						return;
					}
				}
			}

			form = JFormHandler(query);
			if (form) {
				xode ret;

				dprintf(dExecution, "Returned form is:\n%s\n",
					xode_to_prettystr(form));

				ret = xode_new(storedmode);
				xode_put_attrib(ret, "jid", storedjid);
				xode_insert_node(ret, form);

				jctl_do_sendrecv(jwgb, ret);
				xode_free(ret);
			}
			else {
				printf("Form cancelled.\n");
			}
			return;
		}
		else {
			printf("Unknown response from server.\n");
			return;
		}
	}
	else if (!strcmp(xode_get_name(x), "success")) {
		printf("%s\n", cdata);
		return;
	}
	else if (!strcmp(xode_get_name(x), "error")) {
		printf("ERROR: %s\n", cdata);
		return;
	}
	else {
		printf("Unknown response from server.\n");
		return;
	}
}

void
jctl_do_sendrecv(j, x)
	jwgconn j;
	xode x;
{
	j = jwg_new();
	if (!j) {
		fprintf(stderr, "jctl: failed to initialize jwgc connection\n");
		exit(1);
	}
	jwg_event_handler(j, jctl_on_event_handler);
	jwg_start(j);
	if (jwg_getfd(j) < 0) {
		fprintf(stderr, "jctl: failed to create jwgc connection\n");
		exit(1);
	}
	jwg_send(j, x);
	jwg_poll(j, -1);
	jwg_stop(j);
}

int 
main(argc, argv)
	int argc;
	char *argv[];
{
	int arg;
	char *setting;
	xode x, y;

	whoami = argv[0];
	dinit();

	arg = 1;

	if (argc < 2) {
		usage();
	}

	for (; arg < argc; arg++) {
		if (*argv[arg] != '-') {
			if (!strcmp(argv[arg], "help")) {
				usage();
			}
			else if (!strcmp(argv[arg], "shutdown")) {
				x = xode_new("shutdown");
				continue;
			}
			else if (!strcmp(argv[arg], "reread")) {
				x = xode_new("reread");
				continue;
			}
			else if (!strcmp(argv[arg], "nickname")) {
				if ((arg + 3) > argc) {
					usage();
				}
				x = xode_new("nickname");
				xode_put_attrib(x, "jid", argv[++arg]);
				if ((arg + 2) <= argc) {
					xode_put_attrib(x, "nick", argv[++arg]);
				}
				else {
					xode_put_attrib(x, "nick", "");
				}
				continue;
			}
			else if (!strcmp(argv[arg], "group")) {
				if ((arg + 3) > argc) {
					usage();
				}
				x = xode_new("group");
				xode_put_attrib(x, "jid", argv[++arg]);
				if ((arg + 2) <= argc) {
					xode_put_attrib(x, "group", argv[++arg]);
				}
				else {
					xode_put_attrib(x, "group", "");
				}
				continue;
			}
			else if (!strcmp(argv[arg], "subscribe")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("subscribe");
				xode_put_attrib(x, "jid", argv[++arg]);
				continue;
			}
			else if (!strcmp(argv[arg], "unsubscribe")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("unsubscribe");
				xode_put_attrib(x, "jid", argv[++arg]);
				continue;
			}
			else if (!strcmp(argv[arg], "set")) {
				if ((arg + 3) > argc) {
					usage();
				}
				x = xode_new("setvar");
				xode_put_attrib(x, "var", argv[++arg]);
				setting = argv[++arg];
				xode_insert_cdata(x, setting, strlen(setting));
				continue;
			}
			else if (!strcmp(argv[arg], "show")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("showvar");
				xode_put_attrib(x, "var", argv[++arg]);
				continue;
			}
			else if (!strcmp(argv[arg], "register")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("register");
				xode_put_attrib(x, "jid", argv[++arg]);
				storedjid = argv[arg];
				storedmode = "register";
				continue;
			}
			else if (!strcmp(argv[arg], "search")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("search");
				xode_put_attrib(x, "jid", argv[++arg]);
				storedjid = argv[arg];
				storedmode = "search";
				continue;
			}
			else if (!strcmp(argv[arg], "join")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("join");
				xode_put_attrib(x, "room", argv[++arg]);
				continue;
			}
			else if (!strcmp(argv[arg], "leave")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("leave");
				xode_put_attrib(x, "room", argv[++arg]);
				continue;
			}
#ifndef NODEBUG
			else if (!strcmp(argv[arg], "debug")) {
				if ((arg + 2) > argc) {
					usage();
				}
				x = xode_new("debug");
				y = xode_insert_tag(x, "debugflags");
				setting = argv[++arg];
				xode_insert_cdata(y, setting, strlen(setting));
				continue;
			}
#endif /* NODEBUG */
			else {
				fprintf(stderr, "Unknown command: %s\n",
					argv[arg]);
				usage();
			}
		}
		switch (argv[arg][1]) {
			case 'd':
				arg++;
				if (arg >= argc) {
					dprinttypes();
				}
				dparseflags(argv[arg]);
				break;
			case 'h':
				usage();
				break;
			default:
				fprintf(stderr, "Unknown option: %s\n",
					argv[arg]);
				usage();
		}
	}

	jctl_do_sendrecv(jwga, x);
	xode_free(x);

	exit(0);
}
