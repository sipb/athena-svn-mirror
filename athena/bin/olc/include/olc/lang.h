/*
 * lang.h: Random hacks to provide language compatibility between
 * old-C and ANSI-C.  A little stuff also for C++.
 */

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
