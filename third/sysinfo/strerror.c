#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

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
