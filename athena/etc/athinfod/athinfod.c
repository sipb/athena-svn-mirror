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

static const char rcsid[] = "$Id: athinfod.c,v 1.3 1999-10-19 20:22:56 danw Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH_ATHINFO_DEFS "/etc/athena/athinfo.defs"
#define PATH_ATHINFO_ACCESS "/etc/athena/athinfo.access"

static const char *read_query(void);
static void shutdown_input(void);
static const char *get_definition(const char *query);
static void check_enabled(const char *query);
static int first_field_matches(const char *s, const char *word);
static const char *skip_spaces(const char *p);
static const char *skip_nonspaces(const char *p);
static int read_line(FILE *fp, char **buf, int *bufsize);
static void *emalloc(size_t size);
static void *erealloc(void *ptr, size_t size);

int main(int argc, char **argv)
{
  const char *query, *cmd;

  query = read_query();
  shutdown_input();
  cmd = get_definition(query);
  check_enabled(query);
  execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
  fprintf(stderr, "athinfod: cannot run shell, aborting.\n");
  return 1;
}

/* Read the query from stdin and validate it. */
static const char *read_query(void)
{
  char *line = NULL;
  int linesize;
  const char *p;

  if (read_line(stdin, &line, &linesize) != 0)
    {
      fprintf(stderr, "athinfod: couldn't read query.\n");
      exit(0);
    }

  /* Make sure the query consists of printable nonspace characters. */
  for (p = line; *p; p++)
    {
      if (!isprint((unsigned char)*p) || isspace((unsigned char)*p))
	{
	  fprintf(stderr, "athinfod: invalid query.\n");
	  exit(0);
	}
    }
  return line;
}

/* Shut down the input side of the inetd socket and repoint stdin at
 * /dev/null.  This eliminates the possibility of malicious input
 * affecting the commands we execute.
 */
static void shutdown_input(void)
{
  int fd;

  shutdown(STDIN_FILENO, 0);
  close(STDIN_FILENO);
  fd = open("/dev/null", O_RDONLY);
  if (fd == -1)
    {
      fprintf(stderr, "athinfod: cannot open /dev/null, aborting.\n");
      exit(1);
    }
  if (fd != STDIN_FILENO)
    {
      dup2(fd, STDIN_FILENO);
      close(fd);
    }
}

/* Read the definition of query from the athinfo.defs file. */
static const char *get_definition(const char *query)
{
  char *line = NULL;
  int linesize;
  FILE *fp;

  fp = fopen(PATH_ATHINFO_DEFS, "r");
  if (!fp)
    {
      fprintf(stderr,
	      "athinfod: cannot open athinfo definitions file, aborting.\n");
      exit(1);
    }

  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Ignore comment lines. */
      if (*line == '#')
	continue;

      if (first_field_matches(line, query))
	{
	  fclose(fp);
	  return skip_spaces(skip_nonspaces(line));
	}
    }

  fclose(fp);
  fprintf(stderr, "athinfod: unrecognized query.\n");
  exit(0);
}

/* See if this command is enabled. */
static void check_enabled(const char *query)
{
  char *line = NULL;
  int linesize, enabled = 0, val;
  FILE *fp;
  const char *p;

  fp = fopen(PATH_ATHINFO_ACCESS, "r");
  if (!fp)
    {
      fprintf(stderr,
	      "athinfod: cannot open athinfo access file, aborting.\n");
      exit(1);
    }

  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Only pay attention to lines starting with "enable" or "disable". */
      if (first_field_matches(line, "enable"))
	val = 1;
      else if (first_field_matches(line, "disable"))
	val = 0;
      else
	continue;

      /* If we find an exact match, stop.  If we find a glob match,
       * accept that value for now but hold out for an exact match.
       * (This means if there are conflicting lines in the config
       * file, we take the first exact match but the last glob match.
       * Oh well.)
       */
      p = skip_spaces(skip_nonspaces(line));
      if (first_field_matches(p, query))
	{
	  enabled = val;
	  break;
	}
      else if (first_field_matches(p, "*"))
	enabled = val;
    }

  fclose(fp);
  free(line);
  if (!enabled)
    {
      fprintf(stderr, "athinfod: query disabled.\n");
      exit(0);
    }
}

static int first_field_matches(const char *s, const char *word)
{
  int len = strlen(word);

  return (strncasecmp(s, word, len) == 0 &&
	  (isspace((unsigned char)s[len]) || !s[len]));
}

static const char *skip_spaces(const char *p)
{
  while (isspace((unsigned char)*p))
    p++;
  return p;
}

static const char *skip_nonspaces(const char *p)
{
  while (*p && !isspace((unsigned char)*p))
    p++;
  return p;
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
static int read_line(FILE *fp, char **buf, int *bufsize)
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
	  if (len > 1 && (*buf)[len - 2] == '\r')
	    (*buf)[len - 2] = 0;
	  return 0;
	}
      offset = len;

      /* Allocate more space. */
      newbuf = erealloc(*buf, *bufsize * 2);
      *buf = newbuf;
      *bufsize *= 2;
    }
}

static void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    {
      fprintf(stderr, "athinfod: malloc failure, aborting.\n");
      exit(1);
    }
  return ptr;
}

void *erealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    {
      fprintf(stderr, "athinfod: realloc failure, aborting.\n");
      exit(1);
    }
  return ptr;
}
