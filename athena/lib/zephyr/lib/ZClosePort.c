/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZClosePort function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZClosePort.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZClosePort.c,v 1.2 1987-06-10 13:27:01 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZClosePort()
{
	if (__Zephyr_fd >= 0 && __Zephyr_open)
		close(__Zephyr_fd);

	__Zephyr_fd = -1;
	__Zephyr_open = 0;
	
	return (ZERR_NONE);
}
