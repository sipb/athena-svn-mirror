/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_acl_list_destroy.c,v 1.1 1999-09-28 22:07:18 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_acl_list_destroy_c[] = "$Id: fx_acl_list_destroy.c,v 1.1 1999-09-28 22:07:18 danw Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_acl_list_destroy -- free memory allocated for acl list
 */

void
fx_acl_list_destroy(list)
     stringlist_res **list;
{
  register stringlist node;

  if (list && *list) {

#ifdef HAVE_KRB4
    /* be cautious; restore names to previous length */
    for (node = (*list)->stringlist_res_u.list; node; node = node->next)
      _fx_unshorten(node->s);
#endif

    xdr_free(xdr_stringlist_res, (char *) *list);
    *list = NULL;
  }
}
