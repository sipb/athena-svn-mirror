/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmMain.c,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:21 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

/*
 * include extern functions
 */

#include "WmCEvent.h"
#include "WmEvent.h"
#include "WmInitWs.h"


/*
 * Function Declarations:
 */


#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : False)

WmScreenData *pSD;

/*
 * Global Variables:
 */

WmGlobalData wmGD;



/*************************************<->*************************************
 *
 *  main (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This is the main window manager function.  It calls window manager
 *  initializtion functions and has the main event processing loop.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
int
main (argc, argv, environ)
    int argc;
    char *argv[];
    char *environ[];

#else /* _NO_PROTO */
int
main (int argc, char *argv [], char *environ [])
#endif /* _NO_PROTO */
{
    XEvent	event;
    Boolean	dispatchEvent;

#ifndef NO_MULTIBYTE
    XtSetLanguageProc (NULL, (XtLanguageProc)NULL, NULL);
#endif

    /*
     * Initialize the workspace:
     */

    InitWmGlobal (argc, argv, environ);
    
    /*
     * MAIN EVENT HANDLING LOOP:
     */

    for (;;)
    {
        XtAppNextEvent (wmGD.mwmAppContext, &event);


        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 * The lock modifier is "filtered" out for window manager processing.
	 */

	wmGD.attributesWindow = 0L;

	if ((event.type == ButtonPress) || (event.type == ButtonRelease) ||
	     (event.type == KeyPress) || (event.type == KeyRelease))
	{
	    wmGD.currentEventState = event.xbutton.state;
	    if (wmGD.ignoreLockMod)
	    {
	        event.xbutton.state &= ~(LockMask);
	    }
	}

	dispatchEvent = True;
	if (wmGD.menuActive)
	{
	    /*
	     * Do special menu event preprocessing.
	     */

	    if (wmGD.checkHotspot || wmGD.menuUnpostKeySpec ||
		wmGD.menuActive->accelKeySpecs)
	    {
	        dispatchEvent = WmDispatchMenuEvent ((XButtonEvent *) &event);
	    }
	}

	if (dispatchEvent)
	{
	    if (ManagedRoot(event.xany.window))
	    {
	        dispatchEvent = WmDispatchWsEvent (&event);
	    }
	    else
	    {
	        dispatchEvent = WmDispatchClientEvent (&event);
	    }

	    if (dispatchEvent)
	    {
                /*
                 * Dispatch widget related event:
                 */

                XtDispatchEvent (&event);
	    }
	}
    }

} /* END OF FUNCTION main */

