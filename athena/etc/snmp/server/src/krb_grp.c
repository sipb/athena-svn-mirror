/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the Kerberos portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/krb_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.5  90/06/05  16:08:14  tom
 * deleted srvtab code
 * 
 * Revision 1.4  90/05/26  13:38:18  tom
 * athena release 7.0e
 *    madethis code functional
 * 
 * Revision 1.3  90/04/26  17:05:46  tom
 * added rcsid
 * 
 * Revision 1.2  90/04/26  16:51:07  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/krb_grp.c,v 2.0 1992-04-22 02:02:40 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef KERBEROS

#include <krb.h>

/*
 * Function:    lu_kerberos()
 * Description: Top level callback for kerberos. The realm and key version
 *              variables should be split into two functions.
 */
 
int
lu_kerberos(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  int num;

  bzero(lbuf, sizeof(lbuf));

  if (varnode->flags & NOT_AVAIL || varnode->offset <= 0)
    return (BUILD_ERR);

  if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
    num = 1;
  else
    num = instptr->cmp[0];

  if((reqflg == NXT) && (instptr != (objident *) NULL) &&
     (instptr->ncmp != 0))
    num++;

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  
  switch(varnode->offset)
    {
    case N_KRBCREALM:
      if(krb_get_lrealm(lbuf, num) == KFAILURE)
	return(BUILD_ERR);
      if(*lbuf == '\0')
	return(BUILD_ERR);
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;
      return(make_str(&(repl->val), lbuf));

    default:
      syslog (LOG_WARNING, "lu_kerberos: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


#endif KERBEROS
#endif MIT
