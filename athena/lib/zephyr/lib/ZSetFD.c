/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSetFD function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSetFD.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSetFD.c,v 1.6 1991-06-18 13:45:16 jfc Exp $ */

#ifndef lint
static char rcsid_ZSetFD_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSetFD.c,v 1.6 1991-06-18 13:45:16 jfc Exp $";
#endif

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZSetFD(fd)
	int	fd;
{
	(void) ZClosePort();

	__Zephyr_fd = fd;
	__Zephyr_open = 0;
	
	return (ZERR_NONE);
}
