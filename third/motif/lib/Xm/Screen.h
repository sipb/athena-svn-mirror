/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $RCSfile: Screen.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:59 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmScreen_h
#define _XmScreen_h

#include <Xm/Xm.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsScreen
#define XmIsScreen(w) (XtIsSubclass(w, xmScreenClass))
#endif /* XmIsScreen */

/* Class record constants */

typedef struct _XmScreenRec *XmScreen;
typedef struct _XmScreenClassRec *XmScreenClass;
externalref 	WidgetClass xmScreenClass;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmGetXmScreen() ;

#else

extern Widget XmGetXmScreen( 
                        Screen *screen) ;
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScreen_h */

