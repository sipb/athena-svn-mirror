/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/timeout.c,v 1.1 1990-11-14 13:52:42 mar Exp $ */

#include <mit-copyright.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#define FALSE 0
#define TRUE (!FALSE)

#ifndef __STDC__
#define volatile
#endif

int app_pid;
volatile int app_running;


main(argc, argv)
int argc;
char **argv;
{
    int maxidle;
    char *name;
    struct stat stbuf;
    struct timeval now;
    void child(), wakeup();

    name = *argv++;
    if (argc < 3) {
	fprintf(stderr, "usage: %s max-idle-time application command line...\n",
		name);
	exit(1);
    }
    maxidle = atoi(*argv++);
    argc -= 2;

    app_running = TRUE;
    signal(SIGCHLD, child);
    signal(SIGALRM, wakeup);

    /* launch application */
    switch (app_pid = fork()) {
    case 0:
	execv(argv[0], argv);
	fprintf(stderr, "%s: failed to exec application %s\n", name, argv[0]);
	exit(1);
    case -1:
	fprintf(stderr, "%s: failed to fork to create application\n", name);
	exit(1);
    default:
	break;
    }

    /* wait for application to die or idle-time to be reached */
    while (app_running) {
	alarm(10);		/* sleep 10 seconds */
	sigpause(0);
	fstat(1, &stbuf);
	gettimeofday(&now, NULL);
	if (stbuf.st_atime + maxidle < now.tv_sec) {
	    fprintf(stderr, "\nMAX IDLE TIME REACHED.\n");
	    kill(app_pid, SIGINT);
	    exit(0);
	}
    }
    exit(0);
}

void child()
{
    union wait status;
    int pid;

    pid = wait3(&status, WNOHANG, 0);
    if (pid != app_pid)
      return;
    if (WIFEXITED(status))
      app_running = FALSE;
}

void wakeup()
{
}
