/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_directory_destroy.c,v 1.1 1999-09-28 22:07:20 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_directory_destroy_c[] = "$Id: fx_directory_destroy.c,v 1.1 1999-09-28 22:07:20 danw Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_directory_destroy -- free memory allocated for directory
 */

void
fx_directory_destroy(list)
     stringlist_res **list;
{
  if (list && *list) xdr_free(xdr_stringlist_res, (char *) *list);
  *list = NULL;
}
