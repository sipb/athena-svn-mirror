
/*
 * print_version.c -- print a version string
 *
 * $Id: print_version.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


void
print_version (char *invo_name)
{
    printf("%s -- %s\n", invo_name, version_str);
}
