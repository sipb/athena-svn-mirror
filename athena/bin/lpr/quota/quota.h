/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v $
 *	$Author: ghudson $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota.h,v 1.10 1996-09-20 02:07:23 ghudson Exp $
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
#include <string.h>

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
extern int   QD;    /* Quota server "shutdown" for maintainence */
extern int   AN;    /* Account numerator factor */
extern int   AD;    /* Account denomenator factor */

extern char aclname[];       /* Acl filename */
extern char saclname[];      /* Service Acl filename */
extern char qfilename[];     /* Master quota database */
extern char gfilename[];     /* Group quota database */
extern char rfilename[];     /* Report file */
extern char qcapfilename[];  /* Required by quotacap routines */
extern char qcurrency[];             /* The quota currency */
extern char quota_name[];           /* Quota server name (for quotacap) */
extern int  qdefault;                  /* Default quota */


extern char pbuf[]; /* Dont ask :) -Ilham */
extern char *bp;

#ifndef ZEPHYR
char *malloc();
#endif
extern char 	*error_text();

#define TRUE 1
#define FALSE 0

#define ALLOWEDTOPRINT     0
#define NOALLOWEDTOPRINT   1
#define UNKNOWNUSER        2
#define UNKNOWNGROUP       3
#define USERNOTINGROUP     4
#define USERDELETED        5
#define GROUPDELETED       6

#ifdef __STDC__
extern void	make_kname(char *, char *, char *, char *);
extern int	is_suser(char *);
extern int	is_sacct(char *, char *);
extern int	parse_username(unsigned char *, char *, char *, char *);
extern char	*set_service(char *);
#else
extern void	make_kname();
extern int	is_suser();
extern int	is_sacct();
extern int	parse_username();
extern char	*set_service();
#endif
