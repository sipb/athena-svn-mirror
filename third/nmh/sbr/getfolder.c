
/*
 * getfolder.c -- get the current or default folder
 *
 * $Id: getfolder.c,v 1.1.1.1 1999-02-07 18:14:08 danw Exp $
 */

#include <h/mh.h>


char *
getfolder(int wantcurrent)
{
    register char *folder;

    /*
     * If wantcurrent == 1, then try the current folder first
     */
    if (wantcurrent && (folder = context_find (pfolder)) && *folder != '\0')
	return folder;

    /*
     * Else try the Inbox profile entry
     */
    if ((folder = context_find (inbox)) && *folder != '\0')
	return folder;

    /*
     * Else return compile time default.
     */
    return defaultfolder;
}
