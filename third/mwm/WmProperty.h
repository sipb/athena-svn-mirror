/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: WmProperty.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:23 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern SizeHints * GetNormalHints ();
extern void ProcessWmProtocols ();
extern void ProcessMwmMessages ();
extern void SetMwmInfo ();
extern PropWMState * GetWMState ();
extern void SetWMState ();
extern PropMwmHints * GetMwmHints ();
extern PropMwmInfo * GetMwmInfo ();
extern void ProcessWmColormapWindows ();
extern Colormap FindColormap ();
extern MenuItem * GetMwmMenuItems ();
#else /* _NO_PROTO */
extern SizeHints * GetNormalHints (ClientData *pCD);
extern void ProcessWmProtocols (ClientData *pCD);
extern void ProcessMwmMessages (ClientData *pCD);
extern void SetMwmInfo (Window propWindow, long flags, Window wmWindow);
extern PropWMState * GetWMState (Window window);
extern void SetWMState (Window window, int state, Window icon);
extern PropMwmHints * GetMwmHints (ClientData *pCD);
extern PropMwmInfo * GetMwmInfo (Window rootWindowOfScreen);
extern void ProcessWmColormapWindows (ClientData *pCD);
extern Colormap FindColormap (ClientData *pCD, Window window);
extern MenuItem * GetMwmMenuItems (ClientData *pCD);
#endif /* _NO_PROTO */
