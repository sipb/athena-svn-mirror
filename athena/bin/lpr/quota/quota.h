/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v 1.4 1990-07-11 10:17:51 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <strings.h>

#define UDPPROTOCOL	1
#define RETRY_COUNT	2
#define UDPTIMEOUT	3

extern char *progname;

extern short KA;    /* Kerberos Authentication */
extern short MA;    /* Mutual Authentication   */
extern int   DQ;    /* Default quota */
extern char *AF;    /* Acl File */
extern char *QF;    /* Master quota file */
extern char *GF;    /* Group quota file */
extern char *RF;    /* Report file for logger to grok thru */
extern char *QC;    /* Quota currency */
exterm int   QD;    /* Quota server "shutdown" for maintainence */

extern char pbuf[]; /* Dont ask :) -Ilham */
extern char *bp;

char *malloc();

#define TRUE 1
#define FALSE 0

#define ALLOWEDTOPRINT     0
#define NOALLOWEDTOPRINT   1
#define UNKNOWNUSER        2
#define UNKNOWNGROUP       3
#define USERNOTINGROUP     4
#define USERDELETED        5
#define GROUPDELETED       6

