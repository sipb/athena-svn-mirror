/* This file is part of the CREF finder.  It contains the display routines.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/display.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_display_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/display.c,v 1.2 1986-01-22 18:02:45 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */

#include "cref.h"			/* Finder defs. */
#include "globals.h"			/* Global variables. */

/* Function:	init_display() initializes the CREF display.
 * Arguments:	None.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Initialize the terminal by setting up curses, going into CBREAK
 *	mode, and making sure that characters are echoed.
 */

ERRCODE
init_display()
{
  initscr();
  crmode();
  echo();
  clear();
  refresh();
}

/* Function:	center() centers a line on the display.  The line is
 *			not displayed until a call to refresh() is made.
 * Arguments:	row:	Number of row to be centered.
 *		text:	Text to be centered.
 * Returns:	Nothing.
 * Notes:
 */

center(row, text)
     int row;
     char *text;
{
  int start_column;
  
  start_column = (COLS/2) - (strlen(text)/2);
  move(row, start_column);
  addstr(text);
}

#ifdef TEST
message(line, msg)
     int line;
     char *msg;
{
  fprintf(stderr, "%s\n", msg);
}
#else
/* Function:	message() prints a message in the area at the bottom of
 *			the screen.
 * Arguments:	line:	Which of the two bottom lines to use.
 *		text:	Message to print.
 * Returns:	Nothing.
 * Notes:
 */

message(line, text)
     int line;
     char *text;
{
  int row;
  
  move(LINES-1,0);
  clrtoeol();
  move(LINES-2,0);
  clrtoeol();
  refresh();
  
  if (line == 1)
    row = LINES - 2;
  else if (line == 2)
    row = LINES-1;
  else
    {
      mvaddstr(LINES-1, 0, "message: invalid line number.");
      refresh();
      return;
    }
  mvaddstr(row, 0, text);
  refresh();
}
#endif

/* Function:	make_display() creates a menu display for the current
 *			directory.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

make_display()
{
  ENTRY *curr_entry;			/* Current index entry. */
  char current_dir[FILENAME_SIZE];	/* Current directory. */
  char display_line[LINE_LENGTH];	/* Line to display. */
  int max_index_lines;			/* Max # to display. */
  int start_index;			/* Index to start with. */
  int curr_line;			/* Current screen line. */
  int curr_index;			/* Current index. */
  int index_line;			/* Current index line. */
  
  clear();
  strcpy(current_dir, Current_Dir);
  center(0, CREF_HEADER);
  center(1, current_dir);
  curr_index = Index_Start;
  curr_line = 4;
  
  for (index_line = 1; index_line < MAX_INDEX_LINES; index_line++)
    {
      curr_entry = get_entry(curr_index);
      if (curr_entry == NULL)
	break;
      move(curr_line, 15);
      sprintf(display_line, "%3d", curr_index + 1);
      if (curr_entry->type == CREF_DIR)
	strcat(display_line, "* ");
      else
	strcat(display_line, "  ");
      strcat(display_line, curr_entry->title);
      addstr(display_line);
      curr_line++;
      curr_index++;
    }
  if (curr_index >= Entry_Count)
    center(curr_line + 2, "** End of Index **");
  else
    center(curr_line + 2, "** More **");
}

/* Function:	display_entry() displays a CREF entry on the screen.
 * Arguments:	Index:	Index of entry to be displayed.
 * Returns:	Nothing.
 * Notes:
 *	If the entry is a file, it is shown using "more".  If it is a
 *	directory, we simply change the current directory and redisplay.
 */

display_entry(index)
     int index;
{
  ENTRY *entry;				/* Entry to be displayed. */
  
  if ( (entry = get_entry(index)) == NULL)
    {
      message(1, "Invalid entry number.");
      return;
    }
  Current_Index = index;
  if (entry->type == CREF_FILE)
    {
      clear();
      refresh();
      call_program("more", entry->filename);
      standout();
      mvaddstr(LINES-1, 0, "Hit any key to continue");
      standend();
      refresh();
      getch();
      clear();
      make_display();
    }
  else
    {
      Previous_Index = Current_Index;
      set_current_dir(entry->filename);
      make_display();
    }
}
