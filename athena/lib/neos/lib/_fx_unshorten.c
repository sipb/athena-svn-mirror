/**********************************************************************
 * File Exchange client library
 *
 * $Id: _fx_unshorten.c,v 1.3 1999-01-22 23:17:51 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_unshorten_c[] = "$Id: _fx_unshorten.c,v 1.3 1999-01-22 23:17:51 ghudson Exp $";
#endif /* lint */

#include <string.h>
#include "fxcl.h"

/*
 * _fx_unshorten -- restore kerberos realm removed by _fx_shorten
 */

void
_fx_unshorten(shortened_name)
     char *shortened_name;
{
  if (!strchr(shortened_name, '@'))
    shortened_name[strlen(shortened_name)] = '@';
}
