#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/prsepdus.c,v 1.1 1994-09-18 12:56:12 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:33  snmpdev
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
**			prsepdus.c
**
** Functions to parse the 'pdu' and 'trap' non-terminals in the grammar
** for the second parser. All functions return a pointer to the next
** octet to be parsed, or NULL on error. In the event of an error,
** msglen will contain a negative error code.
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

/* these are just primitives - use macros */
#define enterprise(a,b,c,d)		prseobj(a,b,c,d)
#define agentaddr(a,b,c,d)		prseipadd(a,b,c,d)
#define gentrp(a,b,c,d)			prseint(a,b,c,d)
#define spectrp(a,b,c,d)		prseint(a,b,c,d)
#define req_id(a,b,c,d)			prseint(a,b,c,d)
#define err_stat(a,b,c,d)		prseint(a,b,c,d)
#define err_index(a,b,c,d)		prseint(a,b,c,d)

/* pdu  - parse the 'pdu' non-terminal */
char *
pdu(msglen,current,buflen,outbuf)
short *msglen;				/* length of input message */
char *current;				/* input message */
short *buflen;				/* size of output structure */
pdu_type *outbuf;			/* output structure */
{
  if((current = req_id(msglen,current,buflen,(long *)&(outbuf->reqid))) == NULL)
   return(current);
  if((current=err_stat(msglen,current,buflen,(long *)&(outbuf->errstat)))==NULL)
   return(current);
  if((current=err_index(msglen,current,buflen,(long *)&(outbuf->errindex)))==NULL)
   return(current);
  current = varlst(msglen,current,buflen,(var_list_type *)&(outbuf->varlist));
  return(current);
}

/* trap - parse the 'trap' nonterminal */
char *
trap(msglen,current,buflen,outbuf)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
trptype *outbuf;			/* outgoing structure */
{
  if((current=enterprise(msglen,current,buflen,(objident *)&(outbuf->ent)))==NULL) 
   return(current);
  if((current=agentaddr(msglen,current,buflen,(struct in_addr *)&(outbuf->agnt)))==NULL) 
   return(current);
  if((current=gentrp(msglen,current,buflen,(long *)&(outbuf->gtrp)))==NULL) 
   return(current);
  if((current=spectrp(msglen,current,buflen,(long *)&(outbuf->strp)))==NULL) 
   return(current);
  if((current=prsetime(msglen,current,buflen,(u_long *)&(outbuf->tm)))==NULL) 
   return(current);
  current=varlst(msglen,current,buflen,(var_list_type *)&(outbuf->varlist));
  return(current);
}
