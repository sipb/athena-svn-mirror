/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v 1.1 1993-10-12 03:03:14 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_directory_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v 1.1 1993-10-12 03:03:14 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_directory -- list file exchanges available
 */

long
fx_directory(fxp, ret)
     FX *fxp;
     stringlist_res **ret;
{
  long code;
  int dummy = 0;

  *ret = list_courses_1(&dummy, fxp->cl);
  if (!*ret) return(_fx_rpc_errno(fxp->cl));
  code = (*ret)->errno;
  return(code);
}
