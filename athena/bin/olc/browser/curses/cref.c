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

#include <mit-copyright.h>

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <sys/file.h>			/* System file definitions. */
#include <strings.h>
#include "cref.h"			/* CREF finder defs. */
#include "globals.h"			/* Global variable defs. */

char *Prog_Name;			/* Name this program was invoked as */
int CREF;				/*  Are we running as CREF or not? */

#define DELETE	'\177'
#define CTRL_L	'\f'

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
  Prog_Name = rindex(*argv,'/');
  if(Prog_Name == (char *) NULL)
     Prog_Name = *argv;
  if(*Prog_Name == '/')
     ++Prog_Name;

  if(!strcmp(Prog_Name,"cref"))
    CREF = 1;
  else
    CREF = 0;

  if (init_display() != SUCCESS)
    {
      fprintf(stderr, "%s: Can't initialize display, not enough memory.\n",
	      Prog_Name);
      exit(ERROR);
    }
  init_globals();
  init_signals();
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
      addstr((CREF) ? CREF_PROMPT : STOCK_PROMPT);
      clrtoeol();
      refresh();
      noecho();
      command = getch();
      if (isdigit(command))
	{
	  move(LINES - 3, strlen((CREF) ? CREF_PROMPT : STOCK_PROMPT) + 3);
	  echo();
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
      else if (command == DELETE)
	{
	  message(1,"");
	  prev_page();
	}
      else if (command == CTRL_L)
	{
	  message(1,"");
	  redisplay();
	}
      else
	{
	  echo();
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
	    err_abort("Root directory must be full pathname:", filename);
	strcpy(Root_Dir, filename);
	strcpy(Current_Dir, Root_Dir);
	if (check_cref_dir(Current_Dir) != TRUE)
	  err_abort("No CREF index file in this directory:\n", Current_Dir);
	break;
      case 's':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  err_abort("Invalid default storage file:", filename);
	strcpy(Save_File, filename);
	break;
      case 'f':
	strcpy(filename, optarg);
	if (filename[0] == '-' || filename[0] == '/')
	  err_abort("Pathname should be relative to root dir:\n", filename);
	strcpy(Current_Dir, Root_Dir);
	strcat(Current_Dir, "/");
	strcat(Current_Dir, filename);
	if (check_cref_dir(Current_Dir) != TRUE)
	  err_abort("No CREF index file in this directory:\n", Current_Dir);
	break;
      case 'a':
	strcpy(filename, optarg);
	if (filename[0] == '-')
	  err_abort("Invalid abbreviation filename:", filename);
	strcpy(Abbrev_File, filename);
	break;
      case 'c':
	strcpy(filename, optarg);
	if (filename[0] != '/')
	    err_abort("New root directory must be full pathname:\n", filename);
	strcpy(Root_Dir, filename);
	strcpy(Current_Dir, Root_Dir);
	if (create_cref_dir(Current_Dir) != SUCCESS)
	  err_abort("Cannot create new root.", "");
	break;
      case '?':
      default:
	usage();
	err_abort("", "");
	break;
      }
}

/* Function:	usage() prints a usage summary for cref.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

usage()
{
  fprintf(stderr, "Usage: %s [-c new_rootdir] [-r existing_rootdir]\n",
	  Prog_Name);
  fprintf(stderr, "       [-s savefile] [-f file_offset] [-a abbrev_file]\n");
}
