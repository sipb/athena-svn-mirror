
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


/* This file is part of the CREF finder.  It contains functions for
 * parsing and updating the current location in the directory tree.  It
 * contains several local state variables that are only accessible by
 * the functions in this file.
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v $
 *	$Author: ghudson $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v 1.15 1997-04-30 17:29:58 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char *rcsid_update_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/update.c,v 1.15 1997-04-30 17:29:58 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <errno.h>			/* Error codes. */
#include <sys/types.h>
#include <sys/file.h>			/* System file defs. */
#include <string.h>			/* For string functions. */
#include <fcntl.h>
#include <ctype.h>			/* Character type macros. */

#include <browser/cref.h>		/* Finder defs. */
#include <browser/cur_globals.h>	/* Global variables.  */

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
  char temp[MAXPATHLEN];		/* Temporary space. */

  strcpy(temp, dir);
  strcpy(Current_Dir, temp);
  parse_contents();
  return(SUCCESS);
}

/* Function:	parse_contents() parses the contents file of the current
 *			directory into the entry table.
 * Arguments:	None.
 * Returns:	An error code.
 */

ERRCODE
parse_contents()
{
  char contents_name[MAXPATHLEN];	/* Name of contents file. */
  FILE *infile;				/* File ptr. for contents. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char *ptr;				/* Ptr. into input buffer. */
  int i;				/* Index variable. */
  char *title_ptr;			/* Ptr. to title string. */
  char *filename_ptr;			/* Ptr. to filename string. */
  char *format_ptr;			/* Ptr. to text formatter string. */
  char *maintainer_ptr;			/* Ptr. to maintainer string. */
  char *delim_ptr;			/* Ptr. to place of delimiter*/
  char line1[ERRSIZE];
  char line2[ERRSIZE];
  char *preserve_dir;                   /* Ptr. to Current_Dir string*/
  char *reset_dir;                      /* Ptr. to Current_Dir string
					      minus trailing directory*/
  char old_dir[MAXPATHLEN];          /* Current_Dir string minus
					      trailing directory*/
  reset_dir=old_dir;
  preserve_dir = Current_Dir;
  strcpy(contents_name, Current_Dir);
  strcat(contents_name, "/");
  strcat(contents_name, CONTENTS);
  infile = fopen(contents_name, "r");
  if (infile == NULL)
    {
      if ( open(contents_name, O_RDONLY, 0) < 0)
	{
	  if (errno == EACCES)
	    {
	      messages("You are not allowed to read this file.",
		       "Please select a different entry or entry point.");
	      return(PERM_DENIED);
	    }
	  else
	    {
	      sprintf(line1, "No index for entry: %s",
		      Entry_Table[Current_Ind-1].title);
	      messages(line1,"Please select a different entry.");
	      extract_parent_dir(preserve_dir,reset_dir);
	      strcpy(Current_Dir,old_dir);
	      return(ERROR);
	    }
	}
      else
	{
	  sprintf(line1, "%s: parse_contents: strangeness here.", Prog_Name);
	  message(1, line1);
	}
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
      delim_ptr = strchr(ptr, CONTENTS_DELIM);
      if (delim_ptr == NULL)
	{
	  sprintf(line1, "Broken index file for entry: %s",Entry_Table[Current_Ind-1].title);
	  messages(line1, "Please select another entry.");
	  extract_parent_dir(preserve_dir,reset_dir);
	  strcpy(Current_Dir,old_dir);	  
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      title_ptr = delim_ptr + 1;
      if ( !strcmp(inbuf, CREF_ENTRY) )
	Entry_Table[i].type = CREF_FILE;
      else
	Entry_Table[i].type = CREF_DIR;
      delim_ptr = strchr(title_ptr, CONTENTS_DELIM);
      if (delim_ptr == NULL)
	{
	  sprintf(line1, "Broken index file: %s", contents_name);
	  sprintf(line2, "Invalid title field (field 2) in line %d",i+1);
	  messages(line1,line2);
	  extract_parent_dir(preserve_dir,reset_dir);
	  strcpy(Current_Dir,old_dir);	  
	  set_current_dir(Current_Dir);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].title, title_ptr);
      filename_ptr = delim_ptr + 1;
      delim_ptr = strchr(filename_ptr, CONTENTS_DELIM);
      if (delim_ptr == NULL)
	{
	  sprintf(line1, "Broken index file: %s", contents_name);
	  sprintf(line2, "Invalid filename field (field 3) in line %d",i+1);
	  messages(line1, line2);
	  extract_parent_dir(preserve_dir,reset_dir);
	  strcpy(Current_Dir,old_dir);	  
	  set_current_dir(Current_Dir);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].filename, Current_Dir);
      strcat(Entry_Table[i].filename, "/");
      strcat(Entry_Table[i].filename, filename_ptr);
      format_ptr = delim_ptr + 1;
      delim_ptr = strchr(format_ptr, CONTENTS_DELIM);
      if (delim_ptr == NULL)
	{
	  sprintf(line1, "Broken index file: %s", contents_name);
	  sprintf(line2, "Invalid formatter field (field 4) in line %d",i+1);
	  messages(line1, line2);
	  extract_parent_dir(preserve_dir,reset_dir);
	  strcpy(Current_Dir,old_dir);	  
	  set_current_dir(Current_Dir);
	  return(ERROR);
	}
      *delim_ptr = (char) NULL;
      strcpy(Entry_Table[i].formatter, format_ptr);
      maintainer_ptr = delim_ptr + 1;
      strcpy(Entry_Table[i].maintainer, maintainer_ptr);
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
get_entry(ind)
     int ind;
{
  if ( (ind > 0) && (ind <= Entry_Count) )
    return( &(Entry_Table[ind - 1]) );
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
  char global_file[MAXPATHLEN];	/* Global abbrev. file. */

  fp = fopen(Abbrev_File, "r");
  if (fp != NULL)
    {
      read_abbrevs(fp);
      fclose(fp);
    }
  strcpy(global_file, Root_Dir);
  strcat(global_file, "/");
  strcat(global_file, GLOBAL_ABBREV);
  fp = fopen(global_file, "r");
  if (fp != NULL)
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

/*Function extract_parent_dir() extracts the parent directory from
 * a directory pathname.  
 * Arguments: directory path, empty string to contain name of parent
 * directory.
 * Returns: name of parent directory
 * Notes: should only need to pass directory path as an argument. Fix
 *    later.
 */
extract_parent_dir(dir_path,parent_dir)
char *dir_path;
char *parent_dir;

{
  register char *p;

  p = strrchr(dir_path,'/');
  copyn(parent_dir, dir_path, (p--) - dir_path);
    
  return(*parent_dir);
}    

extract_tail(dir_path,tail)
char *dir_path;
char *tail;

{
  register char *p;

  p = strrchr(dir_path,'/');
  copyn(tail, ++p, MAXPATHLEN);
    
  return(*tail);
}    


/*Function copyn() - lifted from /src/bin/csh/sh.file.c
 *Like strncpy but always leave room for trailing \0
 *and always null terminate.
 */
copyn(des, src, count)
        register char *des, *src;
        register count;
{

  while (--count >= 0) {
    *des = *src;
    if (*des == 0)
      return;
    des++;
    src++;
  }
  *des = '\0';
}
