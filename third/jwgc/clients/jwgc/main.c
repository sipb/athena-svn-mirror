#include "mux.h"
#include "file.h"
#include "parser.h"
#include "port.h"
#include "main.h"

#define ALARMTIMEOUT 60

jabconn jab_c = NULL;
jwgconn jwg_c = NULL;
time_t jab_connect_time = 0;
int jab_reauth = 0;

char *whoami = NULL;
char *progname = NULL;
char *description_filename_override = NULL;
struct _Node *program = NULL;

char *current_presence = "available";

char *barebones_desc = "\
fields body\n\
print \"@b(Description file is broken!  This is a barebones config.)\\n\"\n\
print \"Date:    \"+$date+\"\\n\"\n\
print \"Time:    \"+$time+\"\\n\"\n\
print \"From:    \"+$from+\"\\n\"\n\
print \"Status:  \"+$status+\"\\n\"\n\
print \"Show:    \"+$show+\"\\n\"\n\
print \"Type:    \"+$type+\"\\n\"\n\
print \"Subtype: \"+$subtype+\"\\n\"\n\
print \"Body:\\n\"+$body+\"\\n\"\n\
put\n\
exit\n\
";

void 
usage()
{
	fprintf(stderr, "\
Usage: %s [-h] [-f <filename>]\n\
                  [-u <username>] [-domain <domain>]\n\
                  [-r <resource>] [-s <server>] [-j <jid>]\n\
                  [-port <port>] [-priority <priority>]\n\
                  [-ttymode] [-nofork]\n\
                  [-default <driver>] {-disable <driver>}*\n\
                  [output driver options]\n\
", whoami);
#ifdef USE_GPGME
	fprintf(stderr, "\
                  [-gpg] [-nogpg] [-gpgpass <password>]\n\
");
#endif /* USE_GPGME */
#ifdef USE_SSL
	fprintf(stderr, "\
                  [-ssl] [-nossl]\n\
");
#endif /* USE_SSL */
#ifndef NODEBUG
	fprintf(stderr, "\
                  [-debug <flags>]\n\
");
#endif /* NODEBUG */
	exit(1);
}

void 
fake_startup_packet()
{
	xode fake;

	fake = jabutil_msgnew("internal",
			(char *)jVars_get(jVarJID),
			"startup",
			"Jabber Windowgram Client Started...",
			NULL);
	jab_send(jab_c, fake);
	xode_free(fake);
}

int 
read_in_description_file()
{
	FILE *input_file;
	char *defdesc;

	defdesc = (char *)malloc(sizeof(char) * (strlen(DATADIR) + 1 +
			strlen(DEFDESC) + 1));

	sprintf(defdesc, "%s/%s", DATADIR, DEFDESC);
	input_file = (FILE *) locate_file(description_filename_override,
			USRDESC, defdesc);
	free(defdesc);
	if (input_file)
		program = (struct _Node *) parse_file(input_file);
	else
		program = NULL;

	if (program == NULL)
		program = (struct _Node *) parse_buffer(barebones_desc);

	if (program != NULL) {
		fake_startup_packet();
		return 1;
	}
	else {
		return 0;
	}
}

void
check_live_jwgc_on_event_handler(jwgconn conn, jwgpacket packet)
{
	fprintf(stderr, "There is already an active jwgc running!  Exiting...\n");
	exit(0);
}

void 
check_live_jwgc()
{
	jwgconn checkjwg;
	xode x;

	checkjwg = jwg_new();
	if (!checkjwg) {
		dprintf(dExecution, "check_live_jwgc: failed to initialize jwgc connection\n");
		return;
	}

	jwg_event_handler(checkjwg, check_live_jwgc_on_event_handler);
	jwg_start(checkjwg);
	if (jwg_getfd(checkjwg) < 0) {
		dprintf(dExecution, "check_live_jwgc: failed to create jwgc connection\n");
		return;
	}

	x = xode_new("check");
	jwg_send(checkjwg, x);
	xode_free(x);

	jwg_poll(checkjwg, -1);
	jwg_stop(checkjwg);
}

void 
jwgc_init()
{
	jwg_c = jwg_server();
	if (!jwg_c) {
		fprintf(stderr, "jwgc: unable to initialize jwgc connection.\n");
		exit(1);
	}
	jwg_event_handler(jwg_c, jwg_on_event_handler);
	jwg_servstart(jwg_c);
	if (jwg_getfd(jwg_c) < 0) {
		fprintf(stderr, "jwgc: unable to create jwgc connection.\n");
		exit(1);
	}
	mux_add_input_source(jwg_getfd(jwg_c), jwg_servrecv, jwg_c);
}

void 
jabber_init()
{
	jab_reauth = 0;
	jab_c = jab_new((char *)jVars_get(jVarJID), (char *)jVars_get(jVarServer));
	if (!jab_c) {
		fprintf(stderr, "jwgc: unable to create jabber connection.\n");
		exit(1);
	}
	jab_c->port = *(int *)jVars_get(jVarPort);
	jab_packet_handler(jab_c, jab_on_packet_handler);
	jab_state_handler(jab_c, jab_on_state_handler);
	jab_start(jab_c,
#ifdef USE_SSL
		*(int *) jVars_get(jVarUseSSL)
#else /* USE_SSL */
		0
#endif /* USE_SSL */
	);
	jab_auth(jab_c);
	if (jab_c->state == JABCONN_STATE_OFF) {
		fprintf(stderr, "Unable to connect to jabber server.  Retrying in %d seconds.\n", ALARMTIMEOUT);
		jab_reauth = 1;
		return;
	}
	mux_add_input_source(jab_getfd(jab_c), jab_recv, jab_c);

	jab_startup(jab_c);
	jab_connect_time = time(0);
}

void 
jabber_finalize()
{
	jab_stop(jab_c);
	jwg_stop(jwg_c);
	JCleanupSocket();
}

void 
run_initprogs()
{
	int status;
	char *progname = (char *)jVars_get(jVarInitProgs);

	if (!progname)
		return;

	dprintf(dExecution, "Running init program... %s\n", progname);

	status = system(progname);
	if (status == 127) {
		perror("jwgc initprog exec");
		fprintf(stderr, "jwgc initprog of <%s> failed: no shell.\n",
			progname);
	}
	else if (status != -1 && status >> 8) {
		perror("jwgc initprog exec");
		fprintf(stderr, "jwgc initprog of <%s> failed with status [%d].\n",
			progname, status >> 8);
	}
}

static RETSIGTYPE 
signal_exit()
{
	mux_end_loop_p = 1;
}

static RETSIGTYPE 
signal_pipe()
{
	fprintf(stderr, "Lost connection to server.  Retrying in %d seconds...\n", ALARMTIMEOUT);
	mux_delete_input_source(jab_getfd(jab_c));
	jab_delete(jab_c);
	jab_reauth = 1;
}

static RETSIGTYPE 
signal_check()
{
	/* char *rawstr; */
	xode x, y, z;

	dprintf(dExecution, "Checking jabber connection...\n");
	if (jab_reauth) {
		if (jab_c) { jab_delete(jab_c); }
		jabber_init();
	}
	else if (jab_c->state == JABCONN_STATE_ON) {
		/*
		 * we're going to check server up or down by setting a
		 * keepalive 'pref' of sorts.  Lame, but it's the best i can
		 * come up with for now.
		 */
		/*
		x = jabutil_iqnew(JABPACKET__SET, NS_PRIVATE);
		y = xode_get_tag(x, "query");
		z = xode_insert_tag(y, "jwgc");
		xode_put_attrib(z, "xmlns", "jwgc:status");
		xode_put_attrib(z, "keepalive", "1");
		jab_send(jab_c, x);
		xode_free(x);
		*/
		/* This should do it, and more efficiently than above. */
		jab_send_raw(jab_c, " ");
		if (errno == EPIPE) {
			fprintf(stderr, "Lost connection to server.  Retrying in %d seconds...\n", ALARMTIMEOUT);
			mux_delete_input_source(jab_getfd(jab_c));
			jab_delete(jab_c);
			jab_reauth = 1;
		}
	}

	alarm(ALARMTIMEOUT);
}

/*
 * clean up ALL the waiting children, in case we get hit with multiple
 * SIGCHLD's at once, and don't process in time.
 */
static RETSIGTYPE 
signal_child()
{
#ifdef HAVE_WAITPID
	int status;
#else
	union wait status;
#endif
	extern int errno;
	int pid, old_errno = errno;

	do {
#ifdef HAVE_WAITPID
		pid = waitpid(-1, &status, WNOHANG);
#else
		pid = wait3(&status, WNOHANG, (struct rusage *) 0);
#endif
	} while (pid != 0 && pid != -1);
	errno = old_errno;
}

static void 
setup_signals(dofork)
	int dofork;
{
#ifdef _POSIX_VERSION
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (dofork) {
		sa.sa_handler = SIG_IGN;
		sigaction(SIGINT, &sa, (struct sigaction *) 0);
		sigaction(SIGTSTP, &sa, (struct sigaction *) 0);
		sigaction(SIGQUIT, &sa, (struct sigaction *) 0);
		sigaction(SIGTTOU, &sa, (struct sigaction *) 0);
	}
	else {
		/*
		 * clean up on SIGINT; exiting on logout is the user's
		 * problem, now.
		 */
		sa.sa_handler = signal_exit;
		sigaction(SIGINT, &sa, (struct sigaction *) 0);
	}

	/* behavior never changes */
	sa.sa_handler = signal_exit;
	sigaction(SIGTERM, &sa, (struct sigaction *) 0);
	sigaction(SIGHUP, &sa, (struct sigaction *) 0);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, (struct sigaction *)0);
	/*
	sa.sa_handler = signal_pipe;
	sigaction(SIGPIPE, &sa, (struct sigaction *) 0);
	*/

	sa.sa_handler = signal_child;
	sigaction(SIGCHLD, &sa, (struct sigaction *) 0);

	sa.sa_handler = signal_check;
	sigaction(SIGALRM, &sa, (struct sigaction *) 0);

#ifdef _AIX
	sa.sa_flags = SA_FULLDUMP;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGSEGV, &sa, (struct sigaction *) 0);
#endif

#else				/* !POSIX */
	if (dofork) {
		/*
		 * Ignore keyboard signals if forking.  Bad things will
		 * happen.
		 */
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}
	else {
		/*
		 * clean up on SIGINT; exiting on logout is the user's
		 * problem, now.
		 */
		signal(SIGINT, signal_exit);
	}

	/* behavior never changes */
	signal(SIGTERM, signal_exit);
	signal(SIGHUP, signal_exit);
	signal(SIGCHLD, signal_child);
	signal(SIGPIPE, SIG_IGN);
	/* signal(SIGPIPE, signal_pipe); */
	signal(SIGALRM, signal_check);
#endif
}

static void 
detach()
{
	/* detach from terminal and fork. */
	register int i;

	/* to try to get SIGHUP on user logout */
#if defined(_POSIX_VERSION) && !defined(ultrix)
	(void) setpgid(0, tcgetpgrp(1));
#else
	(void) setpgrp(0, getpgrp(getppid()));
#endif

	/* fork off and let parent exit... */
	if ((i = fork())) {
		if (i < 0) {
			perror("jwgc: cannot fork, aborting:");
			exit(1);
		}
		exit(0);
	}
}

int 
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	char prompt[256];
	char **new;
	register char **current;
	int dofork = 1;
	int portset = 0;

	whoami = argv[0];
	progname = argv[0];
#ifndef NODEBUG
	dinit();
	for (new = current = argv + 1; *current; current++) {
		if (string_Eq(*current, "-debug")) {
			argc -= 2;
			current++;
			if (!*current)
				dprinttypes();
			dparseflags(*current);
		}
	}
#endif /* NODEBUG */
	jVars_set_defaults_handler(jwg_set_defaults_handler);
	jVars_init();

	for (new = current = argv + 1; *current; current++) {
		if (string_Eq(*current, "-h") ||
		    string_Eq(*current, "-help")) {
			usage();
		}
#ifndef NODEBUG
		else if (string_Eq(*current, "-debug")) {
			argc -= 2;
			current++;
			/* we already checked this above */
		}
#endif /* NODEBUG */
		else if (string_Eq(*current, "-f") ||
			 string_Eq(*current, "-descfile")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			description_filename_override = *current;
		}
		else if (string_Eq(*current, "-domain")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarDomain, *current);
		}
		else if (string_Eq(*current, "-j") ||
			 string_Eq(*current, "-jid")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarJID, *current);
		}
		else if (string_Eq(*current, "-port")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarPort, *current);
			portset = 1;
		}
		else if (string_Eq(*current, "-priority")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarPriority, *current);
		}
		else if (string_Eq(*current, "-nofork")) {
			argc--;
			dofork = 0;
		}
#ifdef USE_SSL
		else if (string_Eq(*current, "-ssl")) {
			argc--;
			jVars_set(jVarUseSSL, "1");
		}
		else if (string_Eq(*current, "-nossl")) {
			argc--;
			jVars_set(jVarUseSSL, "0");
		}
#endif /* USE_SSL */
#ifdef USE_GPGME
		else if (string_Eq(*current, "-gpg")) {
			argc--;
			jVars_set(jVarUseGPG, "1");
		}
		else if (string_Eq(*current, "-nogpg")) {
			argc--;
			jVars_set(jVarUseGPG, "0");
		}
		else if (string_Eq(*current, "-gpgpass")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarGPGPass, *current);
		}
#endif /* USE_GPGME */
		else if (string_Eq(*current, "-u") ||
			 string_Eq(*current, "-username")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarUsername, *current);
		}
		else if (string_Eq(*current, "-s") ||
			 string_Eq(*current, "-server")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarServer, *current);
		}
		else if (string_Eq(*current, "-r") ||
			 string_Eq(*current, "-resource")) {
			argc -= 2;
			current++;
			if (!*current)
				usage();
			jVars_set(jVarResource, *current);
		}
		else
			*(new)++ = *current;
	}
	*new = *current;

#ifdef USE_SSL
	if (!portset && *(int *)jVars_get(jVarUseSSL)) {
		jVars_set(jVarPort, DEFSSLPORT);
	}
#endif /* USE_SSL */

	if (jVars_get(jVarUsername) == NULL) {
		if (!jVars_set(jVarUsername, getlogin())) {
			printf("No username specified.\n");
			exit(0);
		}
	}

	mux_init();
	var_clear_all_variables();
	init_ports();
	dprintf(dExecution, "Initializing standard ports...\n");
	init_standard_ports(&argc, argv);
	if (argc > 1)
		usage();
	check_live_jwgc();
	setup_signals(dofork);

	if (!jVars_get(jVarJID)) {
		char *domain, *jwgcjid;
		dprintf(dExecution, "Creating JID...\n");
		domain = jVars_get(jVarDomain);
		if (!domain)
			domain = jVars_get(jVarServer);
		jwgcjid = (char *) malloc(sizeof(char) *
			(strlen((char *)jVars_get(jVarUsername))
			+ strlen(domain)
			+ strlen((char *)jVars_get(jVarResource)) + 3));
		sprintf(jwgcjid, "%s@%s/%s",
			(char *)jVars_get(jVarUsername),
			domain,
			(char *)jVars_get(jVarResource));
		dprintf(dExecution, "JID = %s\n", jwgcjid ? jwgcjid : "n/a");
		jVars_set(jVarJID, jwgcjid);
		free(jwgcjid);
	}
	else {
		dprintf(dExecution, "JID already set to %s.\n",
				(char *)jVars_get(jVarJID));
	}
	jwgc_init();
	jabber_init();

	if (dofork)
		detach();

	run_initprogs();

	read_in_description_file();

	alarm(ALARMTIMEOUT);

	mux_loop();

	jabber_finalize();

	exit(0);
}
