#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <libjwgc.h>
#include <libxode.h>

#define MAXRECIPS 100

int nrecips, msgarg, verbose, quiet, nocheck, noping;
char *whoami, *recips[MAXRECIPS];

void 
usage()
{
	fprintf(stderr, "Usage: %s %s[-h] [-n] [-q] [-v] [-t type] [-m message] recips...\n%s\
  -h                 Display help\n\
  -q                 Quiet output\n\
  -v                 Verbose output\n\
  -e                 Encrypt message\n\
  -f                 Send message without checking recipient validity\n\
  -n                 Send message without pinging recipient\n\
  -t <type>          Sets the type of the outgoing message\n\
  -m <message>       Send <message> instead of prompting for one\n\
",
	whoami,
#ifdef NODEBUG
	"",
	""
#else
	"[-d <flags>] ",
	"  -d                 Enable/Disable debugging (leave <flags> blank for usage)\n"
#endif /* NODEBUG */
	);
	exit(1);
}

void 
jwrite_on_event_handler(jwgconn conn, jwgpacket packet)
{
	xode x;
	char *cdata;

	x = packet->x;
	cdata = xode_get_data(x);

	if (!strcmp(xode_get_name(x), "success")) {
		printf("%s\n", cdata);
	}
	else if (!strcmp(xode_get_name(x), "error")) {
		printf("ERROR: %s\n", cdata);
	}
	else {
		printf("Unknown response from server.\n");
	}
}

void 
jwrite_ping_on_event_handler(jwgconn conn, jwgpacket packet)
{
	xode x;
	char *cdata;

	x = packet->x;
	cdata = xode_get_data(x);

	if (!strcmp(xode_get_name(x), "success")) {
		if (verbose) {
			printf("%s\n", cdata);
		}
	}
	else if (!strcmp(xode_get_name(x), "error")) {
		printf("ERROR: %s\n", cdata);
	}
	else {
		printf("Unknown response from server.\n");
	}
}

int 
main(argc, argv)
	int argc;
	char *argv[];
{
	int arg, nchars, msgsize, i, encrypt;
	char *message, *type, *convmessage;
	static char bfr[1000];
	xode x;
	jwgconn jwg;

	whoami = argv[0];
	dinit();

	if (argc < 2)
		usage();

	verbose = quiet = msgarg = nrecips = nocheck = encrypt = noping = 0;
	type = NULL;

	arg = 1;

	for (; arg < argc && !msgarg; arg++) {
		if (*argv[arg] != '-') {
			recips[nrecips++] = argv[arg];
			continue;
		}
		if (strlen(argv[arg]) > 2)
			usage();
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
			case 'q':
				quiet = 1;
				break;
			case 'e':
				encrypt = 1;
				break;
			case 'n':
				noping = 1;
				break;
			case 'f':
				nocheck = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'm':
				if (arg == argc - 1)
					usage();
				noping = 1;
				msgarg = arg + 1;
				break;
			case 't':
				if ((arg + 2) > argc) {
					usage();
				}
				type = argv[++arg];
				break;
			default:
				fprintf(stderr, "Illegal option: %s\n",
					argv[arg]);
				usage();
		}
	}

	if (!nrecips) {
		fprintf(stderr, "No recipients specified.\n");
		usage();
	}

	if (!noping) {
		for (i = 0; i < nrecips || !nrecips; i++) {
			if (verbose)
				printf("Pinging %s\n", recips[i]);

			jwg = jwg_new();
			if (!jwg) {
				fprintf(stderr, "jwrite: failed to initialize jwgc connection\n");
				exit(1);
			}
			jwg_event_handler(jwg, jwrite_ping_on_event_handler);
			jwg_start(jwg);
			if (jwg_getfd(jwg) < 0) {
				fprintf(stderr, "jwrite: failed to create jwgc connection\n");
				exit(1);
			}

			x = xode_new("ping");
			xode_put_attrib(x, "to", recips[i]);
			if (nocheck) {
				xode_put_attrib(x, "dontmatch", "yes");
			}
			if (type) {
				xode_put_attrib(x, "type", type);
			}
			jwg_send(jwg, x);
			xode_free(x);

			jwg_poll(jwg, -1);

			jwg_stop(jwg);
		}
	}

	if (!msgarg && isatty(0))
		printf("Type your message now.  End with control-D or a dot on a line by itself.\n");

	message = (char *)malloc(sizeof(char));
	message[0] = '\0';
	msgsize = 0;

	if (msgarg) {
		int size = msgsize;
		for (arg = msgarg; arg < argc; arg++)
			size += (strlen(argv[arg]) + 1);
		size++;		/* for the newline */
		message = (char *) realloc(message, (unsigned) size + 2);
		for (arg = msgarg; arg < argc; arg++) {
			dprintf(dExecution, "Adding %s...\n", argv[arg]);
			(void) strcpy(message + msgsize, argv[arg]);
			msgsize += strlen(argv[arg]);
			if (arg != argc - 1) {
				message[msgsize] = ' ';
				msgsize++;
			}
		}
		message[msgsize] = '\n';
		msgsize += 1;
		message[msgsize] = '\0';
	}
	else {
		if (isatty(0)) {
			for (;;) {
				unsigned int l;
				if (!fgets(bfr, sizeof bfr, stdin))
					break;
				if (bfr[0] == '.' &&
				    (bfr[1] == '\n' || bfr[1] == '\0'))
					break;
				l = strlen(bfr);
				message = (char *) realloc(message, msgsize + l + 1);
				(void) strcpy(message + msgsize, bfr);
				msgsize += l;
			}
			message = (char *) realloc(message,
						   (unsigned) (msgsize + 1));
			message[msgsize+1] = '\0';
		}
		else {		/* Use read so you can send binary
				 * messages... */
			while ((nchars = read(fileno(stdin), bfr, sizeof bfr))) {
				if (nchars == -1) {
					fprintf(stderr, "Read error from stdin!  Can't continue!\n");
					exit(1);
				}
				message = (char *) realloc(message,
					     (unsigned) (msgsize + nchars));
				(void) memcpy(message + msgsize, bfr, nchars);
				msgsize += nchars;
			}
			/* end of msg */
			message = (char *) realloc(message,
						   (unsigned) (msgsize + 1));
			message[msgsize+1] = '\0';
		}
	}

	dprintf(dExecution, "Message before translation:\n%s\n", message);
	str_to_unicode(message, &convmessage);
	dprintf(dExecution, "Message after translation:\n%s\n", convmessage);

	for (i = 0; i < nrecips || !nrecips; i++) {
		if (verbose)
			printf("Sending message to %s\n", recips[i]);

		jwg = jwg_new();
		if (!jwg) {
			fprintf(stderr, "jwrite: failed to initialize jwgc connection\n");
			exit(1);
		}
		jwg_event_handler(jwg, jwrite_on_event_handler);
		jwg_start(jwg);
		if (jwg_getfd(jwg) < 0) {
			fprintf(stderr, "jwrite: failed to create jwgc connection\n");
			exit(1);
		}

		x = xode_new("message");
		xode_put_attrib(x, "to", recips[i]);
		if (nocheck) {
			xode_put_attrib(x, "dontmatch", "yes");
		}
		if (encrypt) {
			xode_put_attrib(x, "encrypt", "yes");
		}
		if (type) {
			xode_put_attrib(x, "type", type);
		}
		xode_insert_cdata(x, message, strlen(message));
		jwg_send(jwg, x);
		xode_free(x);

		jwg_poll(jwg, -1);
		jwg_stop(jwg);
	}

	exit(0);
}
