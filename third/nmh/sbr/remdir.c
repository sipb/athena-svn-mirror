
/*
 * remdir.c -- remove a directory
 *
 * $Id: remdir.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


int
remdir (char *dir)
{
    context_save();	/* save the context file */
    fflush(stdout);

    if (rmdir(dir) == -1) {
	admonish (dir, "unable to remove directory");
	return 0;
    }
    return 1;
}
