#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/server/snmpservsend.c,v 1.1 1994-09-18 12:56:54 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:16:04  snmpdev
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
/*
 *
 *			snmpservsend.c
 *
 * General purpose function to handle the sending of snmp protocol messages.
 * This function was originally snmpsend(), but was simplified for
 * gateway (server) operations.  All the fancy socket opens and
 * "repeat until timeout" schemes were stripped.
 * Given a message structure, this function will convert the message
 * to the appropriate asn form, add any authentication necessary and
 * send the message off. Returns the size (in bytes) of the message
 * sent, or a negative error code upon failure
 *
 */
#include "../../h/conf.h"
#ifdef BSD

#include "../../h/comm.h"	/* communication specific things */
#include "../../h/snmp_hs.h"	/* general include file */

#include <syslog.h>

short
snmpservsend(s,type,to,msg,sid,sidlen)
int s;					/* socket to send on */
short type;				/* type of message to be sent */
struct sockaddr_in *to;			/* ip address of receipient */
char *msg;				/* message to be sent */
char *sid;				/* session id of message */
short sidlen;				/* length of session id */
{ char msgbuf[SNMPMAXPKT];		/* buffer for message to be sent */
  short msglen;				/* length of protocol message built */
  int sendrc;				/* send return code */
  long version;
  short msgbld();
  short snmpbld();
  short sendauth();

/*
 *  do some error checking on parameters first
 */
  if(to == NULL)			/* if no destination */
   return(NODEST);
  if(sid == NULL)			/* if user didn't provide session id */
   return(NOSID);			/* return an error */
  if(sidlen <= 0)			/* ridiculous session id length? */
   return(BADSIDLEN);			/* return error */

/*
 *  build message in message buffer by calling packet builder
 */
  if((msglen = snmpbld(type,msg,msgbuf,SNMPMAXPKT)) < 0)
   return(msglen);

/*
 *  add authentication
 */
  if((msglen = sendauth(sid,msgbuf,msglen)) < 0)
   return(msglen);

/*
 *  build protocol message - version, session id etc.
 */
  version = 0;		/* current version */
  if((msglen = msgbld(msglen,msgbuf,SNMPMAXPKT,sid,sidlen,version)) < 0)
   return(msglen);

/*
 *  all done with preparations. Now try to send off message
 */
  sendrc=sendto(s,msgbuf,msglen,0,(struct sockaddr *)to,sizeof(struct sockaddr_in));
  if(sendrc < 0) {			/* error in send */
   syslog(LOG_ERR, "snmpsendserv: sendto: %m");
   return(SND_ERR);
  }

  return(msglen);   /*  finished! */
}
#endif /* BSD */
