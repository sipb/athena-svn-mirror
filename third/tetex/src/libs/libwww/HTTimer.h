/*

  					W3C Sample Code Library libwww Timer Class


!
  The Timer Class
!
*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

The Timer class handles timer for libwww and the application. This works
exactly as in X where you create a timer object with a callback function
and a timeout. The callback will be called every time the timer expires.
*/

#ifndef HTTIMER_H
#define HTTIMER_H

#include "wwwsys.h"
#include "HTReq.h"

#ifndef IN_EVENT
typedef struct _HTTimer HTTimer;
#endif

typedef int HTTimerCallback (HTTimer *, void *, HTEventType type);

/*
(
  Create and Delete Timers
)
*/

extern HTTimer * HTTimer_new (HTTimer *, HTTimerCallback *, 
			      void *, ms_t millis,
                              BOOL relative, BOOL repetitive);
extern BOOL HTTimer_delete (HTTimer * timer);
extern BOOL HTTimer_deleteAll (void);
extern int HTTimer_dispatch (HTTimer * timer);
extern ms_t HTTimer_getTime(HTTimer * timer);

/*
(
  Reset an already existing Repetitive Timer
)
*/

extern BOOL HTTimer_refresh(HTTimer * timer, ms_t now);

/*
(
  Has this timer Expired?
)

If so then it's time to call the dispatcher!
*/

extern BOOL HTTimer_hasTimerExpired (HTTimer * timer);

/*
(
  Platform Specific Timers
)

On some platform, timers are supported via events or other OS specific
mechanisms. You can make libwww can support these by registering a platform
specific timer add and timer delete method.
*/

typedef BOOL HTTimerSetCallback (HTTimer * timer);

extern BOOL HTTimer_registerSetTimerCallback (HTTimerSetCallback * cbf);
extern BOOL HTTimer_registerDeleteTimerCallback (HTTimerSetCallback * cbf);

/*
(
  Get the next timer in line
)

Dispatches all expired timers and optionally returns the time till the next
one.
*/

extern int HTTimer_next (ms_t * pSoonest);

/*
*/

#endif /* HTTIMER_H */

/*

  

  @(#) $Id: HTTimer.h,v 1.1.1.1 2000-03-10 17:53:02 ghudson Exp $

*/
