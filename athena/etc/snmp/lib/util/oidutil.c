#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/oidutil.c,v 1.1 1994-09-18 12:56:51 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:16:24  snmpdev
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
**			oidutil.c
**
** Utilities to manipulate object identifiers. Routines are analogous to the
** ones in string(5).
**
**
*****************************************************************************/
#include "../../h/conf.h"
#include "../../h/snmp.h"

/* oidncmp - compare two object identifiers up to ncmp'th subcomponent */
oidncmp(a,b,ncmp)
objident *a,*b;			/* object identifiers to be compared */
short ncmp;			/* number of subidentifiers */
{ int i;			/* index and counter */

  for(i=0; i < ncmp; i++)
   if(a->cmp[i] != b->cmp[i])
    return(a->cmp[i] - b->cmp[i]);
  return(0);
}

/* oidcmp - compare two object identfiers till first one has a 0 subidentifier*/
oidcmp(a,b)
objident *a,*b;			/* object identifiers to be compared */
{ int i;			/* index and counter */
  
  if(a->ncmp != b->ncmp)
   return(a->ncmp - b->ncmp);
  for(i=0; i < a->ncmp; i++)
   if(a->cmp[i] != b->cmp[i])
    return(a->cmp[i] - b->cmp[i]);
  return(0);
}

/* oidcat - concatenate second object identifier onto first */
oidcat(a,b)
objident *a,*b;
{ int i;			/* index and counter */

  for(i=0; a->ncmp < SNMPMXID && i < b->ncmp;i++)
   a->cmp[(a->ncmp)++] = b->cmp[i];
  return(0);
}

/* oidcpy - copy object identifier 2 to object identifier 1 */
oidcpy(a,b)
objident *a,*b;
{
  for(a->ncmp = 0; a->ncmp < b->ncmp; (a->ncmp)++)
   a->cmp[a->ncmp] = b->cmp[a->ncmp];
  return(0);
}
