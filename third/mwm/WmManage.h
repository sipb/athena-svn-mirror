/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmManage.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:22 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void AdoptInitialClients ();
extern void ManageWindow ();
extern void UnManageWindow ();
extern void WithdrawTransientChildren ();
extern void WithdrawWindow ();
extern void ResetWithdrawnFocii ();
extern void FreeClientFrame ();
extern void FreeIcon ();
extern void WithdrawDialog ();
extern void ReManageDialog ();
#else /* _NO_PROTO */
extern void AdoptInitialClients (WmScreenData *pSD);
extern void ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags);
extern void UnManageWindow (ClientData *pCD);
extern void WithdrawTransientChildren (ClientData *pCD);
extern void WithdrawWindow (ClientData *pCD);
extern void ResetWithdrawnFocii (ClientData *pCD);
extern void FreeClientFrame (ClientData *pCD);
extern void FreeIcon (ClientData *pCD);
extern void WithdrawDialog (Widget dialogboxW);
extern void ReManageDialog (WmScreenData *pSD, Widget dialogboxW);
#endif /* _NO_PROTO */
