/*
 * This file is part of an X stock answer browser.
 * It contains the main routine.
 *
 *	Chris VanHaren
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/motif/main.c,v $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/motif/main.c,v 1.6 1991-04-08 21:27:25 lwvanels Exp $";
#endif

#include <Mrm/MrmAppl.h>	/* Motif Toolkit */
#include <Xm/Mu.h>
#include <Wc/WcCreate.h>
#include <stdio.h>
#include "cref.h"
#include <signal.h>

extern char Titles[MAX_TITLES][TITLE_SIZE];
char *program;				/* name of program */

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
  int n;
  Display *dpy;
  XtAppContext app;
  Widget toplevel;
  int spid;
	
  program = "xbrowser";
  *argv = program;
  
/*
 *  First, try opening display.  If this fails, print a 'nice' error
 *  message and exit.
 */

  if ((dpy = XOpenDisplay(NULL)) == NULL)
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

  n=0;
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
  log_startup("xstock",LOGGER_HOST);
#endif
  XtMainLoop();
}
