/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFlushLocations function.
 *
 *	Created by:	Robert French
 *
 *	$Id: ZFlsLocs.c,v 1.8 1999-01-22 23:19:05 ghudson Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef lint
static char rcsid_ZFlushLocations_c[] = "$Id: ZFlsLocs.c,v 1.8 1999-01-22 23:19:05 ghudson Exp $";
#endif

#include <internal.h>

Code_t ZFlushLocations()
{
	int i;
	
	if (!__locate_list)
		return (ZERR_NONE);

	for (i=0;i<__locate_num;i++) {
		free(__locate_list[i].host);
		free(__locate_list[i].time);
		free(__locate_list[i].tty);
	}
	
	free((char *)__locate_list);

	__locate_list = 0;
	__locate_num = 0;

	return (ZERR_NONE);
}
