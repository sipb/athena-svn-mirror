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

/* This file is part of the CREF finder.  It contains functions for executing
 * CREF commands.
 *
 *	$Source:
 *	$Author:
 *      $Header:
 */


#ifndef lint
static char *rcsid_commands_c = "$Header: ";
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
  wait_for_key();
  clear();
  refresh();
  make_display(Current_Index);
}


/* Function:	top() moves to the top of the directory hierarchy.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

top()
{
  set_current_dir(Root_Dir);
  Current_Index=1;
  make_display(Current_Index);
}

/* Function:	prev_entry() displays the previous CREF entry.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_entry()
{
  int new_index;				/* New entry index. */
  
  if ( (new_index = Current_Index - 1) < 1)
    messages("", "Beginning of entries.");
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
    messages("", "No more entries.");
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
	  Index_Start = Prev_Index_Start;
	  Prev_Index_Start = 1;
	  Previous_Index = 1;
	}
    }
  make_display(Index_Start);
}

/* Function:	save_file() saves an entry in a file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

save_to_file()
{
  ENTRY *save_entry;			/* Entry to save. */
  int fd;				/* Input file descriptor. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char error[ERRSIZE];			/* Error message. */
  int save_index;			/* Index of desired entry. */
  char msg[LINE_LENGTH];		/* Message for saving file. */
  char filename[FILENAME_SIZE];		/* Name of file to use. */

  inbuf[0] = (char) NULL;
  message(1, "Save entry? ");
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
  save_index = atoi(inbuf);
  if ( (save_entry = get_entry(save_index)) == NULL)
    {
      messages("", "Invalid entry number.");
      return;
    }
  if (save_entry->type == CREF_DIR)
    {
      messages("", "Can't save directory entry.");
      return;
    }
  if ( (fd = open(save_entry->filename, O_RDONLY, 0)) < 0)
    {
      if (errno == EPERM)
	sprintf(error,"You are not allowed to read this file");
      else sprintf(error, "Unable to open CREF file %s\n", inbuf);
      message(1, error);
      return;
    }
  close(fd);
  sprintf(msg, "Filename (default %s)? ", Save_File);
  message(1, msg);
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    strcpy(filename, Save_File);
  else
    strcpy(filename, inbuf);
  message(1, "");
  copy_file(save_entry->filename, filename);
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
  make_display(Index_Start);
}

/* Function:	prev_page() displays the previous page of the index.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_page()
{
  Index_Start = Index_Start - MAX_INDEX_LINES + 1;
  if (Index_Start < 1)
    Index_Start = 1;
  make_display(Index_Start);
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

/* Function:	goto_entry() jumps to an entry with a specified label.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

goto_entry()
{
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  char new_dir[FILENAME_SIZE];		/* New directory name. */
  char error[ERRSIZE];
  char line1[ERRSIZE];			/* Error message. */
  char line2[ERRSIZE];
  int i;				/* Index variable. */

  inbuf[0] = (char) NULL;
  message(1, "Go to?");
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
 
  i = 0;

  while ( i < Abbrev_Count)
    {
      if ( ! strcmp(inbuf, Abbrev_Table[i].abbrev))
	{
	  if (Abbrev_Table[i].filename[0] == '/')
	    strcpy(new_dir, Abbrev_Table[i].filename);
	  else
	    make_path(Root_Dir, Abbrev_Table[i].filename, new_dir);
	  if (check_cref_dir(new_dir) == TRUE)
	    {
	      set_current_dir(new_dir);
	      Current_Index=1;
	      make_display(Current_Index);
	      message(1,"");
	      return;
	    }
	  else
	    {
	      sprintf(line1, "No index:%s",
		      Abbrev_Table[i].filename);
	      sprintf(line2,"Please select a different abbreviation/entry.");
	      messages(line1,line2);
	      return;
	    }
	}
      i++;
    }
  sprintf(error, "Abbreviation %s is not defined.", inbuf);
  message(1, error);
}

/* Function:	add_abbrev() adds a new abbreviation 
 *			to the current user abbreviation file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

add_abbrev()
{
  FILE *fp;				/* File pointer. */
  char error[ERRSIZE];			/* Error message. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  int index;				/* Entry index. */
  ENTRY *entry;				/* Entry to get abbreviation. */
  int i;				/* Abbrev count variable*/


  if ( (fp = fopen(Abbrev_File, "a")) == (FILE *) NULL)
      {
	sprintf(error, "Unable to open abbreviation file %s", Abbrev_File);
	message(1, error);
	return;
      }
  inbuf[0] = (char) NULL;
  message(1, "Define abbreviation for entry:");
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
  index = atoi(inbuf);
  if ( (entry = get_entry(index)) == NULL)
    {
      message(2, "Invalid entry number.");
      return;
    }
  if (entry->type != CREF_DIR)
    {
      message(2, "Can't define abbreviation for file entry.");
      return;
    }
  i=0;
  while (i < Abbrev_Count)
    {
      if (strcmp(entry->filename,Abbrev_Table[i].filename) == 0)
	{
	  message(2, "Abbreviation for this entry already exists");
	  return;
	}
      i++;
    }
  message(1, "Abbreviation: ");
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
  if (Abbrev_Count >= MAX_ABBREVS)
    {
      message(1, "Maximum number of abbreviations reached.");
      return;
    }
  fprintf(fp, "%s\t%s\n", inbuf, entry->filename);
  strcpy(Abbrev_Table[Abbrev_Count].abbrev, inbuf);
  strcpy(Abbrev_Table[Abbrev_Count].filename, entry->filename);
  Abbrev_Count++;
  Abbrev_Table[Abbrev_Count].abbrev[0] = (char) NULL;
  Abbrev_Table[Abbrev_Count].filename[0] = (char) NULL;
  fclose(fp);
  message(1,"");
  return;
}

/* Function:	list_abbrevs() lists all known abbreviations.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

list_abbrevs()
{
  int index;				/* Index in abbrev. table. */
  int curr_line;			/* Current screen line. */

  clear();
  refresh();
  curr_line = 0;
  printf("Abbreviations are:\n\n");
  for (index = 0; index < Abbrev_Count; index++, curr_line++)
    {
      if (curr_line > LINES - 2)
	{
	  wait_for_key();
	  clear();
	  curr_line = 0;
	}
      printf("%s\t%s\n", Abbrev_Table[index].abbrev,
	     Abbrev_Table[index].filename);
    }
  wait_for_key();
  clear();
  make_display(Current_Index);
}

/* Function:	insert_entry() inserts a new entry into the current CREF
 *			contents file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

insert_entry()
{
  FILE *fp;				/* File pointer. */
  char contents[FILENAME_SIZE];		/* Contents filename. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  int index;				/* Entry index. */
  int i;				/* Index variable. */
  char curr_type[LINE_LENGTH];		/* Current type string. */
  int type;				/* Type of entry. */
  char type_name[LINE_LENGTH];		/* Name of entry type. */
  char title[LINE_LENGTH];		/* Title of entry. */
  char filename[FILENAME_SIZE];		/* Filename to use. */
  char formatter[LINE_LENGTH];		/* Text formatter to use. */
  char maintainer[LINE_LENGTH];		/* Maintainer of file. */
  char newdir[FILENAME_SIZE];		/* New directory pathname. */
  char newfile[FILENAME_SIZE];		/* New file pathname. */
  int row;				/* Row on screen. */
  char *fullpath;
  char *ptail;
  char tail[FILENAME_SIZE];
  char line1[ERRSIZE];
  char line2[ERRSIZE];
  char log_string[LOG_LENGTH];
  char install_dir[FILENAME_SIZE];
  char install_file[FILENAME_SIZE];
  char install_path[FILENAME_SIZE];

  if (Log_File[0] == (char) NULL)
    {
      message(1, "Enter full pathname of log file: ");
      get_input(inbuf);
      if (inbuf[0] == (char) NULL)
	return;
      if ( (fp = fopen(inbuf,"a")) < 0)
	{
	  sprintf(line1, "Unable to open: %s",inbuf);
	  sprintf(line2, "Check the pathname and try again.");
	  messages(line1,line2);
	  return(FALSE);
  	}
      else
	{
	  strcpy(Log_File,inbuf);
	  fclose(fp);
	}
    } 

  row = 0;
  make_path(Current_Dir, CONTENTS, contents);
  inbuf[0] = (char) NULL;
  message(1, "Insert entry as number:");
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
  index = atoi(inbuf);
  if ( index < 1 )
    {
      message(2, "Invalid entry number.");
      return;
    }
  clear();
  refresh();
  while ( (inbuf[0] != 'f') && (inbuf[0] != 'd') )
    {
      mvaddstr(row++, 0, "Is the entry a file [f] or a new directory [d]? ");
      refresh();
      inbuf[0] = (char) NULL;
      get_input(inbuf);
    }
  if (inbuf[0] == 'f')
    {
      type = CREF_FILE;
      mvaddstr(row++, 0, "File to be installed: ");
      refresh();
      inbuf[0] = (char) NULL;
      get_input(inbuf);
      strcpy(install_file,inbuf);
    }
  else if (inbuf[0] == 'd')
    type = CREF_DIR;
  mvaddstr(row++, 0, "Title for index file: ");
  refresh();
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  strcpy(title, inbuf);
  mvaddstr(row++, 0, "Filename to be used: ");
  refresh();
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  strcpy(filename, inbuf);
  if (type == CREF_FILE)
    {
      strcpy(type_name, CREF_ENTRY);
      mvaddstr(row++, 0, "Text formatter to use (<CR> for none): ");
      refresh();
      inbuf[0] = (char) NULL;
      get_input(inbuf);
      if (inbuf[0] == (char) NULL)
	strcpy(formatter, "none");
      else
	strcpy(formatter, inbuf);
    }
  else
    {
      strcpy(formatter, "none");
      strcpy(type_name, CREF_SUBDIR);
    }
  mvaddstr(row++, 0, "Name of maintainer: ");
  refresh();
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  strcpy(maintainer, inbuf);

  mvaddstr(row++, 0, "Is this information correct [y/n]? ");
  refresh();
  inbuf[0] = (char) NULL;
  get_input(inbuf);
  if (inbuf[0] != 'y')
    {
      clear();
      make_display(Current_Index);
      return;
    }

  if (type == CREF_DIR)
    {
      make_path(Current_Dir, filename, newdir);
      if (create_cref_dir(newdir)==ERROR)
	{
	  wait_for_key();
	  clear();
	  make_display(Current_Index);
	  return;
	}
    }
  else if (type == CREF_FILE)
    {
      make_path(Current_Dir, filename, newfile);
      if (copy_file(install_file, newfile)==ERROR)
	{
	  wait_for_key();
	  clear();
	  make_display(Current_Index);
	  return;
	}
    }



  if (index > Entry_Count)
    {
      if ( (fp = fopen(contents, "a")) == (FILE *) NULL)
	{
	  sprintf(line1, "Unable to open:%s", contents);
	  if (type == CREF_FILE)
	    {
	      if (unlink(newfile) < 0)
		  sprintf(line2, "Unable to remove:%s",newfile);
	    }
	  else if (type == CREF_DIR)
	    {
	      if (rmdir(newdir) < 0)
		sprintf(line2, "Unable to rmdir:%s",newdir);
	    }
	  messages(line1,line2);
	  make_display(Current_Index);
	  return;
	}
      fprintf(fp, "%s%c%s%c%s%c%s%c%s\n", type_name, CONTENTS_DELIM,
	      title, CONTENTS_DELIM, filename, CONTENTS_DELIM, formatter,
	      CONTENTS_DELIM, maintainer);
    }
else
  {
    if ( (fp = fopen(contents, "w")) == (FILE *) NULL)
      {
	sprintf(line1, "Unable to open:%s", contents);
	if (type == CREF_FILE)
	  {
	    if (unlink(newfile) < 0)
	      sprintf(line2, "Unable to remove:%s",newfile);
	  }
	else if (type == CREF_DIR)
	  {
	    if (rmdir(newdir) < 0)
	      sprintf(line2, "Unable to rmdir:%s",newdir);
	  }
	messages(line1,line2);
	make_display(Current_Index);
	return;
      }
    
    i = 0;
    while (i < Entry_Count)
      {
	if (i == (index - 1))
	  {
	    fprintf(fp, "%s%c%s%c%s%c%s%c%s\n", type_name, CONTENTS_DELIM,
		    title, CONTENTS_DELIM, filename, CONTENTS_DELIM, formatter,
		    CONTENTS_DELIM, maintainer);
	  }
	if (Entry_Table[i].type == CREF_FILE)
	  strcpy(curr_type, CREF_ENTRY);
	else
	  strcpy(curr_type, CREF_SUBDIR);
	fullpath=Entry_Table[i].filename;
	ptail=tail;
	extract_tail(fullpath,ptail);
	fprintf(fp, "%s%c%s%c%s%c%s%c%s\n", curr_type, CONTENTS_DELIM,
		Entry_Table[i].title,CONTENTS_DELIM, tail,
		CONTENTS_DELIM, Entry_Table[i].formatter,
		CONTENTS_DELIM, Entry_Table[i].maintainer);
	i++;
      }
  }
  fclose(fp);

  make_path(getwd(install_dir),install_file,install_path);  

  if (type == CREF_FILE)
    {
      sprintf(log_string,"File: %s\n Installed as: %s\n Title: %s\n",
	 install_path, newfile, title);  
      log_status(Log_File, log_string);
    }
  if (type == CREF_DIR)
    {
      sprintf(log_string,"New Directory: %s\n Title: %s\n",
	 newdir, title);
      log_status(Log_File, log_string);
    }
  set_current_dir(Current_Dir);
  clear();
  make_display(Current_Index);
}

/* Function:	remove_entry() removes an entry from the CREF
 *			contents file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

delete_entry()
{
  FILE *fp;				/* File pointer. */
  char contents[FILENAME_SIZE];		/* Contents filename. */
  char inbuf[LINE_LENGTH];		/* Input buffer. */
  int index;				/* Entry index. */
  int i;				/* Index variable. */
  char curr_type[LINE_LENGTH];		/* Current type string. */
  ENTRY *entry;				/* Entry to be removed. */
  int j;				/* Index variable. */
  char *fullpath;
  char *ptail;
  char tail[FILENAME_SIZE];
  char line1[ERRSIZE];
  char line2[ERRSIZE];
  char log_string[LOG_LENGTH];

  if (Log_File[0] == (char) NULL)
    {
      message(1, "Enter full pathname of log file: ");
      get_input(inbuf);
      if (inbuf[0] == (char) NULL)
	return;
      if ( (fp = fopen(inbuf,"a")) < 0)
	{
	  sprintf(line1, "Unable to open: %s",inbuf);
	  sprintf(line2, "Check the pathname and try again.");
	  messages(line1,line2);
	  return(FALSE);
  	}
      else
	{
	  strcpy(Log_File,inbuf);
	  fclose(fp);
	}
    } 
  make_path(Current_Dir, CONTENTS, contents);
  inbuf[0] = (char) NULL;
  message(1, "Remove entry: ");
  get_input(inbuf);
  if (inbuf[0] == (char) NULL)
    return;
  index = atoi(inbuf);
  if ( (entry = get_entry(index)) == (ENTRY *) NULL )
    {
      message(2, "Invalid entry number.");
      return;
    }
  if (entry->type == CREF_DIR)
    {
      message(2, "Cannot remove a directory");
      return;
    }


  if ( (fp = fopen(contents, "w")) == (FILE *) NULL)
    {
      message(1, "Unable to open contents file.");
      make_display(Current_Index);
      return;
    }

  i = 1;
  while (i <= Entry_Count)
    {
      j = i - 1;
      if (Entry_Table[j].type == CREF_FILE)
	strcpy(curr_type, CREF_ENTRY);
      else
	strcpy(curr_type, CREF_SUBDIR);
      fullpath=Entry_Table[j].filename;
      ptail=tail;
      extract_tail(fullpath,ptail);


      if (i == index)
	{
	  if (unlink(entry->filename) < 0)
	    {
	      sprintf(line1, "Unable to remove: %s",entry->filename);
	      sprintf(line2, "");
	      messages(line1,line2);
	      fprintf(fp, "%s%c%s%c%s%c%s%c%s\n", curr_type, CONTENTS_DELIM,
		      Entry_Table[j].title,CONTENTS_DELIM, tail,
		      CONTENTS_DELIM, Entry_Table[j].formatter,
		      CONTENTS_DELIM, Entry_Table[j].maintainer);
	      wait_for_key();
	    }
	  else
	    {
	      sprintf(log_string,"Deleted file: %s\n Title: %s\n",
		      entry->filename,Entry_Table[j].title);
	      log_status(Log_File, log_string);
	    }
	}
      else
	{
	  fprintf(fp, "%s%c%s%c%s%c%s%c%s\n", curr_type, CONTENTS_DELIM,
		  Entry_Table[j].title,CONTENTS_DELIM, tail,
		  CONTENTS_DELIM, Entry_Table[j].formatter,
		  CONTENTS_DELIM, Entry_Table[j].maintainer);
	}
      i++;
    }

  fclose(fp);
  
  set_current_dir(Current_Dir);
  clear();
  make_display(Current_Index);
}

redisplay()
{
  set_current_dir(Current_Dir);
  clear();
  make_display(Index_Start);
}

contents_file()
{
  char contents_path[FILENAME_SIZE];

  make_path(Current_Dir,CONTENTS,contents_path);
  clear();
  refresh();
  call_program("more",contents_path);
  wait_for_key();
  clear();
  make_display(Current_Index);
}

dir_contents()
{
  clear();
  refresh();
  call_program("ls",Current_Dir);
  wait_for_key();
  clear();
  make_display(Current_Index);
}
