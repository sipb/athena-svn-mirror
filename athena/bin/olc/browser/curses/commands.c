/* This file is part of the CREF finder.  It contains functions for executing
 * CREF commands.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_commands_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v 1.3 1986-01-22 21:15:34 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <strings.h>			/* String defs. */
#include <sys/file.h>			/* System file definitions. */
#include <errno.h>			/* System error codes. */

#include "cref.h"			/* CREF finder defs. */
#include "globals.h"			/* Global state variable defs. */

/* Function:	print_help() prints help information for CREF commands.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

print_help()
{
  int comm_index;				/* Index in command table. */
  
  clear();
  refresh();
  printf("Commands are:\n\n");
  for (comm_index = 0; comm_index < Command_Count; comm_index++)
    {
      printf("\t%c\t%s\n", Command_Table[comm_index].command,
	     Command_Table[comm_index].help_string);
    }
  printf("   <space>\tDisplay next index page.\n");
  printf("   <number>\tDisplay specified entry.\n");
  standout();
  mvaddstr(LINES-1, 0, "Hit any key to continue");
  standend();
  refresh();
  getch();
  clear();
  make_display();
}


/* Function:	top() moves to the top of the directory hierarchy.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

top()
{
  set_current_dir(Root_Dir);
  make_display();
}

/* Function:	prev_entry() displays the previous CREF entry.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_entry()
{
  int new_index;				/* New entry index. */
  
  if ( (new_index = Current_Index - 1) < 0)
    message(1, "Beginning of entries.");
  else
    display_entry(new_index);
}

/* Function:	next_entry() displays the next CREF entry.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

next_entry()
{
  int new_index;				/* New entry index. */
  
  if ( (new_index = Current_Index + 1) > Entry_Count)
    message(1, "No more entries.");
  else
    display_entry(new_index);
}

/* Function:	up_level() moves up one level in the CREF hierarchy.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

up_level()
{
  char new_dir[FILENAME_SIZE];		/* New current directory. */
  char *tail;				/* Ptr. to tail of path. */
  
  strcpy(new_dir, Current_Dir);
  tail = rindex(new_dir, '/');
  if (tail != NULL)
    {
      *tail = (char) NULL;
      if ( strlen(new_dir) >= strlen(Root_Dir) )
	{
	  set_current_dir(new_dir);
	  Current_Index = Previous_Index;
	  Previous_Index = 0;
	}
    }
  make_display();
}

/* Function:	save_file() saves an entry in a file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

save_to_file()
{
  ENTRY *save_entry;			/* Entry to save. */
  int out_fd;				/* Ouput file descriptor. */
  int in_fd;				/* Input file descriptor. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char error[ERRSIZE];			/* Error message. */
  int save_index;			/* Index of desired entry. */
  int nbytes;				/* Number of bytes read. */
  char msg[LINE_LENGTH];		/* Message for saving file. */
  char filename[FILENAME_SIZE];		/* Name of file to use. */

  message(1, "Save entry? ");
  getstr(inbuf);
  save_index = atoi(inbuf);
  if ( (save_entry = get_entry(save_index)) == NULL)
    {
      message(2, "Invalid entry number.");
      return;
    }
  if (save_entry->type == CREF_DIR)
    {
      message(2, "Can't save directory entry.");
      return;
    }
  if ( (in_fd = open(save_entry->filename, O_RDONLY, 0)) < 0)
    {
      if (errno == EPERM)
	sprintf(error,"You are not allowed to read this file");
      else sprintf(error, "Unable to open CREF file %s\n", inbuf);
      message(1, error);
      return;
    }
  sprintf(msg, "Filename (default %s)? ", Save_File);
  message(1, "Filename? ");
  getstr(inbuf);
  if (inbuf[0] == (char) NULL)
    strcpy(filename, Save_File);
  else
    strcpy(filename, inbuf);
  message(1, "");
  if ((out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
      sprintf(error, "Unable to open file %s\n", filename);
      message(1, error);
      return;
    }
  while ( (nbytes = read(in_fd, inbuf, LINE_LENGTH)) == LINE_LENGTH)
    write(out_fd, inbuf, LINE_LENGTH);
  write(out_fd, inbuf, nbytes);
  close(out_fd);
  close(in_fd);
}

/* Function:	next_page() displays the next page of the index.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

next_page()
{
  int new_index;			/* New index value. */

  new_index = Index_Start + MAX_INDEX_LINES - 3;
  if (new_index < (Entry_Count - MAX_INDEX_LINES + 2))
    Index_Start = new_index;
  else
    Index_Start = Entry_Count - MAX_INDEX_LINES + 2;
  make_display();
}

/* Function:	prev_page() displays the previous page of the index.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_page()
{
  Index_Start = Index_Start - MAX_INDEX_LINES + 1;
  if (Index_Start < 0)
    Index_Start = 0;
  make_display();
}


/* Function:	quit() quits the CREF finder program.
 * Arguments:	None.
 * Returns:	Doesn't return.
 * Notes:
 */

quit()
{
  move(LINES-1, 0);
  refresh();
  echo();
  noraw();
  endwin();
  exit(SUCCESS);
}
