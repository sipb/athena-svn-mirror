
/*
 * strerror.c -- get error message string
 *
 * $Id: strerror.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>

extern int sys_nerr;
extern char *sys_errlist[];


char *
strerror (int errnum)
{
   if (errnum > 0 && errnum < sys_nerr)
	return sys_errlist[errnum];
   else
	return NULL;
}
