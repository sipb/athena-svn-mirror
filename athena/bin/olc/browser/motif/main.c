/*
 * This file is part of an X stock answer browser.
 * It contains the main routine.
 *
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Id: main.c,v 1.13 1999-01-22 23:11:53 ghudson Exp $
 */

#ifndef lint
static char rcsid[]="$Id: main.c,v 1.13 1999-01-22 23:11:53 ghudson Exp $";
#endif

#include <mit-copyright.h>

#include <Mrm/MrmAppl.h>	/* Motif Toolkit */
#include <Mu.h>
#include <X11/Wc/WcCreate.h>
#include <stdio.h>
#include <browser/cref.h>
#include <signal.h>

extern char Titles[MAX_TITLES][TITLE_SIZE];
extern Widget w_top_form, w_bottom_form;
extern Widget w_list, w_text, w_top_lbl, w_bottom_lbl;
extern Widget w_up, w_save, w_help, w_quit, w_dlg_save;

char *program;				/* name of program */
Widget toplevel;

/*
 *  Function:	main() is the startup for the browser.  It initializes
 *			the X display and toolkits, and reads the initial
 *			.index file.
 *  Arguments:	argc:  Number of args in command line.  Used by XtInitialize.
 *		argv:  Array of words in command line.  Used by XtInitialize.
 *  Returns:    Nothing.
 *  Notes:
 *	
 */

void main(argc, argv)
     int argc;
     char *argv[];
{
  Arg arg;
  Display *dpy;
  XtAppContext app;
  int spid;
	
  program = "xbrowser";
  *argv = program;
  
/*
 *  First, try opening display.  If this fails, print a 'nice' error
 *  message and exit.
 */

  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    {
      fprintf(stderr, "%s: Unable to open X display.  Check to make sure that your\n\tDISPLAY environment variable is set.  Type:\n\n\t\tprintenv  DISPLAY\n\n\tto see if it is set.\n\n\tIf it is not, usually the problem can be fixed by\n\tsetting your DISPLAY to 'unix:0.0'.   Type:\n\n\t\tsetenv  DISPLAY  unix:0.0\n\n\tand try running this program again.\n\n", program);
      exit(-1);
    }
  XCloseDisplay(dpy);

/*
 *  If opening display was successful, then initialize toolkit, display,
 *  interface, etc.
 */
  toplevel = XtInitialize(NULL, "Xbrowser", NULL, 0, &argc, argv);
  app = XtWidgetToApplicationContext(toplevel);

  XtSetArg(arg, XtNallowShellResize, TRUE);
  XtSetValues(toplevel, &arg, 1);

  spid = 0;
  if (argc == 3) {
    if (strcmp(argv[1],"-signal") == 0)
      spid = atoi(argv[2]);
  }
  MuInitialize(toplevel);
  MriRegisterMotif(app);
  RegisterApplicationCallbacks(app);
  WcWidgetCreation(toplevel);
  XtRealizeWidget(toplevel);

  XtInstallAllAccelerators(toplevel, toplevel);
  XtInstallAllAccelerators(w_top_form, toplevel);
  XtInstallAllAccelerators(w_bottom_form, toplevel);
  XtInstallAllAccelerators(w_list, toplevel);
  XtInstallAllAccelerators(w_text, toplevel);
  XtInstallAllAccelerators(w_up, toplevel);
  XtInstallAllAccelerators(w_save, toplevel);
  XtInstallAllAccelerators(w_help, toplevel);
  XtInstallAllAccelerators(w_quit, toplevel);

/*
 *  Perform first-time initialization.
 */

  strcpy(Titles[0], "OLC Stock Answer Browser");

/*
 *  Loop forever.  Exiting controlled by the "quit" callback.
 */

  /* Let our invoker know we're ready... */
  if (spid != 0) {
    kill(spid,SIGUSR1);
  }
#ifdef LOG_USAGE
  log_startup("x_stock");
#endif
  XtMainLoop();
}
