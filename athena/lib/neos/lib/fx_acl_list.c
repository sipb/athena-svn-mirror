/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_list.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_list.c,v 1.1 1993-10-12 03:03:37 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_acl_list_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_acl_list.c,v 1.1 1993-10-12 03:03:37 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_acl_list -- fills in list of users in access control list
 */

long
fx_acl_list(fxp, aclname, ret)
     FX *fxp;
     char *aclname;
     stringlist_res **ret;
{
  register stringlist node;

  *ret = list_acl_1(&aclname, fxp->cl);
  if (!*ret) return(_fx_rpc_errno(fxp->cl));

#ifdef KERBEROS
  /* shorten kerberos principals to usernames */
  for (node = (*ret)->stringlist_res_u.list; node; node = node->next)
    _fx_shorten(fxp, node->s);
#endif

  return((*ret)->errno);
}
