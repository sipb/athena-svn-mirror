/**********************************************************************
 * File Exchange client library
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v 1.2 1996-09-20 04:36:06 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_lengthen_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/_fx_lengthen.c,v 1.2 1996-09-20 04:36:06 ghudson Exp $";
#endif /* lint */

#include <string.h>
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
  s = strchr(oldname, '@');
  if (!s) (void) strcat(newname, fxp->extension);
  return(newname);
}
