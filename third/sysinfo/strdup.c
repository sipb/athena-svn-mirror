#ifndef lint
static char *RCSid = "$Id: strdup.c,v 1.1.1.1 1996-10-07 20:16:52 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1994/11/03  00:04:48  mcooper
 * Update RCS header
 *
 * Revision 1.2  1994/10/11  02:04:56  mcooper
 * Revamp to read definetion info from externel file
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
