/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v $
 *	$Id: lumberjack.c,v 1.5 1990-12-07 09:16:04 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v 1.5 1990-12-07 09:16:04 lwvanels Exp $";
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
  char timebuf[26];		/* buffer for storing ascii current time */

  char *temp;			/* pointer used for walking over title to */
				/* remove 's */

/*
 *  Chdir to the directory containing the done'd logs, in case we dump
 *  core or something.
 */

  if (chdir(DONE_DIR))
    {
      time_now(timebuf);
      fprintf(stderr, "%s: lumberjack: unable to chdir to %s.\n",
	      timebuf,DONE_DIR);
      exit(-1);
    }

/*
 *  If we can't open/create the lock file and lock it, exit.
 */

  if ((lock_fd = open(LOCKFILE, O_CREAT, 0)) <= 0)
    {
      time_now(timebuf);
      fprintf(stderr, "%s: lumberjack: unable to create/open file %s.\n",
	      timebuf, LOCKFILE);
      exit(-1);
    }
  if (flock(lock_fd, LOCK_EX | LOCK_NB))
    {
      time_now(timebuf);
      fprintf(stderr, "%s: lumberjack: unable to lock file %s.\n", timebuf,
	      LOCKFILE);
      close(lock_fd);
      exit(-1);
    }

/*
 *  Open the directory so we can get the entries out...
 */
  
  if ((dirp = opendir(".")) == NULL)
    {
      time_now(timebuf);
      fprintf(stderr, "%s: lumberjack: unable to malloc enough memory or open dir.\n", timebuf);
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
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: unable to open file %s.\n",
		    timebuf, next->d_name);
	    continue;
	  }
	  file = fdopen(fd, "r");

	  if (fgets(logname, SIZE, file) == NULL)  {
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: unable to get logfilename from file %s.\n",
		    timebuf, next->d_name);
	    fclose(file);
	    continue;
	  }
	  logname[strlen(logname) - 1] = '\0'; /* fgets leaves a '\n' on */
					       /* the end.  get rid of it. */

	  if (fgets(title, SIZE, file) == NULL)  {
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: unable to get title from file %s.\n",
		    timebuf, next->d_name);
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
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: unable to get topic from file %s.\n",
		    timebuf, next->d_name);
	    fclose(file);
	    continue;
	  }
	  topic[strlen(topic) - 1] = '\0';

	  if (fgets(username, SIZE, file) == NULL)  {
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: unable to get username from file %s.\n",
		    timebuf, next->d_name);
	    fclose(file);
	    continue;
	  }
	  username[strlen(username) - 1] = '\0';
	  fclose(file);

/* If we've made it this far, we've got everything we need to ship to
 * discuss.
 */ 
	  sprintf(syscmd, "%s %s%s -t \'%s: %s\' < %s",
		  DSPIPE, PREFIX, topic, username, title, logname);
	  retval = system(syscmd);
	  /* dspipe sometimes looses and returns a bogus error value (36096) */
	  if (retval != 0) {
	    time_now(timebuf);
	    fprintf(stderr, "%s: lumberjack: bad exit status %d occurred in:\n\t%s\n",
		    timebuf, retval, syscmd);
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
