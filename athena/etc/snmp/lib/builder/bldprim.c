#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/builder/bldprim.c,v 1.1 1994-09-18 12:56:28 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:15:08  snmpdev
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
**			bldprim.c
**
** Functions to build the SMI 'primitives': integer,octet string .... counter.
** All functions return the next octet that can be used for encoding,
** or NULL on error. In the event of an error, the length parameter
** will contain a negative error code.
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

#define OBJMASK		0x7f

/* bldint - build an integer */
char *
bldint(len,current,buflen,num)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
long *num;				/* number to be encoded */
{
/* do error checks */
  if(current == NULL)			/* no buffer for encoding */
   { *len = NOOUTBUF;
     return((char *)NULL);
   }
  if(num == NULL)			/* no number to encode? */
   { *len = NOINBUF;
     return((char *)NULL);
   }

/* encode the integer */
  if(abs((int)(*num)) >= 0 && abs((int)(*num)) < 128) /* fits in one byte? */
   if(*buflen >= 3)			/* if there is space */
    { *num = (long)htonl((u_long)(*num)); /* convert number */
      *current = ((char *)num)[sizeof(long)-1]; /* put in value */
      current--;			/* move back one byte */
      *current = (LENMASK & 0x01);	/* add length */
      current--;			/* move back one byte */
      *current = INTID;			/* add id */
      current--;			/* move back one byte */
      (*buflen) -= 3; (*len) += 3;
    }
   else					/* no space */
    { *len = LENERR;			/* indicate lack of space */
      return((char *)NULL);
    }
  else
   if(abs((int)(*num)) >= 128 && abs((int)(*num)) < 32768) /* two bytes? */
    if(*buflen >= 4)			/* if there is space */
     { *num = (long)htonl((u_long)(*num)); /* convert number */
       *current = ((char *)num)[sizeof(long)-1]; /* put in value */
       current--;			/* move back one byte */
       *current = ((char *)num)[sizeof(long)-2]; /* put in value */
       current--;			/* move back one byte */
       *current = (LENMASK & 0x02);	/* add length */
       current--;			/* move back one byte */
       *current = INTID;			/* add id */
       current--;			/* move back one byte */
       (*buflen) -= 4; (*len) += 4;
     }
    else					/* no space */
     { *len = LENERR;			/* indicate lack of space */
       return((char *)NULL);
     }
   else
    if(abs((int)(*num)) >= 32768 && abs((int)(*num)) < 8388608)/* three? */ 
     if(*buflen >= 5)			/* if there is space */
      { *num = (long)htonl((u_long)(*num)); /* convert number */
        *current = ((char *)num)[sizeof(long)-1]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(long)-2]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(long)-3]; /* put in value */
        current--;			/* move back one byte */
        *current = (LENMASK & 0x03);	/* add length */
        current--;			/* move back one byte */
        *current = INTID;			/* add id */
        current--;			/* move back one byte */
        (*buflen) -= 5; (*len) += 5;
      }
     else					/* no space */
      { *len = LENERR;			/* indicate lack of space */
        return((char *)NULL);
      }
    else
     if(abs((int)(*num)) >= 8388608 && abs((int)(*num)) < 2147483647) /* four */
      if(*buflen >= 6)			/* if there is space */
       { *num = (long)htonl((u_long)(*num)); /* convert number */
         *current = ((char *)num)[sizeof(long)-1]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(long)-2]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(long)-3]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(long)-4]; /* put in value */
         current--;			/* move back one byte */
         *current = (LENMASK & 0x04);	/* add length */
         current--;			/* move back one byte */
         *current = INTID;			/* add id */
         current--;			/* move back one byte */
         (*buflen) -= 6; (*len) += 6;
       }
      else					/* no space */
       { *len = LENERR;			/* indicate lack of space */
         return((char *)NULL);
       }
     else
      { *len = TOOLONG;
	return((char *)NULL);
      }

/* done. return */
  *num = ntohl(*num);			/* get number back to correct order */
  return(current);
}

/* bldstr - build a string */
char *
bldstr(len,current,buflen,str)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
strng *str;				/* string to be encoded */
{ short slen;				/* string length */

  if(current == NULL)			/* no structure for encoding? */
   { *len = NOOUTBUF;			/* no output buffer */
     return((char *)NULL);
   }
  if(str == NULL)			/* no string to encode? */
   { *len = NOINBUF;			/* no input buffer */
     return((char *)NULL);
   }

/* encode the string */
  if(*buflen < str->len)		/* no space? */
   { *len = LENERR;			/* length error */
     return((char *)NULL);
   }
  current -= (str->len-1);
  (*buflen) -= str->len;
  slen = str->len;
  bcopy(str->str,current,(int)(str->len));
  current--;

/* put in the length of the string */
  if((current = bldlen(&slen,current,buflen)) == NULL)
   { *len = slen;
     return((char *)NULL);
   }
  *len += slen;				/* add length of string */

  if(*buflen < 1)			/* if no space to add id */
   { *len = LENERR;
     return((char *)NULL);
   }
  else
   { *current = STRID;
     (*buflen)--; current--; (*len)++;
   }
  return(current);
}

/* bldobj - build an object identifier */
char *
bldobj(len,current,buflen,obj)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
objident *obj;				/* string to be encoded */
{ int i,j;				/* indices and counters */
  long objcmp[SNMPMXID];		/* buffer to construct object id */
  short objlen;				/* length of object buffer used */

  if(current == NULL)			/* no structure for encoding? */
   { *len = NOOUTBUF;			/* no output buffer */
     return((char *)NULL);
   }
  if(obj == NULL)			/* no string to encode? */
   { *len = NOINBUF;			/* no input buffer */
     return((char *)NULL);
   }
  objlen = 0;

/*
   Object identifiers get encoded in three steps: 1) combine first and
   second components into one, 2) encode each component into possibly
   multipart subidentifiers 3) stuff subidentifiers into buffer.
*/
/* combine first and second components */
  objcmp[0] = obj->cmp[0] * 40 + obj->cmp[1]; /* first 'subidentifier' */
  for(i=2; i < obj->ncmp; i++)		/* other 'subidentifiers' */
   objcmp[i-1] = obj->cmp[i];

/* split into components */
  for(i=(obj->ncmp-2); i >= 0; i--)	/* foreach subcomponent */
   { if(*buflen < 1)			/* no more space? */
      { *len = LENERR;
        return((char *)NULL);
      }
     *current = (objcmp[i] & OBJMASK);
     current--; (*buflen)--; (objlen)++;
     objcmp[i] = (objcmp[i] >> 7);
     for(j=0; j < 3; j++)
      if(objcmp[i] != 0)
/* the bitshifts are to extract the parts of the component in order */
       { if(*buflen < 1)
          { *len = LENERR;
	    return((char *)NULL);
          }
         *current = ((objcmp[i] & OBJMASK) | 0x80);
         current--; (*buflen)--; (objlen)++;
         objcmp[i] = (objcmp[i] >> 7);
       }
      else
       break;
   }

/* put in length and identifier */
  if((current = bldlen(&objlen,current,buflen)) == NULL)
   { *len = objlen;
     return((char *)NULL);
   }
  *len += objlen;

  if(*buflen < 1)			/* if no space to add id */
   { *len = LENERR;
     return((char *)NULL);
   }
  else
   { *current = OBJID;
     (*buflen)--; current--; (*len)++;
   }

  return(current);
}

/* bldnull - build a null */
char *
bldnull(len,current,buflen)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
{
/* do error checks */
  if(current == NULL)			/* no structure for encoding? */
   { *len = NOOUTBUF;			/* no output buffer */
     return((char *)NULL);
   }

  if(*buflen < 2)
   { *len = LENERR;
     return((char *)NULL);
   }
 
/* encode a null */
  current--;
  current[0] = NULLID;			/* identifier */
  current[1] = 0x00;			/* null */
  (*len) += 2; (*buflen) -= 2;
  current--;

  return(current);
}

/* bldipadd - build ipaddress */
char *
bldipadd(len,current,buflen,add)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
struct in_addr *add;			/* address to be encoded */
{ short alen;				/* address length */

  if(current == NULL)			/* no structure for encoding? */
   { *len = NOOUTBUF;			/* no output buffer */
     return((char *)NULL);
   }
  if(add == NULL)			/* no string to encode? */
   { *len = NOINBUF;			/* no input buffer */
     return((char *)NULL);
   }

/* encode the address */
  if(*buflen < sizeof(struct in_addr))	/* no space? */
   { *len = LENERR;			/* length error */
     return((char *)NULL);
   }
  current -= (sizeof(struct in_addr)-1);
  (*buflen) -= sizeof(struct in_addr);
  alen = sizeof(struct in_addr);
  bcopy((char *)&(add->s_addr),current,sizeof(struct in_addr));
  current--;

/* put in the length of the string */
  if((current = bldlen(&alen,current,buflen)) == NULL)
   { *len = alen;
     return((char *)NULL);
   }
  *len += alen;				/* add length of string */

  if(*buflen < 1)			/* if no space to add id */
   { *len = LENERR;
     return((char *)NULL);
   }
  else
   { *current = IPID;
     (*buflen)--; current--; (*len)++;
   }
  return(current);
}

/* bldcntr - build a counter */
char *
bldcntr(len,current,buflen,num)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
u_long *num;				/* number to be encoded */
{
/* do error checks */
  if(current == NULL)			/* no buffer for encoding */
   { *len = NOOUTBUF;
     return((char *)NULL);
   }
  if(num == NULL)			/* no number to encode? */
   { *len = NOINBUF;
     return((char *)NULL);
   }

/* encode the integer */
  if(*num < 128)			/* fits in one byte? */
   if(*buflen >= 3)			/* if there is space */
    { *num = htonl(*num);		/* convert number */
      *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
      current--;			/* move back one byte */
      *current = (LENMASK & 0x01);	/* add length */
      current--;			/* move back one byte */
      *current = COUNTERID;		/* add id */
      current--;			/* move back one byte */
      (*buflen) -= 3; (*len) += 3;
    }
   else					/* no space */
    { *len = LENERR;			/* indicate lack of space */
      return((char *)NULL);
    }
  else
   if(*num >= 128 && *num < 32768) /* two bytes? */
    if(*buflen >= 4)			/* if there is space */
     { *num = htonl(*num);		/* convert number */
       *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
       current--;			/* move back one byte */
       *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
       current--;			/* move back one byte */
       *current = (LENMASK & 0x02);	/* add length */
       current--;			/* move back one byte */
       *current = COUNTERID;		/* add id */
       current--;			/* move back one byte */
       (*buflen) -= 4; (*len) += 4;
     }
    else					/* no space */
     { *len = LENERR;			/* indicate lack of space */
       return((char *)NULL);
     }
   else
    if(*num >= 32768 && *num < 8388608)/* three? */ 
     if(*buflen >= 5)			/* if there is space */
      { *num = htonl(*num);		/* convert number */
        *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
        current--;			/* move back one byte */
        *current = (LENMASK & 0x03);	/* add length */
        current--;			/* move back one byte */
        *current = COUNTERID;		/* add id */
        current--;			/* move back one byte */
        (*buflen) -= 5; (*len) += 5;
      }
     else					/* no space */
      { *len = LENERR;			/* indicate lack of space */
        return((char *)NULL);
      }
    else
     if(*num >= 8388608 && *num < (u_long)2147483648) /* four */
      if(*buflen >= 6)			/* if there is space */
       { *num = htonl(*num);		/* convert number */
         *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
         current--;			/* move back one byte */
         *current = (LENMASK & 0x04);	/* add length */
         current--;			/* move back one byte */
         *current = COUNTERID;		/* add id */
         current--;			/* move back one byte */
         (*buflen) -= 6; (*len) += 6;
       }
      else				/* no space */
       { *len = LENERR;			/* indicate lack of space */
         return((char *)NULL);
       }
     else
      if(*num >= (u_long)2147483648 && *num <= (u_long)4294967295) /* five */
       if(*buflen >= 7)			/* if there is space */
        { *num = htonl(*num);		/* convert number */
          *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
          current--;			/* move back one byte */
          *current = 0x00;
          current--;			/* move back one byte */
          *current = (LENMASK & 0x05);	/* add length */
          current--;			/* move back one byte */
          *current = COUNTERID;			/* add id */
          current--;			/* move back one byte */
          (*buflen) -= 7; (*len) += 7;
        }
       else				/* no space */
        { *len = LENERR;		/* indicate lack of space */
          return((char *)NULL);
        }
      else
       { *len = TYPELONG;
 	 return((char *)NULL);
       }

/* done. return */
  *num = ntohl(*num);			/* get number back to correct order */
  return(current);
}

/* bldgauge - build a counter */
char *
bldgauge(len,current,buflen,num)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
u_long *num;				/* number to be encoded */
{
/* do error checks */
  if(current == NULL)			/* no buffer for encoding */
   { *len = NOOUTBUF;
     return((char *)NULL);
   }
  if(num == NULL)			/* no number to encode? */
   { *len = NOINBUF;
     return((char *)NULL);
   }

/* encode the integer */
  if(*num < 128)			/* fits in one byte? */
   if(*buflen >= 3)			/* if there is space */
    { *num = htonl(*num);		/* convert number */
      *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
      current--;			/* move back one byte */
      *current = (LENMASK & 0x01);	/* add length */
      current--;			/* move back one byte */
      *current = GAUGEID;		/* add id */
      current--;			/* move back one byte */
      (*buflen) -= 3; (*len) += 3;
    }
   else					/* no space */
    { *len = LENERR;			/* indicate lack of space */
      return((char *)NULL);
    }
  else
   if(*num >= 128 && *num < 32768) /* two bytes? */
    if(*buflen >= 4)			/* if there is space */
     { *num = htonl(*num);		/* convert number */
       *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
       current--;			/* move back one byte */
       *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
       current--;			/* move back one byte */
       *current = (LENMASK & 0x02);	/* add length */
       current--;			/* move back one byte */
       *current = GAUGEID;		/* add id */
       current--;			/* move back one byte */
       (*buflen) -= 4; (*len) += 4;
     }
    else					/* no space */
     { *len = LENERR;			/* indicate lack of space */
       return((char *)NULL);
     }
   else
    if(*num >= 32768 && *num < 8388608)/* three? */ 
     if(*buflen >= 5)			/* if there is space */
      { *num = htonl(*num);		/* convert number */
        *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
        current--;			/* move back one byte */
        *current = (LENMASK & 0x03);	/* add length */
        current--;			/* move back one byte */
        *current = GAUGEID;		/* add id */
        current--;			/* move back one byte */
        (*buflen) -= 5; (*len) += 5;
      }
     else					/* no space */
      { *len = LENERR;			/* indicate lack of space */
        return((char *)NULL);
      }
    else
     if(*num >= 8388608 && *num < (u_long)2147483648) /* four */
      if(*buflen >= 6)			/* if there is space */
       { *num = htonl(*num);		/* convert number */
         *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
         current--;			/* move back one byte */
         *current = (LENMASK & 0x04);	/* add length */
         current--;			/* move back one byte */
         *current = GAUGEID;			/* add id */
         current--;			/* move back one byte */
         (*buflen) -= 6; (*len) += 6;
       }
      else					/* no space */
       { *len = LENERR;			/* indicate lack of space */
         return((char *)NULL);
       }
     else
      if(*num >= (u_long)2147483648 && *num <= (u_long)4294967295) /* five */
       if(*buflen >= 7)			/* if there is space */
        { *num = htonl(*num);		/* convert number */
          *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
          current--;			/* move back one byte */
          *current = 0x00;
          current--;			/* move back one byte */
          *current = (LENMASK & 0x05);	/* add length */
          current--;			/* move back one byte */
          *current = GAUGEID;			/* add id */
          current--;			/* move back one byte */
          (*buflen) -= 7; (*len) += 7;
        }
       else				/* no space */
        { *len = TYPELONG;		/* indicate lack of space */
          return((char *)NULL);
        }
      else
       { *len = TOOLONG;
 	 return((char *)NULL);
       }

/* done. return */
  *num = ntohl(*num);			/* get number back to correct order */
  return(current);
}

/* bldtime - build time */
char *
bldtime(len,current,buflen,num)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
u_long *num;				/* number to be encoded */
{
/* do error checks */
  if(current == NULL)			/* no buffer for encoding */
   { *len = NOOUTBUF;
     return((char *)NULL);
   }
  if(num == NULL)			/* no number to encode? */
   { *len = NOINBUF;
     return((char *)NULL);
   }

/* encode the integer */
  if(*num < 128)			/* fits in one byte? */
   if(*buflen >= 3)			/* if there is space */
    { *num = htonl(*num);		/* convert number */
      *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
      current--;			/* move back one byte */
      *current = (LENMASK & 0x01);	/* add length */
      current--;			/* move back one byte */
      *current = TIMEID;		/* add id */
      current--;			/* move back one byte */
      (*buflen) -= 3; (*len) += 3;
    }
   else					/* no space */
    { *len = LENERR;			/* indicate lack of space */
      return((char *)NULL);
    }
  else
   if(*num >= 128 && *num < 32768) /* two bytes? */
    if(*buflen >= 4)			/* if there is space */
     { *num = htonl(*num);		/* convert number */
       *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
       current--;			/* move back one byte */
       *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
       current--;			/* move back one byte */
       *current = (LENMASK & 0x02);	/* add length */
       current--;			/* move back one byte */
       *current = TIMEID;		/* add id */
       current--;			/* move back one byte */
       (*buflen) -= 4; (*len) += 4;
     }
    else					/* no space */
     { *len = LENERR;			/* indicate lack of space */
       return((char *)NULL);
     }
   else
    if(*num >= 32768 && *num < 8388608)/* three? */ 
     if(*buflen >= 5)			/* if there is space */
      { *num = htonl(*num);		/* convert number */
        *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
        current--;			/* move back one byte */
        *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
        current--;			/* move back one byte */
        *current = (LENMASK & 0x03);	/* add length */
        current--;			/* move back one byte */
        *current = TIMEID;		/* add id */
        current--;			/* move back one byte */
        (*buflen) -= 5; (*len) += 5;
      }
     else					/* no space */
      { *len = LENERR;			/* indicate lack of space */
        return((char *)NULL);
      }
    else
     if(*num >= 8388608 && *num < (u_long)2147483648) /* four */
      if(*buflen >= 6)			/* if there is space */
       { *num = htonl(*num);		/* convert number */
         *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
         current--;			/* move back one byte */
         *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
         current--;			/* move back one byte */
         *current = (LENMASK & 0x04);	/* add length */
         current--;			/* move back one byte */
         *current = TIMEID;			/* add id */
         current--;			/* move back one byte */
         (*buflen) -= 6; (*len) += 6;
       }
      else					/* no space */
       { *len = LENERR;			/* indicate lack of space */
         return((char *)NULL);
       }
     else
      if(*num >= (u_long)2147483648 && *num <= (u_long)4294967295) /* five */
       if(*buflen >= 7)			/* if there is space */
        { *num = htonl(*num);		/* convert number */
          *current = ((char *)num)[sizeof(u_long)-1]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-2]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-3]; /* put in value */
          current--;			/* move back one byte */
          *current = ((char *)num)[sizeof(u_long)-4]; /* put in value */
          current--;			/* move back one byte */
          *current = 0x00;
          current--;			/* move back one byte */
          *current = (LENMASK & 0x05);	/* add length */
          current--;			/* move back one byte */
          *current = TIMEID;		/* add id */
          current--;			/* move back one byte */
          (*buflen) -= 7; (*len) += 7;
        }
       else				/* no space */
        { *len = LENERR;		/* indicate lack of space */
          return((char *)NULL);
        }
      else
       { *len = TYPELONG;
 	 return((char *)NULL);
       }

/* done. return */
  *num = ntohl(*num);			/* get number back to correct order */
  return(current);
}

/* bldopqe - build an opaque */
char *
bldopqe(len,current,buflen,str)
short *len;				/* length of encoded data */
char *current;				/* buffer into which to encode */
short *buflen;				/* max length of buffer for encoding */
strng *str;				/* string to be encoded */
{ short slen;				/* string length */

  if(current == NULL)			/* no structure for encoding? */
   { *len = NOOUTBUF;			/* no output buffer */
     return((char *)NULL);
   }
  if(str == NULL)			/* no string to encode? */
   { *len = NOINBUF;			/* no input buffer */
     return((char *)NULL);
   }

/* encode the string */
  if(*buflen < str->len)		/* no space? */
   { *len = LENERR;			/* length error */
     return((char *)NULL);
   }
  current -= (str->len-1);
  (*buflen) -= str->len;
  slen = str->len;
  bcopy(str->str,current,(int)(str->len));
  current--;

/* put in the length of the string */
  if((current = bldlen(&slen,current,buflen)) == NULL)
   { *len = slen;
     return((char *)NULL);
   }
  *len += slen;				/* add length of string */

  if(*buflen < 1)			/* if no space to add id */
   { *len = LENERR;
     return((char *)NULL);
   }
  else
   { *current = OPAQUEID;
     (*buflen)--; current--; (*len)++;
   }
  return(current);
}
