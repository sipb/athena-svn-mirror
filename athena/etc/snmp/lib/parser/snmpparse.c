#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/snmpparse.c,v 1.1 1994-09-18 12:56:10 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:51  snmpdev
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
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */
/******************************************************************************
**
**			snmpparse.c
**
** Top-level function for SNMP ASN.1 parser that processes from PDUs down.
**
** Function returns type of message or a negative error code on error
**
**
******************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#include <sys/types.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#include "/usr/netinclude/sys/types.h"
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"
#include "../../h/asn.h"

short
snmpparse(msglen,inbuf,buflen,outbuf)
short msglen;				/* length of input message */
char *inbuf;				/* buffer with input message */
short buflen;				/* size of output buffer */
char *outbuf;				/* output buffer */
{ char *current;			/* current byte being processed */
  char msgid;				/* to get message id */
  short type;				/* type of message received */
  u_short length;			/* length of message */

/* check parameters to function */
  if(msglen < 0)			/* if input is of negative length */
   return(NEGINBUF);
  if(inbuf == (char *)NULL)		/* if no input buffer */
   return(NOINBUF);
  if(buflen < 0)			/* if outbuf is of negative size */
   return(NEGOUTBUF);
  if(outbuf == (char *)NULL)		/* if no output buffer */
   return(NOOUTBUF);

  current = inbuf;

/* determine what sort of message it is */
  if(msglen <= 0)			/* if end of message */
   return(LENERR);
  msgid = *current;
  switch(msgid) {			/* what sort of message? */
   case GETREQID:			/* get-request PDU */
    type = REQ; break;
   case GETNXTID:			/* get-next-request PDU */
    type = NXT; break;
   case GETRSPID:			/* get-response PDU */
    type = RSP; break;
   case SETREQID:			/* set-request PDU */
    type = SET; break;
   case TRAPID:				/* trap PDU */
    type = TRP; break;
   default:
    return(TYP_UNKNOWN);
   }

/*
   The following code really belongs in the functions that parse the
   actual asn structures that represent the various PDUs. However, in
   the interests of efficiency, to avoid passing 'type' to those
   functions and to centralize the code to get to the asn data,
   the parsing of the asn headers is done here.

   parse past length. The length isn't actually used, but it is still necessary
   to get past it
*/

  msglen--; current++;			/* move past ID field of PDU encoding */

  if((current = asnlen(&msglen,current,&length)) == NULL)
   return(msglen);

/* parse contents of whatever PDU we've got now */
  switch(type) {			/* which PDU do we have? */
   case REQ:				/* get-request PDU */
    if((current = pdu(&msglen,current,&buflen,(pdu_type *)outbuf)) == NULL)
     return(msglen);
    break;
   case NXT:				/* get-next-request PDU */
    if((current = pdu(&msglen,current,&buflen,(pdu_type *)outbuf)) == NULL)
     return(msglen);
    break;
   case RSP:				/* get-response PDU */
    if((current = pdu(&msglen,current,&buflen,(pdu_type *)outbuf)) == NULL)
     return(msglen);
    break;
   case SET:				/* set-request PDU */
    if((current = pdu(&msglen,current,&buflen,(pdu_type *)outbuf)) == NULL)
     return(msglen);
    break;
   case TRP:				/* trap PDU */
    if((current = trap(&msglen,current,&buflen,(trptype *)outbuf)) == NULL)
     return(msglen);
    break;
/* the default case should never be executed unless there is a system error */
   default:				/* unknown */
    fprintf(stderr,"parser system error (snmpparse): illegal 'type'\n");
    exit(1);
  }

/* all done now. Return type of message received */
  return(type);
}
