/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
 *	MIT Project Athena
 *
 *	Lucien Van Elsen
 *	MIT Project Athena
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *      Permission to use, copy, modify, and distribute this program
 *      for any purpose and without fee is hereby granted, provided
 *      that this copyright and permission notice appear on all copies
 *      and supporting documentation, the name of M.I.T. not be used
 *      in advertising or publicity pertaining to distribution of the
 *      program without specific prior permission, and notice be given
 *      in supporting documentation that copying and distribution is
 *      by permission of M.I.T.  M.I.T. makes no representations about
 *      the suitability of this software for any purpose.  It is pro-
 *      vided "as is" without express or implied warranty.
 */

/* This file is part of the CREF finder.  It contains miscellaneous useful
 * utilities.
 *
 *	$Id: cref_utils.c,v 2.14 1999-06-28 22:51:38 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char *rcsid_cref_utils_c = "$Id: cref_utils.c,v 2.14 1999-06-28 22:51:38 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <stdio.h>			/* Standard I/O definitions. */
#include <stdlib.h>
#include <curses.h>			/* Curses package defs. */
#include <sys/types.h>
#include <sys/file.h>			/* System file definitions. */
#include <fcntl.h>
#include <ctype.h>			/* Character type macros. */
#include <sys/param.h>			/* System parameters file. */
#include <sys/wait.h>                   /* Exit status macros. */

#ifdef HAVE_TERMIO
#include <termio.h>
#else
#include <sgtty.h>
#endif

#include <grp.h>			/* System group defs. */
#include <sys/time.h>

#include <browser/cref.h>		/* Finder defs. */
#include <browser/cur_globals.h>	/* Global variable defs. */

/* Function:	err_abort() prints an error message and exits.
 * Arguments:	message:	Message to print.
 *		string:		Additional string to print.
 * Returns:	Doesn't return.
 * Notes:
 */

err_abort(message, string)
     char *message;
     char *string;
{
  move(LINES, 0);
  fprintf(stderr, "%s %s\n", message, string);
  wait_for_key();
  echo();
  noraw();
  endwin();
  exit(ERROR);
}

/* Function:	err_exit() prints an error message and exits.
 * Arguments:	message:	Message to print.
 *		string:		Additional string to print.
 * Returns:	Doesn't return.
 * Notes:
 *	The difference between this function and err_abort():
 *	This should be used before the curses window is initialized.
 */

err_exit(message, string)
     char *message;
     char *string;
{
  fprintf(stderr, "%s %s\n", message, string);
  exit(ERROR);
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
     char *program;			/* Name of program to be called. */
     char *argument;			/* Argument to be passed to program. */
{
  int pid;				/* Process id for forking. */
  char error[ERRSIZE];			/* Error message. */
  int status;                           /* Return status of child. */
  
  pid = fork();
  if (pid == -1)
    {
      sprintf(error, "Can't fork to execute %s\n", program);
      message(1, error);
      return(ERROR);
    }
  else if (pid == 0)
    {
      execlp(program, program, argument, 0);
      sprintf(error,"Error execing %s: %s", program, strerror(errno));
      message(1, error);
      _exit(37); /* Hopefully 37 is a safe flag value. */
    }
  else
    {
      wait(&status);
      if((WIFEXITED(status)) && (WEXITSTATUS(status) == 37))
	return(ERROR); /* The execlp failed. */
      else
	return(SUCCESS);
    }
}

/*
 * Function:	display_file() prints a file on a user's terminal.
 * Arguments:	filename:	Name of file to be printed.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, open the file to make sure that it is accessible. If it
 *	is not, log an error and return. Otherwise, attempt to execute
 *	the desired pager ($PAGER or DEFAULT_PAGER (defined in cref.h)
 *	to page the file on the terminal. If this fails, simply print
 *	it line by line. In either case, end by closing the file and
 *	returning.
 */

ERRCODE
display_file(filename)
     char *filename;
{
  FILE *file;                  /* File structure pointer. */
  char *pager;
  int c;
	
  file = fopen(filename, "r");
  if(file == NULL) 
    {
      fprintf(stderr, "display_file: Unable to open file %s\n",
	      filename);
      return(ERROR);
    }

  pager = getenv("PAGER");
  if(pager == NULL)
    pager = DEFAULT_PAGER;

  if(call_program(pager, filename) == ERROR) 
    {
      /* Fall back to displaying the file ourselves. */
      while((c = getc(stdin)) != EOF)
	putc(c, stdout);
      printf("\n");
    }
	
  fclose(file);
  return(SUCCESS);
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
#ifdef HAVE_TERMIO
  struct termio tty;			/* Terminal description structure. */
#else
  struct sgttyb tty;			/* Terminal description structure. */
#endif
  char c;				/* Input character. */
  int x,y;				/* X,Y coordinates. */
  int x_start;				/* Starting X coordinate. */
  int length;				/* Length of input string.  */
  char *ptr;				/* Current character in buffer. */
  
#ifdef HAVE_TERMIO
  if ( ioctl(fileno(stdin), TCGETA, &tty) < 0 )
#else
  if ( ioctl(fileno(stdin), TIOCGETP, &tty) < 0 )
#endif
    {
      message(1, "Cannot access terminal.");
      *buffer = (char) NULL;
      return;
    }
  
  length = strlen(buffer);
  ptr = buffer + length;
  noecho();
  addstr(buffer);
  getyx(stdscr, y, x);
  x_start = x - length;
  refresh();
  while ( (c = getch()) != '\n')
    {
#ifdef HAVE_TERMIO
      if (c == tty.c_cc[VERASE])
#else
      if (c == tty.sg_erase)
#endif
	{
	  if (x > x_start)
	    {
	      x--;
	      move(y, x);
	      clrtoeol();
	      refresh();
	      *ptr = (char) NULL;
	      ptr--;
	    }
	  else if (x == x_start)
	    {
	      move(y, x);
	      clrtoeol();
	      refresh();
	      *ptr = (char) NULL;
	    }
	}
#ifdef HAVE_TERMIO
      else if (c == tty.c_cc[VKILL])
#else
      else if (c == tty.sg_kill)
#endif
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
	  *ptr = c;
	  ptr++;
	  x++;
	  addch(c);
	  refresh();
	}
      else
	putchar('\007');
    }
  *(ptr) = (char) NULL;
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
  char contents[MAXPATHLEN];		/* Pathname of contents file. */
  int fd;				/* File descriptor. */
  
  make_path(dir, CONTENTS, contents);
  fd = open(contents, O_RDONLY, 0);
  if (fd < 0)
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

/* Function:	copy_file() makes a copy of a file.
 * Arguments:	src_file:	Source filename.
 *		dest_file:	Destination filename.
 * Returns:	Nothing.
 * Notes:
 */

copy_file(src_file, dest_file)
     char *src_file;
     char *dest_file;
{
  int in_fd;				/* Input file descriptor. */
  int out_fd;				/* Output file descriptor */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char error[ERRSIZE];			/* Error message. */
  int nbytes;				/* Number of bytes read. */

  in_fd = open(src_file, O_RDONLY, 0);
  if (in_fd < 0)
    {
      sprintf(error, "Unable to open input file %s\n", src_file);
      message(1, error);
      return(ERROR);
    }
  out_fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, CLOSED_FILE);
  if (out_fd < 0)
    {
      sprintf(error, "Unable to open output file %s\n", dest_file);
      message(1, error);
      return(ERROR);
    }
  while ( (nbytes = read(in_fd, inbuf, LINE_LENGTH)) == LINE_LENGTH)
    write(out_fd, inbuf, LINE_LENGTH);
  write(out_fd, inbuf, nbytes);
  close(out_fd);
  close(in_fd);
}

/* Function:	wait_for_key() prints a message at the bottom of the
 *			screen and waits for the user to type a key.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

wait_for_key()
{
  standout();
  mvaddstr(LINES-1, 0, " Hit any key to continue...");
  standend();
  refresh();
  noecho();
  getch();
  echo();
}

/* Function:	create_cref_dir() makes a new CREF directory with an
 *			empty contents file.
 * Arguments:	dir:	Name of directory to create.
 * Returns:	Nothing.
 * Notes:
 */

create_cref_dir(dir)
     char *dir;
{
  FILE *fp;				/* FILE pointer. */
  char contents[MAXPATHLEN];		/* Name of contents file. */
      
  make_path(dir, CONTENTS, contents);
  if (mkdir(dir, CLOSED_DIR) < 0)
    {
      printf("\nUnable to create directory %s.\n", dir);
      return(ERROR);
    }
  else
    {
      fp = fopen(contents, "w");
      if (fp == NULL)
	{
	  printf("\nUnable to create file %s\n", contents);
	  return(ERROR);
	}
      else
	{
	  fclose(fp);
	  return(SUCCESS);
	}
    }
}

log_status(logfile,logstring)
     char *logfile;
     char *logstring;
{
	FILE *fp;
	long current_time;

        fp = fopen(logfile,"a");

		
	time(&current_time);

	fprintf(fp,"%.24s\n %s\n",ctime(&current_time),logstring);
	
	fclose(fp);
	
	return(SUCCESS);
}
