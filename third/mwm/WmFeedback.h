/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmFeedback.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:18 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void        ConfirmAction ();
extern void        HideFeedbackWindow ();
extern void        InitCursorInfo ();
extern void        PaintFeedbackWindow();
extern void        ShowFeedbackWindow();
extern void        ShowWaitState ();
extern void        UpdateFeedbackInfo();
extern void        UpdateFeedbackText();

#else /* _NO_PROTO */

extern void ConfirmAction (WmScreenData *pSD, int nbr);
extern void HideFeedbackWindow (WmScreenData *pSD);
extern void InitCursorInfo (void);
extern void PaintFeedbackWindow (WmScreenData *pSD);
extern void ShowFeedbackWindow (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height, 
				unsigned long style);
extern void ShowWaitState (Boolean flag);
extern void UpdateFeedbackInfo (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height);
extern void UpdateFeedbackText (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height);

#endif /* _NO_PROTO */
