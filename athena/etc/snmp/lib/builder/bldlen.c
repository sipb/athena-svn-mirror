#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/bldlen.c,v 1.1 1994-09-18 12:56:30 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:14:54  snmpdev
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
**			bldlen.c
**
** Function to add a length to an asn structure. Returns next octet
** that can be used for encoding, or NULL on error. In the event
** of an error, the length parameter will contain a negative
** error code.
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

char *
bldlen(len,current,buflen)
short *len;				/* length to be added */
char *current;				/* the buffer for encoding */
short *buflen;				/* max length of buffer */
{ short tlen;				/* temporary length */

/* do error checks */
  if(len == NULL)			/* if no length to add */
   return((char *)NULL);
  if(current == NULL)			/* no place to encode */
   { *len = NOOUTBUF;
     return((char *)NULL);
   }

  tlen = *len;

/* now add the length */
  if(tlen < 128)			/* length will fit in short length */
   if(*buflen < 1)			/* no more space? */
    { *len = LENERR;
      return((char *)NULL);
    }
   else					/* there is space */
    { tlen = (short)htons((u_short)(tlen));
      *current = LENMASK & ((char *)&tlen)[sizeof(short)-1];
      current--; (*len)++; (*buflen)--;
    }
  else					/* more than short length */
   if(tlen >= 128 && tlen < 256)		/* one long length byte */
    if(*buflen < 2)
     { *len = LENERR;
       return((char *)NULL);
     }
    else				/* there is space */
     { tlen = (short)htons((u_short)(tlen));
       *current = ((char *)&tlen)[sizeof(short)-1]; /* put in length */
       current--;
       *current = 0x81;			/* one byte of length */
       current--; (*buflen) -= 2; (*len) += 2;
     }
   else					/* more than one long length byte */
    if(tlen >= 256 && tlen < SNMPMAXPKT) /* two long length bytes */
     if(*buflen < 3)
      { *len = LENERR;
        return((char *)NULL);
      }
     else				/* there is space */
      { tlen = (short)htons((u_short)(tlen));
        *current = ((char *)&tlen)[sizeof(short)-1]; /* put in length */
        current--;
        *current = ((char *)&tlen)[sizeof(short)-2]; /* put in length */
        current--;
        *current = 0x82;		/* two bytes of length */
        current--; (*buflen) -= 3; (*len) += 3;
      }
    else				/* more than max length */
     { *len = TOOLONG;
       return((char *)NULL);
     }
  return(current);
}
