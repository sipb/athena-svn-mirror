/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_host_list_destroy.c,v 1.2 1999-01-22 23:17:58 ghudson Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_host_list_destroy_c[] = "$Id: fx_host_list_destroy.c,v 1.2 1999-01-22 23:17:58 ghudson Exp $";
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
