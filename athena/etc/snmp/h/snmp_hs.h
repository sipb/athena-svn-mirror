/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/snmp_hs.h,v 1.1 1994-09-18 12:57:11 cfields Exp $
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
**			snmp_hs.h
**
** All encompassing include file for applications. An application should
** only have to include this file to get everything specified by
** the Simple Gateway Monitoring Protocol, and Simple Network
** Management Protocol.
**
**
*******************************************************************************/
/* other snmp include files */
#include "snmp.h"			/* snmp structures and definitions */
#include "snmperrs.h"			/* return codes */

/* general constants */
#define REQ_ANY		0		/* any request id */
