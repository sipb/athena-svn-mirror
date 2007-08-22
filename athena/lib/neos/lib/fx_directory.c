/**********************************************************************
 * File Exchange client library
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v 1.2 1998-07-25 21:02:16 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_directory_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_directory.c,v 1.2 1998-07-25 21:02:16 ghudson Exp $";
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
  code = (*ret)->local_errno;
  return(code);
}
