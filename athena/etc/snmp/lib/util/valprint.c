#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/valprint.c,v 1.2 1995-07-12 03:17:52 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18  12:56:50  cfields
 * Initial revision
 *
 * Revision 1.1  89/11/03  15:16:26  snmpdev
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
**			valprint.c
**
** Function to print out a variable value in human readable form on the
** file pointer fp. This function will not append a '\n' to the value 
** in printing, so the application will have to do so, if so desired.
**
**
*****************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#include "/usr/netinclude/arpa/inet.h"
#endif /* SVR3WIN */
#include "../../h/snmp_hs.h"

valprint(val,fp)
objval *val;
FILE *fp;
{ int i;

  if(val == NULL)
   { fprintf(fp,"no value");
     return(0);
   }

/* figure out the type of value, and print accordingly */
  switch(val->type) {			/* determine value type */
   case INT:
    fprintf(fp,"0x%x\t%d",val->value.intgr,val->value.intgr);
    break;
   case STR:
    fprintf(fp,"%s",val->value.str.str);
    break;
   case OBJ:
    for(i=0; i < val->value.obj.ncmp; i++)
     fprintf(fp,"%d ",val->value.obj.cmp[i]);
    break;
   case EMPTY:
    fprintf(fp,"NULL");
    break;
   case IPADD:
    fprintf(fp,"%s",inet_ntoa(val->value.ipadd));
    break;
   case CNTR:
    fprintf(fp,"0x%x\t%u",val->value.cntr,val->value.cntr);
    break;
   case GAUGE:
    fprintf(fp,"0x%x\t%u",val->value.gauge,val->value.gauge);
    break;
   case TIME:
    fprintf(fp,"0x%x\t%u",val->value.time,val->value.time);
    break;
   case OPAQUE:
    fprintf(fp,"unprintable (opaque)");
    break;
   default:
    fprintf(fp,"unknown value type");
    break;
  }
  return(0);
}
