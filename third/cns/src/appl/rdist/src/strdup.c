#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/third/cns/src/appl/rdist/src/strdup.c,v 1.1.1.1 1996-09-06 00:46:17 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1996/06/24 20:49:33  ghudson
 * Initial revision
 *
 * Revision 1.2  1992/04/16  01:28:02  mcooper
 * Some de-linting.
 *
 * Revision 1.1  1992/03/21  02:48:11  mcooper
 * Initial revision
 *
 */


#include <stdio.h>

/*
 * Most systems don't have this (yet)
 */
char *strdup(str)
     char *str;
{
    char 		       *p;
    extern char		       *malloc();
    extern char		       *strcpy();

    if ((p = malloc(strlen(str)+1)) == NULL)
	return((char *) NULL);

    (void) strcpy(p, str);

    return(p);
}
