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
/*   $RCSfile: AtomMgrI.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:44 $ */
/*
*  (c) Copyright 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmAtomMgrI_h
#define _XmAtomMgrI_h

#include <Xm/AtomMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmInternAtomAndName() ;

#else

extern void _XmInternAtomAndName( 
                        Display *display,
                        Atom atom,
                        String name) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmAtomMgrI_h */
