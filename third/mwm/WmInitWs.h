/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: WmInitWs.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:22 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitWmGlobal ();
extern void InitWmScreen ();
extern void InitWmWorkspace ();
extern void ProcessMotifWmInfo ();
extern void SetupWmWorkspaceWindows ();
extern void MakeWorkspaceCursors ();
extern void MakeWmFunctionResources ();
extern void MakeXorGC ();
extern void CopyArgv ();
extern void InitScreenNames ();
#else /* _NO_PROTO */
extern void InitWmGlobal (int argc, char *argv [], char *environ []);
extern void InitWmScreen (WmScreenData *pSD, int sNum);
extern void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD);
extern void ProcessMotifWmInfo (Window rootWindowOfScreen);
extern void SetupWmWorkspaceWindows (void);
extern void MakeWorkspaceCursors (void);
extern void MakeWmFunctionResources (WmScreenData *pSD);
extern void MakeXorGC (WmScreenData *pSD);
extern void CopyArgv (int argc, char *argv []);
extern void InitScreenNames (void);
#endif /* _NO_PROTO */
