/**********************************************************************
 * File Exchange client library
 *
 * $Id: _fx_rpc_errno.c,v 1.2 1999-01-22 23:17:50 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_rpc_errno_c[] = "$Id: _fx_rpc_errno.c,v 1.2 1999-01-22 23:17:50 ghudson Exp $";
#endif /* lint */

#include "fxcl.h"

long
_fx_rpc_errno(cl)
     CLIENT *cl;
{
  struct rpc_err err;
  extern struct rpc_createerr rpc_createerr;

  if (cl) {
    clnt_geterr(cl, &err);
    return((long) err.re_status + ERROR_TABLE_BASE_rpc);
  }
  return((long) rpc_createerr.cf_stat + ERROR_TABLE_BASE_rpc);
}
