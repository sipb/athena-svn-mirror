/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_delete.c,v 1.1 1999-09-28 22:07:19 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_delete_c[] = "$Id: fx_delete.c,v 1.1 1999-09-28 22:07:19 danw Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_delete -- delete a file in the exchange
 */

long
fx_delete(fxp, p)
     FX *fxp;
     Paper *p;
{
  long *ret, code;
  Paper to_delete;
  char new_owner[FX_UNAMSZ], new_author[FX_UNAMSZ];

  paper_copy(p, &to_delete);

#ifdef HAVE_KRB4
  /* lengthen usernames to kerberos principals */
  to_delete.owner = _fx_lengthen(fxp, p->owner, new_owner);
  to_delete.author = _fx_lengthen(fxp, p->author, new_author);
#endif

  ret = delete_1(&to_delete, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
