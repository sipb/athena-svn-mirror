#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/comm/snmprecv.c,v 1.2 1997-02-27 06:40:43 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18 12:56:36  cfields
 * Initial revision
 *
 * Revision 1.1  89/11/03  15:15:19  snmpdev
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
**			snmprecv.c
**
** General purpose function to handle the receiving of snmp packets.
** Upon receipt of an snmp packet, this function will authenticate
** and parse it, ultimately filling in the message buffer provided
** by the caller with the  information from the packet received.
** Returns the type of message received, or a negative indication of
** a fatal error.
**
**
*******************************************************************************/
#include "../../h/conf.h"
#include "../../h/comm.h"
#include "../../h/snmp_hs.h"

short
snmprecv(from,service,req_id,sid,sidlen,msg,msgsz)
struct in_addr *from;			/* ip address of source of packet */
char *service;				/* in case we listen on a known port */
long *req_id;				/* request id of message desired */
char *sid;				/* buf to put session id of msg recvd */
short *sidlen;				/* length of sid received */
char *msg;				/* buffer for message received */
short msgsz;				/* length of message */
{ short msglen;				/* length of message received */
  short pktlen;				/* length of packet received */
  char msgbuf[SNMPMAXPKT];		/* buffer for packet received */
  struct sockaddr_in remote;		/* for remote host info */
  char s_id[SNMPMXSID];			/* temporary sid buffer */
  char hname[HLEN];			/* temporary buffer for hostname */
  struct hostent *hp;			/* to get local host address */
  short s_id_len;			/* temporary sid length */
  short type;				/* type of message received */
  long reqid;				/* temporary - unavoidable */
  short rc;				/* general return code */
  int remotelen;			/* remote comm. info length */
  short msgindex;			/* index to where prot. msg starts */
  long version;				/* protocol version */
  short recvauth();
  short snmpparse();
  short commadd();
  short msgparse();
/* these are just to make life easier */
  getreq *reqmsg;			/* Get Request Message */
  getnext *nxtmsg;			/* Get Next Request Message */
  getrsp *rspmsg;			/* Get Response Message */
  setreq *setmsg;			/* Set Request Message */

/* error checks on parameters */
  if(msg == NULL)			/* if no return buffer provided */
   return(NORECVBUF);
  if(req_id == NULL)			/* if no request id provided */
   return(NOREQID);
/* these are just kludges in case the caller didn't provide buffers */
  if(sid == NULL)			/* caller didn't provide session id */
   sid = s_id;				/* point at some local space */
  if(sidlen == NULL)			/* caller didn't provide sidlen */
   sidlen = &s_id_len;			/* point at some local space */

/* check on state of communications */
  if(s < 0)				/* if we don't have a socket */
   if(*req_id != REQ_ANY)		/* we are waiting for response */
    return(UNINIT_SOCK);		/* return error */
   else					/* we're a server */
    if(service == NULL)			/* we won't have a port */
     return(NOSVC);			/* return error */
    else				/* initialize local communications */
/* initialize local socket struct */
     { memset(&local,0,sizeof(local));
       if(gethostname(hname,HLEN) < 0)
	return(NOHNAME);
       do				/* get address of local host */
	hp = gethostbyname(hname);
       while(hp != NULL && hp->h_addrtype != AF_INET);
       if(hp == NULL)
	return(NOHADDR);
#ifdef NAMED
       memcpy(&local.sin_addr,&(hp->h_addr),hp->h_length);
#else
       memcpy(&local.sin_addr,(hp->h_addr),hp->h_length);
#endif /* NAMED */
       local.sin_family = AF_INET;	/* want internet messages */
/* get well-known port */
       if(strcmp(service,"snmp") == 0)	/* a query server? */
        local.sin_port = (short)htons((u_short)SNMPQRY);
       else				/* a trap server maybe? */
        if(strcmp(service,"snmp-trap") == 0)
         local.sin_port = (short)htons((u_short)SNMPTRAP);
        else
         return(NOSVC);
/* create and bind local socket */
       if((s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
        return(NOSOCK);
       if(bind(s,(struct sockaddr *)&local,sizeof(local)) < 0)
        return(BINDERR);
     }

/* main program loop */
  do {					/* iterate for messages */
   remotelen = sizeof(remote);
   pktlen=recvfrom(s,msgbuf,SNMPMAXPKT,0,(struct sockaddr *)&remote,&remotelen);

/* fill in any buffers caller provided */
   if(from != NULL)
    memcpy(from,&remote.sin_addr,sizeof(struct in_addr));

/* parse top-level message */
   if((msgindex = msgparse(&pktlen,msgbuf,sid,sidlen,&version,&msglen)) < 0)
    return(msgindex);
   if(version != SNMPVER)		/* check that msg has right version */
    continue;				/* go wait for another message */

/* authenticate message */
  if((msgindex = recvauth(sid,sidlen,msgbuf,msglen,msgindex)) < 0)
   continue;				/* go wait for another message */
  
/* parse message */
  if((type = snmpparse(msglen,(msgbuf+msgindex),msgsz,msg)) < 0)
   continue;
/*
   this is unfortunately necessary as we have to extract the
   request id.
*/
  switch(type) {			/* determine message type */
   case REQ:				/* Get Request Message */
    reqmsg = (getreq *)msg;
    reqid = reqmsg->reqid;		/* get request id */
    break;
   case NXT:				/* Get Next Message */
    nxtmsg = (getnext *)msg;
    reqid = nxtmsg->reqid;		/* get request id */
    break;
   case RSP:				/* Get Response Message */
    rspmsg = (getrsp *)msg;
    reqid = rspmsg->reqid;		/* get request id */
    break;
   case TRP:				/* Trap Request Message */
    break;
   case SET:				/* Set Request Message */
    setmsg = (setreq *)msg;
    reqid = setmsg->reqid;		/* get request id */
    break;
   default:				/* Unknown message type */
    continue;				/* go get a known message */
  }

/* decide if a table entry should be made */
  if((type == REQ || type == NXT || type == SET) && *req_id == REQ_ANY)
   if((rc = commadd(reqid,&remote,sizeof(remote))) < 0)
    return(rc);

/* find out what the request id is */
  if(type != TRP && reqid != *req_id) /* not a trap and reqid don't match */
   if(*req_id != REQ_ANY)		/* must be a specific message */
    continue;				/* get another message */
   else					/* any message will do */
    *req_id = reqid;			/* copy in request id */

/* got a message successfully. Return */
  return(type);

  } while (1);				/* repeat forever */

  return(-1);
}
