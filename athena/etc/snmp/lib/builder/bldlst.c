#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/bldlst.c,v 1.1 1994-09-18 12:56:26 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:14:58  snmpdev
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
**			bldlst.c
**
** Functions to build the variable binding list and the components
** that make it up. All functions return the next octet into which
** encoding can occur, or NULL on error. In the event of error,
** the length variable will contain a negative error code.
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

/* a variable name is just an object identifier, so use a macro */
#define bldnm(a,b,c,d)			bldobj(a,b,c,d)

/* bldvarlst - build the variable bindings list */
char *
bldvarlst(len,current,buflen,lst)
short *len;				/* length of encoding data */
char *current;				/* buffer to encode into */
short *buflen;				/* max length of buffer for encoding */
var_list_type *lst;			/* list to be encoded */
{ short vlen;				/* length of a variable */
  int i;				/* index and counter */

/* do error checks */
  if(lst == NULL)			/* no list to encode? */
   { *len = NOINBUF;
     return((char *)NULL);
   }
  if(lst->len > SNMPMAXVARS)		/* too many variables */
   { *len = TOOMANYVARS;
     return((char *)NULL);
   }

/* build the variable list */
  for(i=(lst->len-1); i >= 0; i--)
   { vlen = 0;				/* clear out variable length */
     if((current=bldvar(&vlen,current,buflen,(varbind *)&(lst->elem[i])))==NULL)
      { *len = vlen;
        return((char *)NULL);
      }
     else
      *len += vlen;
   }

/* prepend the length of the list */
  if((current = bldlen(len,current,buflen)) == NULL)
   return((char *)NULL);

/* now add the list identifier */
  if(*buflen <= 0)			/* if there is no more space */
   { *len = LENERR;
     return((char *)NULL);
   }
  else					/* there is still space */
   { *current = SEQID;			/* add list id */
     current--; (*buflen)--; (*len)++;	/* add byte */
   }

  return(current);
}
   
/* bldvar - build a variable */
char *
bldvar(len,current,buflen,v)
short *len;				/* length of encoding data */
char *current;				/* buffer to encode into */
short *buflen;				/* max length of buffer for encoding */
varbind *v;				/* variable to be encoded */
{ short nlen,vlen;			/* length of name, value */

/* do error checks */
  if(v == NULL)				/* no variable to build? */
   { *len = NOINBUF;
     return((char *)NULL);
   }

/* build the variable */
  nlen = vlen = 0;			/* zero out lengths */
  if((current = bldval(&vlen,current,buflen,(objval *)&(v->val))) == NULL)
   { *len = vlen;
     return((char *)NULL);
   }
  if((current = bldnm(&nlen,current,buflen,(objident *)&(v->name))) == NULL)
   { *len = nlen;
     return((char *)NULL);
   }
  (*len) += (nlen + vlen);

/* put in the length */
  if((current = bldlen(len,current,buflen)) == NULL)
   return((char *)NULL);

/* put in list identifier (variable is list of name and value) */
  if(*buflen <= 0)			/* if there is no more space */
   { *len = LENERR;
     return((char *)NULL);
   }
  else					/* there is still space */
   { *current = SEQID;			/* add list id */
     current--; (*buflen)--; (*len)++;	/* add byte */
   }

  return(current);
}

/* bldval - build a value */
char *
bldval(len,current,buflen,v)
short *len;				/* length of encoding data */
char *current;				/* buffer to encode into */
short *buflen;				/* max length of buffer for encoding */
objval *v;				/* value to be encoded */
{
/* do error checks */
  if(v == NULL)				/* if no value to encode */
   { *len = NOINBUF;
     return((char *)NULL);
   }

  switch(v->type) {
   case INT:				/* integer */
    if((current = bldint(len,current,buflen,(long *)&(v->value.intgr))) == NULL)
     return((char *)NULL);
    break;
   case STR:				/* octet string */
    if((current = bldstr(len,current,buflen,(strng *)&(v->value.str))) == NULL)
     return((char *)NULL);
    break;
   case OBJ:				/* object identifier */
    if((current = bldobj(len,current,buflen,(objident *)&(v->value.obj))) == NULL)
     return((char *)NULL);
    break;
   case EMPTY:				/* empty */
    if((current = bldnull(len,current,buflen)) == NULL)
     return((char *)NULL);
    break;
   case IPADD:				/* ipaddress */
    if((current=bldipadd(len,current,buflen,(struct in_addr *)&(v->value.ipadd))) == NULL)
     return((char *)NULL);
    break;
   case CNTR:				/* counter */
    if((current=bldcntr(len,current,buflen,(u_long *)&(v->value.cntr))) == NULL)
     return((char *)NULL);
    break;
   case GAUGE:				/* gauge */
    if((current=bldgauge(len,current,buflen,(u_long *)&(v->value.gauge))) == NULL)
     return((char *)NULL);
    break;
   case TIME:				/* time */
    if((current=bldtime(len,current,buflen,(u_long *)&(v->value.time))) == NULL)
     return((char *)NULL);
    break;
   case OPAQUE:				/* opaque */
    if((current=bldopqe(len,current,buflen,(strng *)&(v->value.opqe))) == NULL)
     return((char *)NULL);
    break;
   default:
    *len = TYP_UNKNOWN;
    return((char *)NULL);
  }
  return(current);
}
