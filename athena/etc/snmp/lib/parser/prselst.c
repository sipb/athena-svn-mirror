#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/prselst.c,v 1.1 1994-09-18 12:56:13 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:31  snmpdev
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
**			prselst.c
**
** Functions to parse the variable bindings list right down to the
** SMI primitives. All functions return a pointer to the next octet
** to be parsed, or NULL on error. In the event of an error, msglen
** contains a negative error code.
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

/* this is really an object identifier,so use a macro */
#define vnm(a,b,c,d)		prseobj(a,b,c,d)

/* varlst - parse a variable bindings list */
char *
varlst(msglen,current,buflen,lst)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
var_list_type *lst;			/* list (outgoing structure component */
{ u_short length;			/* length of list in octets */
  char *oldcurrent;			/* to save value of current */

/* do error checks */
  if(current == NULL)
   { *msglen = NOINBUF;
     return((char *)NULL);
   }
  if(*msglen <= 0)			/* if end of input message */
   { *msglen = LENERR;
     return((char *)NULL);
   } 
  if(*current != SEQID)			/* check that we have a list */
   { *msglen = TYP_MISMATCH;		/* wrong type */
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* determine length of list */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);

/* initialize some things first */
  lst->len = 0;				/* list is currently of zero length */

/*
   the rest of this function is actually to parse the 'varbind' terminal
   but in the interests of efficiency, we put it here to avoid a function
   call
*/
/* parse list */
  while(length > 0)			/* while not end of list */
   { if(lst->len >= SNMPMAXVARS)	/* if too many variables */
      { *msglen = TOOMANYVARS;
	return((char *)NULL);
      }
     oldcurrent = current;		/* save value of current */
     if((current = var(msglen,current,buflen,
		       (varbind *)&(lst->elem[lst->len]))) == NULL)
      return(current);
     (lst->len)++;
     length -= (current - oldcurrent);
   }

/* done. return */
  return(current);
}

/* var - parse the 'var' non-terminal */
char *
var(msglen,current,buflen,v)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* sizeof outgoing structure */
varbind *v;				/* variable (structure component) */
{ u_short length;

/* check that this is a variable */
  if(*msglen <= 0)			/* end of incoming message */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != SEQID)			/* not a sequence? */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get the length of the variable */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);

/* parse variable contents */
  if((current = vnm(msglen,current,buflen,(objident *)&(v->name))) == NULL)
   return(current);
  current = vval(msglen,current,buflen,(objval *)&(v->val));
  return(current);
}

/* vval - parse the 'value' non-terminal */
char *
vval(msglen,current,buflen,val)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* sizeof outgoing structure */
objval *val;				/* the value */
{
/* determine value type and call appropriate function */
  if(*msglen <= 0)
   { *msglen = LENERR;			/* message ended unexpectedly */
     return((char *)NULL);
   }
  switch(*current) {			/* determine value type */
   case INTID:				/* integer */
    val->type = INT;
    current = prseint(msglen,current,buflen, (long *)&(val->value.intgr));
    return(current);
   case STRID:				/* string */
    val->type = STR;
    val->value.str.str = NULL; val->value.str.len = 0;
    current = prsestr(msglen,current,buflen,(strng *)&(val->value.str));
    return(current);
   case NULLID:				/* empty */
    val->type = EMPTY;
    current = prsenull(msglen,current);
    return(current);
   case OBJID:				/* obj */
    val->type = OBJ;
    current = prseobj(msglen,current,buflen,(objident *)&(val->value.obj));
    return(current);
   case IPID:				/* ipaddr */
    val->type = IPADD;
    current = prseipadd(msglen,current,buflen,
			(struct in_addr *)&(val->value.ipadd));
    return(current);
   case COUNTERID:			/* counter */
    val->type = CNTR;
    current = prsecntr(msglen,current,buflen,(u_long *)&(val->value.cntr));
    return(current);
   case GAUGEID:			/* gauge */
    val->type = GAUGE;
    current = prsegauge(msglen,current,buflen,(u_long *)&(val->value.gauge));
    return(current);
   case TIMEID:				/* time */
    val->type = TIME;
    current = prsetime(msglen,current,buflen,(u_long *)&(val->value.time));
    return(current);
   case OPAQUEID:			/* opaque */
    val->type = OPAQUE;
    current = prseopaque(msglen,current,buflen,(strng *)&(val->value.opqe));
    return(current);
   default:				/* unknown */
    *msglen = TYP_MISMATCH;		/* wrong type */
    return((char *)NULL);
  }
}
