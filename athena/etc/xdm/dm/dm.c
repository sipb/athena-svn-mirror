/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.1 1990-10-18 18:58:14 mar Exp $
 *
 * Copyright (c) 1990 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This is the top-level of the display manager and console control
 * for Athena's xlogin.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <utmp.h>
#include <ctype.h>


#ifndef lint
static char *rcsid_main = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/dm.c,v 1.1 1990-10-18 18:58:14 mar Exp $";
#endif

/* Process states */
#define NONEXISTANT	0
#define RUNNING		1
#define STARTUP		2
#define CONSOLELOGIN	7

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

#define X_START_WAIT	30	/* wait up to 30 seconds for X to be ready */


main(argc, argv)
int argc;
char **argv;
{
    void die(), child(), alarm(), xready();
    char *tty, *prog;
    char **xargv, **consoleargv, **loginargv, **parseargs();
    char line[16];
    int pgrp;
    FILE *pidfile;

    if (argc != 5) {
	fprintf(stderr, "usage: %s tty Xserver Console-prog Login-prog\n",
		argv[0]);
	exit(1);
    }
    tty = argv[1];
    xargv = parseargs(argv[2], NULL);
    consoleargv = parseargs(argv[3], NULL);
    loginargv = parseargs(argv[4], tty);

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
    sprintf(line, "/dev/%s", tty);
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
    fprintf(stderr, "Starting X\n");
#endif
    xpid = fork();
    switch (xpid) {
    case 0:
	/* Since stderr is unbuffered, close-on-exec will do the right
	   thing with the stdio library */
        if(fcntl(fileno(stderr), F_SETFD, 1) == -1)
	    fclose(stderr);
	sigsetmask(0);
	/* ignoring SIGUSR1 will cause the server to send us a SIGUSR1
	 * when it is ready to accept connections
	 */
	signal(SIGUSR1, SIG_IGN);
	prog = *xargv;
	*xargv = "-";
	execv(prog, xargv);
	perror("executing X server");
	exit(1);
    case -1:
	perror("Unable to start X server");
	exit(1);
    default:
	x_running = STARTUP;
	signal(SIGUSR1, xready);
	if ((pidfile = fopen(xpidf, "w")) != NULL) {
	    fprintf(pidfile, "%d\n", xpid);
	    fclose(pidfile);
	}
	alarm(X_START_WAIT);
	alarm_running = RUNNING;
#ifdef DEBUG
	fprintf(stderr, "waiting for X\n");
#endif
	sigpause(0);
	if (x_running != RUNNING) {
	    if (alarm_running == NONEXISTANT)
	      fprintf(stderr, "X failed to become ready\n");
	    else
	      fprintf(stderr, "Unable to start X\n");
	    exit(1);
	}
    }

    start_console(consoleargv);

    /* Fire up the X login */
#ifdef DEBUG
    fprintf(stderr, "Starting X Login\n");
#endif
    loginpid = fork();
    switch (loginpid) {
    case 0:
	/* Since stderr is unbuffered, close-on-exec will do the right
	   thing with the stdio library */
        if(fcntl(fileno(stderr), F_SETFD, 1) == -1)
	    fclose(stderr);
	sigsetmask(0);
	execv(loginargv[0], loginargv);
	perror("executing login");
	exit(1);
    case -1:
	perror("Unable to start login");
	exit(1);
    default:
	login_running = RUNNING;
    }

    while (1) {
#ifdef DEBUG
	fprintf(stderr, "waiting...\n");
#endif
	/* Wait for something to hapen */
	sigpause(0);

	if (login_running == STARTUP) {
#ifdef DEBUG
	    fprintf(stderr, "starting console login\n");
#endif
	    kill(xpid, SIGHUP);
#ifndef BROKEN_CONSOLE_DRIVER
	    setpgrp(0, pgrp=0);		/* We have to reset the tty pgrp */
	    ioctl(0, TIOCSPGRP, &pgrp);
#endif
	    execl(login_prog, login_prog, 0);
	    fprintf(stderr, "Unable to start console login\n");
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
#ifdef DEBUG
    fprintf(stderr, "Starting Console: ");
    {
	int i;
	for (i = 0; argv[i]; i++)
	  fprintf(stderr, "\"%s\" ", argv[i]);
	fprintf(stderr, "\n");
    }
#endif
    consolepid = fork();
    switch (consolepid) {
    case 0:
	/* Since stderr is unbuffered, close-on-exec will do the right
	   thing with the stdio library */
        if(fcntl(fileno(stderr), F_SETFD, 1) == -1)
	    fclose(stderr);
	sigsetmask(0);
	execv(argv[0], argv);
	perror("executing console");
	exit(1);
    case -1:
	perror("Unable to start console");
	exit(1);
    default:
	console_running = RUNNING;
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
      kill(xpid, SIGHUP);

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

    pid = wait3(&status, WNOHANG, 0);

#ifdef DEBUG
    fprintf(stderr, "Child %d exited\n", pid);
#endif
    if (pid == xpid) {
#ifdef DEBUG
	fprintf(stderr, "X Server exited\n");
#endif
	x_running = NONEXISTANT;
    } else if (pid == consolepid) {
#ifdef DEBUG
	fprintf(stderr, "Console exited\n");
#endif
	console_running = NONEXISTANT;
    } else if (pid == loginpid) {
#ifdef DEBUG
	fprintf(stderr, "X Login exited\n");
#endif
	if (status.w_retcode == CONSOLELOGIN)
	  login_running = STARTUP;
	else
	  login_running = NONEXISTANT;
    } else {
	fprintf(stderr, "Unexpected SIGCHLD from pid %d\n", pid);
    }
}


void xready()
{
#ifdef DEBUG
    fprintf(stderr, "X Server ready\n");
#endif
    x_running = RUNNING;
}


/* When an alarm happens, just note it and return */

void alarm()
{
#ifdef DEBUG
    fprintf(stderr, "Alarm!\n");
#endif
    alarm_running = NONEXISTANT;
}


/* kill children and go away */

void die()
{
#ifdef DEBUG
    fprintf(stderr, "Killing children and exiting\n");
#endif
    if (login_running == RUNNING)
      kill(loginpid, SIGHUP);
    if (console_running == RUNNING)
      kill(consolepid, SIGHUP);
    if (x_running == RUNNING)
      kill(xpid, SIGHUP);
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
