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
static char *rcsid_update_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v 1.5 1986-01-29 14:48:05 treese Exp $";
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
  char temp[FILENAME_SIZE];		/* Temporary space. */

  strcpy(temp, dir);
  strcpy(Current_Dir, temp);
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
  char *format_ptr;			/* Ptr. to text formatter string. */
  char *spare_ptr;			/* Ptr. to spare string. */
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
	    sprintf(error, "cref: parse_contents: Can't open file %s",
		    contents_name);
	    message(1, error);
	    message(2, "No index for this directory.");
	    return(ERROR);
	  }
	}
      else
	message(1, "cref: parse_contents: strangeness here.");
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
	  fprintf(stderr, "cref: Invalid contents file %s.", contents_name);
	  fprintf(stderr, "Unable to construct contents.");
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
	  sprintf(error, "cref: Invalid title: file %s.", contents_name);
	  message(1, error);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].title, title_ptr);
      filename_ptr = delim_ptr + 1;
      if ( (delim_ptr = index(filename_ptr, CONTENTS_DELIM)) == NULL)
	{
	  sprintf(error, "cref: Invalid filename: file %s.", contents_name);
	  message(1, error);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].filename, Current_Dir);
      strcat(Entry_Table[i].filename, "/");
      strcat(Entry_Table[i].filename, filename_ptr);
      format_ptr = delim_ptr + 1;
      if ( (delim_ptr = index(format_ptr, CONTENTS_DELIM)) == NULL)
	{
	  sprintf(error, "cref: Invalid formatter: file %s.\n", contents_name);
	  message(1, error);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].formatter, format_ptr);
      spare_ptr = delim_ptr + 1;
      strcpy(Entry_Table[i].spare, spare_ptr);
      i++;
    }
  Entry_Count = i;
  Entry_Table[i].type = 0;
  *(Entry_Table[i].title) = (char) NULL;
  *(Entry_Table[i].filename) = (char) NULL;
  fclose(infile);
  return(SUCCESS);
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
  if ( (index > 0) && (index <= Entry_Count) )
    return( &(Entry_Table[index - 1]) );
  else
    return(NULL);
}

/* Function:	make_abbrev_table() generates the abbreviations table from
 *			the appropriate files.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

make_abbrev_table()
{
  FILE *fp;				/* Input FILE pointer. */
  char global_file[FILENAME_SIZE];	/* Global abbrev. file. */

  if ( (fp = fopen(Abbrev_File, "r")) != (FILE *) NULL)
    {
      read_abbrevs(fp);
      fclose(fp);
    }
  strcpy(global_file, Root_Dir);
  strcat(global_file, "/");
  strcat(global_file, GLOBAL_ABBREV);
  if ( (fp = fopen(global_file, "r")) != (FILE *) NULL)
    {
      read_abbrevs(fp);
      fclose(fp);
    }
  Abbrev_Table[Abbrev_Count].abbrev[0] = (char) NULL;
  Abbrev_Table[Abbrev_Count].filename[0] = (char) NULL;
}

/* Function:	read_abbrevs() reads abbreviations from a file and puts
 *			them into the abbreviation table.
 * Arguments:	fp:	A FILE pointer to the input file.
 *		index:	Index to start at in the abbrev. table.
 * Returns:	Nothing.
 * Notes:
 */

read_abbrevs(fp)
     FILE *fp;
     int index;
{
  char inbuf[LINE_LENGTH];		/* Input line. */
  char *in_ptr;				/* Input character pointer. */
  char *label_ptr;			/* Label character pointer. */
  char *name_ptr;			/* Filename character pointer. */

  while ( fgets(inbuf, LINE_LENGTH, fp) != NULL)
    {
      in_ptr = inbuf;
      label_ptr = Abbrev_Table[Abbrev_Count].abbrev;
      while (! isspace(*in_ptr) )
	*label_ptr++ = *in_ptr++;
      *label_ptr = (char) NULL;
      while (isspace(*in_ptr))
	in_ptr++;
      name_ptr = Abbrev_Table[Abbrev_Count].filename;
      while (! isspace(*in_ptr) )
	*name_ptr++ = *in_ptr++;
      *name_ptr = (char) NULL;
      Abbrev_Count++;
    }
}
