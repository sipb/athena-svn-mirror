/* Copyright 1997 by the Massachusetts Institute of Technology.
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

/* This file is part of the Athena login library.  It implements
 * miscellaneous functions.
 */

static const char rcsid[] = "$Id: util.c,v 1.2 1997-10-30 23:58:57 ghudson Exp $";

#include <sys/param.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "al.h"
#include "al_private.h"

#ifdef HAVE_MASTER_PASSWD
#include <db.h>
#include <utmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

const char *al_strerror(int code, char **mem)
{
  /* A future implementation may want to handle internationalization.
   * For now, just return a string literal from a table. */
  const char *errtext[] = {
    "Successful completion",
    "Successful completion with some warnings",
    "Bad Hesiod entry for user",
    "Unknown username",
    "Logins not allowed for non-local accounts",
    "Logins currently disabled",
    "Remote logins not allowed for non-local accounts",
    "Could not add user to passwd file",
    "Could not modify user's session record",
    "Permission denied while modifying system files",
    "Out of memory",
    "Bad session record overwritten",
    "Could not add you to group file",
    "Using a temporary home directory created by a previous login",
    "Attach failed; you have a temporary home directory",
    "Attach failed; you have no home directory",
    "Home directory attach is disabled on this machine"
  };

  assert(code >= AL_SUCCESS && code <= AL_WNOATTACH);
  return errtext[code];
}

void al_free_errmem(char *mem)
{
  /* Do nothing for now. */
}

#ifdef HAVE_MASTER_PASSWD
static struct passwd *lookup(const DBT *key);

struct passwd *al__getpwnam(const char *username)
{
  DBT key;
  char buf[UT_NAMESIZE + 1];
  int len;

  len = strlen(username);
  if (len > UT_NAMESIZE)
    len = UT_NAMESIZE;
  buf[0] = _PW_KEYBYNAME;
  memcpy(buf + 1, username, len);
  key.data = buf;
  key.size = len + 1;
  return lookup(&key);
}

struct passwd *al__getpwuid(uid_t uid)
{
  DBT key;
  char buf[128];

  sprintf(buf, "%c%d", _PW_KEYBYUID, (int) uid);
  key.data = buf;
  key.size = strlen(buf);
  return lookup(&key);
}

static struct passwd *lookup(const DBT *key)
{
  DB *db;
  DBT value;
  unsigned char buf[UT_NAMESIZE + 1];
  int len, success = 0;
  char *p, *buffer;
  struct passwd *pwd;

  /* Open the insecure or secure database depending on whether we're root. */
  db = dbopen((geteuid()) ? _PATH_MP_DB : _PATH_SMP_DB, O_RDONLY, 0,
	      DB_HASH, NULL);
  if (!db)
    return(NULL);

  /* Look up the username. */
  if (db->get(db, key, &value, 0) != 0)
    {
      db->close(db);
      return NULL;
    }
  buffer = malloc(sizeof(struct passwd) + value.size);
  if (!buffer)
    {
      db->close(db);
      return NULL;
    }
  pwd = (struct passwd *) buffer;
  buffer += sizeof(struct passwd);
  memcpy(buffer, value.data, value.size);
#define FIELD(v) v = buffer; buffer += strlen(buffer) + 1
  FIELD(pwd->pw_name);
  FIELD(pwd->pw_passwd);
  memcpy(&pwd->pw_uid, buffer, sizeof(int));
  memcpy(&pwd->pw_gid, buffer + sizeof(int), sizeof(int));
  memcpy(&pwd->pw_change, buffer + 2 * sizeof(int), sizeof(time_t));
  buffer += 2 * sizeof(int) + sizeof(time_t);
  FIELD(pwd->pw_class);
  FIELD(pwd->pw_gecos);
  FIELD(pwd->pw_dir);
  FIELD(pwd->pw_shell);
  memcpy(&pwd->pw_expire, buffer, sizeof(time_t));
  db->close(db);
  return pwd;
}

#else /* not HAVE_MASTER_PASSWD */

static struct passwd *lookup(const char *username, uid_t uid);

struct passwd *al__getpwnam(const char *username)
{
  return lookup(username, 0);
}

struct passwd *al__getpwuid(uid_t uid)
{
  return lookup(NULL, uid);
}

/* If username is NULL, it's a lookup by uid; otherwise it's by name. */
static struct passwd *lookup(const char *username, uid_t uid)
{
  /* BSD 4.3 has /etc/passwd and /etc/passwd.{dir,pag}.  Only implement
   * reading /etc/passwd, since the DBM routines aren't reentrant and
   * we don't really need that level of performance in the login system
   * anyway. */
  FILE *fp;
  int success = 0, linesize = 0, len = (username) ? strlen(username) : 0;
  struct passwd *pwd;
  char *line, *buffer;
  const char *p;

  fp = fopen(PATH_PASSWD, "r");
  if (!fp)
    return NULL;

  while (al__read_line(fp, &line, &linesize) == 0)
    {
      /* See if we got the right entry. */
      if (username && (strncmp(line, username, len) != 0 || line[len] != ':'))
	continue;
      if (!username)
	{
	  p = strchr(line, ':');
	  if (p)
	    p = strchr(p + 1, ':');
	  if (!p || atoi(p + 1) != uid)
	    continue;
	}

      /* Allocate space for the return value. */
      buffer = malloc(sizeof(struct passwd) + strlen(line) + 1);
      if (!buffer)
	{
	  free(line);
	  fclose(fp);
	  return NULL;
	}
      pwd = (struct passwd *) buffer;
      buffer += sizeof(struct passwd);
      memcpy(buffer, line, strlen(line) + 1);

#if defined(BSD) || defined(ultrix)
      pwd->pw_quota = 0;
      pwd->pw_comment = "";
#endif

      /* Set the fields of the returned structure. */
#define NEXT_FIELD	buffer = strchr(buffer, ':'); \
      if (!buffer) { free(pwd); break; } *buffer++ = 0
#define FIELD(v) v = buffer; NEXT_FIELD
					   FIELD(pwd->pw_name);
      FIELD(pwd->pw_passwd);
      pwd->pw_uid = atoi(buffer);
      NEXT_FIELD;
      pwd->pw_gid = atoi(buffer);
      NEXT_FIELD;
      FIELD(pwd->pw_gecos);
      FIELD(pwd->pw_dir);
      pwd->pw_shell = buffer;
      buffer[strlen(buffer) - 1] = 0;
      fclose(fp);
      free(line);
      return pwd;
    }

  /* We lost. */
  if (linesize)
    free(line);
  fclose(fp);
  return NULL;
}
#endif /* HAVE_MASTER_PASSWD */

void al__free_passwd(struct passwd *pwd)
{
  free(pwd);
}

/*
 * This is an internal function.  Its contract is to read a line from a
 * file into a dynamically allocated buffer, zeroing the trailing newline
 * if there is one.  The initial value of *bufsize should be 0, and *buf
 * should be freed by the caller after use iff *bufsize is not 0.  This
 * function returns 0 if a line was successfully read, 1 if the file ended,
 * and -1 if there was an I/O error or if it ran out of memory.
 */

int al__read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*bufsize == 0)
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
