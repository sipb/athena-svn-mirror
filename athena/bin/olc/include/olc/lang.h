/*
 * This file is part of the OLC On-Line Consulting system.
 * lang.h: Random hacks to provide language compatibility between
 * old-C and ANSI-C.  A little stuff also for C++.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/lang.h,v $
 *	$Id: lang.h,v 1.2 1990-05-25 15:07:22 vanharen Exp $
 *	$Author: vanharen $
 */

#include <mit-copyright.h>

#if defined (__cplusplus) || defined (__GNUG__)
/*#define class	struct		/* C++ */
#define is_cplusplus 1
#else
#define is_cplusplus 0
#endif

#ifndef __STDC__
#define const
#define volatile
#define signed
#define OPrototype(X)	()
#else
#define OPrototype(X)	X
#endif
