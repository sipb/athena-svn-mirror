/**********************************************************************
 * File Exchange client library
 *
 * $Author: vrt $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v 1.1 1993-04-27 17:21:12 vrt Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_lengthen_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v 1.1 1993-04-27 17:21:12 vrt Exp $";
#endif /* lint */

#include <strings.h>
#include "fxcl.h"

/*
 * _fx_lengthen -- returns username lengthened to kerberos principal
 */

char *
_fx_lengthen(fxp, oldname, newname)
     FX *fxp;
     char *oldname, *newname;
{
  register char *s;

  (void) strcpy(newname, oldname);
  s = index(oldname, '@');
  if (!s) (void) strcat(newname, fxp->extension);
  return(newname);
}
