#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/varcvt.c,v 1.1 1994-09-18 12:56:48 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:16:28  snmpdev
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
**			varcvt.c
**
** This is the source file for the translator that converts SNMP variables
** from symbolic to numeric form and vice versa. This converter will
** convert variables of form:
**	<symbolic prefix><dotted remainder>
**	<numeric prefix><remainder of bytes>
** and
**	<numeric prefix><remainder of bytes>
**	<symbolic prefix><dotted remainder>
**
**
*****************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#include "/usr/netinclude/sys/types.h"
#include <ctype.h>
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"
#include "../../h/varcvt.h"

/* put this here to prevent it from interfering with other includes */
#define CONVMASK	(u_int)0xff

/* for converting numbers in string form to integers (u_shorts) */
#define decvt(c)	(u_short)( c - '0' )

#define VARMAX		64		/* maximum variables */
#define VARLN		256		/* max length of a config file line */
#define VARSYM		32		/* length of symbolic pfx buffer */	

/* the tree cell */
/*
   note that numeric encodings are implicit in the indices to children:
   the numeric encoding '1' is index 0, numeric encoding '5' is index 4
   numeric encoding n is index n-1.
*/
typedef struct cll {
	char *		nm;		/* symbolic representation */
	struct cll *	chld[VARMAX];	/* children */
} cvtcell;

cvtcell top;				/* the top of the tree */
static short initialized = 0;		/* tree initialized? */

short init_peer();

/* init_cvt - initialize variable converter */
short
init_cvt()
{ char line[VARLN];		/* input line */
  cvtcell *current;		/* current position in tree */
  FILE *cfgfl;			/* configuration file */
  char sympfx[VARSYM];		/* current portion of symbolic prefix */
  short symlen;			/* lengths of prefix portions */
  u_short digit;		/* to convert hex to decimal */
  char *malloc();		/* to allocate space dynamically */
  char *strncpy();		/* to copy strings */
  int i,j;			/* indices and counters */

  initialized = 1;		/* tree is now initialized */

  if((cfgfl = fopen(VARINITFL,"r")) == NULL) /* open init file */
   return(NOINITFL);

/* just to make sure.... */
  top.nm = (char *)NULL;
  for(i=0; i < VARMAX; i++)
   top.chld[i] = (cvtcell *)NULL;

  while(fgets(line,VARLN-1,cfgfl) != NULL) /* while not end of file */
   { current = &top;		/* beginning of tree */
     i = 0;			/* zero out i */

/* while not end of variable description (ie: end of line) */
     while(line[i] != '\n')
/* skip space until symbolic version of prefix */
      { for(; i < VARLN && (line[i] == ' ' || line[i] == '\t') &&
	      line[i] != '\n'; i++)
		;
	if(line[i] == '\n' || i == VARLN)
	 continue;
	symlen = 0;		/* symbolic prefix is 0 right now */
	for(; i < VARLN && !isspace(line[i]); i++)
         if(symlen < VARSYM-1)	/* if there is still space */
	  sympfx[symlen++] = line[i];	/* copy in character */
	 else			/* exceeded length of buffer */
	  { fprintf(stderr,"varcvt: internal symbolic buffer too small\n");
	    exit(1);
	  }
/* skip space until numeric version of prefix */
        for(; i < VARLN && (line[i] == ' ' || line[i] == '\t') &&
	      line[i] != '\n'; i++)
		;
        if(!isdigit(line[i]))
	 return(NONUM);
/* we know numeric prefix is 'd' or 'dd' or 'ddd' (ie: one to three digits) */
	if(isdigit(line[i+1]))	/* if 'dd' or 'ddd' */
         if(isdigit(line[i+2])) /* numeric prefix is 'ddd' */
	  { digit = decvt(line[i])*100 + decvt(line[i+1])*10 + decvt(line[i+2]);
	    i += 3;
	  }
	 else			/* numeric prefix is 'dd' */
	  { digit = decvt(line[i])*10 + decvt(line[i+1]);
	    i += 2;
	  }
	else			/* numeric prefix is 'd' */
	 digit = decvt(line[i++]);
	if(digit < 1 || digit > VARMAX)
	 return(VARBOUNDS);

/* we now have both symbolic and numeric prefixes of variables */
	if(current->chld[digit-1] == NULL)
	 { if((current->chld[digit-1]=(cvtcell *)malloc(sizeof(cvtcell)))==NULL)
	    return(EMALLOC);
	   current = current->chld[digit-1];
	   if((current->nm = (char *)malloc((u_int)(symlen+1))) == NULL)
	    return(EMALLOC);
	   strncpy(current->nm,sympfx,symlen);
	   current->nm[symlen] = '\0'; /* null terminate */
	   for(j=0; j < VARMAX; j++)
	    current->chld[j] = (cvtcell *)NULL;
         }
	else
	 current = current->chld[digit-1];
      }				/* end variable description */
   }				/* end configuration file information */
  fclose(cfgfl);
  return(SNMP_OK);
}

/* symtonum - convert a given variable name to an object identifier */
short
symtonum(name,oid)
strng *name;			/* name to be converted */
objident *oid;			/* buffer for building converted name */
{ short rc;			/* return code */
  cvtcell *current;		/* to traverse tree */
  int i,j;			/* indices and counters */
  char *tstr;			/* to temporarily hold strings */
  char *strcpy();		/* to copy strings */
  long atol();
  
/* check that user didn't make any mistake with parameters */
  if(name == NULL)		/* no name to convert */
   return(NONAME);
  if(oid == NULL)		/* no place to return converted variable */
   return(NOBUF);

/* save input string */
  tstr = name->str;		/* save input string */
  name->str = NULL;
  if((name->str = (char *)malloc(name->len+1)) == NULL)
   return(EMALLOC);
  strncpy(name->str,tstr,(int)(name->len));
  name->str[name->len] = '\0';

/* OK, start looking */
  oid->ncmp = 0;			/* no components yet */
  current = &top;		/* start at top of tree */
  if(!initialized)
   if((rc = init_cvt()) < 0)	/* initialize converter if necessary */
    return(rc);

/* body of program */
  j = 1; 			/* skip first '_' */
  while(j < name->len)
   if(current != NULL)	/* while haven't fallen off tree */
    { for(i=0; i < VARMAX; i++)
       if(current->chld[i] != NULL && 
	  strncmp(current->chld[i]->nm,(name->str+j),strlen(current->chld[i]->nm))
	   == 0)
        { oid->cmp[(oid->ncmp)++] = (u_long)(i+1);
          current = current->chld[i];	/* move on */
 	  break;
        }
       if(i == VARMAX)
        current = (cvtcell *)NULL;
       else
        { for(;j < name->len && name->str[j] != '_'; j++) /* skip to next _ */
 	 		;
          j++;		/* move past '_' */
        }
    }
   else			/* fell off tree */
    { 
      while(j < name->len)	/* while not end of variable */
       { for(i=j; isdigit(name->str[i]) && name->str[i] != '\0' 
		  && name->str[i] != '.'; i++)
				;
	 if(name->str[i] != '\0' && name->str[i] != '.')
	  return(VARUNKNOWN);
	 name->str[i] = '\0';
	 (oid->cmp)[(oid->ncmp)++]=atol((name->str)+j);
	 j = i+1;		/* go to one after digits converted */
       }
    }

/* restore input string */
   free(name->str); name->str = NULL;
   name->str = tstr; tstr = NULL;
   return(SNMP_OK);
}

/* numtosym - convert an object identifier to a name */
short
numtosym(oid,convname)
objident *oid;			/* oid to be converted */
strng *convname;		/* buffer for building converted name */
{ char strbuf[VARLN];		/* buffer for building converted name */
  short rc;			/* return code */
  cvtcell *current;		/* to traverse tree */
  int i;			/* indices and counters */
  
/* check that user didn't make any mistake with parameters */
  if(oid == NULL)		/* no name to convert */
   return(NONAME);
  if(convname == NULL)		/* no place to return converted variable */
   return(NOBUF);

/* OK, start looking */
  convname->len = 0;		/* no name yet */
  if(convname->str != NULL)	/* if there is a buffer */
   free(convname->str);		/* free it */
  current = &top;		/* start at top of tree */
  if(!initialized)
   if((rc = init_cvt()) < 0)	/* initialize converter if necessary */
    return(rc);

/* body of program */
  for(i=0; i < oid->ncmp; i++)
   { strbuf[(convname->len)++] = '_';
     if(oid->cmp[i] > 0 && oid->cmp[i] < VARMAX && current->chld[(oid->cmp)[i]-1] != NULL)
      { strcpy((strbuf+convname->len),
	       current->chld[(oid->cmp)[i]-1]->nm);
        convname->len += strlen(current->chld[(oid->cmp)[i]-1]->nm);
        current = current->chld[(oid->cmp)[i]-1];
      }
     else
      break;
   }
  if(i < oid->ncmp)		/* if fell off tree */
/* foreach extra digit, add digit in string form to convname */
   for(;i < oid->ncmp;i++)
    { sprintf((strbuf+convname->len),"%d",(oid->cmp)[i]);
      convname->len = strlen(strbuf);
      strbuf[(convname->len)++] = '.';
    }
  if(strbuf[(convname->len)-1] == '.')
   strbuf[--(convname->len)] = '\0';	/* null terminate */
  if((convname->str = (char *)malloc(convname->len+1)) == NULL)
   return(EMALLOC);
  strncpy(convname->str,strbuf,(int)(convname->len));
  convname->str[convname->len] = '\0';
  return(SNMP_OK);
}
