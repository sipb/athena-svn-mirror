#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/bldpdus.c,v 1.1 1994-09-18 12:56:25 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:06  snmpdev
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
**			bldpdus.c
**
** Functions to build the PDUs ('pdu' and 'trap') specified by SNMP.
** All functions return a pointer to the next byte into which encoding
** can occur, or NULL on error. In the event of an error, the length
** parameter will contain a negative error code.
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

/* the following are actually just primitives, so macros are used */
#define bldreqid(a,b,c,d)		bldint(a,b,c,d)
#define blderrstat(a,b,c,d)		bldint(a,b,c,d)
#define blderrindx(a,b,c,d)		bldint(a,b,c,d)
#define bldent(a,b,c,d)			bldobj(a,b,c,d)
#define bldgtrp(a,b,c,d)		bldint(a,b,c,d)
#define bldstrp(a,b,c,d)		bldint(a,b,c,d)

/* bldpdu - build a 'pdu' */
char *
bldpdu(len,current,buflen,msg)
short *len;				/* length of encoding data */
char *current;				/* byte to start encoding at */
short *buflen;				/* length of buffer for encoding */
pdu_type *msg;				/* message to encoded */
{ short lstlen,eslen,eilen,rlen;	/* length of list,errstat etc. */

  lstlen = 0;				/* make sure length is 0 */
  eslen = 0;				/* make sure length is 0 */
  eilen = 0;				/* make sure length is 0 */
  rlen = 0;				/* make sure length is 0 */

  if((current = bldvarlst(&lstlen,current,buflen,
			  (var_list_type *)&(msg->varlist))) == NULL)
   { *len = lstlen;
     return((char *)NULL);
   }
  if((current=blderrindx(&eilen,current,buflen,(long *)&(msg->errindex)))==NULL)
   { *len = eslen;
     return((char *)NULL);
   }
  if((current=blderrstat(&eslen,current,buflen,(long *)&(msg->errstat)))==NULL)
   { *len = eslen;
     return((char *)NULL);
   }
  if((current=bldreqid(&rlen,current,buflen,(long *)&(msg->reqid))) == NULL)
   { *len = rlen;
     return((char *)NULL);
   }
  *len = lstlen + eslen + eilen + rlen;

  return(current);
}

/* bldtrp - build a 'trap' */
char *
bldtrp(len,current,buflen,msg)
short *len;				/* length of encoding data */
char *current;				/* byte to start encoding at */
short *buflen;				/* length of buffer for encoding */
trptype *msg;				/* message to encoded */
{ short lstlen,tlen,slen,glen,alen,elen; /* lengths of various pdu fields */

  lstlen = 0;				/* make sure length is 0 */
  tlen = 0;				/* make sure length is 0 */
  slen = 0;				/* make sure length is 0 */
  glen = 0;				/* make sure length is 0 */
  alen = 0;				/* make sure length is 0 */
  elen = 0;				/* make sure length is 0 */

  if((current = bldvarlst(&lstlen,current,buflen,
			  (var_list_type *)&(msg->varlist))) == NULL)
   { *len = lstlen;
     return((char *)NULL);
   }
  if((current = bldtime(&tlen,current,buflen,(u_long *)&(msg->tm))) == NULL)
   { *len = tlen;
     return((char *)NULL);
   }
  if((current = bldstrp(&slen,current,buflen,(long *)&(msg->strp))) == NULL)
   { *len = slen;
     return((char *)NULL);
   }
  if((current = bldgtrp(&glen,current,buflen,(long *)&(msg->gtrp))) == NULL)
   { *len = glen;
     return((char *)NULL);
   }
  if((current = bldipadd(&alen,current,buflen,
			  (struct in_addr *)&(msg->agnt))) == NULL)
   { *len = alen;
     return((char *)NULL);
   }
  if((current = bldent(&elen,current,buflen,(objident *)&(msg->ent))) == NULL)
   { *len = elen;
     return((char *)NULL);
   }
  *len = lstlen + tlen + slen + glen + alen + elen;

  return(current);
}
