/* This file is part of the CREF finder.  It contains functions for
 * parsing and updating the current location in the directory tree.  It
 * contains several local state variables that are only accessible by
 * the functions in this file.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	Created:	8/10/85
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v $
 *	$Author: treese $
 */

#ifndef lint
static char *rcsid_update_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v 1.2 1986-01-22 18:03:41 treese Exp $";
#endif	lint

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <errno.h>			/* Error codes. */
#include <sys/file.h>			/* System file defs. */
#include <strings.h>			/* For string functions. */
#include <ctype.h>			/* Character type macros. */

#include "cref.h"			/* Finder defs. */
#include "globals.h"			/* Global variables.  */

/* Function:	set_current_dir() updates the current directory and parses its
 *			index.
 * Arguments:	dir:	New current directory.
 * Returns:	An error code.
 * Notes:
 */

ERRCODE
set_current_dir(dir)
char *dir;
{
	strcpy(Current_Dir, dir);
	Current_Index = 0;
	parse_contents();
	return(SUCCESS);
}

/* Function:	parse_contents() parses the contents file of the current
 *			directory into the entry table.
 * Arguments:	None.
 * Returns:	An error code.
 * Notes:
 */

ERRCODE
parse_contents()
{
  char contents_name[FILENAME_SIZE];	/* Name of contents file. */
  FILE *infile;				/* File ptr. for contents. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char *ptr;				/* Ptr. into input buffer. */
  int i;				/* Index variable. */
  char *title_ptr;			/* Ptr. to title string. */
  char *filename_ptr;			/* Ptr. to filename string. */
  char *delim_ptr;			/* Ptr. to place of delimiter*/
  char error[ERRSIZE];			/* Error message. */
  
  strcpy(contents_name, Current_Dir);
  strcat(contents_name, "/");
  strcat(contents_name, CONTENTS);
  if ( (infile = fopen(contents_name, "r")) == NULL)
    {
      if ( open(contents_name, O_RDONLY, 0) < 0)
	{
	  if (errno == EACCES)
	    {
	      message(1, "You are not allowed to read this file.\n");
	      return(PERM_DENIED);
	    }
	  else
	    {
	    sprintf(error, "cref: parse_contents: Can't open file %s\n",
		    contents_name);
	    message(1, error);
	    message(2, "No index for this directory.\n");
	    return(ERROR);
	  }
	}
      else
	message(1, "cref: parse_contents: strangeness here.\n");
    }
  i = 0;
  while ( fgets(inbuf, LINE_LENGTH, infile) != NULL)
    {
      inbuf[strlen(inbuf) - 1] = (char) NULL;
      ptr = inbuf;
      while ( isspace(*ptr) )
	ptr++;
      if (*ptr == COMMENT_CHAR)
	continue;
      if (*ptr == (char) NULL)
	continue;
      if ( (delim_ptr = index(ptr, CONTENTS_DELIM)) == NULL)
	{
	  fprintf(stderr, "cref: Invalid contents file %s.\n", contents_name);
	  fprintf(stderr, "Unable to construct contents.\n");
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      title_ptr = delim_ptr + 1;
      if ( !strcmp(inbuf, CREF_ENTRY) )
	Entry_Table[i].type = CREF_FILE;
      else
	Entry_Table[i].type = CREF_DIR;
      if ( (delim_ptr = index(title_ptr, CONTENTS_DELIM)) == NULL)
	{
	  fprintf(stderr, "cref: Invalid contents file %s.\n", contents_name);
	  fprintf(stderr, "Unable to construct contents.\n");
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      filename_ptr = delim_ptr + 1;
      if ( (delim_ptr = index(filename_ptr, CONTENTS_DELIM)) != NULL)
	*delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].title, title_ptr);
      strcpy(Entry_Table[i].filename, Current_Dir);
      strcat(Entry_Table[i].filename, "/");
      strcat(Entry_Table[i].filename, filename_ptr);
      i++;
    }
  Entry_Count = i - 1;
  Entry_Table[i].type = 0;
  *(Entry_Table[i].title) = (char) NULL;
  *(Entry_Table[i].filename) = (char) NULL;
  fclose(infile);
}


/* Function:	get_entry() gets a entry from the table.
 * Arguments:	index:	Index of desired entry.
 * Returns:	A pointer to the desired entry.
 * Notes:
 */

ENTRY *
get_entry(index)
     int index;
{
  if (index > Entry_Count)
    return(NULL);
  else
    return( &(Entry_Table[index]) );
}
