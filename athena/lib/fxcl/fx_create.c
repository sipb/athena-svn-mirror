/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_create.c,v 1.1 1999-09-28 22:07:19 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_create_c[] = "$Id: fx_create.c,v 1.1 1999-09-28 22:07:19 danw Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_create -- create a new file exchange
 */

long
fx_create(fxp, name)
     FX *fxp;
     char *name;
{
  long *ret, code;

  ret = create_course_1(&name, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
