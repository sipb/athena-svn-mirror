/* Copyright 1989, 1999 by the Massachusetts Institute of Technology.
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

/* This file contains the non-rpc top-level rkinit library routines.
 * The routines in the rkinit library that should be called from clients
 * are exactly those defined in this file.
 *
 * The naming convetions used within the rkinit library are as follows:
 * Functions intended for general client use start with rkinit_
 * Functions intended for use only inside the library or server start with
 * rki_
 * Functions that do network communcation start with rki_rpc_
 * Static functions can be named in any fashion.
 */

static const char rcsid[] = "$Id: rk_lib.c,v 1.1 1999-10-05 17:09:54 danw Exp $";

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <netdb.h>
#include <unistd.h>

#include <krb.h>
#include <rkinit.h>
#include <rkinit_err.h>

char *rkinit_errmsg(char *string)
{
    static char errmsg[BUFSIZ];

    if (string) {
	BCLEAR(errmsg);
	strncpy(errmsg, string, sizeof(errmsg) - 1);
    }

    return(errmsg);
}

int rkinit(char *host, char *r_krealm, rkinit_info *info, int timeout)
{
    int status = RKINIT_SUCCESS;
    int version = 0;
    char phost[MAXHOSTNAMELEN];
    jmp_buf timeout_env;
    int (*old_alrm)();
    char origtktfilename[MAXPATHLEN]; /* original ticket file name */
    char tktfilename[MAXPATHLEN]; /* temporary client ticket file */

    extern int (*rki_setup_timer())();
    extern void rki_restore_timer();
    extern void rki_cleanup_rpc();

    BCLEAR(phost);
    BCLEAR(origtktfilename);
    BCLEAR(tktfilename);
    BCLEAR(timeout_env);

    initialize_rkin_error_table();

    if (status = rki_setup_rpc(host))
	return(status);	

    /* The alarm handler longjmps us to here. */
    if ((status = setjmp(timeout_env)) == 0) {

	strcpy(origtktfilename, tkt_string());
	sprintf(tktfilename, "/tmp/tkt_rkinit.%d", getpid());
	krb_set_tkt_string(tktfilename);

	if (timeout)
	    old_alrm = rki_setup_timer(timeout_env);
	
	if ((status = rki_choose_version(&version)) == RKINIT_SUCCESS)
	    status = rki_get_tickets(version, host, r_krealm, info);
    }
    
    if (timeout)
	rki_restore_timer(old_alrm);

    dest_tkt();
    krb_set_tkt_string(origtktfilename);

    rki_cleanup_rpc();

    return(status);
}
