/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/asn.h,v 1.1 1994-09-18 12:57:04 cfields Exp $
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
**			asn.h
**
** Definitions to be used by the ASN.1 parser and packet builder
**
**
*******************************************************************************/
/* Message types */
#define GETREQID	'\240'		/* get-request */
#define GETNXTID	'\241'		/* get-next-request */
#define GETRSPID	'\242'		/* get-response */
#define SETREQID	'\243'		/* set-request */
#define TRAPID		'\244'		/* trap */

/* real ASN.1 'primitives' */
#define INTID		0x02		/* integer */
#define STRID		0x04		/* string */
#define NULLID		0x05		/* null */
#define OBJID		0x06		/* object identifier */
#define SEQID		0x30		/* sequence */

/* SMI defined 'primitives' */
#define IPID		0x40		/* IpAddress */
#define COUNTERID	0x41		/* Counter */
#define GAUGEID		0x42		/* Gauge */
#define TIMEID		0x43		/* TimeTicks */
#define OPAQUEID	0x44		/* Opaque */

/* misc. other definitions */
#define ISLONG		0x80		/* to mask out all but 8th bit */
#define LENMASK		0x7f		/* to mask out 8th bit */
#define INDFTYP		(char)('\200')	/* the indefinite type */

/* function definitions */
extern short snmpparse();		/* parser functions */
extern char * varlst();
extern char * var();
extern char * vval();
extern char * pdu();
extern char * trap();
extern char * prseint();
extern char * prsestr();
extern char * prsenull();
extern char * prseobj();
extern char * prseipadd();
extern char * prsecntr();
extern char * prsegauge();
extern char * prsetime();
extern char * prsestr();
extern char * prseopaque();
extern char * asnlen();
extern char * otherlen();
extern char * getsubident();
extern short snmpbld();
extern char * bldpdu();
extern char * bldtrp();
extern char * bldvarlst();
extern char * bldvar();
extern char * bldval();
extern char * bldint();
extern char * bldstr();
extern char * bldobj();
extern char * bldnull();
extern char * bldipadd();
extern char * bldcntr();
extern char * bldgauge();
extern char * bldtime();
extern char * bldopqe();
extern char * bldlen();
