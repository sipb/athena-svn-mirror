/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmEvent.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:18 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern Boolean		CheckForButtonAction ();
extern Time		GetTimestamp ();
extern Boolean		HandleKeyPress ();
extern void		HandleWsButtonPress ();
extern void		HandleWsButtonRelease ();
extern void		HandleWsConfigureRequest ();
extern void		HandleWsEnterNotify ();
extern void		HandleWsFocusIn ();
extern Boolean		HandleWsKeyPress ();
extern void		HandleWsLeaveNotify ();
extern void		IdentifyEventContext ();
extern void		InitEventHandling ();
extern void		ProcessClickBPress ();
extern void		ProcessClickBRelease ();
extern void		PullExposureEvents ();
extern int		SetupKeyBindings ();
extern Boolean		WmDispatchMenuEvent ();
extern Boolean		WmDispatchWsEvent ();

#else /* _NO_PROTO */

extern Boolean CheckForButtonAction (XButtonEvent *buttonEvent, 
				     Context context, Context subContext, 
				     ClientData *pCD);
extern Time GetTimestamp (void);
extern Boolean HandleKeyPress (XKeyEvent *keyEvent, KeySpec *keySpecs, 
			       Boolean checkContext, Context context, 
			       Boolean onlyFirst, ClientData *pCD);
extern void HandleWsButtonPress (XButtonEvent *buttonEvent);
extern void HandleWsButtonRelease (XButtonEvent *buttonEvent);
extern void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent);
extern void HandleWsEnterNotify (XEnterWindowEvent *enterEvent);
extern void HandleWsFocusIn (XFocusInEvent *focusEvent);
extern Boolean HandleWsKeyPress (XKeyEvent *keyEvent);
extern void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent);
extern void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, 
				  Context *pContext, int *pPartContext);
extern void InitEventHandling (void);
extern void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, 
				Context context, Context subContext);
extern void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, 
				  Context context, Context subContext);
extern void PullExposureEvents (void);
extern int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, 
			     int keyboardMode, long context);
extern Boolean WmDispatchMenuEvent (XButtonEvent *event);
extern Boolean WmDispatchWsEvent (XEvent *event);
#endif /* _NO_PROTO */
