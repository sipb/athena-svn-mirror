/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_destroy.c,v 1.2 1999-01-22 23:17:56 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_destroy_c[] = "$Id: fx_destroy.c,v 1.2 1999-01-22 23:17:56 ghudson Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_destroy -- destroy a file exchange
 */

long
fx_destroy(fxp, name)
     FX *fxp;
     char *name;
{
  long *ret, code;

  ret = delete_course_1(&name, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
