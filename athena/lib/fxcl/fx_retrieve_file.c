/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_retrieve_file.c,v 1.1 1999-09-28 22:07:21 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_retrieve_file_c[] = "$Id: fx_retrieve_file.c,v 1.1 1999-09-28 22:07:21 danw Exp $";
#endif /* lint */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fxcl.h"

/*
 * fx_retrieve_file -- retrieve a file from the exchange
 */

long
fx_retrieve_file(fxp, p, filename)
     FX *fxp;
     Paper *p;
     char *filename;
{
  FILE *fp;
  struct stat buf;
  long code;

  if ((fp = fopen(filename, "w")) == NULL) return((long) errno);
  code = fx_retrieve(fxp, p, fp);
  if (fclose(fp) == EOF && !code) code = (long) errno;

  /* If there's an error, don't leave a zero-length file. */
  if (code) {
    if (!stat(filename, &buf) && !buf.st_size) unlink(filename);
    return(code);
  }

  /* check file status */
  if (p->flags & PAPER_EXECUTABLE) {
    if (stat(filename, &buf)) return((long) errno);
    if (buf.st_mode & S_IEXEC) return(code);
    /* copy read permission to execute permission */
    if (chmod(filename, (int) (((0444 & buf.st_mode) >> 2) | buf.st_mode)))
      return((long) errno);
  }

  return(code);
}
