/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: WmWinState.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:28 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void NormalTransientTransition();
extern void SetClientState ();
extern void SetClientStateWithEventMask ();
extern void ConfigureNewState ();
extern void SetClientWMState ();
extern void MapClientWindows ();
extern void ShowIconForMinimizedClient ();
#else /* _NO_PROTO */
extern void SetClientState (ClientData *pCD, int newState, Time setTime);
extern void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);
extern void ConfigureNewState (ClientData *pcd);
extern void SetClientWMState (ClientData *pCD, int wmState, int mwmState);
extern void MapClientWindows (ClientData *pCD);
extern void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD);
#endif /* _NO_PROTO */
