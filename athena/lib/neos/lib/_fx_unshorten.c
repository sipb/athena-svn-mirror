/**********************************************************************
 * File Exchange client library
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_unshorten.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_unshorten.c,v 1.2 1996-09-20 04:36:08 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_unshorten_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_unshorten.c,v 1.2 1996-09-20 04:36:08 ghudson Exp $";
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
