/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list_destroy.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list_destroy.c,v 1.1 1993-10-12 03:03:48 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_list_destroy_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list_destroy.c,v 1.1 1993-10-12 03:03:48 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_list_destroy -- free memory allocated for paper list
 */

void
fx_list_destroy(plist)
     Paperlist_res **plist;
{
  Paperlist node;

  if (plist && *plist) {

    for (node = (*plist)->Paperlist_res_u.list; node; node = node->next) {
#ifdef KERBEROS
    /* be cautious; restore names to original length */
      _fx_unshorten(node->p.owner);
      _fx_unshorten(node->p.author);
#endif
      xdr_free(xdr_Paperlist_res, (char *) *plist);
      *plist = NULL;
    }
  }
}
