/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.66.2.2 1998-10-30 19:56:19 ghudson Exp $
 *
 * Copyright (c) 1990, 1991 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This is the top-level of the display manager and console control
 * for Athena's xlogin.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utmp.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifdef SOLARIS
#include <sys/strredir.h>
#include <sys/stropts.h>
#include <utmpx.h>
#endif
static sigset_t sig_zero;
static sigset_t sig_cur;
#include <termios.h>
#include <syslog.h>

#include <X11/Xlib.h>
#include <al.h>

#ifndef lint
static char *rcsid_main = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.66.2.2 1998-10-30 19:56:19 ghudson Exp $";
#endif

/* Non-portable termios flags we'd like to set. */
#ifndef IMAXBEL
#define IMAXBEL 0
#endif
#ifndef ECHOCTL
#define ECHOCTL 0
#endif
#ifndef TAB3
#define TAB3 0
#endif

/* Process states */
#define NONEXISTENT	0
#define RUNNING		1
#define STARTUP		2
#define CONSOLELOGIN	3
#define FAILED		4

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

#ifdef SOLARIS
#define L_INCR          1
#define RAW     040
#endif

/* flags used by signal handlers */
pid_t xpid, consolepid, loginpid;
volatile int alarm_running = NONEXISTENT;
volatile int x_running = NONEXISTENT;
volatile int console_running = NONEXISTENT;
volatile int console_tty = 0, console_failed = FALSE;
volatile int login_running = NONEXISTENT;
char *logintty;

#if defined(UTMP_FILE)
char *utmpf = UTMP_FILE;
char *wtmpf = WTMP_FILE;
#elif defined(_PATH_UTMP)
char *utmpf = _PATH_UTMP;
char *wtmpf = _PATH_WTMP;
#else
char *utmpf = "/var/adm/utmp";
char *wtmpf = "/var/adm/wtmp";
#endif
#ifdef SOLARIS
char *utmpfx = UTMPX_FILE;
char *wtmpfx = WTMPX_FILE;
#endif
char *xpids = "/var/athena/X%d.pid";
char *xhosts = "/etc/X%d.hosts";
char *consolepidf = "/var/athena/console.pid";
char *dmpidf = "/var/athena/dm.pid";
char *consolelog = "/var/athena/console.log";

static void die(int signo);
static void child(int signo);
static void catchalarm(int signo);
static void xready(int signo);
static void shutdown(int signo);
static void loginready(int signo);
static char *number(int x);
static char *getconf(char *file, char *name);
static char **parseargs(char *line, char *extra, char *extra1, char *extra2);
static void console_login(char *conf, char *msg);
static void start_console(char *line, char **argv);
static void cleanup(char *tty);
static pid_t fork_and_store(pid_t *var);
static void x_stop_wait(void);
#ifdef SOLARIS
static int grabconsole(void);
#endif

/* the console process will run as daemon */
#define DAEMON 1

#define X_START_WAIT	30	/* wait up to 30 seconds for X to be ready */
#define X_STOP_WAIT	4	/* seconds to wait for graceful shutdown */
#define LOGIN_START_WAIT 60	/* wait up to 1 minute for Xlogin */
#ifndef BUFSIZ
#define BUFSIZ		1024
#endif

static    int max_fd;

#ifdef SOLARIS
static int grabconsole(void)
{
    int	console;
    int		p[2];
    
    if (pipe(p) < 0) {
       fprintf(stderr, "dm: could not open pipe: %s\n",strerror(errno));
       exit(1);
    }

    if ((console = open("/dev/console", O_RDONLY)) < 0 ) {
        fprintf(stderr, "dm:could not open /dev/console: %s\n",
		strerror(errno));
	exit(1);
    }
    if (ioctl(console, SRIOCSREDIR, p[1]) < 0) {
        fprintf(stderr, "dm:could not issue ioctl: %s\n",strerror(errno));
	syslog(LOG_DEBUG, "Can't issue SRIOCSREDIR ioctl: %m");
	exit(1);
    }
    return(p[0]);
}
#endif

/* Setup signals, start X, start console, start login, wait */

int main(int argc, char **argv)
{
    char *consoletty, *conf, *p;
    char **dmargv, **xargv, **consoleargv = NULL, **loginargv;
    char xpidf[256], line[16], buf[256];
    fd_set readfds;
    int pgrp, file, tries, count, console = TRUE;
#ifdef TIOCCONS
    int on = 1;
#endif

    char dpyname[10], dpyacl[40];
    Display *dpy;
    XHostAddress *hosts;
    int nhosts, dpynum = 0;
    struct stat hostsinfo;
    Bool state;
    time_t now, last_console_failure = 0;

#ifdef SOLARIS
    char openv[1024];
#endif
    struct sigaction sigact;
    sigset_t mask;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    (void) sigemptyset(&sig_zero);

/*
 * Note about setting environment variables in dm:
 *
 *   All environment variables passed to dm and set in dm are
 *   subsequently passed to any children of dm. This is usually
 *   true of processes that exec in children, so that's not a
 *   big surprise.
 *
 *   However, xlogin is one of the children dm forks, and it goes
 *   to lengths to ensure that the environments of users logging in
 *   are ISOLATED from xlogin's own environment. Therefore, do not
 *   expect that setting an environment variable here will reach the
 *   user unless you have gone to lengths to make sure that xlogin
 *   passes it on. Put another way, if you set a new environment
 *   variable here, consider whether or not it should be seen by the
 *   user. If it should, go modify verify.c as well. Consider also
 *   whether the variable should be seen _only_ by the user. If so,
 *   make the change only in xlogin, and not here.
 *
 *   As an added complication, xlogin _does_ pass environment variables
 *   on to the pre-login options. Therefore, if you set an environment
 *   variable that should _not_ be seen, you must filter it in xlogin.c.
 *
 *   Confused? Too bad. I'm in a nasty, if verbose, mood this year.
 *
 * General summary:
 *
 *   If you add an environment variable here there are three likely
 *   possibilities:
 *
 *     1. It's for the user only, not needed by any of dm's children.
 *        --> Don't set it here. Set it in verify.c for users and in
 *        --> xlogin.c for the pre-login options, if appropriate.
 *
 *     2. It's for dm and its children only, and _should not_ be seen
 *        by the user or pre-login options.
 *        --> You must filter the option from the pre-login options
 *        --> in xlogin.c. No changes to verify.c are required.
 *
 *     3. It's for dm and the user and the pre-login options.
 *        --> You must pass the option explicitly to the user in
 *        --> verify.c. No changes to xlogin.c are required.
 *
 *                                                   --- cfields
 */
#ifdef SOLARIS
    strcpy(openv, "LD_LIBRARY_PATH=");
    strcat(openv, "/usr/openwin/lib");
    putenv(openv);
    strcpy(openv, "OPENWINHOME=");
    strcat(openv, "/usr/openwin");
    putenv(openv);
#endif

    if (argc != 4 &&
	(argc != 5 || strcmp(argv[3], "-noconsole"))) {
	fprintf(stderr,
		"usage: %s configfile logintty [-noconsole] consoletty\n",
		argv[0]);
	console_login(conf, NULL);
    }
    if (argc == 5) console = FALSE;

    /* parse argument lists */
    conf = argv[1];
    logintty = argv[2];
    consoletty = argv[argc-1];

    openlog("dm", 0, LOG_USER);

    /* We use options from the config file rather than taking
     * them from the command line because the current command
     * line form is gross (why???), and I don't see a good way
     * to extend it without making things grosser or breaking
     * backwards compatibility. So, we take a line from the
     * config file and use real parsing.
     */
    p = getconf(conf, "dm");
    if (p != NULL)
      {
	dmargv = parseargs(p, NULL, NULL, NULL);
	while (*dmargv)
	  {
	    if (!strcmp(*dmargv, "-display"))
	      {
		dmargv++;
		if (*dmargv)
		  {
		    dpynum = atoi(*(dmargv)+1);
		    dmargv++;
		  }
	      }
	    else
	      dmargv++;
	  }
      }

    p = getconf(conf, "X");
    if (p == NULL)
      console_login(conf, "\ndm: Can't find X command line\n");
    xargv = parseargs(p, NULL, NULL, NULL);

    if (console) {
	p = getconf(conf, "console");
	if (p == NULL)
	  console_login(conf, "\ndm: Can't find console command line\n");
#ifdef SOLARIS
          consoleargv = parseargs(p, "-inputfd", "0", NULL);
#else
          consoleargv = parseargs(p, NULL, NULL, NULL);
#endif
    }

    p = getconf(conf, "login");
    if (p == NULL)
      console_login(conf, "\ndm: Can't find login command line\n");
    loginargv = parseargs(p, logintty, "-tty", logintty);

    /* Signal Setup */
    sigact.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sigact, NULL);
    sigaction(SIGTTIN, &sigact, NULL);
    sigaction(SIGTTOU, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL); /* so that X pipe errors don't nuke us */
    sigact.sa_handler = shutdown;
    sigaction(SIGFPE, &sigact, NULL);
    sigact.sa_handler = die;
    sigaction(SIGHUP, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigact.sa_handler = child;
    sigaction(SIGCHLD, &sigact, NULL);
    sigact.sa_handler = catchalarm;
    sigaction(SIGALRM, &sigact, NULL);
    close(0);
    close(1);
    close(2);
    setsid();
    strcpy(line, "/dev/");
    strcat(line, consoletty);
    open(line, O_RDWR, 0622);
    dup2(0, 1);
    dup2(1, 2);

    /* Set the console characteristics so we don't lose later */
    setpgid(0, pgrp=getpid()); /* Reset the tty pgrp  */
    tcsetpgrp(0, pgrp);

    /* save our pid file */
    if ((file = open(dmpidf, O_WRONLY|O_TRUNC|O_CREAT, 0644)) >= 0) {
	write(file, number(getpid()), strlen(number(getpid())));
	close(file);
    }

    /* Fire up X */
    xpid = 0;
    for (tries = 0; tries < 3; tries++) {
	syslog(LOG_DEBUG, "Starting X, try #%d", tries + 1);
	x_running = STARTUP;
	sigact.sa_handler = xready;
	sigaction(SIGUSR1, &sigact, NULL);
	switch (fork_and_store(&xpid)) {
	case 0:
	    if(fcntl(2, F_SETFD, 1) == -1)
	      close(2);
	    (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *)0);

	    /* ignoring SIGUSR1 will cause the server to send us a SIGUSR1
	     * when it is ready to accept connections
	     */
	    sigact.sa_handler = SIG_IGN;
	    sigaction(SIGUSR1, &sigact, NULL);
	    p = *xargv;
	    *xargv = "X";
	    execv(p, xargv);
	    fprintf(stderr,"dm: X server failed exec: %s\n", strerror(errno));
	    _exit(1);
	case -1:
	    fprintf(stderr,"dm: Unable to fork to start X server: %s\n",
		    strerror(errno));
	    break;
	default:
	    sprintf(xpidf, xpids, dpynum);
	    if ((file = open(xpidf, O_WRONLY|O_TRUNC|O_CREAT, 0644)) >= 0) {
		write(file, number(xpid), strlen(number(xpid)));
		close(file);
	    }

	    if (x_running == STARTUP) {
		alarm(X_START_WAIT);
		alarm_running = RUNNING;
		sigsuspend(&sig_zero);
	    }
	    if (x_running != RUNNING) {
		syslog(LOG_DEBUG, "X failed to start; alarm_running=%d",
		       alarm_running);
		if (alarm_running == NONEXISTENT)
		  fprintf(stderr,"dm: Unable to start X\n");
		else
		  fprintf(stderr,"dm: X failed to become ready\n");

		/* If X wouldn't run, it could be that an existing X
		 * process hasn't shut down.  Wait X_STOP_WAIT seconds
		 * for that to happen.
		 */
		x_stop_wait();
	    }
	    sigact.sa_handler = SIG_IGN;
	    sigaction(SIGUSR1, &sigact, NULL);
	}
	if (x_running == RUNNING) break;
    }
    alarm(0);
    if (x_running != RUNNING) {
	syslog(LOG_DEBUG, "Giving up on starting X.");
	console_login(conf, "\nUnable to start X, doing console login "
		      "instead.\n");
    }

    /* Tighten up security a little bit. Remove all hosts from X's
     * access control list, assuming /etc/X0.hosts does not exist or
     * has zero length. If it does exist with nonzero length, this
     * behavior is not wanted. The desired effect of removing all hosts
     * is that only connections from the Unix domain socket will be
     * allowed.       

     * More secure code using Xau also exists, but there wasn't
     * time to completely flesh it out and resolve a couple of
     * issues. This code is probably good enough, but we'll see.
     * Maybe next time. 

     * This code has the added benefit of leaving an X display
     * connection open, owned by dm. This provides a less-hacky
     * solution to the config_console problem, where if config_console
     * is the first program run on user login, it causes the only
     * X app running at the time, console, to exit, thus resetting
     * the X server. Thus this code also allows the removal of the
     * hack in xlogin that attempts to solve the same problem, but
     * fails on the RS/6000 for reasons unexplored.

     * P.S. Don't run this code under Solaris 2.2- (2.3 is safe).
     * Removing all hosts from the acl on that server results in
     * no connections, not even from the Unix domain socket, being
     * allowed. --- cfields
     */

    sprintf(dpyacl, xhosts, dpynum);
    sprintf(dpyname, ":%d", dpynum);
    dpy = XOpenDisplay(dpyname);
    if (dpy != NULL && (stat(dpyacl, &hostsinfo) || hostsinfo.st_size == 0))
      {
	hosts = XListHosts(dpy, &nhosts, &state);
	if (hosts != NULL)
	  {
	    XRemoveHosts(dpy, hosts, nhosts);
	    XFlush(dpy);
	    XFree(hosts);
	  }
      }
    /* else if (dpy == NULL)
     *   Could've sworn the X server was running now.
     *   Follow the original code path. No need introducing new bugs
     *   to this hairy code, just preserve the old behavior as though
     *   this code had never been added.
     */

    /* start up console */
    strcpy(line, "/dev/");
    strcat(line, logintty);
    if (console) start_console(line, consoleargv);
    /* had to use a different tty, make sure xlogin uses it too */
    if (strcmp(logintty, &line[5]))
      strcpy(logintty, &line[5]);

    /* Fire up the X login */
    for (tries = 0; tries < 3; tries++) {
	syslog(LOG_DEBUG, "Starting xlogin, try #%d", tries + 1);
	login_running = STARTUP;
	sigact.sa_handler = loginready;
        sigaction(SIGUSR1, &sigact, NULL);
	switch (fork_and_store(&loginpid)) {
	case 0:
	    max_fd = sysconf(_SC_OPEN_MAX);
	    for (file = 0; file < max_fd; file++)
	      close(file);
	    /* setup new tty */
	    strcpy(line, "/dev/");
	    strcat(line, logintty);
	    open("/dev/null", O_RDONLY, 0);
	    (void) setsid();
	    open(line, O_RDWR, 0);
	    dup2(1, 2);
#ifdef TIOCCONS
	    if (console)
		ioctl(1, TIOCCONS, &on);
#endif
	    (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *)0);
	    /* ignoring SIGUSR1 will cause xlogin to send us a SIGUSR1
	     * when it is ready
	     */
	    sigact.sa_handler = SIG_IGN;
	    sigaction(SIGUSR1, &sigact, NULL);
	    /* dm ignores sigpipe; because of this, all of the children (ie, */
	    /* the entire session) inherit this unless we fix it now */
            sigact.sa_handler = SIG_DFL;;
            sigaction(SIGPIPE, &sigact, NULL);
	    execv(loginargv[0], loginargv);
	    fprintf(stderr,"dm: X login failed exec: %s\n", strerror(errno));
	    _exit(1);
	case -1:
	    fprintf(stderr,"dm: Unable to fork to start X login: %s\n",
		    strerror(errno));
	    break;
	default:
	    alarm(LOGIN_START_WAIT);
	    alarm_running = RUNNING;
	    while (login_running == STARTUP && alarm_running == RUNNING)
	      sigsuspend(&sig_zero);
	    if (login_running != RUNNING) {
		syslog(LOG_DEBUG, "xlogin failed to start; alarm_running=%d",
		       alarm_running);
		kill(loginpid, SIGKILL);
		if (alarm_running != NONEXISTENT)
		  fprintf(stderr,"dm: Unable to start Xlogin\n");
		else
		  fprintf(stderr,"dm: Xlogin failed to become ready\n");
	    }
	}
	if (login_running == RUNNING) break;
    }
    sigact.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sigact, NULL);
    alarm(0);
    if (login_running != RUNNING) {
      syslog(LOG_DEBUG, "Giving up on starting xlogin.");
      console_login(conf, "\nUnable to start xlogin, doing console login "
		    "instead.\n");
    }

    /* main loop.  Wait for SIGCHLD, waking up every minute anyway. */
    (void) sigemptyset(&sig_cur);
    (void) sigaddset(&sig_cur, SIGCHLD);
    (void) sigprocmask(SIG_BLOCK, &sig_cur, NULL);
    while (1) {
	/* Wait for something to hapen */
	if (console_failed) {
	    /* if no console is running, we must copy bits from the console
	     * (master side of pty) to the real console to appear as black
	     * bar messages.
	     */
	    FD_ZERO(&readfds);
	    FD_SET(console_tty, &readfds);
	    (void) sigprocmask(SIG_SETMASK, &sig_zero, &mask);
	    count = select(console_tty + 1, &readfds, NULL, NULL, NULL);
	    (void) sigprocmask(SIG_BLOCK, &mask, NULL);
	    if (count > 0 && FD_ISSET(console_tty, &readfds)) {
		file = read(console_tty, buf, sizeof(buf));
		write(1, buf, file);
	    }
	} else {
	    alarm(60);
	    sigsuspend(&sig_zero);
	}

	if (login_running == STARTUP) {
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, NULL);
	    console_login(conf, "\nConsole login requested.\n");
	}
	if (console_running == FAILED) {
	    console_running = NONEXISTENT;
	    time(&now);
	    if (now - last_console_failure <= 3) {
		/* Give up on console.  Set the console characteristics so
		 * we don't lose later. */
		syslog(LOG_ERR, "Giving up on the console");
		setpgid(0, pgrp=getpid());	/* Reset the tty pgrp */
		tcsetpgrp(0, pgrp);
		console_failed = TRUE;
	    } else
		last_console_failure = now;
	}
	if (console && console_running == NONEXISTENT && !console_failed)
	  start_console(line, consoleargv);
	if (login_running == NONEXISTENT || x_running == NONEXISTENT) {
	  syslog(LOG_DEBUG, "login_running=%d, x_running=%d, quitting",
		 login_running, x_running);
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, NULL);
	  cleanup(logintty);
	  x_stop_wait();
	  _exit(0);
	}
    }
}


/* Start a login on the raw console */

static void console_login(char *conf, char *msg)
{
    int i, graceful = FALSE, cfirst = TRUE, pgrp;
    char *nl = "\r\n";
    struct termios ttybuf;
    char *p, **cargv;

    syslog(LOG_DEBUG, "Performing console login: %s", msg);
    sigemptyset(&sig_zero);
    if (login_running != NONEXISTENT && login_running != STARTUP)
	kill(loginpid, SIGKILL);
    if (console_running != NONEXISTENT) {
	if (cfirst)
	    kill(consolepid, SIGHUP);
	else
	    kill(consolepid, SIGKILL);
	cfirst = FALSE;	
    }
    if (x_running != NONEXISTENT)
	kill(xpid, SIGTERM);

    x_stop_wait();

    p = getconf(conf, "ttylogin");
    if (p == NULL) {
	fprintf(stderr, "dm: Can't find login command line\n");
	exit(1);
    }
    cargv = parseargs(p, NULL, NULL, NULL);

    setpgid(0, pgrp=0);		/* We have to reset the tty pgrp */
    tcsetpgrp(0, pgrp);
    tcflush(0, TCIOFLUSH);

    (void) tcgetattr(0, &ttybuf);
    ttybuf.c_lflag |= (ICANON|ISIG|ECHO);
    (void) tcsetattr(0, TCSADRAIN, &ttybuf);
    (void) sigprocmask(SIG_SETMASK, &sig_zero, NULL);
    max_fd = sysconf(_SC_OPEN_MAX);
    
    for (i = 3; i < max_fd; i++)
      close(i);

    if (msg)
      fprintf(stderr,"%s",msg);
    else
      fprintf(stderr,"%s",nl);
#ifdef SOLARIS 
    close(0); close(1); close(2);
    setsid();
    open("/dev/console", O_RDWR, 0);
    dup2(0, 1);
    dup2(0, 2);
    execv(p, cargv);
#else
    execv(p, cargv);
#endif
    fprintf(stderr,"dm: Unable to start console login: %s\n", strerror(errno));
    _exit(1);
}


/* start the console program.  It will have stdin set to the controling
 * side of the console pty, and stdout set to /dev/console inherited 
 * from the display manager.
 */

static void start_console(char *line, char **argv)
{
    int file, pgrp, i;
    char *number(), c, buf[64], **argvp;
    struct termios tc;
#ifdef SOLARIS
    int fd;
#endif

    syslog(LOG_DEBUG, "Starting console");
    if (console_tty == 0) {
	/* Open master side of pty */
	line[5] = 'p';
	console_tty = open(line, O_RDONLY, 0);
	if (console_tty < 0) {
	    /* failed to open this tty, find another one */
	    for (c = 'p'; c <= 's'; c++) {
		line[8] = c;
		for (i = 0; i < 16; i++) {
		    line[9] = "0123456789abcdef"[i];
		    console_tty = open(line, O_RDONLY, 0);
		    if (console_tty >= 0) break;
		}
		if (console_tty >= 0) break;
	    }
	}
	/* out of ptys, use stdout (/dev/console) */
	if (console_tty < 0) console_tty = 1;
	/* Create console log file owned by daemon */
	if (access(consolelog, F_OK) != 0) {
	    file = open(consolelog, O_CREAT, 0644);
	    close(file);
	}
	chown(consolelog, DAEMON, 0);
    }
    line[5] = 't';

    console_running = RUNNING; 
    switch (fork_and_store(&consolepid)) {
    case 0:
	/* Close all file descriptors except stdout/stderr */
	close(0);
	max_fd = sysconf(_SC_OPEN_MAX);
	for (file = 3; file < max_fd; file++)
	  if (file != console_tty)
	    close(file);
	setsid();
	dup2(console_tty, 0);
	close(console_tty);
#ifdef SOLARIS
        fd = grabconsole();
#endif
	/* Since we are the session leader, we must initialize the tty */
	(void) tcgetattr(0, &tc);
	tc.c_iflag = ICRNL|BRKINT|ISTRIP|ICRNL|IXON|IXANY|IMAXBEL;
	tc.c_oflag = OPOST|ONLCR|TAB3;
	tc.c_lflag = ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHOCTL;
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;

	/* assume that the OS requires us to initialize the remaining
	   c_cc entries if and only if it defines the canonical values
	   for us */
#ifdef CERASE
	tc.c_cc[VERASE] = CERASE;
	tc.c_cc[VKILL] = CKILL;
	tc.c_cc[VEOF] = CEOF;
	tc.c_cc[VINTR] = CINTR;
	tc.c_cc[VQUIT] = CQUIT;
	tc.c_cc[VSTART] = CSTART;
	tc.c_cc[VSTOP] = CSTOP;
	tc.c_cc[VEOL] = _POSIX_VDISABLE;
	/* The following are common extensions to POSIX */
#ifdef VEOL2
	tc.c_cc[VEOL2] = _POSIX_VDISABLE;
#endif
#ifdef VSUSP
	tc.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
	tc.c_cc[VDSUSP] = CDSUSP;
#endif
#ifdef VLNEXT
	tc.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VREPRINT
	tc.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VDISCRD
	tc.c_cc[VDISCRD] = CFLUSH;
#endif
#ifdef VWERSE
	tc.c_cc[VWERSE] = CWERASE;
#endif
#endif /* CERASE */
	tcsetattr(0, TCSANOW, &tc);

	setgid(DAEMON);
	setuid(DAEMON);
	(void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *)0);
#ifdef SOLARIS
	/* Icky hack: last two args are "-inputfd" and "0"; change the last
	 * one to be the console fd. */
	sprintf(buf, "%d", fd);
	for (argvp = argv; *argvp; argvp++)
	    ;
	*(argvp - 1) = buf;
#endif
	execv(argv[0], argv);
	fprintf(stderr,"dm: Failed to exec console: %s\n", strerror(errno));
	_exit(1);
    case -1:
	fprintf(stderr,"dm: Unable to fork to start console: %s\n",
		strerror(errno));
	_exit(1);
    default:
	if ((file = open(consolepidf, O_WRONLY|O_TRUNC|O_CREAT, 0644)) >= 0) {
	    write(file, number(consolepid), strlen(number(consolepid)));
	    close(file);
	}
    }
}


/* Kill children and hang around forever */

static void shutdown(int signo)
{
    int pgrp, i;
    struct termios tc;
    char buf[BUFSIZ];

    syslog(LOG_DEBUG, "Received SIGFPE, performing shutdown");
    if (login_running == RUNNING)
      kill(loginpid, SIGHUP);
    if (console_running == RUNNING)
      kill(consolepid, SIGHUP);
    if (x_running == RUNNING)
      kill(xpid, SIGTERM);

    setpgid(0, pgrp=0);		/* We have to reset the tty pgrp */
    tcsetpgrp(0, pgrp);
    tcflush(0, TCIOFLUSH);

    (void) tcgetattr(0, &tc);
    tc.c_lflag |= (ICANON|ISIG|ECHO);
    (void) tcsetattr(0, TCSADRAIN, &tc);

    (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *)0);

    while (1) {
	i = read(console_tty, buf, sizeof(buf));
	write(1, buf, i);
    }
}


/* Kill children, remove password entry */

static void cleanup(char *tty)
{
    int file, found;
    struct utmp utmp;    
#ifdef SOLARIS
    struct utmpx utmpx;    
    struct utmpx *utx_tmp;
    char new_id[20];
    char *p;
#endif
    char login[9];

    if (login_running == RUNNING)
      kill(loginpid, SIGHUP);
    if (console_running == RUNNING)
      kill(consolepid, SIGHUP);
    if (x_running == RUNNING)
      kill(xpid, SIGTERM);

    found = 0;
#ifndef SOLARIS
    if ((file = open(utmpf, O_RDWR, 0)) >= 0) {
	while (read(file, (char *) &utmp, sizeof(utmp)) > 0) {
	    if (!strncmp(utmp.ut_line, tty, sizeof(utmp.ut_line))
		) {
		strncpy(login, utmp.ut_name, 8);
		login[8] = '\0';
		if (utmp.ut_name[0] != '\0') {
		    strncpy(utmp.ut_name, "", sizeof(utmp.ut_name));
		    lseek(file, (long) -sizeof(utmp), L_INCR);
		    write(file, (char *) &utmp, sizeof(utmp));
		    found = 1;
		}
		break;
	    }
	}
	close(file);
    }
    if (found) {
	if ((file = open(wtmpf, O_WRONLY|O_APPEND, 0644)) >= 0) {
	    strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
	    strncpy(utmp.ut_name, "", sizeof(utmp.ut_name));
            strncpy(utmp.ut_host, "", sizeof(utmp.ut_host));
	    time(&utmp.ut_time);
	    write(file, (char *) &utmp, sizeof(utmp));
	    close(file);
	}
    }
#else /* SOLARIS */
    gettimeofday(&utmpx.ut_tv, NULL);
    utmpx.ut_type = 8   ;
    strcpy(utmpx.ut_line,tty);
    setutxent();
    utx_tmp = getutxline(&utmpx);
    if ( utx_tmp != NULL ) {
      strncpy(login, utx_tmp->ut_name, 8);
      login[8] = '\0';
      strcpy(utmpx.ut_line, utx_tmp->ut_line);
      strcpy(utmpx.ut_user,utx_tmp->ut_name);
      utmpx.ut_pid = getpid();
      if (utx_tmp)
              strcpy(new_id, utx_tmp->ut_id);
      p = strchr(new_id, '/');
      if (p)
              strcpy(p, "\0");
      strcpy(utmpx.ut_id , new_id);
      pututxline(&utmpx);
      getutmp(&utmpx, &utmp);
      setutent();
      pututline(&utmp);
      if ((file = open(wtmpf,O_WRONLY|O_APPEND)) >= 0) {
              write(file, (char *)&utmp, sizeof(utmp));
              close(file);
      }
       if ((file = open("/usr/adm/wtmpx",O_WRONLY|O_APPEND)) >= 0) {
               write(file, (char *)&utmpx, sizeof(utmpx));
              close(file);
      }
    }
#endif /* SOLARIS */

    al_acct_revert(login, loginpid);

    tcflush(0, TCIOFLUSH);
}


/* When we get sigchild, figure out which child it was and set
 * appropriate flags
 */

static void child(int signo)
{
    int pid;
    int status;
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = child;
    sigaction(SIGCHLD, &sigact, NULL);

    while (1) {
	pid = waitpid(-1, &status, WNOHANG);
	if (pid == 0 || pid == -1)
	    return;

	if (pid == xpid) {
	    syslog(LOG_DEBUG, "Received SIGCHLD for xpid (%d), status %d", pid,
		   status);
	    x_running = NONEXISTENT;
	} else if (pid == consolepid) {
	    syslog(LOG_DEBUG,
		   "Received SIGCHLD for consolepid (%d), status %d", pid,
		   status);
	    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		console_running = FAILED;
	    else
		console_running = NONEXISTENT;
	} else if (pid == loginpid) {
	    syslog(LOG_DEBUG, "Received SIGCHLD for loginpid (%d), status %d",
		   pid, status);
	    if (WEXITSTATUS(status) == CONSOLELOGIN)
		login_running = STARTUP;
	    else
		login_running = NONEXISTENT;
	} else {
	    syslog(LOG_DEBUG, "Received SIGCHLD for unknown pid %d", pid);
	    fprintf(stderr,"dm: Unexpected SIGCHLD from pid %d\n",pid);
	}
    }
}


static void xready(int signo)
{
    syslog(LOG_DEBUG, "Received SIGUSR1; setting x_running.");
    x_running = RUNNING;
}

static void loginready(int signo)
{
    syslog(LOG_DEBUG, "Received SIGUSR1; setting login_running.");
    login_running = RUNNING;
}

/* When an alarm happens, just note it and return */

static void catchalarm(int signo)
{
    syslog(LOG_DEBUG, "Received SIGALRM.");
    alarm_running = NONEXISTENT;
}


/* kill children and go away */

static void die(int signo)
{
    syslog(LOG_DEBUG, "Dying on signal %d", signo);
    cleanup(logintty);
    _exit(0);
}

/* Takes a command line, returns an argv-style  NULL terminated array 
 * of strings.  The array is in malloc'ed memory.
 */

static char **parseargs(char *line, char *extra, char *extra1, char *extra2)
{
    int i = 0;
    char *p = line;
    char **ret;
    char *malloc();

    while (*p) {
	while (*p && isspace(*p)) p++;
	while (*p && !isspace(*p)) p++;
	i++;
    }
    ret = (char **) malloc(sizeof(char *) * (i + 4));

    p = line;
    i = 0;
    while (*p) {
	while (*p && isspace(*p)) p++;
	if (*p == 0) break;
	ret[i++] = p;
	while (*p && !isspace(*p)) p++;
	if (*p == 0) break;
	*p++ = 0;
    }
    if (extra)
      ret[i++] = extra;
    if (extra1)
      ret[i++] = extra1;
    if (extra2)
      ret[i++] = extra2;

    ret[i] = NULL;
    return(ret);
}


/* Convert an int to a string, and return a pointer to this string in a
 * static buffer.  The string will be newline and null terminated.
 */

static char *number(int x)
{
#define NDIGITS 16
    static char buffer[NDIGITS];
    char *p = &buffer[NDIGITS-1];

    *p-- = 0;
    *p-- = '\n';
    while (x) {
	*p-- = x % 10 + '0';
	x = x / 10;
    }
    if (p == &buffer[NDIGITS-3])
      *p-- = '0';
    return(p+1);
}


/* Find a named field in the config file.  Config file contains 
 * comment lines starting with #, and lines with a field name,
 * whitespace, then field value.  This routine returns the field
 * value, or NULL on error.
 */

static char *getconf(char *file, char *name)
{
    static char buf[8192];
    static int inited = 0;
    char *p, *ret, *malloc();
    int i;

    if (!inited) {
	int fd;

	fd = open(file, O_RDONLY, 0644);
	if (fd < 0) return(NULL);
	i = read(fd, buf, sizeof(buf));
	if (i >= sizeof(buf) - 1)
	  fprintf(stderr,"dm: warning - config file is to long to parse\n");
	buf[i] = 0;
	close(fd);
	inited = 1;
    }

    for (p = &buf[0]; p && *p; p = strchr(p, '\n')) {
	if (*p == '\n') p++;
	if (p == NULL || *p == 0) return(NULL);
	if (*p == '#') continue;
	if (strncmp(p, name, strlen(name))) continue;
	p += strlen(name);
	if (*p && !isspace(*p)) continue;
	while (*p && isspace(*p)) p++;
	if (*p == 0) return(NULL);
	ret = strchr(p, '\n');
	if (ret)
	  i = ret - p;
	else
	  i = strlen(p);
	ret = malloc(i+1);
	memcpy(ret, p, i+1);
	ret[i] = 0;
	return(ret);
    }
    return(NULL);
}

/* Fork, storing the pid in a variable var and returning the pid.
 * Make sure that the pid is stored before any SIGCHLD can be
 * delivered.
 */
static pid_t fork_and_store(pid_t *var)
{
  sigset_t mask, omask;
  pid_t pid;

  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);
  pid = fork();
  *var = pid;
  sigprocmask(SIG_SETMASK, &omask, NULL);
  return pid;
}

static void x_stop_wait(void)
{
    sigset_t mask, omask;

    /* Wait X_STOP_WAIT seconds for an X server to exit and for the
     * graphics device to recover.  Be paranoid about other signals
     * interrupting the sleep and about prior alarms.
     */
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    alarm(0);
    sleep(X_STOP_WAIT);
    sigprocmask(SIG_SETMASK, &omask, NULL);
}
