/*
 * Copyright (C) 1992 Adobe Systems Incorporated.  All rights reserved.
 *
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/ppanel.c,v 1.1.1.1 1996-10-07 20:25:54 ghudson Exp $
 *
 * RCSLog:
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1992/08/21  16:24:13  snichols
 * Release 4.0
 *
 * Revision 1.3  1992/07/14  23:06:58  snichols
 * Added copyright.
 *
 *
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>    
#include <stdio.h>
#include "PrintPanel.h"

extern char *optarg;
extern int optind;

static void GoAway(w)
    Widget w;
{
    XCloseDisplay(XtDisplay(w));
    exit(0);
}


main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Widget ppanel;
    XtAppContext app;
    Arg args[10];
    int i;
    int c;

    toplevel = XtAppInitialize(&app, "PrintPanelApp", NULL, 0, &argc,
				argv, NULL, NULL, 0);
    i = 0;
    while ((c = getopt(argc, argv, "f:P:d:")) != EOF) {
	switch (c) {
	case 'P':
	case 'd':
	    XtSetArg(args[i], XtNprinterName, optarg); i++;
	    break;
	case 'f':
	    XtSetArg(args[i], XtNfilterName, optarg); i++;
	    break;
	default:
	    fprintf(stderr, "Unrecognized switch %d\n", c);
	    break;
	}
    }
    if (optind > 0) {
	XtSetArg(args[i], XtNinputFileName, argv[optind]); i++;
    }
    ppanel = XtCreateManagedWidget("printpanelwid", printPanelWidgetClass,
	    toplevel, args, i);

    XtAddCallback(ppanel, XtNcancelCallback, GoAway, NULL);

    XtRealizeWidget (toplevel);
    XtAppMainLoop(app);
}
