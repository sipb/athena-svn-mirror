/**********************************************************************
 * File Exchange client library
 *
 * $Id: _fx_rpc_errno.c,v 1.2 2002-03-10 17:54:42 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_rpc_errno_c[] = "$Id: _fx_rpc_errno.c,v 1.2 2002-03-10 17:54:42 ghudson Exp $";
#endif /* lint */

#include "fxcl.h"

long
_fx_rpc_errno(cl)
     CLIENT *cl;
{
  struct rpc_err err;

  if (cl) {
    clnt_geterr(cl, &err);
    return((long) err.re_status + ERROR_TABLE_BASE_rpc);
  }
  return((long) rpc_createerr.cf_stat + ERROR_TABLE_BASE_rpc);
}
