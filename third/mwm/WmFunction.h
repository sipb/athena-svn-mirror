/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: WmFunction.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:19 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern Boolean F_Beep ();
extern Boolean F_Lower ();
extern void Do_Lower ();
extern Boolean F_Circle_Down ();
extern Boolean F_Circle_Up ();
extern Boolean F_Focus_Color ();
extern Boolean F_Exec ();
extern Boolean F_Quit_Mwm ();
extern void Do_Quit_Mwm ();
extern Boolean F_Focus_Key ();
extern void Do_Focus_Key ();
extern Boolean F_Next_Key ();
extern Boolean F_Prev_Cmap ();
extern Boolean F_Prev_Key ();
extern Boolean F_Pass_Key ();
extern Boolean F_Maximize ();
extern Boolean F_Menu ();
extern Boolean F_Minimize ();
extern Boolean F_Move ();
extern Boolean F_Next_Cmap ();
extern Boolean F_Nop ();
extern Boolean F_Normalize ();
extern Boolean F_Normalize_And_Raise ();
extern Boolean F_Pack_Icons ();
extern Boolean F_Post_SMenu ();
extern Boolean F_Kill ();
extern Boolean F_Refresh ();
extern Boolean F_Resize ();
extern Boolean F_Restart ();
extern Boolean F_Restore ();
extern Boolean F_Restore_And_Raise ();
extern void Do_Restart ();
extern void RestartWm ();
extern void DeFrameClient ();
extern Boolean F_Send_Msg ();
extern Boolean F_Separator ();
extern Boolean F_Raise ();
extern void Do_Raise ();
extern Boolean F_Raise_Lower ();
extern Boolean F_Refresh_Win ();
extern Boolean F_Set_Behavior ();
extern void Do_Set_Behavior ();
extern Boolean F_Title ();
extern Boolean F_Screen ();
extern Time GetFunctionTimestamp ();
extern void ReBorderClient ();
extern void ClearDirtyStackEntry ();	/* Fix for 5325 */

#else /* _NO_PROTO */

extern Boolean F_Beep (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Lower (String args, ClientData *pCD, XEvent *event);
extern void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Exec (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event);
extern void Do_Quit_Mwm (Boolean diedOnRestart);
extern Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event);
extern void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags);
extern Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Pass_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Maximize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Menu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Minimize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Move (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Nop (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Kill (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Resize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restart (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern void Do_Restart (Boolean dummy);
extern void RestartWm (long startupFlags);
extern void DeFrameClient (ClientData *pCD);
extern Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Separator (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Raise (String args, ClientData *pCD, XEvent *event);
extern void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event);
extern void Do_Set_Behavior (Boolean dummy);
extern Boolean F_Title (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Screen (String args, ClientData *pCD, XEvent *event);
extern Time GetFunctionTimestamp (XButtonEvent *pEvent);
extern void ReBorderClient (ClientData *pCD, Boolean reMapClient);
extern void ClearDirtyStackEntry (ClientData *pCD);	/* Fix for 5325 */
#endif /* _NO_PROTO */

