/*
 * internal include file for com_err package
 */
#include "sipb-copying.h"
#ifndef __STDC__
#undef const
#define const
#endif

extern int errno;
extern char const * const sys_errlist[];
extern const int sys_nerr;

#if !defined(__STDC__) || defined(__HIGHC__)
int perror ();
#endif
