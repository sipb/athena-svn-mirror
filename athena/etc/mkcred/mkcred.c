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

/* mkcred: used to compile credentials files for knfs mountd */

static const char rcsid[] = "$Id: mkcred.c,v 1.1 1999-03-18 22:59:54 danw Exp $";

#include <sys/param.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <ndbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void add_cred(DBM *db, char *str);
int read_line(FILE *fp, char **buf, int *bufsize);

int main(int argc, char **argv)
{
  FILE *f;
  DBM *db;
  char *line = NULL;
  int bufsize, status;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: mkcred filename\n");
      exit(1);
    }

  f = fopen(argv[1], "r");
  if (!f)
    {
      fprintf(stderr, "mkcred: Can't open %s for reading\n", argv[1]);
      exit(1);
    }

  db = dbm_open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (!db)
    {
      fprintf(stderr, "mkcred: Can't create database files.\n");
      exit(1);
    }

  while ((status = read_line(f, &line, &bufsize)) == 0)
    add_cred(db, line);

  dbm_close(db);

  if (status == -1)
    {
      fprintf(stderr, "mkcred: Error reading input file.\n");
      exit(1);
    }

  return 0;
}

struct ucred {
  uid_t uid;
  gid_t gid;
  int glen;
  gid_t gids[NGROUPS_MAX];
};

void add_cred(DBM *db, char *str)
{
  char *name, *p;
  struct ucred u;
  datum k, d;
  int i;
  int code;

  name = strtok(str, ":");
  if (!name)
    goto err;
  p = strtok(NULL, ":");
  if (!p)
    goto err;
  u.uid = strtoul(p, &p, 10);
  if (*p)
    goto err;
  p = strtok(NULL, ":");
  if (!p)
    goto err;
  u.gid = strtoul(p, &p, 10);
  if (*p)
    goto err;

  for(i = 0, p = strtok(NULL, ":"); i < NGROUPS_MAX && p; i++)
    {      
      u.gids[i] = strtoul(p, &p, 10);
      if (*p)
	goto err;
      p = strtok(NULL, ":");
    }

  u.glen = i+1;

  k.dptr = name;
  k.dsize = strlen(name);
  d.dptr = (char *) &u;
  d.dsize = sizeof(u);
  code = dbm_store(db, k, d, DBM_REPLACE);
  if (code != 0)
    {
      fprintf(stderr, "mkcred: Could not store info for %s\n", name);
      exit(1);
    }
  return;

err:
  fprintf(stderr, "mkcred: Error parsing entry '%s'\n", str);
  exit(1);
}

/* read_line's contract is to read a line from a file into a
 * dynamically allocated buffer, zeroing the trailing newline if there
 * is one. The calling routine may call read_line multiple times with
 * the same buf and bufsize pointers; *buf will be reallocated and
 * *bufsize adjusted as appropriate. The initial value of *buf should
 * be NULL. After the calling routine is done reading lines, it should
 * free *buf. This function returns 0 if a line was successfully read,
 * 1 if the file ended, and -1 if there was an I/O error or if it ran
 * out of memory.
 */

int read_line(FILE *fp, char **buf, int *bufsize)
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
