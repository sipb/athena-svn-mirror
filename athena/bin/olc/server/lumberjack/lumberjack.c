/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: lumberjack.c,v 1.23 1999-03-06 16:48:48 ghudson Exp $
 */

#if !defined(lint) && !defined(SABER)
static char rcsid[] ="$Id: lumberjack.c,v 1.23 1999-03-06 16:48:48 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include <sys/types.h>

#if HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include <sys/file.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef   HAVE_SYSLOG_H
#include   <syslog.h>
#ifndef    LOG_CONS
#define      LOG_CONS 0  /* if LOG_CONS isn't defined, just ignore it */
#endif     /* LOG_CONS */
#endif /* HAVE_SYSLOG_H */

#if !defined(WEXITSTATUS) || !defined(WTERMSIG)
/* BSD, need to define macro to get at exit status */
#define	WEXITSTATUS(st)	(st).w_retcode
#define	WTERMSIG(st)	(st).w_termsig
#endif

#include <olcd.h>		/* needed for SYSLOG_FACILITY */
#include <lumberjack.h>

/*** Support for shipping files to discuss ***/

/* Send off a file to Discuss.
 * Arguments:   fname:   filename for the data
 *              meeting: meeting path ("HOST.MIT.EDU:/usr/spool/discuss/name")
 *              subject: transaction subject
 * Returns: true if the relevant files can be removed.
 */
char send_to_discuss(char *fname, char *meeting, char *subject)
{
  int status;
  int fd;
  char err_buf[SYSLOG_LEN];
  size_t err_len;
  size_t size, i;
  pid_t child, pid;
  int rw[2];
  char *start, *end;

  /* create a pipe that will be used for the child's output. */
  if (pipe(rw) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "pipe: %m (not running dspipe)");
#endif /* HAVE_SYSLOG */
      return 0;
    }

  /* f**k off a child for running dspipe. */
  child = fork();
  if (child == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "fork: %m");
#endif /* HAVE_SYSLOG */
      return 0;
    }

  if (child == 0)
    {
      /* This is the child.  Set up stdout+err to the pipe, stdin from fname */

      close(rw[0]);	/* close the reading end of the pipe in child */
      if (rw[1] != 1)
	{
	  if (dup2(rw[1], 1) == -1)	/* send stdout into the pipe */
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_CRIT, "dup2(%d,1): %m", rw[1]);
#endif /* HAVE_SYSLOG */
	      return 0;
	    }
	}
      if (rw[1] != 2)
	{
	  if (dup2(rw[1], 2) == -1)	/* send stderr into the pipe */
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_CRIT, "dup2(%d,2): %m", rw[1]);
#endif /* HAVE_SYSLOG */
	      return 0;
	    }
	}
      if ((rw[1] != 1) && (rw[1] != 2))
	close(rw[1]);	/* close original pipe FD after we're done with it */

      fd = open(fname, O_RDONLY);
      if (fd == -1)
	{
#ifdef HAVE_SYSLOG
	  syslog(LOG_ERR, "open(%s): %m", fname);
#endif /* HAVE_SYSLOG */
	  return 0;
	}
      if (fd != 0)
	{
	  if (dup2(fd, 0) == -1)	/* send logfile to stdin */
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_CRIT, "dup2(%d,0): %m", fd);
#endif /* HAVE_SYSLOG */
	      return 0;
	    }
	  close(fd);
	}

      execl(DSPIPE_PATH, DSPIPE_PATH, meeting, "-t", subject, NULL);
      /* if we're here, execl failed. */
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "execl: %m");      
#endif /* HAVE_SYSLOG */
      return 0;
    }

  /* This is the parent.  Deal with the child's output and return status. */

  close(rw[1]);		/* close the writing end of the pipe in parent */

  /* We intentionally read and syslog the output from dspipe in
   * (relatively) small chunks, rather than allocating a dynamically sized
   * buffer, because we wish to enforce a limit on syslog message length
   * even if the line length is very large.  We simply read until EOF and
   * deal with the process later.  (Note: SIGCHLD will cause EINTR.)
   */

  err_len = 0;

  size = read(rw[0], err_buf, SYSLOG_LEN-1);
  while ((size == -1) && ((errno == EAGAIN) || (errno == EINTR)))
    size = read(rw[0], err_buf, SYSLOG_LEN-1);  /* redo on EAGAIN or EINTR */

  while (size > 0)
    {
      /* make all characters printable.  (removes NULs in the process) */
      for (i=0; i<size; i++)
	{
	  if (!isprint(err_buf[err_len+i]) && !isspace(err_buf[err_len+i]))
	    err_buf[err_len+i] = '?';
	}
      err_len += size;
      err_buf[err_len] = '\0';	/* NUL-terminate, so we can use string funcs */

      /* if we have a full line of output, ship it out. */
      for (start=err_buf, end=strchr(start, '\n') ;
	   end ;
	   start=end+1,     end=strchr(start, '\n'))
	{
	  *end = '\0';
	  if (start < end)
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_INFO, "dspipe: %s", start);
#endif /* HAVE_SYSLOG */
	    }
	}

      /* move the data to the beginning of the buffer. */
      if (start > err_buf)
	{
	  err_len = (err_buf + err_len) - start;
	  memmove(err_buf, start, err_len+1);  /* +1 is for terminating NUL */
	}

      /* if the buffer is full, ship all of it out anyway. */
      if (err_len == SYSLOG_LEN - 1)
	{
#ifdef HAVE_SYSLOG
	  syslog(LOG_INFO, "dspipe [buffer full]: %s", err_buf);
#endif /* HAVE_SYSLOG */
	  err_len = 0;
	}

      /* read more data */
      size = read(rw[0], err_buf+err_len, (SYSLOG_LEN-1)-err_len);
      while ((size == -1) && ((errno == EAGAIN) || (errno == EINTR)))
	size = read(rw[0], err_buf, (SYSLOG_LEN-1)-err_len);
    }

  err_buf[err_len] = '\0';	/* NUL-terminate, so we can use string funcs */

  /* ship it out whatever's left in the buffer. */
  if (err_len != 0)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_INFO, "dspipe: %s", err_buf);
#endif /* HAVE_SYSLOG */
    }

  close(rw[0]);	/* close pipe FD after we're done with it */

  /* get the child's exit status */
  pid = wait(&status);
  while (pid != child)
    {
      if (pid == -1)
	{
#ifdef HAVE_SYSLOG
	  syslog(LOG_ERR, "wait: %m");
#endif /* HAVE_SYSLOG */
	  if (errno != EINTR)
	    return 0;
	}
      else
	{
#ifdef HAVE_SYSLOG
	  syslog(LOG_ERR, "wait: got data for pid %d, child is pid %d",
		 pid, child);
#endif /* HAVE_SYSLOG */
	}

      pid = wait(&status);
    }

  if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) == 0)
	return 1;	/* Yay!  We made it! */

#ifdef HAVE_SYSLOG
      syslog(LOG_ERR, "dspipe exited with status %d", WEXITSTATUS(status));
#endif /* HAVE_SYSLOG */
    }
  else
    {
      /* received a signal */
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR, "dspipe exited on signal %d", WTERMSIG(status));
#endif /* HAVE_SYSLOG */
    }

  return 0;
}

/*** main ***/

/* Main body of the lumberjack program.
 * Note: This should probably be split up into several smaller, saner
 *    modules.  I did this with parts I made changes to, but it's not done.
 */
int main (int argc, char **argv)
{
  DIR *dirp;			/* pointer to directory */
  struct dirent *next;		/* directory entry */
  int lock_fd;			/* file descriptor of lock file */
  FILE *file;			/* file stream used to read control file */
  char *logname;		/* name of log file */
  char *title;			/* title assigned to log */
  char *topic;			/* topic of question, also meeting name */
  char *username;		/* name of person that asked the question */
  char *prefix;			/* prefix for discuss meeting names */

  char *meeting;		/* full name of the Discuss meeting */
  char *subject;		/* subject of the Discuss transaction */

  char *delim;			/* temporary ptr to delimiters in strings */

/*
 *  Set up syslogging
 */
#ifdef HAVE_SYSLOG
  openlog ("lumberjack", LOG_CONS | LOG_PID, SYSLOG_FACILITY);
#endif /* HAVE_SYSLOG */

/*
 *  Chdir to the directory containing the done'd logs, in case we dump
 *  core or something.
 */

  if (chdir(DONE_DIR))
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "chdir(%s): %m", DONE_DIR);
#endif /* HAVE_SYSLOG */
      exit(2);
    }

/*
 *  If we can't open/create the lock file and lock it, exit.
 */

  lock_fd = open(LOCKFILE, O_CREAT|O_RDWR, 0666);
  if (lock_fd < 0)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "open (lock file): %m");
#endif /* HAVE_SYSLOG */
      exit(2);
    }

  if (do_lock(lock_fd) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_ERR, "getting lock: %m");
#endif /* HAVE_SYSLOG */
      close(lock_fd);
      exit(2);
    }

/*
 * Find out where we're supposed to be putting these logs...
 */
  file = fopen(PREFIXFILE, "r");
  if (file == NULL)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "open (prefix file): %m");
#endif /* HAVE_SYSLOG */
      exit(2);
    }
  prefix = get_line(file);
  if (prefix == NULL)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "read (prefix file): %m");
#endif /* HAVE_SYSLOG */
      exit(2);
    }
  fclose(file);

  delim = strpbrk(prefix, " \t\n");
  if (delim != NULL)
    *delim = '\0';

/*
 *  Open the directory so we can get the entries out...
 */
  
  dirp = opendir(".");
  if (dirp == NULL)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_CRIT, "opendir: %m");
#endif /* HAVE_SYSLOG */
      if (do_unlock(lock_fd) == -1)
	{
#ifdef HAVE_SYSLOG
	  syslog(LOG_WARNING, "clearing lock: %m");
#endif /* HAVE_SYSLOG */
	}
      close(lock_fd);
      exit(2);
    }

/*
 *  Read out the entries and process the ones that begin with "ctrl".
 */

  while ((next = readdir(dirp)) != NULL)
    {
      if (!strncmp(next->d_name, "ctrl", 4))
	{
	  /* Open the control file.  Failure probably means another
	   * lumberjack process got it first.
	   */
	  file = fopen(next->d_name, "r");
	  if (file == NULL)
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_WARNING, "open(%s): %m", next->d_name);
#endif /* HAVE_SYSLOG */
	      continue;
	    }

	  /* Read control data. */
	  logname = get_line(file);
	  if ((logname == NULL) || (*logname == '\0'))
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "unable to get logfile name from %s: %m",
		     next->d_name);
#endif /* HAVE_SYSLOG */
	      fclose(file);
	      continue;
	    }
	  title = get_line(file);
	  if (title == NULL)	/* title may, in fact, be an empty string */
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "unable to get title from %s: %m",
		     next->d_name);
#endif /* HAVE_SYSLOG */
	      fclose(file);
	      continue;
	    }
	  topic = get_line(file);
	  if ((topic == NULL) || (*topic == '\0'))
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "unable to get topic from %s: %m",
		     next->d_name);
#endif /* HAVE_SYSLOG */
	      fclose(file);
	      continue;
	    }
	  username = get_line(file);
	  if ((username == NULL) || (*username == '\0'))
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "unable to get username from %s: %m",
		     next->d_name);
#endif /* HAVE_SYSLOG */
	      fclose(file);
	      continue;
	    }

	  fclose(file);

/* If we've made it this far, we've got everything we need to ship to
 * discuss.
 */ 
	  meeting = malloc(strlen(prefix)+strlen(topic)+1);
	  if (meeting == NULL)
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "%m in malloc()");
#endif /* HAVE_SYSLOG */
	      exit(1);
	    }
	  sprintf (meeting, "%s%s", prefix, topic);

	  subject = malloc(strlen(username)+strlen(title)+3);
	  if (subject == NULL)
	    {
#ifdef HAVE_SYSLOG
	      syslog(LOG_ERR, "%m in malloc()");
#endif /* HAVE_SYSLOG */
	      exit(1);
	    }
	  sprintf (subject, "%s: %s", username, title);

	  if (send_to_discuss(logname, meeting, subject))
	    {
	      /* success: remove the log file and the control file. */
	      unlink(logname);
	      unlink(next->d_name);
	    }
	}
    }
  closedir(dirp);

  if (do_unlock(lock_fd) == -1)
    {
#ifdef HAVE_SYSLOG
      syslog(LOG_WARNING, "clearing lock: %m");
#endif /* HAVE_SYSLOG */
    }
  close(lock_fd);

  exit(0);
}
