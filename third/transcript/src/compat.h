/* compat.h
 *
 * Copyright (C) 1991,1992 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/compat.h,v 1.1.1.1 1996-10-07 20:25:48 ghudson Exp $
 *
 */
#ifndef XPG3

#define SEEK_SET 0

extern void exit();
extern char *malloc();
extern int abs();
extern double atof();
extern int atoi();
extern long atol();
extern char *bsearch();
extern char *getenv();
extern double strtod();
extern void qsort();

#endif /* XPG3 */




