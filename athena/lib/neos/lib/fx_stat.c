/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_stat.c,v 1.2 1999-01-22 23:18:03 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_stat_c[] = "$Id: fx_stat.c,v 1.2 1999-01-22 23:18:03 ghudson Exp $";
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
