#ifndef lint
static char *RCSid = "$Id: strerror.c,v 1.1.1.1 1996-10-07 20:16:52 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1994/11/03  00:04:48  mcooper
 * Update RCS header
 *
 * Revision 1.2  1994/10/11  02:04:56  mcooper
 * Revamp to read definetion info from externel file
 *
 * Revision 1.1  1992/03/21  02:48:11  mcooper
 * Initial revision
 *
 */

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
