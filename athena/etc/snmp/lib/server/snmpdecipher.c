#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/server/snmpdecipher.c,v 1.1 1994-09-18 12:56:56 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:57  snmpdev
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
 *			snmpdecipher.c
 *
 * General purpose function to handle the deciphering of snmp packets.
 * this function will authenticate and parse SGMP packet "msgbuf",
 * ultimately filling in the message buffer provided
 * by the caller with the  information from the packet received.
 * Returns the type of message received, or a negative indication of
 * a fatal error.
 *
 */

#include "../../h/conf.h"
#ifdef BSD

#include "../../h/comm.h"
#include "../../h/snmp_hs.h"

short
snmpdecipher(sid,sidlen,msg,msgbuf,pktlen,msgsz)
char *sid;				/* buf to put session id of msg recvd */
short *sidlen;				/* length of sid received */
char *msg;				/* buffer for message deciphered */
char *msgbuf;				/* buffer with incoming packet */
short pktlen;				/* length of incoming packet */
short msgsz;				/* length of deciphered msg buffer */
{
  short msglen;				/* length of message received */
  char s_id[SNMPMXSID];			/* temporary sid buffer */
  short s_id_len;			/* temporary sid length */
  short type;				/* type of message received */
  short msgindex;			/* index to where prot. msg starts */
  long version;
  short msgparse();
  short recvauth();
  short snmpparse();

/*
 * these are just to make life easier
 */
  getreq *reqmsg;			/* Get Request Message */
  getnext *nxtmsg;			/* Get Next Message */
  getrsp *rspmsg;			/* Get Response Message */
  setreq *setmsg;			/* Set Request Message */

/*
 * error checks on parameters
 */
  if(msg == NULL)			/* if no return buffer provided */
   return(NORECVBUF);

/*
 *  these are just kludges in case the caller didn't provide buffers
 */
  if(sid == NULL)			/* caller didn't provide session id */
   sid = s_id;				/* point at some local space */
  if(sidlen == NULL)			/* caller didn't provide sidlen */
   sidlen = &s_id_len;			/* point at some local space */

/*
 *  parse top-level message
 */
  if((msgindex = msgparse(&pktlen,msgbuf,sid,sidlen,&version,&msglen)) < 0)
   return(msgindex);

  if(version != SNMPVER)		/* check for right version */
   return(BADVERSION);

/*
 *  authenticate message
 */
  if((msgindex = recvauth(sid,sidlen,msgbuf,msglen,msgindex)) < 0)
   return(NOMSGBUF);
  
/*
 *  parse message
 */
  if((type = snmpparse(msglen,(msgbuf+msgindex),msgsz,msg)) < 0)
   return(type);

/*
 *  got a message successfully. Return
 */
  return(type);
}
#endif /* BSD */
