#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/main.c,v 1.2 1990-05-01 14:48:49 epeisach Exp $";
#endif lint

/*	This is the file main.c for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: main(), Quit(), TextExit(), PrintWarning(), PrintError(),
 *                   CreateScroll(), CreatePane(), and AddCursor().
 *	
 *	Created: 	October 22, 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/main.c,v $
 *      $Author: epeisach $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/main.c,v 1.2 1990-05-01 14:48:49 epeisach Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "globals.h"
#include "mit-copyright.h"

/*	Function Name: main
 *	Description: This is the main driver for Xman.
 *	Arguments: argc, argv - the command line arguments.
 *	Returns: return, what return.
 */


static XtResource resources[] = {
  {"textFontNormal", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.normal), XtRString, NORMALFONT},
  {"textFontItalic", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.italic), XtRString, ITALICFONT},
  {"topCursor", XtCCursor, XtRCursor, sizeof(Cursor), 
     (Cardinal) &main_cursor, XtRString, MAIN_CURSOR},
  {"helpCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     (Cardinal) &help_cursor, XtRString, HELP_CURSOR},
  {"helpFile", XtCFile, XtRString, sizeof(char *),
     (Cardinal) &(help_file_name), XtRString, HELPFILE},
};

void 
main(argc,argv)
char ** argv;
int argc;
{
  Widget top,scroll,pane;	/* The top level widget, and scroll widget, 
			           and the pane widget.*/
  FILE * file;

  top = XtInitialize(argv[0],"Topwidget",NULL,(unsigned int) 0,
		     (Cardinal*) &argc,argv);

  XtGetApplicationResources( (Widget) top, (caddr_t) NULL, 
			    resources, XtNumber(resources),
			    NULL, (Cardinal) 0);
  if (!fonts.normal)
	XtError("failed to get the textFontNormal font");
  if (!fonts.italic)
	fonts.italic = fonts.normal;

#ifdef DEBUG
  printf("debugging mode, snychronizing the X server.\n");
  XSynchronize( XtDisplay(top), 1);
#endif

/* Initialize Help. */

  help_widget = NULL;

  if (argc > 1) {
    if ( (file = fopen(argv[1],"r")) == NULL) {
      printf("Could not open file -  %s\n",argv[1]); 
      exit(1);
    }
    pane = CreatePane(top);
    scroll = CreateScroll(pane);
    XtManageChild(pane);
    InitPage(scroll,file);
    XtAddEventHandler(scroll,(unsigned int) ButtonPressMask|ButtonReleaseMask,
		      FALSE, TextExit, NULL);
    XtRealizeWidget(top);
    AddCursor(top,main_cursor); /* must be done after realize. */
    XtMainLoop();
  }
    printf("usage: xmore filename\n");
}

/*	Function Name: CreateScroll
 *	Description: This function creates the scrollByLineWidget for Xmore.
 *	Arguments: parent - the parent widget.
 *	Returns: none.
 */

Widget
CreateScroll(parent)
Widget parent;
{
  Widget scroll;		/* The scrollByLine Widget we are creating. */
  Arg arglist[10];		/* The arglist */
  Cardinal num_args;		/* The number of arguments in the arglist. */
  MemoryStruct * memory_struct;	/* The memory structure. */
  int font_height;

  static XtCallbackRec Callback[] = {
    { PrintPage, NULL },
    { NULL, NULL },
  };

  /* find out how tall the font is. */

  font_height = (fonts.normal->max_bounds.ascent + 
		   fonts.normal->max_bounds.descent);

/* Initialize the memory structure. */

  memory_struct = (MemoryStruct *) malloc(sizeof(MemoryStruct));
  memory_struct->top_line = NULL;
  memory_struct->top_of_page = NULL;
  
  global_memory_struct = memory_struct;

  Callback[0].closure = (caddr_t) memory_struct;
  num_args = (Cardinal) 0;
  XtSetArg(arglist[num_args], XtNcallback, Callback);
  num_args++;
  XtSetArg(arglist[num_args], XtNfontHeight, font_height);
  num_args++;  
  XtSetArg(arglist[num_args], XtNallowVert, TRUE);
  num_args++;  
  
  scroll = XtCreateWidget("ScrolledWidget",scrollByLineWidgetClass,
			  parent,arglist,num_args);
  XtManageChild(scroll);

  return(scroll);
}

/*	Function Name: CreatePane
 *	Description: This function Creates a vPaned widget with a help
 *                   button in the top pane.
 *	Arguments: parent - the parent of the vpane.
 *	Returns: pane - the Vpaned widget.
 */

Widget
CreatePane(parent)
Widget parent;
{
  Widget pane,form,quit,help;	/* Several widget names. */
  Arg arglist[3];		/* An arglist. */
  Cardinal num_args;	      /* The number of argument in the current list. */
  int height, height_vert, border_width; /* The sizes of the quit widget. */

  num_args = (Cardinal) 0;
/* TopLevel overrides but this gives it something to work with. */
  XtSetArg(arglist[num_args], XtNwidth, DEFAULT_WIDTH);
  num_args++;
  XtSetArg(arglist[num_args], XtNheight, DEFAULT_HEIGHT);
  num_args++;

#if XtSpecificationRelease < 4
  pane = XtCreateWidget("VPanedTop",vPanedWidgetClass,
			  parent,arglist, num_args);
#else
  pane = XtCreateWidget("PanedTop",panedWidgetClass,
			  parent,arglist, num_args);
#endif

  num_args = 0;
  form = XtCreateWidget("formButtons",formWidgetClass,pane,
			arglist,num_args);

  num_args = (Cardinal) 0;
  quit = XtCreateManagedWidget("Click Here To Quit",commandWidgetClass,
			       form,arglist,num_args);
  XtAddCallback(quit, XtNcallback, Quit, NULL);

  XtSetArg(arglist[num_args], XtNfromHoriz, quit);
  num_args++;
  
  help = XtCreateManagedWidget("Click Here For Help",commandWidgetClass,
			       form,arglist,num_args);
  XtAddCallback(help, XtNcallback, PopupHelp, NULL);

/*
 * Mildly confusing method of setting the max paramater for
 * This pane to be its height.
 */

  num_args = (Cardinal) 0;
  XtSetArg(arglist[num_args], XtNheight, &height);
  num_args++;
  XtSetArg(arglist[num_args], XtNvertDistance, &height_vert);
  num_args++;
  XtSetArg(arglist[num_args], XtNborderWidth, &border_width);
  num_args++;
  XtGetValues(quit, arglist, num_args);
#if XtSpecificationRelease < 4
  XtPanedSetMinMax( form, 2, height + 2 * (height_vert + border_width) );
#else
  XawPanedSetMinMax( form, 2, height + 2 * (height_vert + border_width) );
#endif

  XtManageChild(form);
  return(pane);
}

/*	Function Name: TextExit
 *	Description: closes the display and quits.
 *	Arguments: widget - the widget that called the event.
 *                 junk - closure (not used).
 *                 event - the event structure.
 *	Returns: none.
 */

/* ARGSUSED */
void 
TextExit(w,junk,event)
Widget w;
caddr_t junk;
XEvent * event;
{
  switch(event->type) {
  case ButtonPress:
    break;
  case ButtonRelease:
    if (event->xbutton.button == 2) {
      Quit(w,NULL,NULL);
    }
    break;
  default:
    break;
  }
}

/*	Function Name: Quit
 *	Description: closes the display and quits.
 *	Arguments: widget - the widget that called the event.
 *                 junk, garbage - closure and callback (not used).
 *	Returns: none.
 */

/* ARGSUSED */
void 
Quit(w,junk,garbage)
Widget w;
caddr_t junk,garbage;
{
  XCloseDisplay(XtDisplay(w));
  exit(0);
}

/*	Function Name: PrintWarning
 *	Description: This function prints a warning message to stderr.
 *	Arguments: string - the specific warning string.
 *	Returns: none
 */

void
PrintWarning(string)
char * string;
{
  fprintf(stderr,"Xmore Warning: %s\n",string);
}

/*	Function Name: PrintError
 *	Description: This Function prints an error message and exits.
 *	Arguments: string - the specific message.
 *	Returns: none. - exits tho.
 */

void
PrintError(string)
char * string;
{
  fprintf(stderr,"Xmore Error: %s\n",string);
#ifdef DEBUG
  fprintf(stderr,"\n\nbye,bye\n\n\n\n\nsniff...\n");
#endif
  exit(42);
}

/*	Function Name: AddCursor
 *	Description: This function adds the cursor to the window.
 *	Arguments: w - the widget to add the cursor to.
 *                 cursor - the cursor to add to this widget.
 *	Returns: none
 */

void
AddCursor(w,cursor)
Widget w;
Cursor cursor;
{

  if (!XtIsRealized(w)) {
    PrintWarning("Widget is not realized, no cursor added.\n");
    return;
  }
  XDefineCursor(XtDisplay(w),XtWindow(w),cursor);
}
