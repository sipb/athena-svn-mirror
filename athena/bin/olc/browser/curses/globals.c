/* This file is part of the CREF finder.  It contains the primary definitions
 * of the global variables.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Created:	1/18/86
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/globals.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_globals_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/globals.c,v 1.1 1986-01-22 18:03:12 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */

#include "cref.h"			/* CREF definitions. */

/* Function declarations for the command table. */

extern ERRCODE	print_help();
extern ERRCODE	top();
extern ERRCODE	prev_entry();
extern ERRCODE	next_entry();
extern ERRCODE	up_level();
extern ERRCODE	save_to_file();
extern ERRCODE	next_page();
extern ERRCODE	prev_page();
extern ERRCODE	quit();

/* Command table. */

COMMAND Command_Table[] = {
	'?',	print_help,	"Print help information.",
	'h',	print_help,	"Print help information.",
	'n',	next_entry,	"Go to the next entry.",
	'p',	prev_entry,	"Go to the previous entry.",
	'q',	quit,		"Quit CREF.",
	's',	save_to_file,	"Save an entry to a file.",
	't',	top,		"Go to the top level.",
	'u',	up_level,	"Go up one level.",
	'+',	next_page,	"Display next page of index.",
	'-',	prev_page,	"Display previous page of index."
};

/* State Variables. */

char Current_Dir[FILENAME_SIZE];	/* Current CREF directory. */
char Root_Dir[FILENAME_SIZE];		/* CREF root directory. */
int Current_Index;			/* Current CREF entry. */
int Previous_Index;			/* Upper level CREF entry. */
ENTRY Entry_Table[MAX_ENTRIES];		/* Table of CREF entries. */
int Entry_Count;			/* Number of entries. */
int Index_Start;			/* Current top of index. */
int Is_Consultant;			/* Is user a consultant? */
int Command_Count;			/* Number of CREF commands. */
char Save_File[FILENAME_SIZE];		/* Default save file. */

/* Function:	init_globals() initializes the global state variables.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 *	All global state variables (listed in globals.h) should be
 *	initialized to their defaults here.  Command line processing
 *	to change these should be done later.
 */

init_globals(argc, argv)
{
  char *ptr;				/* Useful pointer. */

  Command_Count = sizeof(Command_Table)/sizeof(COMMAND);
  strcpy(Root_Dir, CREF_ROOT);
  set_current_dir(Root_Dir);
  Current_Index = 1;
  Previous_Index = 1;
  Index_Start = 0;
  Is_Consultant = FALSE;
  strcpy(Save_File, "cref_info");
}
