/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send_file.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send_file.c,v 1.1 1993-10-12 03:03:52 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_send_file_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send_file.c,v 1.1 1993-10-12 03:03:52 probe Exp $";
#endif /* lint */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include "fxcl.h"

/*
 * fx_send_file -- send a file to the exchange
 */

long
fx_send_file(fxp, p, filename)
     FX *fxp;
     Paper *p;
     char *filename;
{
  FILE *fp;
  struct stat buf;
  Paper to_send;
  long code;

  paper_copy(p, &to_send);

  /* check file status */
  if (stat(filename, &buf)) return((long) errno);
  if (buf.st_mode & S_IFDIR) return((long) EISDIR);
  if (buf.st_mode & S_IEXEC) to_send.flags |= PAPER_EXECUTABLE;

  /* send file with correct name */
  if (!to_send.filename) {
    to_send.filename = rindex(filename, '/') + 1;
    if (to_send.filename == (char *) 1) to_send.filename = filename;
  }
  if ((fp = fopen(filename, "r")) == NULL) return((long) errno);

  code = fx_send(fxp, &to_send, fp);
  (void) fclose(fp);
  return(code);
}
