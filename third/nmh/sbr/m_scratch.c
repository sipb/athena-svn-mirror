
/*
 * m_scratch.c -- construct a scratch file
 *
 * $Id: m_scratch.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


char *
m_scratch (char *file, char *template)
{
    char *cp;
    static char buffer[BUFSIZ], tmpfil[BUFSIZ];

    snprintf (tmpfil, sizeof(tmpfil), "%sXXXXXX", template);
    mktemp (tmpfil);
    if ((cp = r1bindex (file, '/')) == file)
	strncpy (buffer, tmpfil, sizeof(buffer));
    else
	snprintf (buffer, sizeof(buffer), "%.*s%s", cp - file, file, tmpfil);
    unlink (buffer);

    return buffer;
}