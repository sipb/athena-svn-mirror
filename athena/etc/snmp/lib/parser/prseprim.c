#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/prseprim.c,v 1.2 1997-02-27 06:40:52 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18 12:56:15  cfields
 * Initial revision
 *
 * Revision 1.1  89/11/03  15:15:40  snmpdev
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
**			prseprim.c
**
** Functions to parse the SMI 'primitives'. All functions return a pointer
** to the next octet to be parsed, or NULL on error. If there is an error,
** msglen will contain a negative error return code.
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

extern char *malloc();			/* just to keep lint happy */

#define CONVMASK		(u_long)0xff
#define OBJMASK			(u_long)0x7f

/* prseint - parse an integer */
char *
prseint(msglen,current,buflen,num)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
long *num;				/* number to be parsed into */
{ u_short length;			/* length of integer */
  u_short oldlen;			/* to differentiate 1st byte from rest*/

/* error checks */
  if(*msglen <= 0 || current == NULL)	/* unexpected end of message */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != INTID)			/* if not an integer */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get integer length. It should be <= sizeof(long) octets */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length > sizeof(long) && (length != sizeof(long)+1 || *current != 0x00))
   { *msglen = TOOLONG;
     return((char *)NULL);
   }

/* accumulate integer */
  *num = 0;				/* clear number */
  oldlen = length;			/* save original length */
  while(length > 0)
   { if(*msglen <= 0)			/* if end of message */
      { *msglen = LENERR;		/* unexpected end of message */
        return((char *)NULL);
      }
     if(*buflen <= 0)
      { *msglen = OUTERR;
        return((char *)NULL);
      }
/* this will put the value into host byte order as well as extract it */
     if(oldlen == length)		/* if first byte */
      *num = (*num * 256)+ *current;
     else				/* not first byte, so ignore sign */
      *num = (*num * 256) + ((u_long)(*current) & CONVMASK);
     length--; (*buflen)--;
     current++; (*msglen)--;
   }

/* done. return */
  return(current);
}

/* prsestr - parse the 'string' non-terminal */
char *
prsestr(msglen,current,buflen,strg)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
strng *strg;				/* string (structure component */
{ u_short length;

/* do error checks */
  if(*msglen <= 0 || current == NULL)	/* end of message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != STRID)			/* not a string? */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;

/* get length of string */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return((char *)NULL);

/* get string */
  if(*msglen < length || *buflen < length)
   { *msglen = TOOLONG;
     return((char *)NULL);
   }
  if(strg->str != NULL)
   free(strg->str);			/* just in case */
  if((strg->str = (char *)malloc(length+1)) == NULL)
   { *msglen = EMALLOC;
     return((char *)NULL);
   }
  memcpy(strg->str,current,(int)length);
  strg->str[length] = '\0';		/* null terminate */
  strg->len = length;
  current += length;
  *msglen -= length; *buflen -= length;

  return(current);
}

/* prsenull - parse the 'empty' */
char *
prsenull(msglen,current)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
{ u_short length;

/* do error checks */
  if(*msglen <= 0 || current == NULL)	/* unexpected end of message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != NULLID)
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get length. should be zero */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length != 0)
   { *msglen = TYPERR;			/* type error */
     return((char *)NULL);
   }
  return(current);
}

/* prseobj - parse an object identifier */
char *
prseobj(msglen,current,buflen,obj)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
objident *obj;				/* the object identifier */
{ u_short length;			/* length */

/* do error checks */
  if(*msglen <= 0 || current == NULL)	/* end of incoming message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != OBJID)			/* if not an object id */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;

/* get length */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length <= 0)
   return(current);

/* get object id */
/* we take advantage of the fact that first two components are small */
  obj->cmp[0] =	((u_long)(*current) & CONVMASK) / 40;
  obj->cmp[1] =	((u_long)(*current) & CONVMASK) -  obj->cmp[0] * 40;
  obj->ncmp = 2;			/* start at second component */
  obj->cmp[2] = 0;			/* zero out component */
  current++; (*msglen)--; (*buflen) -= 2; /* move on */ 
  length--;
  while(length > 0)			/* while there is still oids left */
   { if(*msglen <= 0)			/* if there is no space */
      { *msglen = LENERR;
        return((char *)NULL);
      }
     if(*buflen <= 0)			/* if there is no space */
      { *msglen = OUTERR;
        return((char *)NULL);
      }
     obj->cmp[obj->ncmp] += (*current & OBJMASK);
     if((*current & 0x80) == 0)		/* if this was 'last' component */
      obj->cmp[++(obj->ncmp)] = 0;	/* zero out next subident, inc cntr */
     else				/* not last */
      obj->cmp[obj->ncmp] *= 128;	/* shift 7 bits */
     current++; (*buflen)--; (*msglen)--; /* move on */
     length--;				/* one byte less */
   }
  
  return(current);
}

/* prseipadd - parse the 'ipadd' non-terminal */
char *
prseipadd(msglen,current,buflen,add)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
struct in_addr *add;			/* ip address (structure component) */
{ u_short length;			/* length */

/* do error checks */
  if(*msglen <= 0 || current == NULL)	/* end of incoming message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != IPID)			/* if not an object id */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;

/* get length */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length != sizeof(struct in_addr))	/* if wrong length for address */
   { *msglen = TYPERR;
     return((char *)NULL);
   }

/* get address */
  if(*msglen < length)			/* end of incoming message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*buflen < length)			/* end of outgoing buffer? */
   { *msglen = OUTERR;
     return((char *)NULL);
   }
  memset(&(add->s_addr),current,(int)length);
  current += length;
  *msglen -= length; *buflen -= length;

  return(current);
}

/* prsecntr - parse a counter */
char *
prsecntr(msglen,current,buflen,num)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
u_long *num;				/* number to be parsed into */
{ u_short length;			/* length of counter */
  u_short oldlen;			/* to differentiate 1st byte from rest*/

/* error checks */
  if(*msglen <= 0 || current == NULL)	/* unexpected end of message */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != COUNTERID)		/* if not a counter */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get counter length. It should be <= sizeof(long) octets */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length > sizeof(long) && (length != sizeof(long)+1 || *current != 0x00))
   { *msglen = TOOLONG;
     return((char *)NULL);
   }

/* accumulate integer */
  *num = 0;				/* clear number */
  oldlen = length;			/* save original length */
  while(length > 0)
   { if(*msglen <= 0)			/* if end of message */
      { *msglen = LENERR;		/* unexpected end of message */
        return((char *)NULL);
      }
     if(*buflen <= 0)
      { *msglen = OUTERR;
        return((char *)NULL);
      }
/* this will put the value into host byte order as well as extract it */
     if(oldlen == length)		/* if first byte */
      *num = (*num * 256)+ *current;
     else				/* not first byte, so ignore sign */
      *num = (*num * 256) + ((u_long)(*current) & CONVMASK);
     length--; (*buflen)--;
     current++; (*msglen)--;
   }

/* done. return */
  return(current);
}

/* prsegauge - parse a gauge */
char *
prsegauge(msglen,current,buflen,num)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
u_long *num;				/* number to be parsed into */
{ u_short length;			/* length of gauge */
  u_short oldlen;			/* to differentiate 1st byte from rest*/

/* error checks */
  if(*msglen <= 0 || current == NULL)	/* unexpected end of message */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != GAUGEID)		/* if not an integer */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get gauge length. It should be <= sizeof(long) octets */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length > sizeof(long) && (length != sizeof(long)+1 || *current != 0x00))
   { *msglen = TOOLONG;
     return((char *)NULL);
   }

/* accumulate integer */
  *num = 0;				/* clear number */
  oldlen = length;			/* save original length */
  while(length > 0)
   { if(*msglen <= 0)			/* if end of message */
      { *msglen = LENERR;		/* unexpected end of message */
        return((char *)NULL);
      }
     if(*buflen <= 0)
      { *msglen = OUTERR;
        return((char *)NULL);
      }
/* this will put the value into host byte order as well as extract it */
     if(oldlen == length)		/* if first byte */
      *num = (*num * 256)+ *current;
     else				/* not first byte, so ignore sign */
      *num = (*num * 256) + ((u_long)(*current) & CONVMASK);
     length--; (*buflen)--;
     current++; (*msglen)--;
   }

/* done. return */
  return(current);
}

/* prsetime - parse a time value */
char *
prsetime(msglen,current,buflen,time)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
u_long *time;				/* time to be parsed into */
{ u_short length;			/* length of time value */
  u_short oldlen;			/* to differentiate 1st byte from rest*/

/* error checks */
  if(*msglen <= 0 || current == NULL)	/* unexpected end of message */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != TIMEID)		/* if not an integer */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;		/* move on */

/* get time length. It should be <= sizeof(u_long) octets */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return(current);
  if(length > sizeof(long) && (length != sizeof(long)+1 || *current != 0x00))
   { *msglen = TOOLONG;
     return((char *)NULL);
   }

/* accumulate integer */
  *time = 0;				/* clear number */
  oldlen = length;			/* save original length */
  while(length > 0)
   { if(*msglen <= 0)			/* if end of message */
      { *msglen = LENERR;		/* unexpected end of message */
        return((char *)NULL);
      }
     if(*buflen <= 0)
      { *msglen = OUTERR;
        return((char *)NULL);
      }
/* this will put the value into host byte order as well as extract it */
     if(oldlen == length)		/* if first byte */
      *time = (*time * 256)+ *current;
     else				/* not first byte, so ignore sign */
      *time = (*time * 256) + ((u_long)(*current) & CONVMASK);
     length--; (*buflen)--;
     current++; (*msglen)--;
   }

/* done. return */
  return(current);
}

/* prseopaque - parse the 'string' non-terminal */
char *
prseopaque(msglen,current,buflen,opqe)
short *msglen;				/* length of incoming message */
char *current;				/* incoming message */
short *buflen;				/* size of outgoing structure */
strng *opqe;				/* opaque (structure component */
{ u_short length;

/* do error checks */
  if(*msglen <= 0 || current == NULL)	/* end of message? */
   { *msglen = LENERR;
     return((char *)NULL);
   }
  if(*current != OPAQUEID)			/* not an opaque? */
   { *msglen = TYP_MISMATCH;
     return((char *)NULL);
   }
  current++; (*msglen)--;

/* get length of string */
  if((current = asnlen(msglen,current,&length)) == NULL)
   return((char *)NULL);

/* get string */
  if(*msglen < length || *buflen < length)
   { *msglen = TOOLONG;
     return((char *)NULL);
   }
  if(opqe->str != NULL)
   free(opqe->str);			/* just in case */
  if((opqe->str = (char *)malloc(length+1)) == NULL)
   { *msglen = EMALLOC;
     return((char *)NULL);
   }
  memset(opqe->str,current,(int)length);
  opqe->str[length] = '\0';		/* null terminate */
  opqe->len = length;
  current += length;
  *msglen -= length; *buflen -= length;

  return(current);
}
