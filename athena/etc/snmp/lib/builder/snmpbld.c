#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/snmpbld.c,v 1.1 1994-09-18 12:56:23 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:15  snmpdev
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
/*****************************************************************************
**
**			snmpbld.c
**
** File contains the top-level function in the NYSERNet SNMP distribution's
** packet builder. 'snmpbld' will return the length of the message
** built, or a negative error code if there is an error.
**
** Note that the packet builder will build the packet backwards,
** from the last byte of the contents to the first byte of the
** message identifier from the end of the buffer to the beginning and 
** then, after finishing, move the entire contents of the buffer 
** so the message built begins at the beginning of the buffer.
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
snmpbld(type,msg,buf,buflen)
short type;				/* type of message to be built */
char *msg;				/* message to be encoded */
char *buf;				/* buffer to put encoded msg into */
short buflen;				/* length of buffer */
{ short length;				/* length of pdu contents */
  char *current;			/* current byte to start encoding at */

/* check validity of parameters first */
  if(msg == NULL)			/* no message to encode? */
   return(NOINBUF);
  if(buf == NULL)			/* no buffer for encoded message? */
   return(NOOUTBUF);
  if(buflen < 0)			/* buffer is of negative length? */
   return(NEGOUTBUF);

  current = buf+buflen-1;		/* start at the end of buffer */

/* done with preliminary checks. Now build pdus */
  switch(type) {			/* determine type of message */
   case REQ:
    if((current = bldpdu(&length,current,&buflen,(pdu_type *)msg)) == NULL)
     return(length);
    if((current = bldlen(&length,current,&buflen)) == NULL)
     return(length);
    if(buflen > 0)
      *current = GETREQID;		/* put in message id */
    else
     return(LENERR);
    break;
   case NXT:
    if((current = bldpdu(&length,current,&buflen,(pdu_type *)msg)) == NULL)
     return(length);
    if((current = bldlen(&length,current,&buflen)) == NULL)
     return(length);
    if(buflen > 0)
     *current = GETNXTID;		/* put in message id */
    else
     return(LENERR);
    break;
   case RSP:
    if((current = bldpdu(&length,current,&buflen,(pdu_type *)msg)) == NULL)
     return(length);
    if((current = bldlen(&length,current,&buflen)) == NULL)
     return(length);
    if(buflen >0)
     *current = GETRSPID;		/* put in message id */
    else
     return(LENERR);
    break;
   case SET:
    if((current = bldpdu(&length,current,&buflen,(pdu_type *)msg)) == NULL)
     return(length);
    if((current = bldlen(&length,current,&buflen)) == NULL)
     return(length);
    if(buflen > 0)
     *current = SETREQID;		/* put in message id */
    else
     return(LENERR);
    break;
   case TRP:
    if((current = bldtrp(&length,current,&buflen,(trptype *)msg)) == NULL)
     return(length);
    if((current = bldlen(&length,current,&buflen)) == NULL)
     return(length);
    if(buflen > 0)
     *current = TRAPID;			/* put in message id */
    else
     return(LENERR);
    break;
   default:
    return(TYP_UNKNOWN);
  }
  buflen--; length++; current--;	/* add message id byte */

  return(length);
}
