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
 * Revision 1.2  90/04/26  16:51:07  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/krb_grp.c,v 1.3 1990-04-26 17:05:46 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef KERBEROS

#include <krb.h>


/*
 * Function:    lu_kerberos()
 * Description: Top level callback for kerberos. Not completed.
 */
 
int
lu_kerberos(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char buf[BUFSIZ];
  char *ptr;

  bzero(buf, BUFSIZ);

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */
  repl->val.type = STR;  
  
  switch(varnode->offset)
    {
    case N_KRBCREALM:
      krb_get_realm(buf, 1);
      repl->val.value.str.len = strlen(buf);
      repl->val.value.str.str = (char *) malloc(sizeof(char) * 
				repl->val.value.str.len);
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_kerberos: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}

#endif KERBEROS
#endif MIT
