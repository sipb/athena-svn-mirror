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

/* This file contains the network parts of the rkinit server. */

static const char rcsid[] = "$Id: rpc.c,v 1.1 1999-10-05 17:10:00 danw Exp $";

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#include <rkinit.h>
#include <rkinit_err.h>

#include "rkinitd.h"

#define RKINITD_TIMEOUT 60

static int in;			/* sockets */
static int out;

static char errbuf[BUFSIZ];

void error();

static int timeout(void)
{
    syslog(LOG_WARNING, "rkinitd timed out.\n");
    exit(1);

    return(0);			/* To make the compiler happy... */
}

/*
 * This function does all the network setup for rkinitd.
 * It returns true if we were started from inetd, or false if 
 * we were started from the commandline.
 * It causes the program to exit if there is an error. 
 */
int setup_rpc(int notimeout)		
{
    struct itimerval timer;	/* Time structure for timeout */
    struct sigaction act, oact;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    /* For now, support only inetd. */
    in = 0;
    out = 1;

    if (! notimeout) {
	SBCLEAR(timer);

	/* Set up an itimer structure to send an alarm signal after timeout
	   seconds. */
	timer.it_interval.tv_sec = RKINITD_TIMEOUT;
	timer.it_interval.tv_usec = 0;
	timer.it_value = timer.it_interval;
	
	/* Start the timer. */
	if (setitimer (ITIMER_REAL, &timer, (struct itimerval *)0) < 0) {
	    sprintf(errbuf, "setitimer: %s", strerror(errno));
	    rkinit_errmsg(errbuf);
	    error();
	    exit(1);
	}

	act.sa_handler= (void (*)()) timeout;
	(void) sigaction (SIGALRM, &act, NULL);
    }

    return(TRUE);
}

void rpc_exchange_version_info(int *c_lversion, int *c_hversion, 
			       int s_lversion, int s_hversion)
{
    u_char version_info[VERSION_INFO_SIZE];
    u_long length = sizeof(version_info);
    
    if (rki_get_packet(in, MT_CVERSION, &length, (char *)version_info) !=
	RKINIT_SUCCESS) {
	error();
	exit(1);
    }

    *c_lversion = version_info[0];
    *c_hversion = version_info[1];

    version_info[0] = s_lversion;
    version_info[1] = s_hversion;

    if (rki_send_packet(out, MT_SVERSION, length, (char *)version_info) != 
	RKINIT_SUCCESS) {
	error();
	exit(1);
    }
}
    
void rpc_get_rkinit_info(rkinit_info *info)
{
    u_long length = sizeof(rkinit_info);
    
    if (rki_get_packet(in, MT_RKINIT_INFO, &length, (char *)info)) {
	error();
	exit(1);
    }
    
    info->lifetime = ntohl(info->lifetime);
}

void rpc_send_error(char *errmsg)
{
    if (rki_send_packet(out, MT_STATUS, strlen(errmsg), errmsg)) {
	error();
	exit(1);
    }
}

void rpc_send_success(void)
{
    if (rki_send_packet(out, MT_STATUS, 0, "")) {
	error();
	exit(1);
    }
}

void rpc_exchange_tkt(KTEXT cip, MSG_DAT *scip)
{
    u_long length = MAX_KTXT_LEN;

    if (rki_send_packet(out, MT_SKDC, cip->length, (char *)cip->dat)) {
	error();
	exit(1);
    }
    
    if (rki_get_packet(in, MT_CKDC, &length, (char *)scip->app_data)) {
	error();
	exit(1);
    }
    scip->app_length = length;
}

void rpc_getauth(KTEXT auth, struct sockaddr_in *caddr, 
		 struct sockaddr_in *saddr)
{
    int addrlen = sizeof(struct sockaddr_in);

    if (rki_rpc_get_ktext(in, auth, MT_AUTH)) {
	error();
	exit(1);
    }

    if (getpeername(in, caddr, &addrlen) < 0) {
	sprintf(errbuf, "getpeername: %s", strerror(errno));
	rkinit_errmsg(errbuf);
	error();
	exit(1);
    }

    if (getsockname(out, saddr, &addrlen) < 0) {
	sprintf(errbuf, "getsockname: %s", strerror(errno));
	rkinit_errmsg(errbuf);
	error();
	exit(1);
    }
}
