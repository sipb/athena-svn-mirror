/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.2 1990-10-19 18:51:05 mar Exp $
 *
 * Copyright (c) 1990 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This is the top-level of the display manager and console control
 * for Athena's xlogin.
 */

#include <mit-copyright.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <utmp.h>
#include <ctype.h>
#include <strings.h>


#ifndef lint
static char *rcsid_main = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.2 1990-10-19 18:51:05 mar Exp $";
#endif

#ifndef NULL
#define NULL 0
#endif

/* Process states */
#define NONEXISTANT	0
#define RUNNING		1
#define STARTUP		2
#define CONSOLELOGIN	3

/* flags used by signal handlers */
int alarm_running = NONEXISTANT;
int xpid, x_running = NONEXISTANT;
int consolepid, console_running = NONEXISTANT;
int loginpid, login_running = NONEXISTANT;

char deactivate[] ="/etc/athena/deactivate";
#ifdef ultrix
char login_prog[]="/etc/athena/console-getty";
#else
char login_prog[]="/bin/login";
#endif ultrix
char utmpf[]="/etc/utmp";
char wtmpf[]="/usr/adm/wtmp";
char xpidf[]="/usr/tmp/X0.pid";
char consolepidf[]="/usr/tmp/console.pid";
char consolef[] ="/dev/console";

#define X_START_WAIT	30	/* wait up to 30 seconds for X to be ready */


main(argc, argv)
int argc;
char **argv;
{
    void die(), child(), alarm(), xready();
    char *tty, *conf, *p, *number(), *getconf();
    char **xargv, **consoleargv, **loginargv, **parseargs();
    char line[16];
    int pgrp, file;
    struct sgttyb mode;

    if (argc != 3) {
	message("usage: ");
	message(argv[0]);
	message(" configfile tty\n");
	exit(1);
    }
    conf = argv[1];
    tty = argv[2];
    p = getconf(conf, "X");
    if (p == NULL) {
	message("Can't find X command line\n");
	exit(1);
    }
    xargv = parseargs(p, NULL);
    p = getconf(conf, "console");
    if (p == NULL) {
	message("Can't find console command line\n");
	exit(1);
    }
    consoleargv = parseargs(p, NULL);
    p = getconf(conf, "login");
    if (p == NULL) {
	message("Can't find login command line\n");
	exit(1);
    }
    loginargv = parseargs(p, tty);

    /* Signal Setup */
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);	/* so that X pipe errors don't nuke us */
    signal(SIGHUP, die);
    signal(SIGINT, die);
    signal(SIGTERM, die);
    signal(SIGCHLD, child);
    signal(SIGALRM, alarm);

    close(0);
    close(1);
    close(2);
    strcpy(line, "/dev/");
    strcat(line, tty);
    open(line, O_RDWR, 0622);
    dup2(0, 1);
    dup2(1, 2);
#ifndef BROKEN_CONSOLE_DRIVER
    /* Set the console characteristics so we don't lose later */
#ifdef TIOCCONS
    ioctl (0, TIOCCONS, 0);		/* Grab the console   */
#endif TIOCCONS
    setpgrp(0, pgrp=getpid());		/* Reset the tty pgrp */
    ioctl (0, TIOCSPGRP, &pgrp);
#endif

    /* Fire up X */
#ifdef DEBUG
    message("Starting X\n");
#endif
    xpid = fork();
    switch (xpid) {
    case 0:
        if(fcntl(2, F_SETFD, 1) == -1)
	  close(2);
	sigsetmask(0);
	/* ignoring SIGUSR1 will cause the server to send us a SIGUSR1
	 * when it is ready to accept connections
	 */
	signal(SIGUSR1, SIG_IGN);
	p = *xargv;
	*xargv = "-";
	execv(p, xargv);
	message("X server failed exec\n");
	exit(1);
    case -1:
	message("Unable to start X server\n");
	exit(1);
    default:
	x_running = STARTUP;
	signal(SIGUSR1, xready);
	if ((file = open(xpidf, O_WRONLY|O_TRUNC|O_CREAT, 0644)) >= 0) {
	    write(file, number(xpid), strlen(number(xpid)));
	    close(file);
	}
	alarm(X_START_WAIT);
	alarm_running = RUNNING;
#ifdef DEBUG
	message("waiting for X\n");
#endif
	sigpause(0);
	if (x_running != RUNNING) {
	    if (alarm_running == NONEXISTANT)
	      message("X failed to become ready\n");
	    else
	      message("Unable to start X\n");
	    exit(1);
	}
	signal(SIGUSR1, SIG_IGN);
    }

    start_console(consoleargv);

    /* Fire up the X login */
#ifdef DEBUG
    message("Starting X Login\n");
#endif
    loginpid = fork();
    switch (loginpid) {
    case 0:
        if(fcntl(2, F_SETFD, 1) == -1)
	  close(2);
	sigsetmask(0);
	execv(loginargv[0], loginargv);
	message("X login failed exec\n");
	exit(1);
    case -1:
	message("Unable to start X login\n");
	exit(1);
    default:
	login_running = RUNNING;
    }

    while (1) {
#ifdef DEBUG
	message("waiting...\n");
#endif
	/* Wait for something to hapen */
	sigpause(0);

	if (login_running == STARTUP) {
#ifdef DEBUG
	    message("starting console login\n");
#endif
	    kill(xpid, SIGKILL);
#ifndef BROKEN_CONSOLE_DRIVER
	    setpgrp(0, pgrp=0);		/* We have to reset the tty pgrp */
	    ioctl(0, TIOCSPGRP, &pgrp);
#endif
	    ioctl(0, TIOCGETP, &mode);
	    mode.sg_flags = mode.sg_flags & ~RAW | ECHO;
	    ioctl(0, TIOCSETP, &mode);
	    execl(login_prog, login_prog, 0);
	    message("Unable to start console login\n");
	}
	if (console_running == NONEXISTANT)
	  start_console(consoleargv);
	if (login_running == NONEXISTANT || x_running == NONEXISTANT) {
	    cleanup(tty);
	    exit(0);
	}
    }
}


/* start the console program */

start_console(argv)
char **argv;
{
    int file;

#ifdef DEBUG
    message("Starting Console\n");
#endif
    consolepid = fork();
    switch (consolepid) {
    case 0:
        if(fcntl(2, F_SETFD, 1) == -1)
	  close(2);
	sigsetmask(0);
	execv(argv[0], argv);
	message("Failed to exec console\n");
	exit(1);
    case -1:
	message("Unable to start console\n");
	exit(1);
    default:
	console_running = RUNNING;
	if ((file = open(consolepidf, O_WRONLY|O_TRUNC|O_CREAT, 0644)) >= 0) {
	    write(file, number(consolepid), strlen(number(consolepid)));
	    close(file);
	}
    }
}


/* Kill children, remove password entry, and deactivate */

cleanup(tty)
char *tty;
{
    int in_use, file, found;
    struct utmp utmp;    

    if (login_running == RUNNING)
      kill(loginpid, SIGHUP);
    if (console_running == RUNNING)
      kill(consolepid, SIGHUP);
    if (x_running == RUNNING)
      kill(xpid, SIGKILL);

    found = in_use = 0;
    if ((file = open(utmpf, O_RDWR, 0)) >= 0) {
	while (read(file, (char *) &utmp, sizeof(utmp)) > 0) {
	    if (!strncmp(utmp.ut_line, tty)) {
		utmp.ut_name[0] = 0;
		lseek(file, (long) -sizeof(utmp), L_INCR);
		write(file, (char *) &utmp, sizeof(utmp));
		found = 1;
	    } else if (*utmp.ut_name) in_use = 1;
	}
    }
    close(file);
    if (found) {
	if ((file = open(wtmpf, O_WRONLY|O_APPEND, 0644)) >= 0) {
	    strcpy(utmp.ut_line, tty);
	    strcpy(utmp.ut_name, "");
	    strcpy(utmp.ut_host, "");
	    time(&utmp.ut_time);
	    write(file, (char *) &utmp, sizeof(utmp));
	    close(file);
	}
    }

    if (!in_use)
      execl(deactivate, deactivate, 0);
}


/* When we get sigchild, figure out which child it was and set
 * appropriate flags
 */

void child()
{
    int pid;
    union wait status;
    char *number();

    pid = wait3(&status, WNOHANG, 0);

#ifdef DEBUG
    message("Child exited "); message(number(pid));
#endif
    if (pid == xpid) {
#ifdef DEBUG
	message("X Server exited\n");
#endif
	x_running = NONEXISTANT;
    } else if (pid == consolepid) {
#ifdef DEBUG
	message("Console exited\n");
#endif
	console_running = NONEXISTANT;
    } else if (pid == loginpid) {
#ifdef DEBUG
	message("X Login exited\n");
#endif
	if (status.w_retcode == CONSOLELOGIN)
	  login_running = STARTUP;
	else
	  login_running = NONEXISTANT;
    } else {
	message("Unexpected SIGCHLD from pid ");message(number(pid));
    }
}


void xready()
{
#ifdef DEBUG
    message("X Server ready\n");
#endif
    x_running = RUNNING;
}


/* When an alarm happens, just note it and return */

void alarm()
{
#ifdef DEBUG
    message("Alarm!\n");
#endif
    alarm_running = NONEXISTANT;
}


/* kill children and go away */

void die()
{
#ifdef DEBUG
    message("Killing children and exiting\n");
#endif
    if (login_running == RUNNING)
      kill(loginpid, SIGHUP);
    if (console_running == RUNNING)
      kill(consolepid, SIGHUP);
    if (x_running == RUNNING)
      kill(xpid, SIGKILL);
    exit(0);
}


/* Takes a command line, returns an argv-style  NULL terminated array 
 * of strings.  The array is in malloc'ed memory.
 */

char **parseargs(line, extra)
char *line;
char *extra;
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
    ret = (char **) malloc(sizeof(char *) * (i + 2));

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
    ret[i] = NULL;
    return(ret);
}


/* Used for logging errors and other messages so we don't need to link
 * against the stdio library.
 */

message(s)
char *s;
{
    int i = strlen(s);
    write(2, s, i);
}


/* Convert an int to a string, and return a pointer to this string in a
 * static buffer.  The string will be newline and null terminated.
 */

char *number(x)
int x;
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


char *getconf(file, name)
char *file;
char *name;
{
    static char buf[1024];
    static int inited = 0;
    char *p, *ret;
    int i;

    if (!inited) {
	int fd;

	fd = open(file, O_RDONLY, 0644);
	if (fd < 0) return(NULL);
	i = read(fd, buf, sizeof(buf));
	buf[i] = 0;
	close(fd);
	inited = 1;
    }

    for (p = &buf[0]; p && *p; p = index(p, '\n')) {
	if (*p == '\n') p++;
	if (p == NULL || *p == 0) return(NULL);
	if (*p == '#') continue;
	if (strncmp(p, name, strlen(name))) continue;
	p += strlen(name);
	if (*p && !isspace(*p)) continue;
	while (*p && isspace(*p)) p++;
	if (*p == 0) return(NULL);
	ret = index(p, '\n');
	if (ret)
	  i = ret - p;
	else
	  i = strlen(p);
	ret = malloc(i+1);
	bcopy(p, ret, i+1);
	return(ret);
    }
    return(NULL);
}
