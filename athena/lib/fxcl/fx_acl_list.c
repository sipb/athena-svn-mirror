/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_acl_list.c,v 1.1 1999-09-28 22:07:18 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_acl_list_c[] = "$Id: fx_acl_list.c,v 1.1 1999-09-28 22:07:18 danw Exp $";
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

#ifdef HAVE_KRB4
  /* shorten kerberos principals to usernames */
  for (node = (*ret)->stringlist_res_u.list; node; node = node->next)
    _fx_shorten(fxp, node->s);
#endif

  return((*ret)->local_errno);
}
