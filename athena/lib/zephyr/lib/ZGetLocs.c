/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZGetLocations function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZGetLocs.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZGetLocs.c,v 1.1 1987-06-20 19:21:17 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

#define min(a,b) ((a)<(b)?(a):(b))
	
Code_t ZGetLocations(location,numlocs)
	char *location[];
	int *numlocs;
{
	int i;
	
	if (!__locate_list)
		return (ZERR_NOLOCATIONS);

	if (__locate_next == __locate_num)
		return (ZERR_NOMORELOCS);
	
	for (i=0;i<min(*numlocs,__locate_num-__locate_next);i++)
		location[i] = __locate_list[i+__locate_next];

	if (__locate_num-__locate_next < *numlocs)
		*numlocs = __locate_num-__locate_next;

	__locate_next += *numlocs;
	
	return (ZERR_NONE);
}
