/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/snmperrs.h,v 1.1 1994-09-18 12:57:14 cfields Exp $
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
**			snmperrs.h
**
** Return codes (mostly errors) for NYSERNet snmp implementation 
**
**
*******************************************************************************/
/* 0 through -9 are general errors */
#define SNMP_OK			0	/* the only non-error code */
#define EMALLOC			-1	/* malloc error */
#define GENERR			-2	/* generic error */

/* -10 through -29 are errors from the parser and packet builder */
#define NEGINBUF		-10	/* input buffer of negative length */
#define NEGOUTBUF		-11	/* output buffer of negative length */
#define NOINBUF			-12	/* no input buffer */
#define NOOUTBUF		-13	/* no outbuf buffer */
#define TYP_UNKNOWN		-14	/* unknown mesg type */
#define LENERR			-15	/* unexpected end of input message */
#define UNSPLEN			-16	/* indefinite length type */
#define OUTERR			-17	/* no more output buffer */
#define TYP_MISMATCH		-18	/* wrong type */
#define TOOMANYVARS		-19	/* too many variables in message */
#define TOOLONG			-20	/* asn structure too long */
#define TYPERR			-21	/* asn structure wrong for type */
#define PKTLENERR		-22	/* max size of packet exceeded */
#define TYPELONG		-23	/* value is too long for type */
#define NO_SID			-24	/* no session id */
/* -30 to -39 are communications table function errors */
#define TBLFULL			-30	/* table full */
#define NOSUCHID		-31	/* no such request id */
/* -40 to -59 are communications function errors */
#define UNINIT_SOCK		-40	/* uninitialized socket */
#define NODEST			-41	/* no destination */
#define NOSID			-42	/* no session id provided */
#define BADSIDLEN		-43	/* no session id length */
#define NOSVC			-44	/* service requested unavailable */
#define REQID_UNKNOWN		-45	/* unknown request id */
#define SND_TMO			-46	/* send timeout */
#define NORECVBUF		-47	/* no receive buffer */
#define NOREQID			-48	/* no request id provided */
#define NOSOCK			-49	/* couldn't open socket */
#define BINDERR			-50	/* bind error */
#define SND_ERR			-51	/* generic send error */
#define NOHNAME			-52	/* no host name */
#define NOHADDR			-53	/* no local host address */
#define BADVERSION		-54	/* bad protocol version */
/* -60  to -69 are variable converter errors */
#define NOINITFL		-60	/* no varcvt initialization file */
#define VARBOUNDS		-61	/*numeric variable not in legal bounds*/
#define NOBUF			-62	/* buffer for conversion non-existent */
#define NONUM			-63	/* no numeric correspondent */
#define NONAME			-64	/* no name for conversion */
#define CONVUNKNOWN		-65	/* unknown conversion */
#define VARUNKNOWN		-66	/* unknown variable */
/* -70 to -79 are authentication function errors */
#define NOMSGBUF		-70	/* buffers are missing */
