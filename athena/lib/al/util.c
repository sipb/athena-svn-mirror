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

static const char rcsid[] = "$Id: util.c,v 1.8 1999-09-02 14:37:20 ghudson Exp $";

#include <sys/param.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
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
    "You are not allowed to log into this machine",
    "Logins currently disabled",
    "You are not allowed to log into this machine remotely",
    "Could not add user to passwd file",
    "Could not modify user's session record",
    "Permission denied while modifying system files",
    "No such file or directory",
    "Out of memory",
    "Bad session record overwritten",
    "Could not add you to group file",
    "Using a temporary home directory created by a previous login",
    "Attach failed; you have a temporary home directory",
    "Attach failed; you have no home directory",
    "Home directory attach is disabled on this machine"
  };

  assert(code >= 0 && code < (sizeof(errtext) / sizeof(*errtext)));
  return errtext[code];
}

void al_free_errmem(char *mem)
{
  /* Do nothing for now. */
}

/* The next couple of functions (al__getpwnam() and al__getpwuid())
 * are here because libal, being a library, shouldn't be stomping on
 * the static memory returned by the native operating system's
 * getpwnam() and getpwuid() calls.  Unfortunately, we have to write a
 * lot of code which does the same thing as libc does.  Some day we
 * may be able to assume that all modern platforms have getpwnam_r()
 * and getpwuid_r() and have a single, simple version of these
 * functions.
 */

#ifdef HAVE_MASTER_PASSWD
static struct passwd *lookup(const DBT *key);

struct passwd *al__getpwnam(const char *username)
{
  DBT key;
  char buf[UT_NAMESIZE + 1];
  int len;

  /* Paranoia: don't find an empty username. */
  if (!*username)
    return NULL;
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
  db = _dbopen(_PATH_SMP_DB, O_RDONLY, 0, DB_HASH, NULL);
  if (!db)
    db = _dbopen(_PATH_MP_DB, O_RDONLY, 0, DB_HASH, NULL);
  if (!db)
    return NULL;

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

#else /* HAVE_MASTER_PASSWD */

static struct passwd *lookup(const char *username, uid_t uid);

struct passwd *al__getpwnam(const char *username)
{
  /* Paranoia: don't find an empty username. */
  if (!*username)
    return NULL;
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
  int linesize, len = (username) ? strlen(username) : 0;
  struct passwd *pwd;
  char *line = NULL, *buffer;
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
      strcpy(buffer, line);

#if defined(BSD) || defined(ultrix)
      pwd->pw_quota = 0;
      pwd->pw_comment = "";
#endif

      /* Set the fields of the returned structure. */
#define BAD_LINE	{ free(pwd); break; }
#define NEXT_FIELD	{ buffer = strchr(buffer, ':'); \
	if (!buffer) BAD_LINE; *buffer++ = 0; }
#define FIELD(v)	{ v = buffer; NEXT_FIELD; }
      FIELD(pwd->pw_name);
      FIELD(pwd->pw_passwd);
      if (!isdigit(*buffer))
	BAD_LINE;
      pwd->pw_uid = atoi(buffer);
      NEXT_FIELD;
      if (!isdigit(*buffer))
	BAD_LINE;
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
  free(line);
  fclose(fp);
  return NULL;
}
#endif /* HAVE_MASTER_PASSWD */

void al__free_passwd(struct passwd *pwd)
{
  free(pwd);
}

/* This is an internal function.  Its contract is to read a line from a
 * file into a dynamically allocated buffer, zeroing the trailing newline
 * if there is one.  The calling routine may call al__read_line multiple
 * times with the same buf and bufsize pointers; *buf will be reallocated
 * and *bufsize adjusted as appropriate.  The initial value of *buf
 * should be NULL.  After the calling routine is done reading lines, it
 * should free *buf.  This function returns 0 if a line was successfully
 * read, 1 if the file ended, and -1 if there was an I/O error or if it
 * ran out of memory.
 */

int al__read_line(FILE *fp, char **buf, int *bufsize)
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

/* Disallow usernames which might walk the filesystem, usernames which
 * contain nonprintables, or usernames which might not play nice with the
 * passwd file. */
int al__username_valid(const char *username)
{
  if (!*username || *username == '.')
    return 0;
  for (; *username; username++)
    {
      if (!isprint(*username) || *username == '/' || *username == ':')
	return 0;
    }
  return 1;
}
