/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_move.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_move.c,v 1.1 1993-10-12 03:03:57 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_move_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_move.c,v 1.1 1993-10-12 03:03:57 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_move -- change Paper info for file in the exchange
 */

long
fx_move(fxp, src, dest)
     FX *fxp;
     Paper *src, *dest;
{
  TwoPaper pp;
  long *ret, code;
  char src_owner[FX_UNAMSZ], src_author[FX_UNAMSZ];
  char dest_owner[FX_UNAMSZ], dest_author[FX_UNAMSZ];

  paper_copy(src, &pp.src);
  paper_copy(dest, &pp.dest);

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  pp.src.owner = _fx_lengthen(fxp, src->owner, src_owner);
  pp.src.author = _fx_lengthen(fxp, src->author, src_author);
  pp.dest.owner = _fx_lengthen(fxp, dest->owner, dest_owner);
  pp.dest.author = _fx_lengthen(fxp, dest->author, dest_author);
#endif

  ret = move_1(&pp, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
