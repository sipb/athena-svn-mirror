/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v $
 *	$Id: lumberjack.c,v 1.8 1990-12-12 17:00:43 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v 1.8 1990-12-12 17:00:43 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <stdio.h>

#include <olcd.h>

#include "lumberjack.h"		/* contains the name of the dir we want */

main (argc, argv)
     int argc;
     char **argv;
{
  DIR *dirp;			/* pointer to directory */
  struct direct *next;		/* directory entry */
  int lock_fd;			/* file descriptor of lock file */
  int fd;			/* file descriptor of control file */
  int retval;			/* Error code returned by system */
  FILE *file;			/* file stream used to read control file */

  char logname[SIZE];		/* name of log file */
  char title[SIZE];		/* title assigned to log */
  char topic[SIZE];		/* topic of question, also meeting name */
  char username[SIZE];		/* name of person that asked the question */

  char syscmd[SIZE];		/* buffer for constructing sys call */
  char logbuf[255];		/* buffer for constructing error message */

  char *temp;			/* pointer used for walking over title to */
				/* remove 's */
  char  prefix[128];		/* prefix for discuss meeting names */

/*
 *  Set up syslogging
 */

#if defined(ultrix)
#ifdef LOG_CONS
	openlog ("olc", LOG_CONS | LOG_PID);
#else
	openlog ("olc", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
	openlog ("olc", LOG_CONS | LOG_PID);
#else
	openlog ("olc", LOG_PID);
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

  if ((lock_fd = open(LOCKFILE, O_CREAT, 0)) < 0)
    {
      syslog(LOG_ERR,"open (lock file): %m");
      exit(-1);
    }
  if (flock(lock_fd, LOCK_EX | LOCK_NB))
    {
      syslog(LOG_ERR,"flock: %m");
      close(lock_fd);
      exit(-1);
    }

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
      flock(lock_fd, LOCK_UN);
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
	  
	  temp = title;
	  while (*temp != '\0') {
	    if (*temp == '\'') *temp = '\"';
	    temp++;
	  }


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
	  sprintf(syscmd, "%s %s%s -t \'%s: %s\' < %s",
		  DSPIPE, prefix, topic, username, title, logname);
	  retval = system(syscmd);
	  /* dspipe sometimes looses and returns a bogus error value (36096) */
	  if (retval != 0) {
	    sprintf(logbuf,"Bad exit status %d in: %s %%m", retval, syscmd);
	    syslog(LOG_WARNING,logbuf);
	  }
	  else
	    {
	      unlink(logname);
	      unlink(next->d_name);
	    }
	}
    }

  closedir(dirp);
  flock(lock_fd, LOCK_UN);
}
