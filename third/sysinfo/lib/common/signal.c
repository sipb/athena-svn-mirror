/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Signal functions
 */

#include "defs.h"
#include <signal.h>
#include <unistd.h>

/*
 * Return the name of SigNum signal
 */
extern char *SignalName(SigNum)
     int			SigNum;
{
    char		       *What = NULL;
    static char			Buff[128];

    switch (SigNum) {
    case SIGALRM:
	What = "alarm";
	break;
    case SIGINT:
	What = "interrupt";
	break;
    default:
	(void) snprintf(Buff, sizeof(Buff), "signal %d", SigNum);
	What = Buff;
    }

    return What;
}

/*
 * Start the Timer
 */
extern void TimeOutStart(Time, SignalHandler)
     time_t		        Time;
#if	defined(__STDC__)
     void 		      (*SignalHandler)(int);
#else
     void 		      (*SignalHandler);
#endif	/* __STDC__ */
{
    (void) signal(SIGALRM, SignalHandler);
    alarm(Time);
}

/*
 * End the timeout 
 */
extern void TimeOutEnd()
{
    alarm(0);
    (void) signal(SIGALRM, SIG_DFL);
}
