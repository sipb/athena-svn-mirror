#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/msgbld.c,v 1.2 1997-02-27 06:40:33 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18 12:56:31  cfields
 * Initial revision
 *
 * Revision 1.1  89/11/03  15:15:13  snmpdev
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
**			msgbld.c
**
** Function to build top-level SNMP message.
**
**
******************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"
#include "../../h/asn.h"

msgbld(msglen,msgbuf,buflen,sid,sidlen,version)
short msglen;				/* length of message so far */
char *msgbuf;				/* buffer to encode mesage */
short buflen;				/* total length of buffer */
char *sid;				/* session id (community) */
short sidlen;				/* length of session id */
long version;				/* protocol version */
{ char *current;			/* octet where encoding should start */
  short len;				/* length */
  short blen;				/* fake buffer length */
  int i;				/* index and counter */

/* error checks */
  if(msgbuf == NULL)
   return(NOOUTBUF);
  if(sid == NULL)
   return(NO_SID);
  if(msglen < 0 || buflen < 0 || sidlen < 0)
   return(NEGINBUF);
  current = msgbuf+buflen-msglen-1;	/* index to correct place */
  blen = buflen-msglen;			/* get correct buffer length */

/* put in community (session id) */
  current -= (sidlen - 1);
  blen -= sidlen;
  len = sidlen;
  memcpy(current,sid,(int)sidlen);
  current--;
  if((current = bldlen(&len,current,&blen)) == NULL)
   return(len);
  if(blen < 1)				/* if there is no space */
   return(LENERR);
  *current = STRID;			/* put in id */
  blen--; len++; current--;

/* put in version - couldn't possibly be more than 255 */
/* this is version 1 of protocol has version == 0 */
  current -= 2;				/* move back 2 */
  current[0] = 0x02;			/* integer id */
  current[1] = 0x01;			/* version length */
  version = (long)htonl((u_long)version);
  current[2] = ((char *)&version)[sizeof(int)-1];
  blen -= 3; len += 3; current--;

/* put in length of entire message */
/* at this point len == length of sid encoding + length of version encoding */
  len += msglen;			/* add in length of data encoding */
  if((current = bldlen(&len,current,&blen)) == NULL)
   return(len);

/* add in LAST (first, actually) byte, seqid for entire protocol message */
  if(blen < 1)
   return(LENERR);
  *current = SEQID;			/* a sequence */
  len++; blen--;

/* now copy buffer forward */ 
  for(i=0; i < len; i++)
   msgbuf[i] = msgbuf[blen+i];

  return(len);
}
