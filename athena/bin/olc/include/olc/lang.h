/*
 * This file is part of the OLC On-Line Consulting system.
 * lang.h: Random hacks to provide language compatibility between
 * old-C and ANSI-C.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/lang.h,v $
 *	$Id: lang.h,v 1.3 1991-04-08 21:00:39 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __STDC__
#define const
#define volatile
#define signed
#define OPrototype(X)	()
#else
#define OPrototype(X)	X
#endif
