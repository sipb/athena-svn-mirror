/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the lumberjack program, which gets rid of old logs.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v $
 *	$Id: lumberjack.c,v 1.2 1990-07-31 10:25:17 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/lumberjack/lumberjack.c,v 1.2 1990-07-31 10:25:17 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <stdio.h>

#include <olc/olc.h>
#include <olcd.h>

#include "lumberjack.h"		/* contains the name of the dir we want */

#define DSPIPE	"/usr/athena/dspipe" /* name of program to send off logs */

#ifdef OLX
#define PREFIX	"FIONAVAR.MIT.EDU:/usr/spool/discuss/ot" /* meeting prefix */
#else
#define PREFIX	"MATISSE.MIT.EDU:/usr/spool/discuss/o"
#endif

#define LOCKFILE  "lockfile"	/* name of lockfile */

#define SIZE	1024

main (argc, argv)
     int argc;
     char **argv;
{
  DIR *dirp;			/* pointer to directory */
  struct direct *next;		/* directory entry */
  int lock_fd;			/* file descriptor of lock file */
  int fd;			/* file descriptor of control file */
  FILE *file;			/* file stream used to read control file */

  char logname[SIZE];		/* name of log file */
  char title[SIZE];		/* title assigned to log */
  char topic[SIZE];		/* topic of question, also meeting name */
  char username[SIZE];		/* name of person that asked the question */

  char syscmd[SIZE];		/* buffer for constructing sys call */
  

/*
 *  Chdir to the directory containing the done'd logs, in case we dump
 *  core or something.
 */

  if (chdir(DONE_DIR))
    {
      fprintf(stderr, "lumberjack: unable to chdir to %s.\n",
	      DONE_DIR);
      exit(-1);
    }

/*
 *  If we can't open/create the lock file and lock it, exit.
 */

  if ((lock_fd = open(LOCKFILE, O_CREAT, 0)) <= 0)
    {
      fprintf(stderr, "lumberjack: unable to create/open file %s.\n", LOCKFILE);
      exit(-1);
    }
  if (flock(lock_fd, LOCK_EX | LOCK_NB))
    {
      fprintf(stderr, "lumberjack: unable to lock file %s.\n", LOCKFILE);
      close(lock_fd);
      exit(-1);
    }

/*
 *  Open the directory so we can get the entries out...
 */
  
  if ((dirp = opendir(".")) == NULL)
    {
      fprintf(stderr, "lumberjack: unable to malloc enough memory or open dir.\n");
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
	    fprintf(stderr, "lumberjack: unable to open file %s.\n", next->d_name);
	    continue;
	  }
	  file = fdopen(fd, "r");

	  if (fgets(logname, SIZE, file) == NULL)  {
	    fprintf(stderr, "lumberjack: unable to get logfilename from file %s.\n",
		    next->d_name);
	    fclose(file);
	    continue;
	  }
	  logname[strlen(logname) - 1] = '\0'; /* fgets leaves a '\n' on */
					       /* the end.  get rid of it. */

	  if (fgets(title, SIZE, file) == NULL)  {
	    fprintf(stderr, "lumberjack: unable to get title from file %s.\n",
		    next->d_name);
	    fclose(file);
	    continue;
	  }
	  title[strlen(title) - 1] = '\0';

	  if (fgets(topic, SIZE, file) == NULL)  {
	    fprintf(stderr, "lumberjack: unable to get topic from file %s.\n",
		    next->d_name);
	    fclose(file);
	    continue;
	  }
	  topic[strlen(topic) - 1] = '\0';

	  if (fgets(username, SIZE, file) == NULL)  {
	    fprintf(stderr, "lumberjack: unable to get username from file %s.\n",
		    next->d_name);
	    fclose(file);
	    continue;
	  }
	  username[strlen(username) - 1] = '\0';

/* If we've made it this far, we've got everything we need to ship to
 * discuss.
 */ 
	  sprintf(syscmd, "%s %s%s -t '%s: %s' < %s",
		  DSPIPE, PREFIX, topic, username, title, logname);
	  if (system(syscmd))
	    fprintf(stderr, "lumberjack: an error occurred in:\n\t%s\n", syscmd);
	  else
	    {
	      unlink(logname);
	      unlink(next->d_name);
	      fclose(file);
	    }
	}
    }

  closedir(dirp);
  flock(lock_fd, LOCK_UN);
}
