/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmResParse.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:26 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#include <stdio.h>

#ifdef _NO_PROTO

extern void ProcessWmFile ();
extern void ProcessCommandLine ();
extern void ProcessMotifBindings ();
extern FILE          * FopenConfigFile ();
extern void            FreeMenuItem ();
extern unsigned char * GetNextLine ();
extern unsigned char * GetString ();

extern Boolean          ParseBtnEvent ();
extern Boolean          ParseKeyEvent ();
extern void            ParseButtonStr ();
extern void            ParseKeyStr ();
extern MenuItem      * ParseMwmMenuStr ();
 
extern int             ParseWmFunction ();
extern void            ProcessWmFile ();
extern void            PWarning ();
extern void            SaveMenuAccelerators ();
extern void             ScanAlphanumeric ();
extern void            ScanWhitespace ();
extern void            ToLower ();
extern void		SyncModifierStrings();

#else /* _NO_PROTO */

extern void ProcessWmFile (WmScreenData *pSD);
extern void ProcessCommandLine (int argc,  char *argv[]);
extern void ProcessMotifBindings (void);
extern FILE          * FopenConfigFile (void);
extern void            FreeMenuItem (MenuItem *menuItem);
extern unsigned char * GetNextLine (void);
extern unsigned char * GetString (unsigned char **linePP);
extern Boolean ParseBtnEvent (unsigned char  **linePP,
                              unsigned int *eventType,
                              unsigned int *button,
                              unsigned int *state,
                              Boolean      *fClick);

extern void            ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
extern void            ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
extern Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
extern MenuItem      * ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
extern int             ParseWmFunction (unsigned char **linePP, unsigned int res_spec, WmFunction *pWmFunction);
extern void            ProcessWmFile (WmScreenData *pSD);
extern void            PWarning (char *message);
extern void            SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
extern void      ScanAlphanumeric (unsigned char **linePP);
extern void            ScanWhitespace(unsigned char  **linePP);
extern void            ToLower (unsigned char  *string);
extern void		SyncModifierStrings(void);
#endif /* _NO_PROTO */
