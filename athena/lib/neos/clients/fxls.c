/**********************************************************************
 * File Exchange fxls client
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxls.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxls.c,v 1.1 1993-10-12 03:08:59 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fxcreate_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxls.c,v 1.1 1993-10-12 03:08:59 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <fxcl.h>

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp;
  stringlist_res *sr;
  stringlist l;
  long code;

  if ((fxp = fx_open("", &code)) == NULL) {
    com_err(argv[0], code, "while connecting.");
    exit(1);
  }

  code = fx_directory(fxp, &sr);
  fx_close(fxp);
  if (code) {
    com_err(argv[0], code, "");
    exit(1);
  }

  for(l = sr->stringlist_res_u.list; l != NULL; l = l->next)
    printf("%s\n", l->s);

  exit(0);
}
