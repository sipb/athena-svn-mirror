/* This file is part of the CREF finder.  It contains the primary command
 * loop.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Created:	8/10/85
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_cref_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.c,v 1.5 1986-01-23 20:33:32 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <sys/file.h>			/* System file definitions. */

#include "cref.h"			/* CREF finder defs. */
#include "globals.h"			/* Global variable defs. */

/* Function:	main() is the starting point for the CREF finder.
 * Arguments:	argc:	Number of command line arguments.
 *		argv:	Array of command line arguments.
 * Returns:	Nothing.
 * Notes:
 *	The arguments to this function are not currently used.
 */

main(argc, argv)
     int argc;
     char *argv[];
{
  check_consultant();
  init_globals();
  parse_args(argc, argv);
  set_current_dir(Current_Dir);
  init_display();
  make_display();
  command_loop();
}

/* Function:	command_loop() takes commands and executes the
 *			appropriate functions.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

command_loop()
{
  int index;				/* Index in command table. */
  int command;				/* Input command. */
  int entry_index;			/* Index of entry. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  
  while (1)
    {
      move(LINES - 3, 3);
      addstr(CREF_PROMPT);
      clrtoeol();
      refresh();
      command = getch();
      if (isdigit(command))
	{
	  move(LINES - 3, strlen(CREF_PROMPT) + 3);
	  clrtoeol();
	  message(1, "");
	  mvaddstr(LINES-2, 3, "Read section? ");
	  refresh();
	  *inbuf = (char) command;
	  *(inbuf+1) = (char) NULL;
	  get_input(inbuf);
	  if (*inbuf != (char) NULL)
	    {
	      move(LINES-2, 0);
	      clrtoeol();
	      refresh();
	      entry_index = atoi(inbuf);
	      display_entry(entry_index - 1);
	    }
	}
      else if (command == '\n')
	continue;
      else if (command == ' ')
	next_page();
      else
	{
	  for (index = 0; index < Command_Count; index++)
	    {
	      if (command == (int) Command_Table[index].command)
		{
		  (*(Command_Table[index].procedure))();
		  break;
		}
	    }
	  if (index == Command_Count)
	    message(1, "Invalid command.  Type '?' for help.");
	}
    }
}	

/* Function:	parse_args() parse the command line.
 * Arguments:	argc:	Number of arguments.
 *		argv:	Argument vector.
 * Returns:	Nothing.
 * Notes:
 */

parse_args(argc, argv)
     int argc;
     char *argv[];
{
  extern int optind;			/* getopt() option index. */
  extern char *optarg;			/* getopt() argument pointer. */
  int c;				/* Option character. */
  char filename[FILENAME_SIZE];		/* Option filename directory. */

  while ( (c = getopt(argc, argv, "r:s:f:a:")) != EOF)
    switch (c)
      {
      case 'r':
	strcpy(filename, optarg);
	if (filename[0] != '/')
	  {
	    fprintf(stderr, "Invalid root directory %s\n", filename);
	    exit(ERROR);
	  }
	strcpy(Root_Dir, filename);
	strcpy(Current_Dir, Root_Dir);
	check_cref_dir(Current_Dir);
	break;
      case 's':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  {
	    fprintf(stderr, "Invalid default storage file %s\n", filename);
	    exit(ERROR);
	  }
	strcpy(Save_File, filename);
	break;
      case 'f':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  {
	    fprintf(stderr, "Invalid file offset %s\n", filename);
	    exit(ERROR);
	  }
	strcpy(Current_Dir, Root_Dir);
	strcat(Current_Dir, "/");
	strcat(Current_Dir, filename);
	check_cref_dir(Current_Dir);
	break;
      case 'a':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  {
	    fprintf(stderr, "Invalid abbreviation filename %s\n", filename);
	    exit(ERROR);
	  }
	set_abbrev_name(filename);
	break;
      case '?':
      default:
	fprintf(stderr, "cref: unknown option\n");
	usage();
	exit();
      }
}

/* Function:	usage() prints a usage summary for cref.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

usage()
{
  fprintf(stderr, "Usage: cref [-r rootdir] [-s savefile] [-f file_offset] ");
  fprintf(stderr, "[-a abbrev_file]");
}

/* Function:	check_cref_dir() checks to ensure that a directory is a
 *			valid CREF directory.  If not, it exits the program.
 * Arguments:	dir:	Directory to be checked.
 * Returns:	Nothing.
 * Notes:
 */

check_cref_dir(dir)
char *dir;
{
  char contents[FILENAME_SIZE];		/* Pathname of contents file. */
  int fd;				/* File descriptor. */

  strcpy(contents, dir);
  strcat(contents, "/");
  strcat(contents, CONTENTS);

  if ( (fd = open(contents, O_RDONLY, 0)) < 0)
    {
      fprintf(stderr, "%s not a CREF directory.\n", dir);
      exit(ERROR);
    }
  else
    {
      close(fd);
      return;
    }
}
