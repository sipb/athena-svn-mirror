/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/access/access_off.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/access/access_off.c,v 1.3 1994-06-23 12:30:40 vrt Exp $
 */

#ifndef lint
static char *rcsid_access_off_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/access/access_off.c,v 1.3 1994-06-23 12:30:40 vrt Exp $";
#endif lint

#include <stdio.h>
#include <signal.h>
#include <errno.h>



main(argc,argv)
int argc;
char **argv;
{
    FILE *pidfile;
    int pid;
    int status;
    char *pindex;

#if defined(SOLARIS) || defined(_IBMR2)
    if ((pidfile = fopen("/etc/athena/inetd.pid", "r")) == NULL) {
#else
    if ((pidfile = fopen("/etc/inetd.pid", "r")) == NULL) {
#endif
	printf("cannot read process id file--daemon probably not running\n");
	exit(1);
    }
    if (fscanf(pidfile, "%d", &pid) != 1) {
	printf("error reading process id file\n");
	exit(1);
    }
    fclose(pidfile);
    if ((pindex = strrchr(argv[0], '/')) == (char *) NULL)
      pindex = argv[0];
    else pindex++;

    if (status = kill(pid, !strcmp(pindex, "access_off") ?
		      SIGUSR2 : SIGUSR1)) {
			  if (status != ESRCH)
			    perror("error killing daemon");
			  exit(1);
		      }
    exit(0);
}
