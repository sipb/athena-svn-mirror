/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
 *	MIT Project Athena
 *
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

/* This file is part of the CREF finder.  It contains the primary command
 * loop.
 *
 *	$Source:
 *	$Author:
 *      $Header:
 */


#ifndef lint
static char *rcsid_cref_c = "$Header: ";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <sys/file.h>			/* System file definitions. */
#include <strings.h>
#include "cref.h"			/* CREF finder defs. */
#include "globals.h"			/* Global variable defs. */

/* Function:	main() is the starting point for the CREF finder.
 * Arguments:	argc:	Number of command line arguments.
 *		argv:	Array of command line arguments.
 * Returns:	Nothing.
 * Notes:
 */

main(argc, argv)
     int argc;
     char *argv[];
{
  init_display();
  init_globals();
  parse_args(argc, argv);
  set_current_dir(Current_Dir);
  make_abbrev_table();
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
	  message(1, "Read section:");
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
	      display_entry(entry_index);
	    }
	}
      else if (command == '\n')
	{
	  message(1,"");
	  continue;
	}
      else if (command == ' ')
	{
	  message(1,"");
	  next_page();
	}
      else
	{
	  message(1,"");
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

  while ( (c = getopt(argc, argv, "r:s:f:a:c:")) != EOF)
    switch (c)
      {
      case 'r':
	strcpy(filename, optarg);
	if (filename[0] != '/')
	    err_exit("Root directory must be full pathname:", filename);
	strcpy(Root_Dir, filename);
	strcpy(Current_Dir, Root_Dir);
	if (check_cref_dir(Current_Dir) != TRUE)
	  err_exit("No CREF index file in this directory:\n", Current_Dir);
	break;
      case 's':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  err_exit("Invalid default storage file:", filename);
	strcpy(Save_File, filename);
	break;
      case 'f':
	strcpy(filename, optarg);
	if (filename[0] == '-' || filename[0] == '/')
	  err_exit("Pathname should be relative to root dir:\n", filename);
	strcpy(Current_Dir, Root_Dir);
	strcat(Current_Dir, "/");
	strcat(Current_Dir, filename);
	if (check_cref_dir(Current_Dir) != TRUE)
	  err_exit("No CREF index file in this directory:\n", Current_Dir);
	break;
      case 'a':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  err_exit("Invalid abbreviation filename:", filename);
	strcpy(Abbrev_File, filename);
	break;
      case 'c':
	strcpy(filename, optarg);
	if (filename[0] != '/')
	    err_exit("New root directory must be full pathname:\n", filename);
	strcpy(Root_Dir, filename);
	strcpy(Current_Dir, Root_Dir);
	if (create_cref_dir(Current_Dir) != SUCCESS)
	  err_abort("cref: Cannot create new root.\n");
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
  fprintf(stderr, "Usage: cref [-c new_rootdir[-r existing_rootdir]\n");
  fprintf(stderr, "       [-s savefile] [-f file_offset] [-a abbrev_file]\n");
}
