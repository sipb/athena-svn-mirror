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

/* This file is part of the CREF finder.  It contains the primary definitions
 * of the global variables.
 *
 *	$Id: globals.c,v 2.9 1999-03-06 16:47:24 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char *rcsid_globals_c = "$Id: globals.c,v 2.9 1999-03-06 16:47:24 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <ctype.h>			/* Character type macros. */
#include <pwd.h>			/* Password file entry defs. */
#include <string.h>			/* POSIX string functions */
#include <stdlib.h>			/* Many uselful library functions */

#include <browser/cref.h>		/* CREF definitions. */
#include <browser/cur_globals.h>	/* Global variable defs. */

#include <cfgfile/configure.h>		/* Configuration file support */

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
extern ERRCODE	goto_entry();
extern ERRCODE	add_abbrev();
extern ERRCODE	list_abbrevs();
extern ERRCODE	insert_entry();
extern ERRCODE	delete_entry();
extern ERRCODE  redisplay();
extern ERRCODE  contents_file();
extern ERRCODE  dir_contents();

/* Command table. */

COMMAND Command_Table[] = {
        'r',    redisplay,      "Redisplay the screen.",
	'?',	print_help,	"Print help information.",
	'a',	add_abbrev,	"Add a new abbreviation.",
	'g',	goto_entry,	"Go to a specified entry.",
	'h',	print_help,	"Print help information.",
	'i',	insert_entry,	"Insert a new entry.",
	'l',	list_abbrevs,	"List all abbreviations.",
	'f',    contents_file,  "Display contents file.",
        'c',    dir_contents,   "List directory contents.",
	'n',	next_entry,	"Go to the next entry.",
	'p',	prev_entry,	"Go to the previous entry.",
	'q',	quit,		"Quit CREF.",
	'd',	delete_entry,	"delete an entry from CREF.",
	's',	save_to_file,	"Save an entry to a file.",
	't',	top,		"Go to the top level.",
	'u',	up_level,	"Go up one level.",
	'+',	next_page,	"Display next page of index.",
	'-',	prev_page,	"Display previous page of index.",
	'b',	prev_page,	"Display previous page of index."
};

/* State Variables. */

char Current_Dir[MAXPATHLEN];	/* Current CREF directory. */
char Root_Dir[MAXPATHLEN];		/* CREF root directory. */
char *Subdir_Arg = NULL;		/* Argument to the -f flag (if any)*/
int Current_Ind;			/* Current CREF entry. */
int Previous_Ind;			/* Upper level CREF entry. */
ENTRY Entry_Table[MAX_ENTRIES];		/* Table of CREF entries. */
int Entry_Count;			/* Number of entries. */
int Ind_Start;			/* Current top of index. */
int Command_Count;			/* Number of CREF commands. */
char Save_File[MAXPATHLEN];		/* Default save file. */
char Abbrev_File[MAXPATHLEN];	/* Abbreviation filename. */
ABBREV Abbrev_Table[MAX_ABBREVS];	/* Abbreviation table. */
int Abbrev_Count;			/* Number of abbreviations. */
char Log_File[MAXPATHLEN];           /* Administrative log file. */
char *Header = NULL;			/* Display header for the browser */
char *Prompt = NULL;			/* Prompt string for the browser */

/* Function:	init_globals() initializes the global state variables.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 *	All global state variables (listed in cur_globals.h) should be
 *	initialized to their defaults here.  Command-line processing
 *	and data from configuration files should change these later on.
 */

void init_globals()
{
  struct passwd *user_info;		/* User password entry. */

  Command_Count = sizeof(Command_Table)/sizeof(COMMAND);
  Current_Ind = 1;
  Entry_Count = 0;
  Previous_Ind = 0;
  Ind_Start = 1;
  Abbrev_Count = 0;
  Log_File[0] = '\0';
  Root_Dir[0] = '\0';
  Current_Dir[0] = '\0';
  strcpy(Save_File, "cref_info");

  user_info = getpwuid(getuid());
  if (user_info != NULL) {
    strcpy(Abbrev_File, user_info->pw_dir);
    strcat(Abbrev_File, "/");
  }
  else {
    strcpy(Abbrev_File, "/");
  }
  strcat(Abbrev_File, USER_ABBREV);

}

/* Constants used in reading the config file. */

static char *cf_root = NULL;

/* The following variable contains data which automates the parsing of
 * the configuration file.  Each line contains
 *     "KEYWORD",  PARSE_FUNCTION,  &DATA [, VALUE]
 *
 * "KEYWORD" is the keyword to be defined for the configuration.
 * PARSE_FUNCTION is the function that will parse out the argument(s)
 *    (if any) to this keyword.  Reasonable values include:
 *
 *   config_set_quoted_string:
 *      Read a quoted string and store it in DATA, which should be a char*
 *      initialized to NULL.  The keyword should be only used once per file.
 *   config_set_string_list:
 *      Read a quoted string and append it to the list of strings in DATA,
 *      which should be a char**.  May be repeated to create list.
 *   config_set_char_constant:
 *      Set DATA, which should be a single-character constant, to VALUE.
 *      Does not take any arguments.  The keyword should only be used once
 *      per configuration file, but this isn't checked.
 *
 * &DATA is a pointer to the location where the data is to be stored.
 *    For our purposes, this will be one of the fields of Incarnation.
 *    Since compiler type-checking is impossible here (because this
 *    field must take values of different types), make sure that in any
 *    fields you add, type of DATA matches that required by PARSE_FUNCTION.
 * VALUE is an additional value pased to the parse_function.  Most
 *    functions ignore this, but config_set_char_constant uses it.
 *    If unused, it should be omitted for clarity (it defaults to 0).
 */
static config_keyword browser_config[] = {
  {"root_dir",		config_set_quoted_string,	&cf_root},
  {"header",		config_set_quoted_string,	&Header},
  {"prompt",		config_set_quoted_string,	&Prompt},
  {NULL,              NULL}
};


/* Function:	read_config() initializes some global state variables
 *		from the config file, unless they have already been set.
 * Arguments:	client_name: path to the invoked client from argv[0].
 *			Used to determine the name of the config file.
 * Returns:	Nothing.
 * Non-local returns: exits if malloc() failes.
 */

void read_config(char *client_name, char *cfg_path)
{
  char *client, *start, *end;
  FILE *cfg;

  /* extract the client name from client_path */
  start = strrchr(client_name, '/');
  if (start == NULL)
    start = client_name;
  else
    start++;
  end = strchr(start, '.');
  if (end == NULL)
    end = strchr(start, '\0');

  client = malloc(end-start+1);
  if (client == NULL)
    err_abort("Out of memory, giving up!","");
  strncpy(client, start, end-start);
  client[end-start] = '\0';

  /* Try opening the config file. */
  if (cfg_path == NULL)
    cfg_path = BROWSER_CONFIG_PATH;
  cfg = cfg_fopen_in_path(client, BROWSER_CONFIG_EXT, cfg_path);

  /* If a config file is found, set values we haven't set yet. */
  if (cfg != NULL) {
    cfg_read_config(client, cfg, browser_config);

    /* Header and Prompt are dealt with automatically, but we need to
     * set Root_Dir from the allocated copy if Root_Dir wasn't set. */
    if (cf_root && ! Root_Dir[0])
      strcpy(Root_Dir, cf_root);
  }

  /* If Subdir_Arg is set, it means -f flag was used.
   * We need to deal here, since Root_Dir may have been undefined before. */
  strcpy(Current_Dir, Root_Dir);
  if (Subdir_Arg) {
    strcat(Current_Dir, "/");
    strcat(Current_Dir, Subdir_Arg);
  }

  /* Check if Current_Dir is in fact valid. */
  if (check_cref_dir(Current_Dir) != TRUE)
    err_abort("No CREF index file in this directory:\n", Current_Dir);

  /* Set other values to defaults if they haven't been set yet. */
  if (Header == NULL) {  
    Header = malloc(sizeof(FALLBACK_HEADER));
    if (Header == NULL)
      err_abort("Out of memory, giving up!","");
    strcpy(Header, FALLBACK_HEADER);
  }
  if (Prompt == NULL) {
    Prompt = malloc(sizeof(FALLBACK_PROMPT));
    if (Prompt == NULL)
      err_abort("Out of memory, giving up!","");
    strcpy(Prompt, FALLBACK_PROMPT);
  }
}

