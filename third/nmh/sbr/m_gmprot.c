
/*
 * m_gmprot.c -- return the msg-protect value
 *
 * $Id: m_gmprot.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


int
m_gmprot (void)
{
    register char *cp;

    return atooi ((cp = context_find ("msg-protect")) && *cp ? cp : msgprot);
}
