/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_host_list_destroy.c,v 1.1 1999-09-28 22:07:20 danw Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_host_list_destroy_c[] = "$Id: fx_host_list_destroy.c,v 1.1 1999-09-28 22:07:20 danw Exp $";
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
