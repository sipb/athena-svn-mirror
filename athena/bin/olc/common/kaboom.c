/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains THINGS THAT GO KABOOM, bwahahahahahahaha!
 * [Specifically, the code to obrain a coredump from a running process.]
 *
 * Copyright (C) 1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: kaboom.c,v 1.1 1999-03-06 16:48:16 ghudson Exp $
 */

static char rcsid[] ="$Id: kaboom.c,v 1.1 1999-03-06 16:48:16 ghudson Exp $";

#include <mit-copyright.h>
#include "config.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <olc/olc.h>

#include <server_defines.h>

/* Move an old corefile (if any) out of the way.
 * Arguments:   none.
 * Returns:     nothing.
 */
void stash_olc_corefile(void)
{
  char new_name[FILENAME_MAX];    /* enough space for any pathname */
  time_t now;

  if (access(CORE_DIR "/core", F_OK) == 0)
    {
      /* A core file exists.  Deal. */

      /* Generate a file name containing the current date and time. */
      time(&now);
      strftime(new_name, sizeof(new_name),
	       CORE_DIR "/core.%Y-%m-%d_%H:%M",
	       localtime(&now));

      /* Stash the corefile under the new name, complain on failure. */
      if (rename(CORE_DIR "/core", new_name) < 0)
	{
	  olc_perror("stashing corefile failed");
	}
    }
}

/* Generate a core dump of the running process; move any previously
 * existing coredump out of the way before dumping.
 * Arguments:   none.
 * Returns:     nothing.
 */
void dump_current_core_image(void)
{
  /* fork() and abort() */
  switch (fork())
    {
    case -1:				/* fork error (in parent) */
      olc_perror ("kaboom(): fork()");
      return;
      break;
    case 0:				/* child */
      /* move any old core files out of the way */
      stash_olc_corefile();
      /* change to the coredump directory */
      if (chdir(CORE_DIR) < 0)
	{
	  olc_perror("kaboom(): can't change wdir");
	  return;
	}
      /* dump core */
      abort();
      olc_perror ("kaboom(): abort failed???");
      exit(2);
      break;
    }
}
