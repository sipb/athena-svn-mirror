/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmColormap.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:17 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitWorkspaceColormap ();
extern void InitColormapFocus ();
#ifndef OLD_COLORMAP
extern void ForceColormapFocus ();
#endif
extern void SetColormapFocus ();
extern void WmInstallColormap ();
extern void ResetColormapData ();
extern void ProcessColormapList();
#else /* _NO_PROTO */
extern void InitWorkspaceColormap (WmScreenData *pSD);
extern void InitColormapFocus (WmScreenData *pSD);
#ifndef OLD_COLORMAP
extern void ForceColormapFocus (WmScreenData *pSD, ClientData *pCD);
#endif
extern void SetColormapFocus (WmScreenData *pSD, ClientData *pCD);
extern void WmInstallColormap (WmScreenData *pSD, Colormap colormap);
extern void ResetColormapData (ClientData *pCD, Window *pWindows, int count);
extern void ProcessColormapList (WmScreenData *pSD, ClientData *pCD);
#endif /* _NO_PROTO */
