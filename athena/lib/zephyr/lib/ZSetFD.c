/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSetFD function.
 *
 *	Created by:	Robert French
 *
 *	$Id: ZSetFD.c,v 1.8 1999-01-22 23:19:28 ghudson Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef lint
static char rcsid_ZSetFD_c[] = "$Id: ZSetFD.c,v 1.8 1999-01-22 23:19:28 ghudson Exp $";
#endif

#include <internal.h>

Code_t ZSetFD(fd)
	int	fd;
{
	(void) ZClosePort();

	__Zephyr_fd = fd;
	__Zephyr_open = 0;
	
	return (ZERR_NONE);
}
