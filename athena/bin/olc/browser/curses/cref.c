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
static char *rcsid_cref_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.c,v 1.3 1986-01-22 18:02:27 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */

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
  char read_msg[LINE_LENGTH];		/* Secondary prompt. */
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
	  sprintf(read_msg, "Read section? %c", (char) command);
	  mvaddstr(LINES-2, 3, read_msg);
	  refresh();
	  *inbuf = (char) command;
	  getstr(inbuf+1);
	  move(LINES-2, 0);
	  clrtoeol();
	  refresh();
	  entry_index = atoi(inbuf);
	  display_entry(entry_index - 1);
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
