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

/* This file implements utility functions used by larvnetd. */

static const char rcsid[] = "$Id: util.c,v 1.1 1998-09-01 20:57:48 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "larvnetd.h"

void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    {
      syslog(LOG_ALERT, "emalloc: malloc size %lu failed, aborting",
	     (unsigned long) size);
      exit(1);
    }
  return ptr;
}

void *erealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    {
      syslog(LOG_ALERT, "erealloc: realloc size %lu failed, aborting",
	     (unsigned long) size);
      exit(1);
    }
  return ptr;
}

char *estrdup(const char *s)
{
  char *new_s;

  new_s = emalloc(strlen(s) + 1);
  strcpy(new_s, s);
  return new_s;
}

char *estrndup(const char *s, size_t n)
{
  char *new_s;

  new_s = emalloc(n + 1);
  memcpy(new_s, s, n);
  new_s[n] = 0;
  return new_s;
}

/* Read a line from a file into a dynamically allocated buffer,
 * zeroing the trailing newline if there is one.  The calling routine
 * may call read_line multiple times with the same buf and bufsize
 * pointers; *buf will be reallocated and *bufsize adjusted as
 * appropriate.  The initial value of *buf should be NULL.  After the
 * calling routine is done reading lines, it should free *buf.  This
 * function returns 0 if a line was successfully read, 1 if the file
 * ended, and -1 if there was an I/O error.
 */

int read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*buf == NULL)
    {
      *buf = emalloc(128);
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
      newbuf = erealloc(*buf, *bufsize * 2);
      *buf = newbuf;
      *bufsize *= 2;
    }
}
