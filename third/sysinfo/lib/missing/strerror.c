#if !defined(lint)
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

#if	!defined(HAVE_STRERROR)

#include <stdio.h>
#include <sys/errno.h>

/*
 * Return string for system error number "Num".
 */
char *strerror(Num)
     int			Num;
{
    extern int 			sys_nerr;
    extern char 	       *sys_errlist[];
    static char 		Unknown[100];

    if (Num < 0 || Num > sys_nerr) {
	(void) sprintf(Unknown, "Error %d", Num);
	return(Unknown);
    } else
	return(sys_errlist[Num]);
}
#endif	/* !HAVE_STRERROR */
