/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: WmMenu.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:22 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void      ActivateCallback ();
extern Widget    CreateMenuWidget ();
extern void      FreeCustomMenuSpec ();
extern void      MWarning ();
extern MenuSpec *MakeMenu ();
extern void      PostMenu ();
extern void      TraversalOff ();
extern void      TraversalOn ();
extern void      UnpostMenu ();
extern void      UnmapCallback ();


#else /* _NO_PROTO */

extern void ActivateCallback (Widget w, XtPointer client_data, 
			      XtPointer call_data);
extern Widget CreateMenuWidget (WmScreenData *pSD, String menuName, 
				Widget parent, Boolean fTopLevelPane, 
				MenuSpec *topMenuSpec, 
				MenuItem *moreMenuItems);
extern void FreeCustomMenuSpec (MenuSpec *menuSpec);
extern void MWarning (char *format, char *message);
extern MenuSpec *MakeMenu (WmScreenData *pSD, String menuName, 
			   Context initialContext, Context accelContext, 
			   MenuItem *moreMenuItems, Boolean fSystemMenu);
extern void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, 
		      unsigned int button, Context newContext, long flags, 
		      XEvent *passedInEvent);
extern void TraversalOff (MenuSpec *menuSpec);
extern void TraversalOn (MenuSpec *menuSpec);
extern void UnmapCallback (Widget w, XtPointer client_data, XtPointer call_data);
extern void UnpostMenu (MenuSpec *menuSpec);

#endif /* _NO_PROTO */
