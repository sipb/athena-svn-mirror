/* trimcpy.c - strip [lt]wsp and replace newlines with spaces */
#ifndef       lint
static char ident[] = "@(#)$Id: trimcpy.c,v 1.1.1.1 1996-10-07 07:13:54 ghudson Exp $";
#endif	/*        lint */


#include "../h/mh.h"
#include <ctype.h>
#include <stdio.h>


char *trimcpy (cp)
register char *cp;
{
    register char  *sp;

    while (isspace (*cp))
	cp++;
    for (sp = cp + strlen (cp) - 1; sp >= cp; sp--)
	if (isspace (*sp))
	    *sp = 0;
	else
	    break;
    for (sp = cp; *sp; sp++)
	if (isspace (*sp))
	    *sp = ' ';

    return getcpy (cp);
}
