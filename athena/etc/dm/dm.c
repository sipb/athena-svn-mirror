/* $Id: dm.c,v 1.12 2000-02-15 15:54:32 ghudson Exp $
 *
 * Copyright (c) 1990, 1991 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This is the top-level of the display manager and console control
 * for Athena's xlogin.
 */

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifdef HAVE_SYS_STRREDIR_H
#include <sys/strredir.h>
#endif
#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <utmp.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
#include <termios.h>
#include <syslog.h>
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#ifdef HAVE_PTY_H
#include <pty.h>
#endif

#include <X11/Xlib.h>
#include <al.h>

#ifndef lint
static const char rcsid[] = "$Id: dm.c,v 1.12 2000-02-15 15:54:32 ghudson Exp $";
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

static sigset_t sig_zero;
static sigset_t sig_cur;

/* flags used by signal handlers */
pid_t xpid, consolepid, loginpid;
volatile int alarm_running = NONEXISTENT;
volatile int x_running = NONEXISTENT;
volatile int console_running = NONEXISTENT;
volatile int console_failed = FALSE;
volatile int login_running = NONEXISTENT;
char *logintty;
int console_tty = 0;

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
static char *getconf(char *file, char *name);
static char **parseargs(char *line, char *extra, char *extra1, char *extra2);
static void console_login(char *conf, char *msg);
static void start_console(int fd, char **argv, int redir);
static void cleanup(char *tty);
static pid_t fork_and_store(pid_t *var);
static void x_stop_wait(void);
static void writepid(char *file, pid_t pid);

#ifndef HAVE_LOGOUT
static void logout(const char *line);
#endif
#ifndef HAVE_LOGIN_TTY
static int login_tty(int fd);
#endif
#ifndef HAVE_OPENPTY
static int openpty(int *amaster, int *aslave, char *name,
		   struct termios *termp, struct winsize *winp);
static int termsetup(int fd, struct termios *termp, struct winsize *winp);
#endif


/* the console process will run as daemon */
#define DAEMON 1

#define X_START_WAIT	30	/* wait up to 30 seconds for X to be ready */
#define LOGIN_START_WAIT 60	/* wait up to 1 minute for Xlogin */
#ifndef BUFSIZ
#define BUFSIZ		1024
#endif

static int max_fd;

/* Setup signals, start X, start console, start login, wait */

int main(int argc, char **argv)
{
  char *consoletty, *conf, *p;
  char **dmargv, **xargv, **consoleargv = NULL, **loginargv;
  char xpidf[256], line[16], buf[256];
  fd_set readfds;
  int pgrp, file, tries, count, redir = TRUE;
  char dpyname[10], dpyacl[40];
  Display *dpy;
  XHostAddress *hosts;
  int nhosts, dpynum = 0;
  struct stat hostsinfo;
  Bool state;
  time_t now, last_console_failure = 0;
  struct sigaction sigact;
  sigset_t mask;
#if defined(SRIOCSREDIR) || defined(TIOCCONS)
  int on;
#endif
  int fd;
  char loginttyname[256];
  int ttyfd;

  logintty = &loginttyname[5]; /* skip over the /dev/ */

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
#ifdef notdef
  putenv("LD_LIBRARY_PATH=/usr/openwin/lib");
  putenv("OPENWINHOME=/usr/openwin");
#endif

  if (argc < 2)
    {
      fprintf(stderr, "dm: first argument must be configuration file\n");
      sleep(60);
      exit(1);
    }

  conf = argv[1];

  if (argc != 4 && (argc != 5 || strcmp(argv[3], "-noconsole")))
    {
      fprintf(stderr,
	      "usage: %s configfile logintty [-noconsole] consoletty\n",
	      argv[0]);
      console_login(conf, NULL);
    }
  if (argc == 5)
    redir = FALSE;

  /* parse argument lists */
  /* ignore argv[2] */
  consoletty = argv[argc - 1];

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
		  dpynum = atoi(*(dmargv) + 1);
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

  p = getconf(conf, "console");
  if (p == NULL)
    console_login(conf, "\ndm: Can't find console command line\n");

  consoleargv = parseargs(p, NULL, NULL, NULL);

  p = getconf(conf, "login");
  if (p == NULL)
    console_login(conf, "\ndm: Can't find login command line\n");
  loginargv = parseargs(p, logintty, "-tty", logintty);

  /* Signal Setup */
  sigact.sa_handler = SIG_IGN;
  sigaction(SIGTSTP, &sigact, NULL);
  sigaction(SIGTTIN, &sigact, NULL);
  sigaction(SIGTTOU, &sigact, NULL);
  /* so that X pipe errors don't nuke us */
  sigaction(SIGPIPE, &sigact, NULL);
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

  strcpy(line, "/dev/");
  strcat(line, consoletty);

  fd = open(line, O_RDWR);
  if (fd == -1)
    {
      syslog(LOG_ERR, "Cannot open %s: %m", line);
      /* This probably won't work, but it seems to be the appropriate
	 punt location. */
      console_login(conf, "Cannot open tty.\n");
    }

  login_tty(fd);

  /* Set the console characteristics so we don't lose later */
  setpgid(0, pgrp = getpid());	/* Reset the tty pgrp  */
  tcsetpgrp(0, pgrp);

  /* save our pid file */
  writepid(dmpidf, getpid());

  /* Fire up X */
  xpid = 0;
  for (tries = 0; tries < 3; tries++)
    {
      syslog(LOG_DEBUG, "Starting X, try #%d", tries + 1);
      x_running = STARTUP;
      sigact.sa_handler = xready;
      sigaction(SIGUSR1, &sigact, NULL);
      switch (fork_and_store(&xpid))
	{
	case 0:
	  if (fcntl(2, F_SETFD, 1) == -1)
	    close(2);
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *) 0);

	  /* ignoring SIGUSR1 will cause the server to send us a SIGUSR1
	   * when it is ready to accept connections
	   */
	  sigact.sa_handler = SIG_IGN;
	  sigaction(SIGUSR1, &sigact, NULL);
	  p = *xargv;
	  *xargv = "X";
	  execv(p, xargv);
	  fprintf(stderr, "dm: X server failed exec: %s\n", strerror(errno));
	  _exit(1);
	case -1:
	  fprintf(stderr, "dm: Unable to fork to start X server: %s\n",
		  strerror(errno));
	  break;
	default:
	  sprintf(xpidf, xpids, dpynum);
	  writepid(xpidf, xpid);

	  if (x_running == STARTUP)
	    {
	      alarm(X_START_WAIT);
	      alarm_running = RUNNING;
	      sigsuspend(&sig_zero);
	    }
	  if (x_running != RUNNING)
	    {
	      syslog(LOG_DEBUG, "X failed to start; alarm_running=%d",
		     alarm_running);
	      if (alarm_running == NONEXISTENT)
		fprintf(stderr, "dm: Unable to start X\n");
	      else
		fprintf(stderr, "dm: X failed to become ready\n");

	      /* If X wouldn't run, it could be that an existing X
	       * process hasn't shut down.  Wait X_STOP_WAIT seconds
	       * for that to happen.
	       */
	      x_stop_wait();
	    }
	  sigact.sa_handler = SIG_IGN;
	  sigaction(SIGUSR1, &sigact, NULL);
	}
      if (x_running == RUNNING)
	break;
    }
  alarm(0);
  if (x_running != RUNNING)
    {
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

  /* set up the console pty */
  if (openpty(&console_tty, &ttyfd, loginttyname, NULL, NULL)==-1)
    console_login(conf, "Cannot allocate pseudo-terminal\n");

  /* start up console */
  start_console(console_tty, consoleargv, redir);

  /* Fire up the X login */
  for (tries = 0; tries < 3; tries++)
    {
      syslog(LOG_DEBUG, "Starting xlogin, try #%d", tries + 1);
      login_running = STARTUP;
      sigact.sa_handler = loginready;
      sigaction(SIGUSR1, &sigact, NULL);
      switch (fork_and_store(&loginpid))
	{
	case 0:
	  max_fd = sysconf(_SC_OPEN_MAX);
	  for (file = 0; file < max_fd; file++)
	    {
	      if (file != ttyfd)
		close(file);
	    }

	  login_tty(ttyfd);
	  
	  file = open("/dev/null", O_RDONLY);
	  if (file >= 0)
	    {
	      dup2(file, 0);
	      if (file != 0)
		close(file);
	    }
	  
	  if (redir)
	    {
	      /* really ought to check the return status of these */
#ifdef SRIOCSREDIR
	      on = open("/dev/console", O_RDONLY);
	      if (on >= 0)
		{
		  ioctl(on, SRIOCSREDIR, 1);
		  close(on);
		}
#else
#ifdef TIOCCONS
	      on = 1;
	      ioctl(1, TIOCCONS, &on);
#endif
#endif
	    }
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *) 0);
	  /* ignoring SIGUSR1 will cause xlogin to send us a SIGUSR1
	   * when it is ready
	   */
	  sigact.sa_handler = SIG_IGN;
	  sigaction(SIGUSR1, &sigact, NULL);
	  /* dm ignores sigpipe; because of this, all of the children (ie, */
	  /* the entire session) inherit this unless we fix it now */
	  sigact.sa_handler = SIG_DFL;
	  sigaction(SIGPIPE, &sigact, NULL);
	  execv(loginargv[0], loginargv);
	  fprintf(stderr, "dm: X login failed exec: %s\n", strerror(errno));
	  _exit(1);
	case -1:
	  fprintf(stderr, "dm: Unable to fork to start X login: %s\n",
		  strerror(errno));
	  break;
	default:
	  alarm(LOGIN_START_WAIT);
	  alarm_running = RUNNING;
	  while (login_running == STARTUP && alarm_running == RUNNING)
	    sigsuspend(&sig_zero);
	  if (login_running != RUNNING)
	    {
	      syslog(LOG_DEBUG, "xlogin failed to start; alarm_running=%d",
		     alarm_running);
	      kill(loginpid, SIGKILL);
	      if (alarm_running != NONEXISTENT)
		fprintf(stderr, "dm: Unable to start Xlogin\n");
	      else
		fprintf(stderr, "dm: Xlogin failed to become ready\n");
	    }
	}
      if (login_running == RUNNING)
	break;
    }
  sigact.sa_handler = SIG_IGN;
  sigaction(SIGUSR1, &sigact, NULL);
  alarm(0);
  if (login_running != RUNNING)
    {
      syslog(LOG_DEBUG, "Giving up on starting xlogin.");
      console_login(conf, "\nUnable to start xlogin, doing console login "
		    "instead.\n");
    }

  /* main loop.  Wait for SIGCHLD, waking up every minute anyway. */
  (void) sigemptyset(&sig_cur);
  (void) sigaddset(&sig_cur, SIGCHLD);
  (void) sigprocmask(SIG_BLOCK, &sig_cur, NULL);
  while (1)
    {
      /* Wait for something to hapen */
      if (console_failed)
	{
	  /* if no console is running, we must copy bits from the console
	   * (master side of pty) to the real console to appear as black
	   * bar messages.
	   */
	  FD_ZERO(&readfds);
	  FD_SET(console_tty, &readfds);
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, &mask);
	  count = select(console_tty + 1, &readfds, NULL, NULL, NULL);
	  (void) sigprocmask(SIG_BLOCK, &mask, NULL);
	  if (count > 0 && FD_ISSET(console_tty, &readfds))
	    {
	      file = read(console_tty, buf, sizeof(buf));
	      write(1, buf, file);
	    }
	}
      else
	{
	  alarm(60);
	  sigsuspend(&sig_zero);
	}

      if (login_running == STARTUP)
	{
	  (void) sigprocmask(SIG_SETMASK, &sig_zero, NULL);
	  console_login(conf, "\nConsole login requested.\n");
	}
      if (console_running == FAILED)
	{
	  console_running = NONEXISTENT;
	  time(&now);
	  if (now - last_console_failure <= 3)
	    {
	      /* Give up on console.  Set the console characteristics so
	       * we don't lose later. */
	      syslog(LOG_ERR, "Giving up on the console");
	      setpgid(0, pgrp = getpid());	/* Reset the tty pgrp */
	      tcsetpgrp(0, pgrp);
	      console_failed = TRUE;
	    }
	  else
	    last_console_failure = now;
	}
      if (console_running == NONEXISTENT && !console_failed)
	start_console(console_tty, consoleargv, redir);
      if (login_running == NONEXISTENT || x_running == NONEXISTENT)
	{
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
  int i, cfirst = TRUE, pgrp;
  char *nl = "\r\n";
  struct termios ttybuf;
  char *p, **cargv;
  int fd;

  syslog(LOG_DEBUG, "Performing console login: %s", msg);
  sigemptyset(&sig_zero);
  if (login_running != NONEXISTENT && login_running != STARTUP)
    kill(loginpid, SIGKILL);
  if (console_running != NONEXISTENT)
    {
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
  if (p == NULL)
    {
      fprintf(stderr, "dm: Can't find login command line\n");
      exit(1);
    }
  cargv = parseargs(p, NULL, NULL, NULL);

  setpgid(0, pgrp = 0);		/* We have to reset the tty pgrp */
  tcsetpgrp(0, pgrp);
  tcflush(0, TCIOFLUSH);

  (void) tcgetattr(0, &ttybuf);
  ttybuf.c_lflag |= (ICANON | ISIG | ECHO);
  (void) tcsetattr(0, TCSADRAIN, &ttybuf);
  (void) sigprocmask(SIG_SETMASK, &sig_zero, NULL);
  max_fd = sysconf(_SC_OPEN_MAX);

  for (i = 3; i < max_fd; i++)
    close(i);

  if (msg)
    fprintf(stderr, "%s", msg);
  else
    fprintf(stderr, "%s", nl);

  fd = open("/dev/console", O_RDWR);
  if (fd >= 0)
    login_tty(fd);

  execv(p, cargv);

  fprintf(stderr, "dm: Unable to start console login: %s\n", strerror(errno));
  _exit(1);
}


/* Start the console program.  It will have stdin set to the controling
 * side of the console pty, and stdout set to /dev/console inherited 
 * from the display manager.
 */

static void start_console(int fd, char **argv, int redir)
{
  int file;

  syslog(LOG_DEBUG, "Starting console");

  /* Create console log file owned by daemon */
  if (access(consolelog, F_OK) != 0)
    {
      file = open(consolelog, O_CREAT, 0644);
      close(file);
    }
  chown(consolelog, DAEMON, 0);

  console_running = RUNNING;
  switch (fork_and_store(&consolepid))
    {
    case 0:
      /* Close all file descriptors except stdout/stderr */
      close(0);
      max_fd = sysconf(_SC_OPEN_MAX);
      for (file = 3; file < max_fd; file++)
	if (file != fd)
	  close(file);
      setsid();
      dup2(fd, 0);
      close(fd);

      setgid(DAEMON);
      setuid(DAEMON);
      (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *) 0);
      execv(argv[0], argv);
      fprintf(stderr, "dm: Failed to exec console: %s\n", strerror(errno));
      _exit(1);
    case -1:
      fprintf(stderr, "dm: Unable to fork to start console: %s\n",
	      strerror(errno));
      _exit(1);
    default:
      writepid(consolepidf, consolepid);
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

  setpgid(0, pgrp = 0);		/* We have to reset the tty pgrp */
  tcsetpgrp(0, pgrp);
  tcflush(0, TCIOFLUSH);

  (void) tcgetattr(0, &tc);
  tc.c_lflag |= (ICANON | ISIG | ECHO);
  (void) tcsetattr(0, TCSADRAIN, &tc);

  (void) sigprocmask(SIG_SETMASK, &sig_zero, (sigset_t *) 0);

  while (1)
    {
      i = read(console_tty, buf, sizeof(buf));
      write(1, buf, i);
    }
}


/* Kill children, remove password entry */

static void cleanup(char *tty)
{
  int file;
  struct utmp utmp;
  char login[sizeof(utmp.ut_name) + 1];

  if (login_running == RUNNING)
    kill(loginpid, SIGHUP);
  if (console_running == RUNNING)
    kill(consolepid, SIGHUP);
  if (x_running == RUNNING)
    kill(xpid, SIGTERM);

  /* Find out what the login name was, so we can feed it to libal. */
  if ((file = open(utmpf, O_RDWR, 0)) >= 0)
    {
      while (read(file, (char *) &utmp, sizeof(utmp)) > 0)
	{
	  if (!strncmp(utmp.ut_line, tty, sizeof(utmp.ut_line)))
	    {
	      strncpy(login, utmp.ut_name, sizeof(utmp.ut_name));
	      login[sizeof(utmp.ut_name)] = '\0';
	    }
	}
      close(file);
    }

  /* Update the utmp & wtmp. */

  logout(tty);
  
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

  while (1)
    {
      pid = waitpid(-1, &status, WNOHANG);
      if (pid == 0 || pid == -1)
	return;

      if (pid == xpid)
	{
	  syslog(LOG_DEBUG, "Received SIGCHLD for xpid (%d), status %d", pid,
		 status);
	  x_running = NONEXISTENT;
	}
      else if (pid == consolepid)
	{
	  syslog(LOG_DEBUG,
		 "Received SIGCHLD for consolepid (%d), status %d", pid,
		 status);
	  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	    console_running = FAILED;
	  else
	    console_running = NONEXISTENT;
	}
      else if (pid == loginpid)
	{
	  syslog(LOG_DEBUG, "Received SIGCHLD for loginpid (%d), status %d",
		 pid, status);
	  if (WEXITSTATUS(status) == CONSOLELOGIN)
	    login_running = STARTUP;
	  else
	    login_running = NONEXISTENT;
	}
      else
	{
	  syslog(LOG_DEBUG, "Received SIGCHLD for unknown pid %d", pid);
	  fprintf(stderr, "dm: Unexpected SIGCHLD from pid %d\n", pid);
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

  while (*p)
    {
      while (*p && isspace((unsigned char)*p))
	p++;
      while (*p && !isspace((unsigned char)*p))
	p++;
      i++;
    }
  ret = (char **) malloc(sizeof(char *) * (i + 4));

  p = line;
  i = 0;
  while (*p)
    {
      while (*p && isspace((unsigned char)*p))
	p++;
      if (*p == 0)
	break;
      ret[i++] = p;
      while (*p && !isspace((unsigned char)*p))
	p++;
      if (*p == 0)
	break;
      *p++ = 0;
    }
  if (extra)
    ret[i++] = extra;
  if (extra1)
    ret[i++] = extra1;
  if (extra2)
    ret[i++] = extra2;

  ret[i] = NULL;
  return (ret);
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
  char *p, *ret;
  int i;

  if (!inited)
    {
      int fd;

      fd = open(file, O_RDONLY, 0644);
      if (fd < 0)
	return (NULL);
      i = read(fd, buf, sizeof(buf));
      if (i >= sizeof(buf) - 1)
	fprintf(stderr, "dm: warning - config file is to long to parse\n");
      buf[i] = 0;
      close(fd);
      inited = 1;
    }

  for (p = &buf[0]; p && *p; p = strchr(p, '\n'))
    {
      if (*p == '\n')
	p++;
      if (p == NULL || *p == 0)
	return (NULL);
      if (*p == '#')
	continue;
      if (strncmp(p, name, strlen(name)))
	continue;
      p += strlen(name);
      if (*p && !isspace((unsigned char)*p))
	continue;
      while (*p && isspace((unsigned char)*p))
	p++;
      if (*p == 0)
	return (NULL);
      ret = strchr(p, '\n');
      if (ret)
	i = ret - p;
      else
	i = strlen(p);
      ret = malloc(i + 1);
      memcpy(ret, p, i + 1);
      ret[i] = 0;
      return (ret);
    }
  return (NULL);
}

/* Fork, storing the pid in a variable var and returning the pid.
 * Make sure that the pid is stored before any SIGCHLD can be
 * delivered.
 */
static pid_t fork_and_store(pid_t * var)
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

  sigfillset(&mask);
  sigdelset(&mask, SIGALRM);
  sigprocmask(SIG_BLOCK, &mask, &omask);
  alarm(0);
  while (waitpid(xpid, NULL, 0) == -1 && errno == EINTR)
    ;
  x_running = NONEXISTENT;
  sigprocmask(SIG_SETMASK, &omask, NULL);
}

/* Write a pid to a file */
static void writepid(char *file, pid_t pid)
{
  FILE *fp;

  fp = fopen(file, "w");

  if (fp == NULL)
    return;

  fprintf(fp, "%lu\n", (unsigned long)pid);
  fclose(fp);
}

#ifndef HAVE_LOGOUT
#ifdef HAVE_PUTUTLINE
static void logout(const char *line)
{
  struct utmp utmp;
  struct utmp *putmp;
#ifdef HAVE_PUTUTXLINE
  struct utmpx utmpx;
#endif
#ifndef HAVE_UPDWTMP
  int file;
#endif

  strcpy(utmp.ut_line, line);
  setutent();
  putmp = getutline(&utmp);
  if (putmp != NULL)
    {
      time(&utmp.ut_time);
      strncpy(utmp.ut_line, putmp->ut_line, sizeof(utmp.ut_line));
      strncpy(utmp.ut_user, putmp->ut_name, sizeof(utmp.ut_name));
      utmp.ut_pid = getpid();
      strncpy(utmp.ut_id, putmp->ut_id, sizeof(utmp.ut_id));
      utmp.ut_type = DEAD_PROCESS;
      pututline(&utmp);
#ifdef HAVE_PUTUTXLINE
      getutmpx(&utmp, &utmpx);
      setutxent();
      pututxline(&utmpx);
      endutxent();
#endif /* HAVE_PUTUTXLINE */

      updwtmp(wtmpf, &utmp);
    }
  endutent();
}
#endif
#endif

#ifndef HAVE_LOGIN_TTY
static int login_tty(int fd)
{
#ifndef TIOCSCTTY
  char *name;
#endif
  int ttyfd;

  setsid();

#ifdef TIOCSCTTY
  if (ioctl(fd, TIOCSCTTY, NULL) == -1)
    return(-1);
  ttyfd = fd;
#else
  name = ttyname(fd);
  if (ttyname == NULL)
    return(-1);
  close(fd);
  ttyfd = open(name, O_RDWR);
#endif

  if (ttyfd == -1)
    return(-1);

  dup2(ttyfd, STDIN_FILENO);
  dup2(ttyfd, STDOUT_FILENO);
  dup2(ttyfd, STDERR_FILENO);

  if (ttyfd > STDERR_FILENO)
    close(ttyfd);

  return(0);
}
#endif

#ifndef HAVE_OPENPTY
#if HAVE__GETPTY
/* Oooh, we're on an sgi. */
static int openpty(int *amaster, int *aslave, char *name,
		   struct termios *termp, struct winsize *winp)
{
  char *p;

  p = _getpty(amaster, O_RDWR, 0600, 0);

  if (p == NULL)
    return(-1);

  if (name != NULL)
    strcpy(name, p);

  *aslave = open(p, O_RDWR);
  if (*aslave < 0)
    {
      close(*amaster);
      return(-1);
    }

  if (termsetup(*aslave, termp, winp) < 0)
    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }

  return(0);
}
#elif HAVE_GRANTPT
static int openpty(int *amaster, int *aslave, char *name,
		   struct termios *termp, struct winsize *winp)
{
  char *p;
  
  *amaster = open("/dev/ptmx", O_RDWR);
  if (*amaster < 0)
    return(-1);

  if (grantpt(*amaster) < 0)
    {
      close(*amaster);
      return(-1);
    }

  if (unlockpt(*amaster) < 0)
    {
      close(*amaster);
      return(-1);
    }

  p = ptsname(*amaster);

  if (name != NULL)
    strcpy(name, p);

  *aslave = open(p, O_RDWR);
  if(*aslave < 0)
    {
      close(*amaster);
      return(-1);
    }

  if (ioctl(*aslave, I_PUSH, "ptem") < 0)

    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }
  
  if (ioctl(*aslave, I_PUSH, "ldterm") < 0)
    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }
  
  if (ioctl(*aslave, I_PUSH, "ttcompat") < 0)
    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }

  if (termsetup(*aslave, termp, winp) < 0)
    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }

  return(0);
}
#else
/* traditional bsd way */
static int openpty(int *amaster, int *aslave, char *name,
		   struct termios *termp, struct winsize *winp);
{
  char s[20];
  char *p, *q;

  *amaster == -1;

  strcpy(s, "/dev/ptyxx");
  for (p = "pqrstuvwxyzPQRST"; *p; p++)
    {
      s[8] = *p;
      for(q = "0123456789abcdef"; *q; q++)
	{
	  s[9] = *q;

	  *amaster = open(s, O_RDWR);
	  if (*amaster == ENOENT)
	    return(-1);
	  else
	    continue;

	  s[5]='t';

	  break;
	}
      
      if (*amaster != -1)
	  break;
    }

  if (name != NULL)
    strcpy(s, name);

  *aslave = open(s, O_RDWR);

  if (*aslave < 0)
    {
      close(*amaster);
      return(-1);
    }


  if (termsetup(*aslave, termp, winp) < 0)
    {
      close(*amaster);
      close(*aslave);
      return(-1);
    }

  return(0);
}
#endif

static int termsetup(int fd, struct termios *termp, struct winsize *winp)
{
  if (termp != NULL)
      if (tcsetattr(fd, TCSANOW, termp) < 0)
	  return(-1);

  if (winp != NULL)
    if (ioctl(fd, TIOCSWINSZ, winp) < 0)
      return(-1);

  return(0);
}
#endif
