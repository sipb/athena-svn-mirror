/* Copyright 1989,1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This is the main source file for rkinit. */

static const char rcsid[] = "$Id: rkinitd.c,v 1.2 1999-12-09 22:24:00 danw Exp $";

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pwd.h>
#include <krb.h>
#include <des.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

#include <rkinit.h>
#include <rkinit_err.h>

#include "rkinitd.h"

static int inetd = TRUE;	/* True if we were started by inetd */

static void usage(void)
{
    syslog(LOG_ERR, "rkinitd usage: rkinitd [-notimeout]\n");
    exit(1);
}

void error(void)
{
    char errbuf[BUFSIZ];

    strcpy(errbuf, rkinit_errmsg(0));
    if (strlen(errbuf)) {
	if (inetd)
	    syslog(LOG_ERR, "rkinitd: %s", errbuf);
	else
	    fprintf(stderr, "rkinitd: %s\n", errbuf);
    }
}

int main(int argc, char *argv[])
{
    int version;		/* Version of the transaction */

    int notimeout = FALSE;	/* Should we not timeout? */

    static char    *envinit[1];	/* Empty environment */
    extern char    **environ;	/* This process's environment */

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
#endif /* DEBUG */

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

    if (choose_version(&version) != RKINIT_SUCCESS) {
	error();
	exit(1);
    }

    if (get_tickets(version) != RKINIT_SUCCESS) {
	error();
	exit(1);
    }

    return 0;
}
