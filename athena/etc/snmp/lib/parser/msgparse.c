#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/msgparse.c,v 1.1 1994-09-18 12:56:18 cfields Exp $";
#endif

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:26  snmpdev
 * Initial revision
 * 
 */

/******************************************************************************
**
**			msgparse.c
**
** Function to parse top level SNMP message 
**
**
*****************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"
#include "../../h/asn.h"

short
msgparse(pktlen,msgbuf,sid,sidlen,version,msglen)
short *pktlen;				/* length of packet */
char *msgbuf;				/* message to be parsed */
char *sid;				/* session id (community) */
short *sidlen;				/* length of session id */
long *version;				/* version */
short *msglen;				/* length of data (PDU) */
{ u_short dlen;				/* length of data */
  char *current;			/* current byte being parsed */
  short buflen;
  char *oldcurrent;
  strng s_id;

/* error checks */
  if(msgbuf == NULL)
   return(NOINBUF);
  if(sid == NULL || sidlen == NULL)
   return(NOSID);
  if(version == NULL || msglen == NULL)
   return(GENERR);
  current = msgbuf;

/* check that we have a sequence here */
  if(*current != SEQID)			/* if not a sequence */
   return(TYP_MISMATCH);
  current++;

/* get length of data */
  if((current = asnlen(pktlen,current,&dlen)) == NULL)
   return(*pktlen);

/* get version */
  buflen = 5;
  oldcurrent = current;
  if((current = prseint(pktlen,current,&buflen,(long *)version)) == NULL)
   return(*pktlen);
  dlen -= (current - oldcurrent);

/* get session id */
  s_id.str = NULL; s_id.len = 0;
  buflen = 128;				/* nonsensical figure */
  oldcurrent = current;
  if((current = prsestr(pktlen,current,&buflen,&s_id)) == NULL)
   return(*pktlen);
  strcpy(sid,s_id.str);
  free(s_id.str);
  *sidlen = s_id.len;
  dlen -= (current - oldcurrent);

/* return with message index and message length */
  *msglen = dlen;
  return(current-msgbuf);
}
  
