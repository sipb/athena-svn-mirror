/* Copyright 1999 by the Massachusetts Institute of Technology.
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

/* This is the server side of the networked write system. */

static const char rcsid[] = "$Id: writed.c,v 1.2 1999-10-19 17:14:54 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  char line[BUFSIZ], *p;
  char *av[6] = { "write", "-f", 0, 0, 0, 0 };
  int ac;

  if (!fgets(line, BUFSIZ, stdin))
    exit(1);

  ac = 2;
  for (p = strtok(line, " \t\r\n"); p && ac < 5; p = strtok(NULL, " \t\r\n"))
    av[ac++] = p;

  /* Put the socket on stdin, stdout, and stderr */
  dup2(0, 1);
  dup2(0, 2);
  execv(WRITE_PROG, av);
  exit(1);
}
