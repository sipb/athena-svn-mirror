/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: WmResource.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:26 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void GetAppearanceGCs ();
extern GC   GetHighlightGC ();
extern void MakeAppearanceResources ();
extern void ProcessClientResources ();
extern void ProcessWmResources ();
extern void ProcessScreenListResource ();
extern void ProcessAppearanceResources ();
extern void ProcessGlobalScreenResources ();
extern void ProcessScreenResources ();
extern void ProcessWorkspaceResources ();
extern void SetStdClientResourceValues ();
extern void SetStdGlobalResourceValues ();
extern void SetStdScreenResourceValues ();
extern char *WmRealloc ();
extern char *WmMalloc ();
extern void SetupDefaultResources ();
extern Boolean SimilarAppearanceData ();
#else /* _NO_PROTO */
extern void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
extern GC   GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
extern void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
extern void ProcessWmResources (void);
extern void SetStdGlobalResourceValues (void);
extern void ProcessScreenListResource (void);
extern void ProcessAppearanceResources (WmScreenData *pSD);
extern void ProcessGlobalScreenResources (void);
extern void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
extern void ProcessWorkspaceResources (WmWorkspaceData *pWS);
extern void ProcessClientResources (ClientData *pCD);
extern void SetStdClientResourceValues (ClientData *pCD);
extern void SetStdScreenResourceValues (WmScreenData *pSD);
extern char *WmRealloc (char *ptr, unsigned size);
extern char *WmMalloc (char *ptr, unsigned size);
extern void SetupDefaultResources (WmScreenData *pSD);
extern Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);
#endif /* _NO_PROTO */

extern char builtinSystemMenu[];
extern char builtinKeyBindings[];
extern char builtinRootMenu[];
extern char builtinSystemMenuName[];

#define Monochrome(screen) ( DefaultDepthOfScreen(screen) == 1 )
