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


/* This file is part of the CREF finder.  It contains the display routines.
 *
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/display.c,v $
 *	$Author: ghudson $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/display.c,v 2.9 1997-04-30 17:28:02 ghudson Exp $
 */


#ifndef lint
static char *rcsid_display_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/display.c,v 2.9 1997-04-30 17:28:02 ghudson Exp $";
#endif

#include <mit-copyright.h>

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */

#include <browser/cref.h>		/* Finder defs. */
#include <browser/cur_globals.h>	/* Global variables. */

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
  if (! initscr())
    return(ERROR);
  cbreak();
  echo();
  clear();
  refresh();
  return(SUCCESS);
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

/* #ifdef TEST
   message(line, msg)
   int line;
   char *msg;
   {
   fprintf(stderr, "%s\n", msg);
   }
   #else
*/
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
  
  standout();
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
  mvaddstr(row, 3, text);
  refresh();
  standend();
  move(row,strlen(text) + 4);
  refresh();
}
/* #endif */

messages(first_line,second_line)
     char *first_line;
     char *second_line;
{
  move(LINES-2,0);
  clrtoeol();
  move(LINES-1,0);
  clrtoeol();
  refresh();
  standout();
  mvaddstr(LINES-2,3,first_line);
  mvaddstr(LINES-1,3,second_line);
  standend();
  refresh();
}


/* Function:	make_display() creates a menu display for the current
 *			directory.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

make_display()
{
  ENTRY *curr_entry;			/* Current index entry. */
  char current_dir[MAXPATHLEN];	/* Current directory. */
  char display_line[LINE_LENGTH];	/* Line to display. */
  int curr_line;			/* Current screen line. */
  int curr_ind;			/* Current index. */
  int ind_line;			/* Current index line. */
  
  for (curr_line = 0; curr_line < LINES - 2 ; curr_line++)
    {
      move(curr_line, 0);
      clrtoeol();
    }
  strcpy(current_dir, Current_Dir);
  center(0, Header);
  center(1, current_dir);
  curr_ind = Ind_Start;
  curr_line = 3;
  
  for (ind_line = 0; ind_line < MAX_INDEX_LINES; ind_line++)
    {
      curr_entry = get_entry(curr_ind);
      if (curr_entry == NULL)
	break;
      move(curr_line, 15);
      sprintf(display_line, "%3d", curr_ind);
      if (curr_entry->type == CREF_DIR)
	strcat(display_line, "* ");
      else
	strcat(display_line, "  ");
      strcat(display_line, curr_entry->title);
      addstr(display_line);
      curr_line++;
      curr_ind++;
    }
  if (curr_ind > Entry_Count)
    center(curr_line + 1, "** End of Index **");
  else
    center(curr_line + 1, "** More **");
}

/* Function:	display_entry() displays a CREF entry on the screen.
 * Arguments:	Index:	Index of entry to be displayed.
 * Returns:	Nothing.
 * Notes:
 *	If the entry is a file, it is shown using "more".  If it is a
 *	directory, we simply change the current directory and redisplay.
 */

display_entry(ind)
     int ind;
{
  ENTRY *entry;				/* Entry to be displayed. */
  
  entry = get_entry(ind);
  if (entry == NULL)
    {
      message(1, "Invalid entry number.");
      return;
    }
  Current_Ind = ind;
  if (entry->type == CREF_FILE)
    {
#ifdef LOG_USAGE
      log_view(entry->filename);
#endif
      clear();
      refresh();
      reset_shell_mode();
      call_program("more", entry->filename);
      reset_prog_mode();
      refresh();
      wait_for_key();
      clear();
      make_display();
    }
  else if (entry->type == CREF_DIR)
    {
      Previous_Ind = Current_Ind;
      set_current_dir(entry->filename);
      Ind_Start = Current_Ind = 1;
      make_display();

    }
  else
    message(1, "Invalid CREF contents.");
}
