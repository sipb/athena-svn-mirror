/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/snmp.h,v 1.1 1994-09-18 12:57:07 cfields Exp $
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
**			snmp.h
**
** Data structures and type definitions defined by the Simple Network
** Management Protocol. The major data structures and definitions
** in this file are defined in the SGMP and SNMP RFCs, so be careful
** what you change, if you have to change anything at all.
**
**
*******************************************************************************/
/* definitions of internal constants - size of arrays etc. */
#define SNMPSTRLEN	128		/* max length of a string */
#define SNMPMAXVARS	16		/* max number of variables per msg */
#define SNMPMAXVALS	16		/* max number of values per msg */
#define SNMPMAXPKT	484		/* max length of a packet */
#define SNMPMXSID	16		/* max length of a session id */
#define SNMPQRY		161		/* port used for SNMP queries */
#define SNMPTRAP	162		/* port used for SNMP  traps */
#define SNMPMXID	24		/* max number of objid subids */
#define SNMPVER		0		/* current protocol version */

#define SNMPTRUE	1		/* true */
#define SNMPFALSE	0		/* false */

#ifdef BSD
#ifndef _TYPES_
#ifndef H_TYPES_H
#include <sys/types.h>
#endif /*H_TYPES_H*/
#endif /*_TYPES_*/
#endif /* BSD */

#ifdef SVR3WIN
#ifndef TYPES_H_INCLUDED
#include "/usr/netinclude/sys/types.h"
#endif /* TYPES_H_INCLUDED */
#endif /* SVR3WIN */

#ifndef __IN_HEADER__
#ifndef IPPROTO_IP
#ifdef BSD
#include <netinet/in.h>
#endif /* BSD */
#endif /* IPPROTO_IP */
#endif /*__IN_HEADER__*/

#ifdef SVR3WIN
#ifndef IPPROTO_UDP
#include "/usr/netinclude/netinet/in.h"
#endif /* IPPROTO_UDP */
#endif /* SVR3WIN */

/* definitions of message and sub-message types. This should not be changed */
typedef struct {			/* string definition */
	u_short		len;		/* length of string */
	char		*str;		/* the string */
} strng;

typedef struct {			/* an object identifier */
	short		ncmp;		/* number of components */
	u_long 		cmp[SNMPMXID];	/* components */
} objident;

typedef struct {			/* a value */
	short			type;	/* value type */
	union { strng		str;	/* string */
		long		intgr;	/* integer (long) */
		objident	obj;	/* object identifier */
		struct in_addr	ipadd;	/* ip address */
		u_long		cntr;	/* counter */
		u_long		gauge;	/* gauge */
		u_long		time;	/* time */
		strng		opqe;	/* opaque */
	      } value;			/* actual value */
} objval;

typedef struct {			/* a variable */
	objident	name;		/* variable name */
	objval		val;		/* value */
} varbind;

typedef struct {			/* the variable list type */
	short 		len;		/* length of list */
	varbind		elem[SNMPMAXVARS]; /* elements */
} var_list_type;

typedef struct {			/* the 'PDU' type */
	long		reqid;		/* request id */
	long		errstat;	/* error status */
	long		errindex;	/* error index */
	var_list_type	varlist;	/* variable list */
} pdu_type;

typedef pdu_type	getreq;		/* get-request PDU */

typedef pdu_type	getnext;	/* get-next-request PDU */

typedef pdu_type	getrsp;		/* get-response PDU */

typedef pdu_type	setreq;		/* set-request PDU */

typedef struct {			/* trap PDU */
	objident	ent;		/* enterprise */
	struct in_addr	agnt;		/* agent address */
	long		gtrp;		/* generic trap */
	long		strp;		/* specific trap */
	u_long		tm;		/* time stamp */
	var_list_type	varlist;	/* variable list */
} trptype;

/* return codes */
#define NOERR		0		/* successful return */
#define TOOBIG		1		/* response too large */
#define NOSUCH		2		/* bad variable name */
#define	BADVAL		3		/* bad variable value */
#define RDONLY		4		/* set of a read-only variable */
#define GENERRS		5		/* generic system error */

/* trap types */
#define COLDSTART	0		/* Cold Start Trap */
#define WARMSTART	1		/* Warm Start Trap */
#define LINKDOWN	2		/* Link Failure Trap */
#define LINKUP		3		/* Link Up Trap */
#define AUTHFAIL	4		/* Authentication Failure Trap */
#define EGPNHBRLOST	5		/* EGP Neighbor Lost Trap */
#define VENDOR		6		/* Vendor Specific */

#ifdef MIT
/* MIT - vendor specific traps */
#define UNUSED          0               /* Workstation Logout   */
#define INUSE           1               /* Wokstation  Login    */
#endif MIT 

/* Message types */
#define REQ		1		/* get-request PDU type */
#define NXT		2		/* get-next-request PDU type */
#define RSP		3		/* get-response PDU type */
#define SET		4		/* set-request PDU type */
#define TRP		5		/* trap PDU type */

/*
 *  definitions of interface status variable.
 */
#define	OP_NORMAL	0x1		/* operating normally */
#define	INTF_DOWN	0x2		/* interface down */
#define TESTING_LINK	0x3		/* interface testing link */

/*
 *  definitions of route type variables.
 */
#define TO_OTHER		0x1		/* other route type */
#define TO_INVALID		0x2		/* invalid route */
#define TO_DIRECT		0x3		/* route to direct network */
#define TO_REMOTE		0x4		/* remote route */

/* primitive types */
#define INT		0		/* integer */
#define STR		1		/* octet string */
#define OBJ		2		/* object identifier */
#define EMPTY		3		/* empty */
#define IPADD		4		/* net address */
#define CNTR		5		/* counter */
#define GAUGE		6		/* gauge */
#define TIME		7		/* time */
#define OPAQUE		8		/* opaque */
