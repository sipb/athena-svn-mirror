/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list.c,v 1.1 1993-10-12 03:03:46 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_list_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_list.c,v 1.1 1993-10-12 03:03:46 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_list -- fill in a Paperlist_res for given criterion
 */

long
fx_list(fxp, p, ret)
     FX *fxp;
     Paper *p;
     Paperlist_res **ret;
{
  register Paperlist node;
  Paper criterion;
  char new_owner[FX_UNAMSZ], new_author[FX_UNAMSZ];

  /* take care of null pointers */
  if (p) paper_copy(p, &criterion);
  else paper_clear(&criterion);

  if (!criterion.location.host)
    criterion.location.host = ID_WILDCARD;
  if (!criterion.author) criterion.author = AUTHOR_WILDCARD;
  if (!criterion.owner) criterion.owner = OWNER_WILDCARD;
  if (!criterion.filename) criterion.filename = FILENAME_WILDCARD;
  if (!criterion.desc) criterion.desc = FX_DEF_DESC;

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  if (strcmp(criterion.owner, OWNER_WILDCARD))
    criterion.owner = _fx_lengthen(fxp, criterion.owner, new_owner);
  if (strcmp(criterion.author, AUTHOR_WILDCARD))
    criterion.author = _fx_lengthen(fxp, criterion.author, new_author);
#endif

  /* try to retrieve list */
  if ((*ret = list_1(&criterion, fxp->cl)) == NULL)
    return(_fx_rpc_errno(fxp->cl));

#ifdef KERBEROS
  /* shorten kerberos principals to usernames */
  for (node = (*ret)->Paperlist_res_u.list; node; node = node->next) {
    _fx_shorten(fxp, node->p.owner);
    _fx_shorten(fxp, node->p.author);
  }
#endif

  return((*ret)->errno);
}
