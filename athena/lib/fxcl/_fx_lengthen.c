/**********************************************************************
 * File Exchange client library
 *
 * $Id: _fx_lengthen.c,v 1.1 1999-09-28 22:07:17 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid__fx_lengthen_c[] = "$Id: _fx_lengthen.c,v 1.1 1999-09-28 22:07:17 danw Exp $";
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
