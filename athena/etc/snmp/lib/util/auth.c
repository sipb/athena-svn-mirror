#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/auth.c,v 1.1 1994-09-18 12:56:45 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:16:12  snmpdev
 * Initial revision
 * 
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
**			auth.c
**
** Authentication functions. This should really be in a directory
** of its own, but currently the authentication is trivial, so
** it has been temporarily place in the util directory
**
**
*******************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#endif /* SVR3WIN */
#include "../../h/snmperrs.h"

/* sendauth - do send authentication. Returns length of message */
short
sendauth(sid,msgbuf,msglen)
char *sid;				/* session id */
char *msgbuf;				/* protocol msg to be authenticated */
short msglen;				/* length of protocol message */
{
/* any authentication that should be done on an outgoing message should
   be done here. But since only the null authentication function is done,
   we just return the message length.
*/
   if(msgbuf == NULL)
    return(NOMSGBUF);
   return(msglen);
}

/* recvauth - do receive authentication. Returns index where prot. msg starts */
short 
recvauth(sid,sidlen,msgbuf,msglen,msgindex)
char *sid;				/* session id */
short *sidlen;				/* length of session id */
char *msgbuf;				/* message to be authenticated */
short msglen;				/* length of msg to be authenticated */
short msgindex;				/* start of msg to be authenticated */
{
  return(msgindex);
}
