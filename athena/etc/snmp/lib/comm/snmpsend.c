#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/comm/snmpsend.c,v 1.1 1994-09-18 12:56:34 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:21  snmpdev
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
**			snmpsend.c
**
** General purpose function to handle the sending of snmp protocol messages.
** Given a message structure, this function will convert the message
** to the appropriate asn form, add any authentication necessary and
** send the message off. Returns the size (in bytes) of the message
** sent, or a negative error code upon failure
**
**
*******************************************************************************/
#include "../../h/conf.h"
#include "../../h/comm.h"		/* communication specific things */
#include "../../h/snmp_hs.h"		/* general include file */

int s = -1;				/* socket file descriptor */
struct sockaddr_in local;		/* local communications info */

short
snmpsend(type,req_id,to,service,msg,sid,sidlen,timeout)
short type;				/* type of message to be sent */
long req_id;				/* request id of message to be sent */
struct in_addr *to;			/* ip address of receipient */
char *service;				/* service to use in sending */
char *msg;				/* message to be sent */
char *sid;				/* session id of message */
short sidlen;				/* length of session id */
long timeout;				/* how long to try sending */
{ char msgbuf[SNMPMAXPKT];		/* buffer for message to be sent */
  short msglen;				/* length of protocol message built */
  struct sockaddr_in *dest;		/* pointer to dest. sock. info. */
  struct sockaddr_in remote;		/* struct for remote host socket */
  char hname[HLEN];			/* name of local host */
  struct hostent *hp;			/* to get address of local host */
  long oldtime,curtime;			/* to keep track of time */
  int sendrc;				/* send return code */
  long version;				/* protocol version */
  short snmpbld();
  short sendauth();

/* do some error checking on parameters first */
  if(to == NULL)			/* if no destination */
   return(NODEST);
  if(sid == NULL)			/* if user didn't provide session id */
   return(NOSID);			/* return an error */
  if(sidlen <= 0)			/* ridiculous session id length? */
   return(BADSIDLEN);			/* return error */
  time(&oldtime);			/* get start time */

/* check on state of communications */
  if(s < 0)				/* communications not yet initialized */
   if(service == NULL)			/* this is a reply? */
    return(UNINIT_SOCK);		/* uninitalized socket */
   else					/* we're initializing conversation */
    { bzero((char *)&local,sizeof(local)); /* empty structure */
      if(gethostname(hname,HLEN) < 0)	/* get local host name */
       return(NOHNAME);
      do
       hp = gethostbyname(hname);
      while(hp != NULL && hp->h_addrtype != AF_INET);
      if(hp == NULL)
       return(NOHADDR);
#ifdef NAMED
      bcopy((char *)&(hp->h_addr),(char *)&local.sin_addr,hp->h_length);
#else
      bcopy((char *)(hp->h_addr),(char *)&local.sin_addr,hp->h_length);
#endif /* NAMED */
      local.sin_family = AF_INET;	/* use internet family */
      local.sin_port = (short)0;	/* have system find port */
/* create and bind local socket */
      if((s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
       return(NOSOCK);			/* return global error number */
      if(bind(s,(struct sockaddr *)&local,sizeof(local)) < 0)
       return(BINDERR);
    }

/* build message in message buffer by calling packet builder */
  if((msglen = snmpbld(type,msg,msgbuf,SNMPMAXPKT)) < 0)
   return(msglen);

/*  add authentication. */
  if((msglen = sendauth(sid,msgbuf,msglen)) < 0)
   return(msglen);

/* build protocol message - version, session id etc. */
  version = 0;				/* current protocol version */
  if((msglen = msgbld(msglen,msgbuf,SNMPMAXPKT,sid,sidlen,version)) < 0)
   return(msglen);

/* get remote host information, either from table or by building */
/* create socket structure */
/* get service */
  if(service != NULL)			/* we're sending to a well-known port */
   { bzero((char *)&remote,sizeof(remote));
     bcopy((char *)to,(char *)&remote.sin_addr,sizeof(struct in_addr));
     remote.sin_family = AF_INET;	/* internet address family */
     if(strcmp(service,"snmp") == 0)
      remote.sin_port = (short)htons((u_short)SNMPQRY);
     else				/* maybe a trap */
      if(strcmp(service,"snmp-trap") == 0)
       remote.sin_port = (short)htons((u_short)SNMPTRAP);
      else
       return(NOSVC);
     dest = &remote;			/* point at correct structure */
   }
  else					/* we're replying */
   if((dest = comminfo(req_id)) == NULL)
    return(REQID_UNKNOWN);		/* return error */

/* all done with preparations. Now try to send off message */
  do {					/* try to send message */
   sendrc=sendto(s,msgbuf,msglen,0,(struct sockaddr *)dest,sizeof(struct sockaddr_in));
   if(sendrc < 0)			/* error in send */
    return(SND_ERR);
   time(&curtime);
  } while (sendrc != msglen && (curtime - oldtime) < timeout);
  if((curtime -oldtime) >= timeout)	/* if send timed out */
    return(SND_TMO);			/* return timeout */
  
/* decide if we want to terminate communications */
  if(req_id == REQ_ANY)			/* sent a trap */
   { shutdown(s,2);			/* shutdown socket */
     close(s);				/* close socket */
   }
  else					/* not a trap */
   if (service == NULL)			/* sent a response */
    commrm(req_id);			/* remove entry with key req_id */

/* all done */
  return(msglen);
}
