/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v 1.2 1990-04-25 11:48:27 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"

#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <strings.h>

#define DEFACLFILE	"/usr/spool/quota/acl"
#define DEFSACLFILE	"/usr/spool/quota/sacl"
#define DBM_DEF_FILE	"/usr/spool/quota/quota_db"
#define DEFREPORTFILE	"/usr/spool/quota/report"
#define DEFCAPFILE      "/usr/spool/quota/quotacap"
#define LOGTRANSFILE		"/usr/spool/quota/logtrans"
#define DEFQUOTA        500
#define DEFCURRENCY     "cents"
#define DEFSERVICE	"athena"

#define QUOTASERVENT	3701	
#define QUOTASERVENTNAME "lpallow"

#define UDPPROTOCOL	1
#define RETRY_COUNT	2
#define UDPTIMEOUT	3

extern char *progname;

extern short KA;    /* Kerberos Authentication */
extern short MA;    /* Mutual Authentication   */
extern int   DQ;    /* Default quota */
extern char *AF;    /* Acl File */
extern char *QF;    /* Master quota file */
extern char *RF;    /* Report file for logger to grok thru */
extern char *QC;    /* Quota currency */

extern char pbuf[]; /* Dont ask :) -Ilham */
extern char *bp;

char *malloc();

#define KLPQUOTA_SERVICE "rcmd"

#define TRUE 1
#define FALSE 0

#define ALLOWEDTOPRINT 0
#define NOALLOWEDTOPRINT 1
#define UNKNOWNUSER 2

void PROTECT(), UNPROTECT(), CHECK_PROTECT();
