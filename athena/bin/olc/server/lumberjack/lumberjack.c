/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v $
 *	$Id: lumberjack.c,v 1.18 1992-01-07 20:15:03 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v 1.18 1992-01-07 20:15:03 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/dir.h>
#if defined(_IBMR2) && defined(ZEPHYR)
/* Conflict in definitions between AIX's dir.h and zephyr.h for STAT; keep */
/* the Zephyr one */
#undef STAT
#endif
#include <sys/file.h>
#include <sys/wait.h>
#include <strings.h>
#include <stdio.h>
#ifdef _POSIX_SOURCE
#include <dirent.h>
#include <fcntl.h>
#endif

#include <olcd.h>

#include <lumberjack.h>		/* contains the name of the dir we want */

main (argc, argv)
     int argc;
     char **argv;
{
  DIR *dirp;			/* pointer to directory */
#ifdef _POSIX_SOURCE
  struct dirent *next;
#else
  struct direct *next;		/* directory entry */
#endif
  int lock_fd;			/* file descriptor of lock file */
  int fd;			/* file descriptor of control file */
  int retval;			/* Error code returned by system */
  FILE *file;			/* file stream used to read control file */
#ifdef _POSIX_SOURCE
  int status;
#else
  union wait status;
#endif
#ifdef _POSIX_SOURCE
  struct flock flk;
#endif
  char logname[SIZE];		/* name of log file */
  char title[SIZE];		/* title assigned to log */
  char topic[SIZE];		/* topic of question, also meeting name */
  char username[SIZE];		/* name of person that asked the question */

  char logbuf[SIZE];		/* buffer for constructing error message */

  char  prefix[128];		/* prefix for discuss meeting names */
  char *temp;

/*
 *  Set up syslogging
 */

#ifdef mips
#ifdef LOG_CONS
	openlog ("rpd", LOG_CONS | LOG_PID);
#else
	openlog ("rpd", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
	openlog ("rpd", LOG_CONS | LOG_PID, SYSLOG_LEVEL);
#else
	openlog ("rpd", LOG_PID, SYSLOG_LEVEL);
#endif /* LOG_CONS */
#endif /* ultrix */

/*
 *  Chdir to the directory containing the done'd logs, in case we dump
 *  core or something.
 */

  if (chdir(DONE_DIR))
    {
      syslog(LOG_ERR,"chdir: %m");
      exit(-1);
    }

/*
 *  If we can't open/create the lock file and lock it, exit.
 */

  if ((lock_fd = open(LOCKFILE, O_CREAT|O_RDWR, 0666)) < 0)
    {
      syslog(LOG_ERR,"open (lock file): %m");
      exit(-1);
    }
#ifdef _POSIX_SOURCE
  flk.l_type = F_WRLCK;
  flk.l_whence = SEEK_SET;
  flk.l_start = 0;
  flk.l_len = 1;
  if (fcntl(lock_fd,F_SETLK,&flk) == -1) {
    syslog(LOG_ERR,"getting lock: %m");
    close(lock_fd);
    exit(-1);
  }
#else
  if (flock(lock_fd, LOCK_EX | LOCK_NB))
    {
      syslog(LOG_ERR,"flock: %m");
      close(lock_fd);
      exit(-1);
    }
#endif

/*
 * Find out where we're supposed to be putting these logs...
 */
  if ((fd = open(PREFIXFILE, O_RDONLY)) < 0) {
    syslog(LOG_ERR,"open (prefix file): %m");
    exit(-1);
  }
  retval = read(fd,prefix,128);
  if (retval == -1) {
    syslog(LOG_ERR,"read (prefix file): %m");
    exit(-1);
  }
  close(fd);
  temp = index(prefix,'\n');
  if (temp != NULL) *temp = '\0';
  temp = index(prefix,' ');
  if (temp != NULL) *temp = '\0';

/*
 *  Open the directory so we can get the entries out...
 */
  
  if ((dirp = opendir(".")) == NULL)
    {
      syslog(LOG_ERR,"opendir: %m");
      close(lock_fd);
#ifdef _POSIX_SOURCE
      flk.l_type = F_UNLCK;
      if (fcntl(lock_fd,F_SETLK,&flk) == -1) {
	syslog(LOG_ERR,"clearing lock: %m");
	close(lock_fd);
      }
#else
      flock(lock_fd, LOCK_UN);
#endif
      exit(-1);
    }

/*
 *  Read out the entries and process the ones that begin with "ctrl".
 */

  while ((next = readdir(dirp)) != NULL)
    {
      if (!strncmp(next->d_name, "ctrl", 4))
	{
	  if ((fd = open(next->d_name, O_RDONLY, 0)) <= 0)  {
	    sprintf(logbuf,"open (%s) %%m",next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    continue;
	  }
	  file = fdopen(fd, "r");

	  if (fgets(logname, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get logfilename from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  logname[strlen(logname) - 1] = '\0'; /* fgets leaves a '\n' on */
					       /* the end.  get rid of it. */

	  if (fgets(title, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get title from %s %%m", next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  title[strlen(title) - 1] = '\0';
	  
	  if (fgets(topic, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get topic from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  topic[strlen(topic) - 1] = '\0';

	  if (fgets(username, SIZE, file) == NULL)  {
	    sprintf(logbuf,"unable to get time from %s %%m",
		    next->d_name);
	    syslog(LOG_WARNING,logbuf);
	    fclose(file);
	    continue;
	  }
	  username[strlen(username) - 1] = '\0';
	  fclose(file);

/* If we've made it this far, we've got everything we need to ship to
 * discuss.
 */ 
#ifdef NO_VFORK
	  retval = fork();
#else
	  retval = vfork();
#endif
	  if (retval == -1) {
	    perror("lumberjack: fork");
	    continue;
	  }
	  if (retval == 0) {
	    char av1[SIZE], av3[SIZE]; 
	    int fd = open(logname, O_RDONLY);
	    if (fd == -1) {
	      perror("lumberjack: open");
	      return -errno;
	    }
	    if (fd != 0)
	      if (dup2(fd, 0) == -1) {
		perror("lumberjack: dup2");
		return -errno;
	      }
	    sprintf (av1, "%s%s", prefix, topic);
	    sprintf (av3, "%s: %s", username, title);
	    retval = execl(DSPIPE, DSPAV0, av1, "-t", av3, NULL);
	    perror("lumberjack: execl");
	    return (-retval);
	  }
 	  /* Assume dspipe is the only child */
 	  retval = wait(&status);
 	  if (retval == -1) {
	    perror("lumberjack: wait");
	    continue;
	  }
 	  if (WIFEXITED(status)) {
	    /* dspipe sometimes loses and returns a bogus error value (36096) */
#ifdef _POSIX_SOURCE
	    if (WEXITSTATUS(status) != 0) {
	      fprintf(stderr, "lumberjack: %s exited %d\n", DSPIPE,
		      WEXITSTATUS(status));
#else
	    if (status.w_retcode != 0) {
	      fprintf(stderr, "lumberjack: %s exited %d\n", DSPIPE,
		      status.w_retcode);
#endif
	    } else {
	      unlink(logname);
	      unlink(next->d_name);
	    }
	  } else /* signal */
#ifdef _POSIX_SOURCE
	    fprintf(stderr, "lumberjack: %s exited with signal %d\n",
		    DSPIPE, WTERMSIG(status));
#else
	    fprintf(stderr, "lumberjack: %s exited with signal %d\n",
		    DSPIPE, status.w_stopsig);
#endif
	}
    }
  closedir(dirp);
#ifdef _POSIX_SOURCE
  flk.l_type = F_UNLCK;
  if (fcntl(lock_fd,F_SETLK,&flk) == -1) {
    syslog(LOG_ERR,"clearing lock: %m");
    close(lock_fd);
  }
#else
  flock(lock_fd, LOCK_UN);
#endif
}
