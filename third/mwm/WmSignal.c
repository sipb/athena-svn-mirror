/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmSignal.c,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:26 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <signal.h>
#include "WmGlobal.h"

/*
 * include extern functions
 */

#include "WmFeedback.h"
#include "WmFunction.h"


/*
 * Function Declarations:
 */

#ifdef _NO_PROTO
void SetupWmSignalHandlers ();
void QuitWmSignalHandler ();
#else /* _NO_PROTO */
void SetupWmSignalHandlers (int);
void QuitWmSignalHandler (int);
#endif /* _NO_PROTO */




/*
 * Global Variables:
 */



/*************************************<->*************************************
 *
 *  SetupWmSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetupWmSignalHandlers (dummy)
    int dummy;
#else /* _NO_PROTO */
void SetupWmSignalHandlers (int dummy)
#endif /* _NO_PROTO */
{
    void (*signalHandler) ();


    signalHandler = (void (*)())signal (SIGINT, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGINT, QuitWmSignalHandler);
    }

    signalHandler = (void (*)())signal (SIGHUP, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGHUP, QuitWmSignalHandler);
    }

    signal (SIGQUIT, QuitWmSignalHandler);

    signal (SIGTERM, QuitWmSignalHandler);


} /* END OF FUNCTION SetupWmSignalHandlers */



/*************************************<->*************************************
 *
 *  QuitWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a signal that is to terminate the
 *  window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void QuitWmSignalHandler (dummy)
    int dummy;
#else /* _NO_PROTO */
void QuitWmSignalHandler (int dummy)
#endif /* _NO_PROTO */
{
    if (wmGD.showFeedback & WM_SHOW_FB_KILL)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
	XFlush(DISPLAY);
	SetupWmSignalHandlers(0);	 /* dummy paramater */
    }
    else
    {
	Do_Quit_Mwm(False);
    }

} /* END OF FUNCTION QuitWmSignalHandler */
