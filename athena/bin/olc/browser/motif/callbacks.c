/*
 * This file is part of an X stock answer browser.
 * It contains the callbacks for the widgets used in the
 * X-based interface.
 *
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/motif/callbacks.c,v $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/motif/callbacks.c,v 1.10 1992-05-06 13:01:30 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include <Mrm/MrmAppl.h>
#include <Xm/Mu.h>
#include "cref.h"
#include <Xm/Text.h>
#include <Xm/SelectioB.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

extern int errno;
extern char* program;			/* name of program */
extern Widget toplevel;

#define  ERROR		-1
#define  SUCCESS	0
#define  MotifString(s)		XmStringLtoRCreate(s, XmSTRING_DEFAULT_CHARSET)


char Indexes[MAX_TITLES][MAXPATHLEN];	/* Index files */
char Titles[MAX_TITLES][TITLE_SIZE];	/* Titles of entries down to here. */
char CurrentDir[BUFSIZ];		/* Current CREF directory. */
ENTRY new_entry_table[MAX_ENTRIES];	/* Entry table to hold new entries. */
ENTRY EntryTable[MAX_ENTRIES];		/* Table of entries. */
int Deep = 0;
char buffer[BUFSIZ];

Widget w_top_form, w_bottom_form;
Widget w_list, w_text, w_top_lbl, w_bottom_lbl;
Widget w_up, w_save, w_help, w_quit, w_dlg_save;

/*
 *  Procedures
 *
 */

int
MakeMenu(dir)
     char *dir;
{
  int n;
  Arg args[2];
  char temp[BUFSIZ];
  XmString strings[MAX_ENTRIES];

  if (ParseContents(dir) == ERROR)
    return(ERROR);
  
  for(n = 0; new_entry_table[n].title[0] != (char) NULL; n++)
    {
      EntryTable[n].type = new_entry_table[n].type;
      strcpy(EntryTable[n].title, new_entry_table[n].title);
      strcpy(EntryTable[n].filename, new_entry_table[n].filename);
      strcpy(EntryTable[n].formatter, new_entry_table[n].formatter);
      strcpy(EntryTable[n].maintainer, new_entry_table[n].maintainer);
      sprintf(temp, "%2d %c %s", n+1,
	      (EntryTable[n].type == SUBDIR) ? '*' : ' ',
	      EntryTable[n].title);
      strings[n] = MotifString(temp);
    }
  XtSetArg(args[0], XmNitems, strings);
  XtSetArg(args[1], XmNitemCount, n);
  XtSetValues(w_list, args, 2);
  strcpy(CurrentDir, dir);
  return(SUCCESS);
}

int
ParseContents(dir)
     char *dir;
{
  char contents_name[BUFSIZ];	/* Name of index file. */
  char inbuf[BUFSIZ];		/* Input buffer to read into. */
  char error[BUFSIZ];		/* Space for error message. */
  char *ptr;			/* Ptr. into input buffer. */
  char *title_ptr;		/* Pointer to title string. */
  char *filename_ptr;		/* Pointer to filename. */
  char *format_ptr;		/* Pointer to format. */
  char *maintainer_ptr;		/* Pointer to maintainer. */
  char *delim_ptr;		/* Pointer to delimiter. */
  FILE *infile;			/* File pointer. */
  int fd;			/* File descriptor. */
  int i;			/* Counter. */

  sprintf(contents_name, "%s/%s", dir, CONTENTS);

  if ((fd = open(dir, O_RDONLY, 0)) < 0)	 /* does directory exist? */
    if (errno == ENOENT) {
      sprintf(error, "Directory `%s' does not exist.", dir);
      MuError(error);
      return(ERROR);
    }
  close(fd);

  if ((infile = fopen(contents_name, "r")) == NULL) {
    if ((fd = open(contents_name, O_RDONLY, 0)) < 0) {
      if (errno == EACCES) {
	MuError("You are not allowed to read this file.\nPlease select a different entry.");
	close(fd);
	return(ERROR);
      }
      else {
	sprintf(error,
		"No index file: %s\nPlease select a different entry.",
		dir);
	MuError(error);
	return(ERROR);
      }
    }
    else
      MuError("An error occurred while trying to open the file.");
  }

  i = 0;
  while ( fgets(inbuf, LINE_LENGTH, infile) != NULL) {
    inbuf[strlen(inbuf) - 1] = (char) NULL;
    ptr = inbuf;
    while ( isspace(*ptr) )
      ptr++;
    if (*ptr == COMMENT_CHAR)
      continue;
    if (*ptr == (char) NULL)
      continue;
    if ( (delim_ptr = index(ptr, CONTENTS_DELIM)) == NULL) {
      sprintf(error, "Broken index file: %s\nPlease select another entry.",
	      dir);
      MuError(error);
      return(ERROR);
    }
    *delim_ptr = '\0';
    title_ptr = delim_ptr + 1;
    if ( !strcmp(inbuf, B_ENTRY) )
      new_entry_table[i].type = PLAINFILE;
    else
      new_entry_table[i].type = SUBDIR;
    if ( (delim_ptr = index(title_ptr, CONTENTS_DELIM)) == NULL) {
      sprintf(error, "Broken index file: %s.\nInvalid title field (field 2) in line %d", contents_name, i+1);
      MuError(error);
      return(ERROR);
    }
    *delim_ptr = '\0';
    strcpy(new_entry_table[i].title, title_ptr);
    filename_ptr = delim_ptr + 1;
    if ((delim_ptr = index(filename_ptr, CONTENTS_DELIM)) == NULL){
      sprintf(error, "Broken index file: %s\nInvalid filename field (field 3) in line %d", contents_name, i+1);
      MuError(error);
      return(ERROR);
    }
    *delim_ptr = '\0';
    strcpy(new_entry_table[i].filename, filename_ptr);
    format_ptr = delim_ptr + 1;
    if ((delim_ptr = index(format_ptr, CONTENTS_DELIM)) == NULL) {
      sprintf(error, "Broken index file: %s\nInvalid formatter field (field 4) in line %d.", contents_name, i+1);
      MuError(error);
      return(ERROR);
    }
    *delim_ptr = '\0';
    strcpy(new_entry_table[i].formatter, format_ptr);
    maintainer_ptr = delim_ptr + 1;
    strcpy(new_entry_table[i].maintainer, maintainer_ptr);
    i++;
  }
  new_entry_table[i].type = 0;
  new_entry_table[i].title[0] = (char) NULL;
  new_entry_table[i].filename[0] = (char) NULL;
  fclose(infile);
  return(SUCCESS);
}

int show_file(text_widget, file)
     Widget text_widget;
     char file[BUFSIZ];
{
  Arg arg;
  struct stat buf;
  char *text;
  int fd;
  char error[BUFSIZ];		/* Space for error message. */
  static int init = 0;
  
  if (init == 0) {
      XtSetArg(arg, XmNsensitive, TRUE);
      XtSetValues(w_save, &arg, 1);
      init = 1;
    }

  if (stat(file, &buf) < 0) {
    sprintf(error, "Error stat'ing file %s\n",file);
    MuError(error);
    return(ERROR);
  }

  if ((text = (char *) malloc((1 + buf.st_size) * sizeof(char)))
      == (char *) NULL)
    {
      sprintf(error, "Not enough memory to read in file\n\"%s\".",
	      file);
      MuError(error);
      return(ERROR);
    }

  if ((fd = open(file, O_RDONLY, 0)) < 0)
    {
      sprintf(error, "Unable to open text file\n\"%s\"\nfor reading.",
	      file);
      MuError(error);
      return(ERROR);
    }

  if ((read(fd, text, buf.st_size)) != buf.st_size)
    {
      sprintf(error, "Unable to read text correctly from file\n\"%s\".",
	      file);
      MuError(error);
      return(ERROR);
    }

  text[buf.st_size] = '\0';
  XmTextSetString(text_widget, text);
  close(fd);
  free(text);
#ifdef LOG_USAGE
  log_view(file);
#endif
  return(SUCCESS);
}


void setPromptCB (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;
  Widget TextBox;

  sprintf(buffer, "%s/%s", getenv("HOME"), "browser.info");
  TextBox = XmSelectionBoxGetChild(w, XmDIALOG_TEXT);
  XtSetArg(arg, XmNvalue, buffer);
  XtSetValues(TextBox, &arg, 1);
  _XmGrabTheFocus(TextBox, NULL);
}


void saveCB (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmSelectionBoxCallbackStruct *callback_data;
{
  int fd;
  char *text;
  char error[BUFSIZ];		/* Space for error message. */
  char *new_buf;
  Widget TextBox;

  TextBox = XmSelectionBoxGetChild(w_dlg_save, XmDIALOG_TEXT);
  new_buf = XmTextGetString(TextBox);

  if ((fd = open(new_buf, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
    {
      sprintf(error, "Unable to open file\n\"%s\"\nfor writing.",
	      buffer);
      MuError(error);
      return;
    }
  text = XmTextGetString(w_text);
  write(fd, text, strlen(text));
}
  

void selectCB (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmListCallbackStruct *callback_data;
{
  char file[BUFSIZ];
  int item;
  Arg arg;
  
  item = callback_data->item_position - 1;
  if (EntryTable[item].type == PLAINFILE)
    {
      sprintf(file, "%s/%s", CurrentDir, EntryTable[item].filename);
      if (show_file(w_text, file) == SUCCESS)
	{
	  XtSetArg(arg, XmNlabelString, MotifString(EntryTable[item].title));
	  XtSetValues(w_bottom_lbl, &arg, 1);
	}
    }
  else
    {
      sprintf(file, "%s/%s", CurrentDir, EntryTable[item].filename);
      Deep++;
      strcpy(Indexes[Deep], file);
      strcpy(Titles[Deep], EntryTable[item].title);
      if (MakeMenu(file) == SUCCESS)
	{
	  XtSetArg(arg, XmNlabelString, MotifString(Titles[Deep]));
	  XtSetValues(w_top_lbl, &arg, 1);
	  if (Deep == 1) {
	    XtSetArg(arg, XmNsensitive, TRUE);
	    XtSetValues(w_up, &arg, 1);
	  }
	}
    }
}
  

void upCB (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;
  
  if (Deep == 0) {
      MuError("Cannot go up any further.");
      return;
    }
  
  Deep--;

  if (MakeMenu(Indexes[Deep]) == SUCCESS) {
    XtSetArg(arg, XmNlabelString, MotifString(Titles[Deep]));
    XtSetValues(w_top_lbl, &arg, 1);
    if (Deep == 0) {
      XtSetArg(arg, XmNsensitive, FALSE);
      XtSetValues(w_up, &arg, 1);
    }      
  }
  else {
    MuError("Error creating menu");
    Deep++;
  }
}
  

void setCursorCB (w, cursor, callback_data)
     Widget w;
     char *cursor;
     XmAnyCallbackStruct *callback_data;
{
  while (XtIsSubclass(w, shellWidgetClass) != True)
    w = XtParent(w);
  XDefineCursor(XtDisplay(w), XtWindow(w),
		XCreateFontCursor(XtDisplay(w),
				  (unsigned int) atoi(cursor)));
  XFlush(XtDisplay(w));
}


void createCB (w, string, callback_data)
     Widget w;
     char *string;
     XmAnyCallbackStruct *callback_data;
{
  if (!strcmp(string, "top_lbl"))
    {
      w_top_lbl = w;
      return;
    }

  if (!strcmp(string, "bottom_lbl"))
    {
      Arg arg;

      w_bottom_lbl = w;
      XtSetArg(arg, XmNlabelString, MotifString(" "));
      XtSetValues(w, &arg, 1);
      return;
    }

  if (!strcmp(string, "text"))
    {
      w_text = w;
      return;
    }

  if (!strcmp(string, "up")) {
    w_up = w;
    return;
  }

  if (!strcmp(string, "save")) {
    w_save = w;
    return;
  }

  if (!strcmp(string, "help")) {
    w_help = w;
    return;
  }

  if (!strcmp(string, "quit")) {
    w_quit = w;
    return;
  }

  if (!strcmp(string, "top")) {
    w_top_form = w;
    return;
  }

  if (!strcmp(string, "bottom")) {
    w_bottom_form = w;
    return;
  }

  if (!strcmp(string, "save_dlg")) {
    w_dlg_save = w;
    XtDestroyWidget(XmSelectionBoxGetChild(w,XmDIALOG_HELP_BUTTON));
    return;
  }

  if (!strcmp(string, "help_dlg")) {
    XtDestroyWidget(XmMessageBoxGetChild(w,XmDIALOG_HELP_BUTTON));
    XtDestroyWidget(XmMessageBoxGetChild(w,XmDIALOG_CANCEL_BUTTON));
    return;
  }

  if (!strcmp(string, "list"))
    {
      char file[BUFSIZ];
      struct stat buf;

      w_list = w;
      strcpy(CurrentDir, BASE_DIR);
      sprintf(file, "%s/%s", CurrentDir, "stock_answers");
      strcpy(Indexes[0], file);

#ifdef ATHENA
      /* At Athena, we use attach to mount the stock answer filesystem; your */
      /* method may differ at other sites */
      if (stat(file, &buf))
	system("/bin/athena/attach -n -q olc-stock");	/* attach filsys */
      if (stat(file, &buf))
	{
	  fprintf(stderr, "%s: Unable to attach olc-stock filesystem.\n",
		  program);
	  exit(-1);
	}
#endif
      if (MakeMenu(file) != SUCCESS)
	{
	  fprintf(stderr, "%s: Unable to initialize menus.\n",
		  program);
	  exit(-1);
	}
      return;
    }
}


void RegisterApplicationCallbacks ( app )
     XtAppContext app;
{
#define RCALL(name, func, client) WcRegisterCallback(app, name, func, client)
  RCALL( "setPromptCB",		setPromptCB,		NULL      );
  RCALL( "saveCB",		saveCB,			NULL      );
  RCALL( "selectCB",		selectCB,		NULL      );
  RCALL( "upCB",		upCB,			NULL      );
  RCALL( "setCursorCB",		setCursorCB,		NULL      );
  RCALL( "createCB",		createCB,		NULL      );
#undef RCALL
}
