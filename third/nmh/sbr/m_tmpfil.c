
/*
 * m_tmpfil.c -- construct a temporary file
 *
 * $Id: m_tmpfil.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


char *
m_tmpfil (char *template)
{
    static char tmpfil[BUFSIZ];

    snprintf (tmpfil, sizeof(tmpfil), "/tmp/%sXXXXXX", template);
    unlink(mktemp(tmpfil));

    return tmpfil;
}
