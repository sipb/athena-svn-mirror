#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/help.c,v 1.2 1990-05-01 14:48:44 epeisach Exp $";
#endif lint

/*	This is the file help.c for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: CreateHelp(), PopupHelp(), and PopdownHelp().
 *	
 *	Created: 	January 19, 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/help.c,v $
 *      $Author: epeisach $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/help.c,v 1.2 1990-05-01 14:48:44 epeisach Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "globals.h"
#include "mit-copyright.h"

#define Y_POS 50

/*	Function Name: CreateHelp.
 *	Description: This function creates the help widget so that it will be
 *                   ready to be displayed.
 *	Arguments:
 *	Returns: FALSE if it could not create help window.
 */

Boolean
CreateHelp()
{
  struct stat fileinfo;		/* file information from fstat. */
  FILE * help_file;		/* The stream of the help file. */
  char * help_page;		/* The help text strored in memory. */
  int help_width;		/* The width of the help window. (default). */
  Arg arglist[10];		/* The arglist */
  Cardinal num_args;		/* The number of arguments. */
  Widget pane;			/* The Vpane, that will contain the help info.
				   */
  static XtCallbackRec Callbacks[] = {
    { PopdownHelp, NULL },
    { NULL, NULL },
  };
  
  if (help_widget != NULL)	/* If we already have a help widget. 
				   then do not create one. */
    return(TRUE);

/* Open help_file and read it into memory. */

/*
 * Get file size and allocate a chunk of memory for the file to be 
 * copied into.
 */

  if( (help_file = fopen(help_file_name, "r")) == NULL ) {
    PrintWarning("Could not open help file, NO HELP WILL BE AVALIABLE.");
    return(FALSE);
  }

  if ( stat(help_file_name, &fileinfo) ) {
    PrintWarning("Failure in fstat, NO HELP WILL BE AVALIABLE.");
    return(FALSE);
  }

  /* leave space for the NULL */
  help_page = (char *) malloc(fileinfo.st_size + 1);	

  if (help_page == NULL) {
    PrintWarning(
      "Could not allocate memory for help file, NO HELP WILL BE AVALIABLE.");
    return(FALSE);
  }

/*
 * Copy the file into memory. 
 */
 
  fread(help_page,sizeof(char),fileinfo.st_size,help_file); 
  fclose(help_file);
    
/* put NULL at end of buffer. */

  *(help_page + fileinfo.st_size) = '\0';

/*
 * Help file now loaded in to memory. Create widgets do display it. 
 */

  num_args = 0;
  XtSetArg(arglist[num_args], XtNallowShellResize, TRUE);
  num_args++;

  help_widget = XtCreateApplicationShell("help", applicationShellWidgetClass, 
				   arglist, num_args);

  num_args = 0;
  help_width = DisplayWidth(XtDisplay(help_widget), 
			    DefaultScreen(XtDisplay(help_widget)));
  help_width /= 2;
  XtSetArg(arglist[num_args], XtNwidth, help_width);
  num_args++;

#if XtSpecificationRelease < 4
  pane = XtCreateWidget("Help_VPaned",vPanedWidgetClass,help_widget,
			arglist,num_args);
#else
  pane = XtCreateWidget("Help_Paned",panedWidgetClass,help_widget,
			arglist,num_args);
#endif

  num_args = 0;
  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;
  XtSetArg(arglist[num_args], XtNlabel, "Done With Help");
  num_args++;
  XtSetArg(arglist[num_args], XtNcallback, Callbacks);
  num_args++;

  (void) XtCreateManagedWidget("Help_Quit",commandWidgetClass, pane,
			       arglist, num_args);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNborderWidth, 0);
  num_args++;
  XtSetArg(arglist[num_args], XtNstring, help_page);
  num_args++;
#if XtSpecificationRelease < 4
  XtSetArg(arglist[num_args], XtNtextOptions, scrollVertical);
  num_args++;
#else
  XtSetArg(arglist[num_args],XtNscrollVertical,XawtextScrollAlways);
  num_args++;
  XtSetArg(arglist[num_args],XtNscrollHorizontal,XawtextScrollWhenNeeded);
  num_args++;
  XtSetArg(arglist[num_args],XtNuseStringInPlace, TRUE);
  num_args++;
#endif
  /* make the text shown a square box. */
  XtSetArg(arglist[num_args], XtNheight, help_width);
  num_args++;
  

#if XtSpecificationRelease < 4
  (void) XtCreateManagedWidget("Help_Text",asciiStringWidgetClass, pane,
			       arglist, num_args);
#else
  (void) XtCreateManagedWidget("Help_Text",asciiTextWidgetClass, pane,
			       arglist, num_args);
#endif

  XtManageChild(pane);
  XtRealizeWidget(help_widget);
  AddCursor(help_widget,help_cursor);

  return(TRUE);
}

/*	Function Name: PopdownHelp
 *	Description: This function pops down the help widget.
 *	Arguments: w - the widget we are calling back from. 
 *                 number - (closure) the number to switch on.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */

void
PopdownHelp(w,number,junk)
Widget w;
caddr_t number,junk;
{
  XtPopdown(help_widget);
}

/*	Function Name: PopupHelp
 *	Description: This function pops up the help widget, unless no
 *                   help could be found.
 *	Arguments: w - the widget we are calling back from. 
 *                 number - (closure) the number to switch on.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */

void
PopupHelp(w,number,junk)
Widget w;
caddr_t number,junk;
{
  if (CreateHelp())
    XtPopup(help_widget,XtGrabNone);
}

