#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/third/cns/src/appl/rdist/src/strerror.c,v 1.1.1.1 1996-09-06 00:46:17 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1996/06/24 20:49:33  ghudson
 * Initial revision
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
