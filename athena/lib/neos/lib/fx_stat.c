/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_stat.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_stat.c,v 1.1 1993-10-12 03:03:33 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_stat_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_stat.c,v 1.1 1993-10-12 03:03:33 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_stat -- return file exchange server statistics
 */

long
fx_stat(fxp, ret)
     FX *fxp;
     server_stats **ret;
{
  int dummy;

  *ret = server_stats_1(&dummy, fxp->cl);
  if (!*ret) return(_fx_rpc_errno(fxp->cl));
  return(0L);
}
