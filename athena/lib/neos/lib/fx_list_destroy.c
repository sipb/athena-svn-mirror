/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_list_destroy.c,v 1.2 1999-01-22 23:18:00 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_list_destroy_c[] = "$Id: fx_list_destroy.c,v 1.2 1999-01-22 23:18:00 ghudson Exp $";
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
