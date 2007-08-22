/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_stat_destroy.c,v 1.2 1999-01-22 23:18:04 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_stat_destroy_c[] = "$Id: fx_stat_destroy.c,v 1.2 1999-01-22 23:18:04 ghudson Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_stat_destroy -- free memory allocated for server stats
 */

void
fx_stat_destroy(stats)
     server_stats **stats;
{
  if (stats && *stats) {
    xdr_free(xdr_server_stats, *stats);
    *stats = NULL;
  }
}
