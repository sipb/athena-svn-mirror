#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/parser/prselen.c,v 1.1 1994-09-18 12:56:16 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:28  snmpdev
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
**		prselen.c
**
** File contains functions to parse the length of an asn structure -
** the functions that parse the 'asnlen', 'otherlen' and 'longlen'
** non-terminals in the grammar.
**
** All functions return a pointer to the octet immediately after the 
** last one they parsed.
**
**
*******************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"
#include "../../h/asn.h"

/* asnlen - parse the 'asnlen' non-terminal */
char *
asnlen(msglen,current,length)
short *msglen;					/* ptr to length of message */
char *current;					/* current octet */
u_short *length;				/* ptr to length */
{
  *length = 0;					/* make sure length is 0 */
  if(*msglen <= 0)				/* if we're out of message */
   { *msglen = LENERR;				/* record error */
     return((char *)NULL);
   }

  if((*current & ISLONG) == 0)			/* is a short length */
   { *length = (unsigned short)(*current & LENMASK); /* get short length */
     current++; (*msglen)--;			/* move on */
     return(current);
   }
  else						/* not a short length */
   { current = otherlen(current,msglen,length);	/* parse an 'otherlen' */
     return(current);
   }
}

/* otherlen - parse the 'otherlen' non-terminal */
char *
otherlen(current,msglen,length)
char *current;					/* current octet */
short *msglen;					/* ptr to length of message */
u_short *length;				/* ptr to length */
{ short lenlen;					/* length of length */
/* do some error checking first */
  if(*msglen <= 0)				/* if we're out of message */
   { *msglen = LENERR;				/* record error */
     return((char *)NULL);
   }
  if(*current == INDFTYP)			/* indefinite length type? */
   { *msglen = UNSPLEN;				/* unsupported length type */
     return((char *)NULL);
   }

/*
   at this point, a function to parse the 'longlen' non-terminal should
   be called. However, in the interests of efficiency, an optimization has
   been performed by doing the 'longlen' parsing here. The following
   code can be viewed as the inline substitution of the function to
   perform the parsing of 'longlen'.
*/
  lenlen = (unsigned short)(*current & LENMASK); /* get length of the length */
  if(lenlen > 2)				/* length too long */
   { *msglen = PKTLENERR;
     return((char *)NULL);
   }
  current++; (*msglen)--;			/* on to actual length */
  *length = 0;					/* clear length */
  while(lenlen > 0)				/* iterate to accumulate len */
   { if(*msglen <= 0)				/* if msglen exceeded */
      { *msglen = LENERR;				/* record error */
        return((char *)NULL);
      }
     *length = (*length * 256)+(0x00ff & (unsigned short)(*current));
     lenlen--;					/* one more byte processed */
     current++; (*msglen)--;
   }
  return(current);
}
