/*
 * internal include file for com_err package
 */
#include "mit-sipb-copyright.h"
#ifndef __STDC__
#undef const
#define const
#endif

#ifdef NEED_SYS_ERRLIST
extern char const * const sys_errlist[];
extern const int sys_nerr;
#endif
