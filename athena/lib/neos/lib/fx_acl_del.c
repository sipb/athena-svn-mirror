/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_del.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_del.c,v 1.1 1993-10-12 03:03:43 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_acl_del_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_del.c,v 1.1 1993-10-12 03:03:43 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_acl_del -- delete a person from an access control list
 */

long
fx_acl_del(fxp, aclname, person)
     FX *fxp;
     char *aclname, *person;
{
  long *ret, code = 0L;
  acl_maint maint;
  char principal[FX_UNAMSZ];

  maint.aclname = aclname;
  maint.aclparam = person;

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  if (strcmp(person, OWNER_WILDCARD))
    maint.aclparam = _fx_lengthen(fxp, person, principal);
#endif

  ret = delete_acl_1(&maint, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
