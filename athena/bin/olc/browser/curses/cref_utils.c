/* This file is part of the CREF finder.  It contains miscellaneous useful
 *
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_cref_utils_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v 1.4 1986-01-25 15:08:06 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <sys/file.h>			/* System file definitions. */
#include <ctype.h>			/* Character type macros. */
#include <sys/param.h>			/* System parameters file. */
#include <sgtty.h>			/* TTY definitions. */
#include <grp.h>			/* System group defs. */

#include "cref.h"			/* Finder defs. */
#include "globals.h"			/* Global variable defs. */

/* Function:	err_abort() prints an error message and exits.
 * Arguments:	message:	Message to print.
 * Returns:	Doesn't return.
 * Notes:
 */

err_abort(message)
     char *message;
{
  move(LINES, 0);
  echo();
  noraw();
  endwin();
  fprintf(stderr, "%s\n", message);
  exit(ERROR);
}

/* Function:	err_exit() prints an error message and exits.
 * Arguments:	message:	Message to print.
 *		string:		Additional string to print.
 * Returns:	Doesn't return.
 * Notes:
 *	There are two differences between this function and err_abort():
 *	  (1) This is used before the curses window is initialized.
 *	  (2) This allows an additional string argument to be printed.
 */

err_exit(message, string)
     char *message;
     char *string;
{
  fprintf(stderr, "%s %s\n", message, string);
  exit(ERROR);
}

/* Function:	check_consultant() checks to see if the user is a
 *			consultant and sets the consultant flag.
 * Arguments:	None.
 * Returns:	TRUE if consultant, FALSE otherwise.
 * Notes:
 */

int
check_consultant()
{
  int group_ids[NGROUPS];			/* Array of group ID's. */
  struct group *consult_group;		/* Consult group entry. */
  int i;					/* Index variable. */
  
  if (getgroups(NGROUPS, group_ids) < 0)
    err_abort("cref: Unable to get group ID list.");
  if ( (consult_group = getgrnam(CONSULT_GROUP)) == NULL)
    err_abort("cref: Unable to get group entry.");
  for (i = 0; i < NGROUPS; i++)
    {
      if (group_ids[i] == consult_group->gr_gid)
	{
	  Is_Consultant = TRUE;
	  return(TRUE);
	}
    }
  Is_Consultant = FALSE;
  return(FALSE);
}

/* Function:	call_program() executes the named program by forking the
 *			main process.
 * Arguments:	program:	Name of the program to execute.
 *		argument:	Argument to be passed to the program.
 *	Note: Currently we support only a single argument, though it
 *	may be necessary to extend this later.
 * Returns:	An error code.
 * Notes:
 *	First, we fork a new process.  If the fork is unsuccessful, this
 *	fact is logged, and we return an error.  Otherwise, the child
 *	process (pid = 0) exec's the desired program, while the parent
 *	program waits for it to finish.
 */

ERRCODE
call_program(program, argument)
     char *program;				/* Name of program to be called. */
     char *argument;				/* Argument to be passed to program. */
{
  int pid;			/* Process id for forking. */
  char error[ERRSIZE];		/* Error message. */
  extern int errno;
  extern char *sys_errlist[];
  
  if ( (pid = fork() ) == -1)
    {
      sprintf(error, "Can't fork to execute %s\n", program);
      message(1, error);
      return(ERROR);
    }
  else if ( pid == 0 )
    {
      execlp(program, program, argument, 0);
      sprintf(error,"Error execing %s: %s", program,
	      sys_errlist[errno]);
      message(1, error);
      return(ERROR);
    }
  else	{
    wait(0);
    return(SUCCESS);
  }
}

/* Function:	get_input() gets an input string from the terminal.  It
 *			handles line editing even in CBREAK mode.
 * Arguments:	buffer:	Buffer to hold input string.  It may also be
 *			initialized with an a string to print at the
 *			beginning.
 * Returns:	Nothing.
 * Notes:
 */

get_input(buffer)
     char *buffer;
{
  struct sgttyb tty;			/* Terminal description structure. */
  char c;				/* Input character. */
  int x,y;				/* X,Y coordinates. */
  int x_start;				/* Starting X coordinate. */
  int length;				/* Length of input string.  */
  char *ptr;				/* Current character in buffer. */
  
  if ( ioctl(fileno(stdin), TIOCGETP, &tty) < 0 )
    {
      message(1, "Cannot access terminal.");
      *buffer = (char) NULL;
      return;
    }
  
  length = strlen(buffer);
  ptr = buffer + length - 1;
  noecho();
  getyx(stdscr, y, x);
  x++;
  x_start = x - length;
  addstr(buffer);
  refresh();
  while ( (c = getch()) != '\n')
    {
      if (c == tty.sg_erase)
	{
	  if (x > xstart)
	    {
	      x--;
	      move(y, x);
	      clrtoeol();
	      refresh();
	      *ptr = (char) NULL;
	      ptr--;
	    }
	}
      else if (c == tty.sg_kill)
	{
	  x = x_start;
	  move(y,x);
	  clrtoeol();
	  refresh();
	  ptr = buffer;
	  *buffer = (char) NULL;
	}
      else if (isalnum(c) || ispunct(c) || isspace(c))
	{
	  ptr++;
	  *ptr = c;
	  addch(c);
	  x++;
	  refresh();
	}
      else
	putchar('\007');
    }
  *(ptr+1) = (char) NULL;
  echo();
}

/* Function:	check_cref_dir() checks to ensure that a directory is a
 *			valid CREF directory
 * Arguments:	dir:	Directory to be checked.
 * Returns:	TRUE or FALSE.
 * Notes:
 */

check_cref_dir(dir)
     char *dir;
{
  char contents[FILENAME_SIZE];		/* Pathname of contents file. */
  int fd;				/* File descriptor. */
  
  make_path(dir, CONTENTS, contents);
  if ( (fd = open(contents, O_RDONLY, 0)) < 0)
    return(FALSE);
  else
    {
      close(fd);
      return(TRUE);
    }
}

/* Function:	make_path() concatenates two filenames into a single pathname.
 * Arguments:	head:	Head of new pathname.
 *		tail:	Tail of new pathname.
 *		buffer:	Buffer to hold new pathname.
 * Returns:	Nothing.
 * Notes:
 */

make_path(head, tail, buffer)
     char *head;
     char *tail;
     char *buffer;
{
  strcpy(buffer, head);
  strcat(buffer, "/");
  strcat(buffer, tail);
}
