
/*
 * m_name.c -- return a message number as a string
 *
 * $Id: m_name.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>

static char name[BUFSIZ];


char *
m_name (int num)
{
    if (num <= 0)
	return "?";

    snprintf (name, sizeof(name), "%d", num);
    return name;
}
