/* strindex.c - "unsigned" lexical index */
#ifndef	lint
static char ident[] = "@(#)$Id: strindex.c,v 1.1.1.1 1996-10-07 07:13:54 ghudson Exp $";
#endif	/* lint */


int  stringdex (p1, p2)
register char  *p1,
               *p2;
{
    register char  *p;

    if (p1 == 0 || p2 == 0) return(-1);		/* XXX */

    for (p = p2; *p; p++)
	if (uprf (p, p1))
	    return (p - p2);

    return (-1);
}
