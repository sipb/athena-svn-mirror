/**********************************************************************
 * File Exchange fxdestroy client
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxdestroy.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxdestroy.c,v 1.1 1993-10-12 03:09:29 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fxdestroy_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxdestroy.c,v 1.1 1993-10-12 03:09:29 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <fxcl.h>

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp;
  long code;

  if (argc == 2) {
    if ((fxp = fx_open(argv[1], &code)) == NULL) {
      com_err(argv[0], code, "while connecting");
      exit(1);
    }
    if (code = fx_destroy(fxp, argv[1])) {
      com_err(argv[0], code, "trying to destroy %s", argv[1]);
      exit(1);
    }
    printf("Destroyed %s.\n", argv[1]);
    exit(0);
  }
  fprintf(stderr, "Usage: %s <course name>\n", argv[0]);
  exit(1);
}
