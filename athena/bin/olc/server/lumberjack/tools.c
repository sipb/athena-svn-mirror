/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the tools for lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1997 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: tools.c,v 1.2 1999-06-10 18:41:29 ghudson Exp $
 */

#if !defined(lint) && !defined(SABER)
static char rcsid[] ="$Id: tools.c,v 1.2 1999-06-10 18:41:29 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#include <olcd.h>
#include <lumberjack.h>

/*** Locking functions ***/

/* Create an exclusive lock for an entire file.
 * Arguments:   fd: open file descriptor for the file
 * Returns: -1 on failure, anything different (sigh) on success
 */
int do_lock(int fd)
{
#if defined(HAVE_FCNTL) && defined(F_SETLK) && defined(F_WRLCK)
  struct flock flk;

  flk.l_type = F_WRLCK;
  flk.l_whence = SEEK_SET;
  flk.l_start = 0;
  flk.l_len = 0;	/* lock to EOF, ie. the entire file */
  return fcntl(fd, F_SETLK, &flk);
#else /* not (HAVE_FCNTL and have F_SETLK and F_WRLCK) */
  return flock(fd, LOCK_EX | LOCK_NB);
#endif /* not (HAVE_FCNTL and have F_SETLK and F_WRLCK) */
}

/* Release an exclusive lock for an entire file.
 * Arguments:   fd: open (and locked) file descriptor for the file
 * Returns: -1 on failure, anything different (sigh) on success
 */
int do_unlock(int fd)
{
#if defined(HAVE_FCNTL) && defined(F_SETLK) && defined(F_WRLCK)
  struct flock flk;

  flk.l_type = F_UNLCK;
  flk.l_whence = SEEK_SET;
  flk.l_start = 0;
  flk.l_len = 0;	/* lock to EOF, ie. the entire file */
  return fcntl(fd, F_SETLK, &flk);
#else /* not (HAVE_FCNTL and have F_SETLK and F_WRLCK) */
  return flock(fd, LOCK_UN);
#endif /* not (HAVE_FCNTL and have F_SETLK and F_WRLCK) */
}

/*** I/O helpers ***/

/* Read a line of arbitrary length from an I/O stream.
 * Arguments:   fd:   the stream to read.  (Must be open for reading!)
 * Returns: A pointer to the data read, or NULL if an error or EOF
 *          occurs and no data is available.  Any terminating newlines
 *          will be stripped from the data.
 * Non-local returns: exits with exit code 1 if out of memory.
 */
char *get_line (FILE *fd)
{
  char *buf;
  size_t size, len;

  /* Allocate an initial buffer. */
  size = LINE_CHUNK;
  buf = malloc(size);
  if (buf == NULL)
    {
      syslog(LOG_ERR, "get_line: %m in malloc()");
      exit(1);
    }

  /* Read some data, returning NULL if we fail. */
  if (fgets(buf, size, fd) == NULL)
    return NULL;
 
  while (1)
    {
      len = strlen(buf);
      /* If the string is empty, return NULL.
       * (This shouldn't ever happen unless fd contains '\0' characters.)
       */
      if (len == 0)
	return NULL;
      /* If the string contains a full line, strip '\n' and return the rest. */
      if (buf[len-1] == '\n') {
	buf[len-1] = '\0';
	return buf;
      }
      /* If the line is shorter than buf is, we ran into EOF; return data. */
      if (len+1 < size)
	return buf;

      /* OK, the only other option is that our line was too short... */
      size += LINE_CHUNK;
      buf = realloc(buf, size);
      if (buf == NULL)
	{
	  syslog(LOG_ERR, "get_line: %m in malloc()");
	  exit(1);
	}

      /* Try reading more data and see if we fare better. */
      if (fgets(buf+len, size-len, fd) == NULL)
	return buf;
    }
}
