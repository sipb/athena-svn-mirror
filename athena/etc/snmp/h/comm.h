/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/comm.h,v 1.1 1994-09-18 12:57:05 cfields Exp $
 *
 * $Log: not supported by cvs2svn $
 */

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
IS STRICTLY PROHIBITED.  (C) 1988 NYSERNET, INC.  (SUBJECT TO 
LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
*/
/*******************************************************************************
**
**			comm.h
**
** Header file containing other includes, structures and definitions
** needed for the communications functions.
**
**
*******************************************************************************/
#ifdef BSD
#include <stdio.h>			/* for various definitions */
#include <sys/types.h>			/* definitions of sys types */
#include <sys/socket.h>			/* socket definitions */
#include <netinet/in.h>			/* internet definitions */
#include <netdb.h>			/* to access net database */
#include <errno.h>			/* for extended error codes */
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>			/* for various definitions */
#include "/usr/netinclude/sys/types.h"	/* definitions of sys types */
#include "/usr/netinclude/sys/socket.h"	/* socket definitions */
#include "/usr/netinclude/netinet/in.h"	/* internet definitions */
#include "/usr/netinclude/netdb.h"	/* to access net database */
#include "/usr/netinclude/errno.h"	/* for extended error codes */
#endif /* SVR3WIN */

#define HLEN			32	/* max length of local host name */

/* global variables */
extern int s;				/* socket file descriptor */
extern struct sockaddr_in local;	/* local communications information */
extern int errno;			/* global errno */

/* function definitions for communications table functions */
extern struct sockaddr_in *comminfo();	/* get communications information */
extern short commadd();			/* add a table entry */
extern short commrm();			/* remove a table entry */
