/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmError.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:17 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void WmInitErrorHandler ();
extern int WmXErrorHandler ();
extern int WmXIOErrorHandler ();
extern void WmXtErrorHandler ();
extern void WmXtWarningHandler ();
extern void Warning ();
#else /* _NO_PROTO */
extern void WmInitErrorHandler (Display *display);
extern int WmXErrorHandler (Display *display, XErrorEvent *errorEvent);
extern int WmXIOErrorHandler (Display *display);
extern void WmXtErrorHandler (char *message);
extern void WmXtWarningHandler (char *message);
extern void Warning (char *message);
#endif /* _NO_PROTO */
