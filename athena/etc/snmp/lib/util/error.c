#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/error.c,v 1.1 1994-09-18 12:56:46 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:16:19  snmpdev
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
**			error.c
**
** Function to print meaningful error messages.
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
#include "../../h/snmperrs.h"

error(code,fp)
short code;
FILE *fp;
{ 
  switch (code)	{		/* figure out the error */
/* generic errors */
   case SNMP_OK:		/* only non-error */
    fprintf(fp,"snmp: no error\n");
    break;
   case EMALLOC:		/* malloc error */
    fprintf(fp,"generic error: memory allocatation error\n");
    break;
   case GENERR:			/* generic error */
    fprintf(fp,"generic error: unspecified error\n");
    break;
/* parser and builder errors */
   case NEGINBUF:		/* negative input buffer length */
    fprintf(fp,"asn function error: input buffer of negative length\n");
    break;
   case NEGOUTBUF:		/* negative output buffer length */
    fprintf(fp,"asn function error: output buffer of negative length\n");
    break;
   case NOINBUF:		/* no input buffer */
    fprintf(fp,"asn function error: no input buffer\n");
    break;
   case NOOUTBUF:		/* no output buffer */
    fprintf(fp,"asn function error: no output buffer\n");
    break;
   case TYP_UNKNOWN:		/* unknown asn type */
    fprintf(fp,"asn function error: unknown asn type\n");
    break;
   case LENERR:			/* input buffer ended */
    fprintf(fp,"asn function error: unexpected end to input buffer\n");
    break;
   case UNSPLEN:		/* indefinite length */
    fprintf(fp,"asn function error: indefinite lengths are not supported\n");
    break;
   case OUTERR:			/* output buffer ended */
    fprintf(fp,"asn function error: output buffer insufficiently large\n");
    break;
   case TYP_MISMATCH:		/* wrong type */
    fprintf(fp,"asn function error: unexpected asn type encountered\n");
    break;
   case TOOMANYVARS:		/* too many variables */
    fprintf(fp,"asn function error: variable list has too many variables\n");
    break;
   case TOOLONG:		/* asn structure too long */
    fprintf(fp,"asn function error: asn structure was too long\n");
    break;
   case TYPERR:			/* asn structure wrong */
    fprintf(fp,"asn function error: asn structure wrong for type\n");
    break;
   case PKTLENERR:		/* max length of packet exceeded */
    fprintf(fp,"asn function error:maximum allowed size for packet exceeded\n");
    break;
   case TYPELONG:		/* value too long for type */
    fprintf(fp,"asn function error: value too long for type\n");
    break;
   case NO_SID:			/* no session id */
    fprintf(fp,"asn function error: no session id found/provided\n");
    break;
/* communications table function errors */
   case TBLFULL:
    fprintf(fp,"table error: table overflow\n");
    break;
   case NOSUCHID:
    fprintf(fp,"table error: id not found\n");
    break;
/* communications function errors */
   case UNINIT_SOCK:
    fprintf(fp,"communications error: socket not initialized\n");
    break;
   case NODEST:
    fprintf(fp,"communications error: no destination provided in send\n");
    break;
   case NOSID:
    fprintf(fp,"communications error: no session id provided in send\n");
    break;
   case BADSIDLEN:
    fprintf(fp,"communications error: no session id length in send\n");
    break;
   case NOSVC:
    fprintf(fp,"communications error: unknown service\n");
    break;
   case REQID_UNKNOWN:
    fprintf(fp,"communications error: no such request id in table\n");
    break;
   case SND_TMO:
    fprintf(fp,"communications error: send timeout\n");
    break;
   case NORECVBUF:
    fprintf(fp,"communications error: no receive buffer\n");
    break;
   case NOREQID:
    fprintf(fp,"communications error: no request id in call to receive\n");
    break;
   case NOSOCK:
    fprintf(fp,"communications error: cannot create socket\n");
    break;
   case BINDERR:
    fprintf(fp,"communications error: cannot bind socket\n");
    break;
   case SND_ERR:
    fprintf(fp,"communications error: generic error in send\n");
    break;
   case NOHNAME:
    fprintf(fp,"communications error: cannot find local host name\n");
    break;
   case NOHADDR:
    fprintf(fp,"communications error: cannot find local host address\n");
    break;
   case BADVERSION:
    fprintf(fp,"communications error: bad protocol version\n");
    break;
/* authentication function errors */
   case NOMSGBUF:
    fprintf(fp,"authentication error: no message to authenticate\n");
    break;
/* variable converter errors */
   case NOINITFL:
    fprintf(fp,"converter error: no initialization file\n");
    break;
   case VARBOUNDS:
    fprintf(fp,"converter error: numeric variable not in legal bounds\n");
    break;
   case NOBUF:
    fprintf(fp,"converter error: no buffer for conversion\n");
    break;
   case NONUM:
    fprintf(fp,"converter error: no numeric correspondent\n");
    break;
   case NONAME:
    fprintf(fp,"converter error: no variable name to convert\n");
    break;
   case CONVUNKNOWN:
    fprintf(fp,"converter error: unknown conversion requested\n");
    break;
   case VARUNKNOWN:
    fprintf(fp,"converter error: unknown variable\n");
    break;
   default:
    fprintf(fp,"error printer error: unknown error type\n");
    break;
  }
  return(0);
}
