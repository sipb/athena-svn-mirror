#ifndef lint
static char *RCSid = "$Id: strdup.c,v 1.1.1.2 1998-02-12 21:32:11 ghudson Exp $";
#endif


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
