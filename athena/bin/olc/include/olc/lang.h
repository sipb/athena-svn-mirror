/*
 * This file is part of the OLC On-Line Consulting system.
 * lang.h: Random hacks to provide language compatibility between
 * old-C and ANSI-C.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: lang.h,v 1.4 1999-01-22 23:13:40 ghudson Exp $
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
