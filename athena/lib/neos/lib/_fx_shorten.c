/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_shorten.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_shorten.c,v 1.1 1993-10-12 03:03:26 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_shorten_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_shorten.c,v 1.1 1993-10-12 03:03:26 probe Exp $";
#endif /* lint */

#include <strings.h>
#include "fxcl.h"

/*
 * _fx_shorten -- strip off local kerberos realm
 */

void
_fx_shorten(fxp, name)
     FX *fxp;
     char *name;
{
  register char *s;
  s = index(name, '@');
  if (s)
    if (strcmp(s, fxp->extension) == 0)
      *s = '\0';
}
