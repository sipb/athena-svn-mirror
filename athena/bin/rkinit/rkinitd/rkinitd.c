/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rkinitd.c,v 1.1 1989-11-12 19:34:58 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rkinitd.c,v $
 * $Author: qjb $
 *
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rkinitd.c,v 1.1 1989-11-12 19:34:58 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <signal.h>
#include <sys/time.h>
#include <pwd.h>
#include <krb.h>
#include <des.h>
#include <syslog.h>

#include <rkinit.h>
#include <rkinit_err.h>
#include <rkinit_private.h>

extern int errno;
extern char *sys_errlist[];

static int inetd = TRUE;	/* True if we were started by inetd */

void usage() 
{
    syslog(LOG_ERR, "rkinitd usage: rkinitd [-notimeout]\n");
    exit(1);
}

void error()
{
    char errbuf[BUFSIZ];
    
    strcpy(errbuf, rkinit_errmsg(0));
    if (inetd)
	syslog(LOG_ERR, "rkinitd: %s", errbuf);
    else
	fprintf(stderr, "rkinitd: %s\n", errbuf);
}

main(argc, argv)
  int argc;
  char *argv[];
{
    int version;		/* Version of the transaction */

    int notimeout = FALSE;	/* Should we not timeout? */

    static char    *envinit[1];	/* Empty environment */
    extern char    **environ;	/* This process's environment */

    int status = 0;		/* General error code */

    /*
     * Clear the environment so that this process does not inherit 
     * kerberos ticket variable information from the person who started
     * the process (if a person started it...).
     */
    environ = envinit;

    /* Initialize com_err error table */
    initialize_rkin_error_table();

#ifdef DEBUG
    /* This only works if the library was compiled with DEBUG defined */
    rki_i_am_server();
#endif DEBUG

    /* 
     * Make sure that we are running as root or can arrange to be 
     * running as root.  We need both to be able to read /etc/srvtab
     * and to be able to change uid to create tickets.
     */
    
    (void) setuid(0);
    if (getuid() != 0) {
	syslog(LOG_ERR, "rkinitd: not running as root.\n");
	exit(1);
    }

    /* Determine whether to time out */
    if (argc == 2) {
	if (strcmp(argv[1], "-notimeout"))
	    usage();
	else
	    notimeout = TRUE;
    }
    else if (argc != 1)
	usage();

    inetd = setup_rpc(notimeout);

    if ((status = choose_version(&version) != RKINIT_SUCCESS)) {
	error();
	exit(1);
    }

    if ((status = get_tickets(version) != RKINIT_SUCCESS)) {
	error();
	exit(1);
    }

    exit(0);
}

    
