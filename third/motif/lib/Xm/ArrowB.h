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
/*   $RCSfile: ArrowB.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:44 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowButton_h
#define _XmArrowButton_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsArrowButton
#define XmIsArrowButton(w) XtIsSubclass(w, xmArrowButtonWidgetClass)
#endif /* XmIsArrowButton */

externalref WidgetClass xmArrowButtonWidgetClass;

typedef struct _XmArrowButtonClassRec * XmArrowButtonWidgetClass;
typedef struct _XmArrowButtonRec      * XmArrowButtonWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateArrowButton() ;

#else

extern Widget XmCreateArrowButton( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmArrowButton_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
