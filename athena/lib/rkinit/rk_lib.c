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

static const char rcsid[] = "$Id: rk_lib.c,v 1.2 1999-12-09 22:23:54 danw Exp $";

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>

#include <krb.h>
#include <rkinit.h>
#include <rkinit_err.h>

static void rki_setup_timer(sigjmp_buf env, struct sigaction *osa);
static void rki_restore_timer(struct sigaction *osa);

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
    int status;
    int version = 0;
    char phost[MAXHOSTNAMELEN];
    jmp_buf timeout_env;
    struct sigaction osa;
    char origtktfilename[MAXPATHLEN]; /* original ticket file name */
    char tktfilename[MAXPATHLEN]; /* temporary client ticket file */

    BCLEAR(phost);
    BCLEAR(origtktfilename);
    BCLEAR(tktfilename);
    BCLEAR(timeout_env);

    initialize_rkin_error_table();

    status = rki_setup_rpc(host);
    if (status)
	return(status);

    /* The alarm handler longjmps us to here. */
    status = setjmp(timeout_env);
    if (status == 0) {
	strcpy(origtktfilename, tkt_string());
	sprintf(tktfilename, "/tmp/tkt_rkinit.%lu", (unsigned long)getpid());
	krb_set_tkt_string(tktfilename);

	if (timeout)
	    rki_setup_timer(timeout_env, &osa);

	status = rki_choose_version(&version);
	if (status == RKINIT_SUCCESS)
	    status = rki_get_tickets(version, host, r_krealm, info);
    }

    if (timeout)
	rki_restore_timer(&osa);

    dest_tkt();
    krb_set_tkt_string(origtktfilename);

    rki_cleanup_rpc();

    return(status);
}


char *rki_mt_to_string(int mt)
{
    char *string = 0;

    switch(mt) {
      case MT_STATUS:
	string = "Status message";
	break;
      case MT_CVERSION:
	string = "Client version";
	break;
      case MT_SVERSION:
	string = "Server version";
	break;
      case MT_RKINIT_INFO:
	string = "Rkinit information";
	break;
      case MT_SKDC:
	string = "Server kdc packet";
	break;
      case MT_CKDC:
	string = "Client kdc packet";
	break;
      case MT_AUTH:
	string = "Authenticator";
	break;
      case MT_DROP:
	string = "Drop server";
	break;
      default:
	string = "Unknown message type";
	break;
    }

    return(string);
}

static char errbuf[BUFSIZ];

int rki_choose_version(int *version)
{
    int s_lversion;		/* lowest version number server supports */
    int s_hversion;		/* highest version number server supports */
    int status;

    status = rki_rpc_exchange_version_info(RKINIT_LVERSION, RKINIT_HVERSION,
					   &s_lversion, &s_hversion);
    if (status != RKINIT_SUCCESS)
	return(status);

    *version = min(RKINIT_HVERSION, s_hversion);
    if (*version < max(RKINIT_LVERSION, s_lversion)) {
	sprintf(errbuf,
		"Can't run version %d client against version %d server.",
		RKINIT_HVERSION, s_hversion);
	rkinit_errmsg(errbuf);
	status = RKINIT_VERSION;
    }

    return(status);
}

int rki_send_rkinit_info(int version, rkinit_info *info)
{
    int status;

    status = rki_rpc_send_rkinit_info(info);
    if (status != RKINIT_SUCCESS)
	return(status);

    return(rki_rpc_get_status());
}

#define RKINIT_TIMEOUTVAL 60

static sigjmp_buf timeout_env;

static void rki_timeout(void)
{
    sprintf(errbuf, "%d seconds exceeded.", RKINIT_TIMEOUTVAL);
    rkinit_errmsg(errbuf);
    siglongjmp(timeout_env, RKINIT_TIMEOUT);
}

static void set_timer(int secs)
{
    struct itimerval timer;	/* Time structure for timeout */

    /* Set up an itimer structure to send an alarm signal after TIMEOUT
       seconds. */
    timer.it_interval.tv_sec = secs;
    timer.it_interval.tv_usec = 0;
    timer.it_value = timer.it_interval;

    (void) setitimer (ITIMER_REAL, &timer, (struct itimerval *)0);
}

static void rki_setup_timer(sigjmp_buf env, struct sigaction *osa)
{
    struct sigaction sa;

    memmove(timeout_env, env, sizeof(sigjmp_buf));
    set_timer(RKINIT_TIMEOUTVAL);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = rki_timeout;
    sigaction(SIGALRM, &sa, osa);
}

static void rki_restore_timer(struct sigaction *osa)
{
    set_timer(0);
    sigaction(SIGALRM, osa, NULL);
}
