/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/console/config_console.c,v 1.1 1990-10-22 19:40:40 mar Exp $
 *
 * Copyright (c) 1990 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * This program allows a user to manipulate the console window.
 * This is done by sending signals to it.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <strings.h>

char *pidfile = "/etc/console.pid";


main(argc, argv)
int argc;
char **argv;
{
    char *name, buf[256];
    FILE *f;
    int pid;
    int sig = SIGHUP;

    name = rindex(argv[0], '/');
    if (name == NULL)
      name = argv[0];
    else
      name++;

    if (!strcmp(name, "show_console"))
      sig = SIGUSR1;
    else if (!strcmp(name, "hide_console"))
      sig = SIGUSR2;	

    if (argc == 2 && !strcmp(argv[1], "-hide"))
      sig = SIGUSR2;
    else if (argc == 2 && !strcmp(argv[1], "-show"))
      sig = SIGUSR1;
    else if (argc == 2 && !strcmp(argv[1], "-config"))
      sig = SIGHUP;
    else if (argc != 1) {
	fprintf(stderr, "Usage: %s [-config | -show | -hide]\n", name);
	exit(2);
    }

    f = fopen(pidfile, "r");
    if (f == NULL) {
	sprintf(buf, "%s: unable to find console", name);
	perror(buf);
	fprintf(stderr, "\tlooking for file %s\n", pidfile);
	exit(1);
    }
    fgets(buf, sizeof(buf), f);
    fclose(f);

    pid = atoi(buf);
    if (pid == 0 || pid == 1) {
	fprintf(stderr, "%s: unable to find proper console\n", name);
	fprintf(stderr, "\tconsole cannot have PID %d\n", pid);
	exit(1);
    }

    if (kill(pid, sig) < 0) {
	sprintf(buf, "%s: unable to signal console", name);
	perror(buf);
	exit(1);
    }
    exit(0);
}
