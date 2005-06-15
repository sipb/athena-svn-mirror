/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZGetLocations function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZGetLocs.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZGetLocs.c,v 1.1.1.1 2005-06-15 16:40:32 ghudson Exp $ */

#ifndef lint
static char rcsid_ZGetLocations_c[] = "$Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZGetLocs.c,v 1.1.1.1 2005-06-15 16:40:32 ghudson Exp $";
#endif

#include "internal.h"

#define min(a,b) ((a)<(b)?(a):(b))
	
Code_t ZGetLocations(location, numlocs)
    ZLocations_t *location;
    int *numlocs;
{
    int i;
	
    if (!__locate_list)
	return (ZERR_NOLOCATIONS);

    if (__locate_next == __locate_num)
	return (ZERR_NOMORELOCS);
	
    for (i=0;i<min(*numlocs, __locate_num-__locate_next);i++)
	location[i] = __locate_list[i+__locate_next];

    if (__locate_num-__locate_next < *numlocs)
	*numlocs = __locate_num-__locate_next;

    __locate_next += *numlocs;
	
    return (ZERR_NONE);
}
