/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* Program to convert an old attachtab into a new one. */

static const char rcsid[] = "$Id: atconvert.c,v 1.1 1999-03-19 16:35:09 danw Exp $";

#include <stdio.h>

#include <locker.h>

int main(int argc, char **argv)
{
  locker_context context;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: atconvert filename\n");
      exit(1);
    }

  if (getuid() != 0)
    {
      fprintf(stderr, "You must be root to run this program.\n");
      exit(1);
    }

  if (locker_init(&context, 0, NULL, NULL))
    exit(1);
  locker_convert_attachtab(context, argv[1]);
  locker_end(context);

  return 0;
}
