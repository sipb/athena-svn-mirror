/* Copyright 1985-1999 by the Massachusetts Institute of Technology.
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

/* This is dent, a program to indent the standard input. */

static const char rcsid[] = "$Id: dent.c,v 1.4 1999-09-16 00:02:55 danw Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void indent(FILE *fptr, int nspaces);
void spaceout(int spaces);
static int read_line(FILE *fp, char **buf, int *bufsize);

int main(int argc, char **argv)
{
  FILE *fptr = stdin;	/* initially read standard input file */
  int i;
  int nspaces = 8;

  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-')
	{
	  if (sscanf(&argv[i][1], "%d", &nspaces) == 0)
	    {
	      fprintf(stderr, "%s: argument must be integer.\n", argv[0]);
	      exit(1);
	    }
	  continue;
	}

      fptr = fopen(argv[i],"r");
      if (!fptr)
	{
	  fprintf(stderr,"%s: no such file %s.\n", argv[0], argv[i]);
	  exit(1);
	}
      indent(fptr, nspaces);
      fclose(fptr);
    }

  if (fptr == stdin)
    indent(fptr, nspaces);

  return 0;
}

void indent(FILE *fptr, int nspaces)
{
  int j, linesize;
  char *line = NULL;

  while (read_line(fptr, &line, &linesize) == 0)
    {
      if (strlen(line) >= 1)
	{
	  spaceout(nspaces);
	  for (j = 0; j < strlen(line); j++)
	    {
	      putchar(line[j]);
	      if (line[j] == '\r')
		spaceout(nspaces);
	    }
	  putchar('\n');
	}
    }
}

void spaceout(int spaces)
{
  while (spaces-- > 0)
    putchar(' ');
}

/* This is an internal function.  Its contract is to read a line from a
 * file into a dynamically allocated buffer, zeroing the trailing newline
 * if there is one.  The calling routine may call read_line multiple
 * times with the same buf and bufsize pointers; *buf will be reallocated
 * and *bufsize adjusted as appropriate.  The initial value of *buf
 * should be NULL.  After the calling routine is done reading lines, it
 * should free *buf.  This function returns 0 if a line was successfully
 * read, 1 if the file ended, and -1 if there was an I/O error or if it
 * ran out of memory.
 */

static int read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*buf == NULL)
    {
      *buf = malloc(128);
      if (!*buf)
	return -1;
      *bufsize = 128;
    }

  while (1)
    {
      if (!fgets(*buf + offset, *bufsize - offset, fp))
	return (offset != 0) ? 0 : (ferror(fp)) ? -1 : 1;
      len = offset + strlen(*buf + offset);
      if ((*buf)[len - 1] == '\n')
	{
	  (*buf)[len - 1] = 0;
	  return 0;
	}
      offset = len;

      /* Allocate more space. */
      newbuf = realloc(*buf, *bufsize * 2);
      if (!newbuf)
	return -1;
      *buf = newbuf;
      *bufsize *= 2;
    }
}
