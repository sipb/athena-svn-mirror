/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmIPlace.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:20 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitIconPlacement ();
extern int GetNextIconPlace ();
extern void CvtIconPlaceToPosition ();
extern int FindIconPlace ();
extern int CvtIconPositionToPlace ();
extern void PackRootIcons ();
extern void MoveIconInfo ();
#else /* _NO_PROTO */
extern void InitIconPlacement (WmWorkspaceData *pWS);
extern int GetNextIconPlace (IconPlacementData *pIPD);
extern void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY);
extern int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y);
extern int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y);
extern void PackRootIcons (void);
extern void MoveIconInfo (IconPlacementData *pIPD, int p1, int p2);
#endif /* _NO_PROTO */
