/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmSignal.h,v $ $Revision: 1.1.1.1 $ $Date: 1997-03-25 09:12:26 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void SetupWmSignalHandlers ();
extern void QuitWmSignalHandler ();
#else /* _NO_PROTO */
extern void SetupWmSignalHandlers (int);
extern void QuitWmSignalHandler (int);
#endif /* _NO_PROTO */
