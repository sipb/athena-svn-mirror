/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * static char sccsid[] = "@(#)logwtmp.c	5.2 (Berkeley) 9/20/88";
 */

#ifndef lint
static char sccsid[] = "@(#)logwtmp.c	5.2 (Berkeley) 9/22/88";
#endif /* not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef POSIX
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#else
#include <ttyent.h>
#endif
#include <utmp.h>
#ifdef SOLARIS
#include <utmpx.h>
#endif
#ifdef ATHENA
#define UTMPFILE	"/etc/utmp"
#endif
#define	WTMPFILE	"/usr/adm/wtmp"
#ifdef SOLARIS
#define UTMPXFILE	"/etc/utmpx"
#define	WTMPXFILE	"/usr/adm/wtmpx"
#endif
static int fd;
#ifdef ATHENA
static int utmpfd = -1, ptrvalid = 0;
static off_t ptr;
#endif

#ifdef ATHENA

#ifndef MAX
#define MAX(x,y) ((x > y) ? x : y)
#endif

loguwtmp(linepid, name, host)
	char *linepid, *name, *host;
{
#ifndef SOLARIS
  struct utmp ut;
#else
  struct utmpx ut;
#endif
  struct stat buf;
  time_t time();
#ifndef _AIX
  char *strncpy();
#endif
  int cc, ttynum;
  char line[10];
  static int ftpline = 0, firsttime = 1;
#ifdef SOLARIS
    struct flock lock;
#endif
  if (firsttime)
    {
      if (utmpfd == -1)
#ifndef SOLARIS
	utmpfd = open(UTMPFILE, O_RDWR, 0);
#else
	utmpfd = open(UTMPXFILE, O_RDWR, 0);
#endif
      ptrvalid = 0; ptr = 0;

      /*
       * We find the entry in utmp we want to use only once,
       * then just continue using it until we exit.
       * Unfortunately, utmp handling doesn't work with
       * a general mechanism; it's dependent on assumptions
       * about ttys or small numbers of lines for both BSD
       * in general and in AIX convenience routines.
       */
      if (utmpfd >= 0)
	{
#if defined(_AIX) || defined(SOLARIS)
	  /*
	   * Under AIX, we run through all of utmp looking for
	   * the appropriate place to put an ftp entry.
	   */
	  /* 
	   * Same for SOLARIS
	   */
#else
	  /*
	   * Under BSD, the utmp entry location is determined
	   * by where your tty is in /etc/ttys. But we aren't
	   * using a tty. So we figure out how many ttys there
	   * are, and then write to utmp BEYOND the location
	   * of the last tty so we don't collide.
	   */
	  setttyent();
	  ttynum = 20; /* Safety in case /etc/ttys grows... */
	  while (getttyent() != 0)
	    ttynum++;
	  endttyent();
	  ptr = ttynum * sizeof(ut);
#endif

	  /*
	   * Standard tty handling in BSD doesn't require utmp
	   * to be locked since the /dev/tty* files provide the
	   * required locking mechanism. We have no such luxury;
	   * furthermore, locking of the utmp file is required
	   * under AIX, even if IBM's own software doesn't do it.
	   */
#ifndef SOLARIS
	  flock(utmpfd, LOCK_EX);
#else
          lock.l_type = F_WRLCK;
          lock.l_start = 0;
          lock.l_whence = 0;
       	  lock.l_len = 0;
          fcntl(utmpfd, F_SETLK, &lock);
#endif
	  lseek(utmpfd, ptr, L_SET);

	  /* Scan for a line with the name ftpX */
	  while ((cc = read(utmpfd, &ut, sizeof(ut))) == sizeof(ut))
	    {
	      if (!strncmp("ftp", ut.ut_line, 3))
		{
		  /* If the ftp line here is empty, we're set. Emptiness
		   * is determined by a null username under BSD, and
		   * type DEAD_PROCESS under AIX - because I say so.
		   * The AIX manpage is not rich in suggestions
		   * of how this _should_ be done, and other software
		   * varies. */
#if defined(_AIX) || defined(SOLARIS)
		  if (ut.ut_type == DEAD_PROCESS)
#else
		  if (ut.ut_name[0] == '\0')
#endif
		    break;
		  ftpline++;
		}
	      ptr += cc;
	    }
	  if (cc == 0) /* EOF: add a new entry; leave ptr unchanged */
	    {
	      if (!fstat(utmpfd, &buf))
		{
		  ptrvalid = 1;
		}
	      else
		{ /*
		   * The fstat should never fail. The only reason
		   * it is done here is to get the current length
		   * of the file should it later prove necessary
		   * to truncate it to its original length when
		   * a write only partially succeeds.
		   */
#ifndef SOLARIS
		  flock(utmpfd, LOCK_UN);
#else
		  lock.l_type = F_UNLCK;
		  lock.l_start = 0;
		  lock.l_whence = 0;
		  lock.l_len = 0;
		  fcntl(utmpfd, F_SETLK, &lock);
#endif
		  close(utmpfd);
		  utmpfd = -1;
		}
	    }
	  else
	    if (cc == sizeof(ut))
	      {
		ptrvalid = 1;
	      }
	    else /* The utmp file has a bad length. Don't touch it. */
	      {
#ifndef SOLARIS
		flock(utmpfd, LOCK_UN);
#else
		lock.l_type = F_UNLCK;
		lock.l_start = 0;
		lock.l_whence = 0;
		lock.l_len = 0;
		fcntl(utmpfd, F_SETLK, &lock);
#endif
		close(utmpfd);
		utmpfd = -1;
	      }
	}
    }

  memset(&ut, 0, sizeof(ut));
  if (ptrvalid)
    {
      /* Do this if we got to deal with utmp and got a real line number */
      sprintf(line, "ftp%d", ftpline);
      (void)strncpy(ut.ut_line, line, sizeof(ut.ut_line));
    }
  else /* otherwise, use the passed in line for wtmp logging only */
    (void)strncpy(ut.ut_line, linepid, sizeof(ut.ut_line));
  (void)strncpy(ut.ut_name, name, sizeof(ut.ut_name));
  (void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
#ifndef SOLARIS
  (void)time(&ut.ut_time);
#else
  (void) gettimeofday(&ut.ut_tv, NULL);
#endif
#if defined(_AIX) || defined(SOLARIS)
  /* Note that name is only \0 in the case where the program
   * is exiting. */
  ut.ut_type = (name[0] == '\0' ? DEAD_PROCESS : USER_PROCESS);
  ut.ut_exit.e_exit = 2;
  ut.ut_pid = getpid();
#endif

  if (ptrvalid)
    {
      lseek(utmpfd, ptr, L_SET);
      cc = write(utmpfd, &ut, sizeof(ut));
      if (firsttime && cc != sizeof(ut))
	{
	  (void)ftruncate(utmpfd, buf.st_size);
	  ptrvalid = 0;
#ifndef SOLARIS
	  flock(utmpfd, LOCK_UN);
#else
	  lock.l_type = F_UNLCK;
	  lock.l_start = 0;
	  lock.l_whence = 0;
	  lock.l_len = 0;
	  fcntl(utmpfd, F_SETLK, &lock);
#endif

	  close(utmpfd);
	  utmpfd = -1;
	}
      else
	if (firsttime)
#ifndef SOLARIS
	  flock(utmpfd, LOCK_UN);
#else
	  lock.l_type = F_UNLCK;
	  lock.l_start = 0;
	  lock.l_whence = 0;
	  lock.l_len = 0;
	  fcntl(utmpfd, F_SETLK, &lock);
#endif

      /* In case this was the first time around and we had it locked...
       * Note that the file lock is only necessary for when we allocate
       * our slot. Afterwards, until we release the slot, its ours.
       */
    }

  firsttime = 0;

#ifndef SOLARIS
  if (!fd && (fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) < 0)
    return;
  if (!fstat(fd, &buf)) {
    if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
	sizeof(struct utmp))
      (void)ftruncate(fd, buf.st_size);
  }
#else
  (void) updwtmpx(WTMPXFILE, &ut);
#endif
}

int user_logged_in(who)
     char *who;
{
#ifndef SOLARIS
  struct utmp ut;
#else
  struct utmpx ut;
#endif
  off_t p = 0;
#if defined(_AIX) || defined(SOLARIS)
  static int pid = 0;

  if (pid == 0)
    pid = getpid();
#endif

  if (utmpfd == -1)
    {
#ifndef SOLARIS
      utmpfd = open(UTMPFILE, O_RDWR, 0);
#else
      utmpfd = open(UTMPXFILE, O_RDWR, 0);
#endif
      if (utmpfd == -1)
	return 0; /* should this be what we do? XXX */
    }

  lseek(utmpfd, p, L_SET);
  while (read(utmpfd, &ut, sizeof(ut)) == sizeof(ut))
    {
      if (!strcmp(ut.ut_name, who) && 
#if defined(_AIX) || defined(SOLARIS)
	  (ut.ut_type == USER_PROCESS) &&
	  (ut.ut_pid != pid))
#else
	  ((ptrvalid && p != ptr) || !ptrvalid))
#endif
	return 1;
      else
	p += sizeof(ut);
    }

  return 0;
}
#endif /* ATHENA */

logwtmp(line, name, host)
	char *line, *name, *host;
{
#ifndef SOLARIS
	struct utmp ut;
#else
	struct utmpx ut;
#endif
	struct stat buf;
	time_t time();
#ifndef _AIX
	char *strncpy();
#endif

#ifndef SOLARIS
	if (!fd && (fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) < 0)
#else
	if (!fd && (fd = open(WTMPXFILE, O_WRONLY|O_APPEND, 0)) < 0)
#endif
		return;
	if (!fstat(fd, &buf)) {
	        memset(&ut, 0, sizeof(ut));
		(void)strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void)strncpy(ut.ut_name, name, sizeof(ut.ut_name));
		(void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
#ifndef SOLARIS
		(void)time(&ut.ut_time);
#else
		(void) gettimeofday(&ut.ut_tv, NULL);
#endif
#if defined(_AIX) || defined(SOLARIS)
		ut.ut_pid = getpid();
		if (name[0] != '\0')
		  ut.ut_type = USER_PROCESS;
		else
		  ut.ut_type = DEAD_PROCESS;
#endif /* _AIX */

		if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		    sizeof(struct utmp))
			(void)ftruncate(fd, buf.st_size);
	}
}
