/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list_destroy.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list_destroy.c,v 1.1 1993-10-12 03:04:20 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_host_list_destroy_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list_destroy.c,v 1.1 1993-10-12 03:04:20 probe Exp $";
#endif /* lint */

#include <fxcl.h>

/*
 * fx_host_list_destroy -- free memory allocated for host list
 */

void
fx_host_list_destroy(node)
     stringlist node;
{
  if (node) {
    fx_host_list_destroy(node->next);
    if (node->s)
      free(node->s);
    free((char *) node);
  }
  return;
}
