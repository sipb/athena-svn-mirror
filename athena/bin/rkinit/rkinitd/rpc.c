/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rpc.c,v 1.1 1989-11-12 19:35:18 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rpc.c,v $
 * $Author: qjb $
 *
 * This file contains the network parts of the rkinit server.
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/rpc.c,v 1.1 1989-11-12 19:35:18 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#include <rkinit.h>
#include <rkinit_err.h>
#include <rkinit_private.h>

#define RKINITD_TIMEOUT 60

extern int errno;
extern char *sys_errlist[];

static int in;			/* sockets */
static int out;

static char errbuf[BUFSIZ];

void error();

static int timeout()
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
int setup_rpc(notimeout)
  int notimeout;		/* True if we should not timeout */
{
    struct itimerval timer;	/* Time structure for timeout */

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
	    sprintf(errbuf, "setitimer: %s", sys_errlist[errno]);
	    rkinit_errmsg(errbuf);
	    error();
	    exit(1);
	}

	signal(SIGALRM, timeout);
    }

    return(TRUE);
}

void rpc_exchange_version_info(c_lversion, c_hversion, 
			       s_lversion, s_hversion)
  int *c_lversion;
  int *c_hversion;
  int s_lversion;
  int s_hversion;
{
    u_char version_info[VERSION_INFO_SIZE];
    int length = sizeof(version_info);
    
    if (rki_get_packet(in, MT_CVERSION, &length, version_info) !=
	RKINIT_SUCCESS) {
	error();
	exit(1);
    }

    *c_lversion = version_info[0];
    *c_hversion = version_info[1];

    version_info[0] = s_lversion;
    version_info[1] = s_hversion;

    if (rki_send_packet(out, MT_SVERSION, length, version_info) != 
	RKINIT_SUCCESS) {
	error();
	exit(1);
    }
}
    
void rpc_get_rkinit_info(info)
  rkinit_info *info;
{
    u_long length = sizeof(rkinit_info);
    
    if (rki_get_packet(in, MT_RKINIT_INFO, &length, (char *)info)) {
	error();
	exit(1);
    }
    
    info->lifetime = ntohl(info->lifetime);
}

void rpc_send_error(errmsg)
  char *errmsg;
{
    if (rki_send_packet(out, MT_STATUS, strlen(errmsg), errmsg)) {
	error();
	exit(1);
    }
}

void rpc_send_success()
{
    if (rki_send_packet(out, MT_STATUS, 0, "")) {
	error();
	exit(1);
    }
}

void rpc_exchange_tkt(cip, scip)
  KTEXT cip;
  MSG_DAT *scip;
{
    int length = MAX_KTXT_LEN;

    if (rki_send_packet(out, MT_SKDC, cip->length, cip->dat)) {
	error();
	exit(1);
    }
    
    if (rki_get_packet(in, MT_CKDC, &length, scip->app_data)) {
	error();
	exit(1);
    }
    scip->app_length = length;
}

void rpc_getauth(auth, caddr, saddr)
  KTEXT auth;
  struct sockaddr_in *caddr;
  struct sockaddr_in *saddr;
{
    int addrlen = sizeof(struct sockaddr_in);

    if (rki_rpc_get_ktext(in, auth, MT_AUTH)) {
	error();
	exit(1);
    }

    if (getpeername(in, caddr, &addrlen) < 0) {
	sprintf(errbuf, "getpeername: %s", sys_errlist[errno]);
	rkinit_errmsg(errbuf);
	error();
	exit(1);
    }

    if (getsockname(out, saddr, &addrlen) < 0) {
	sprintf(errbuf, "getsockname: %s", sys_errlist[errno]);
	rkinit_errmsg(errbuf);
	error();
	exit(1);
    }
}
