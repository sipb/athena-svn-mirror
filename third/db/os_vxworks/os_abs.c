/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char revid[] = "$Id: os_abs.c,v 1.1.1.1 2002-02-11 16:29:42 ghudson Exp $";
#endif /* not lint */

#include "db_int.h"
#include "iosLib.h"

/*
 * __os_abspath --
 *	Return if a path is an absolute path.
 */
int
__os_abspath(path)
	const char *path;
{
	DEV_HDR *dummy;
	char *ptail;

	/*
	 * VxWorks devices can be rooted at any name at all.
	 * Use iosDevFind() to see if name matches any of our devices.
	 */
	if ((dummy = iosDevFind(path, &ptail)) == NULL)
		return (0);
	/*
	 * If the routine used the default device, then we are not
	 * an abs path.
	 */
	if (ptail == path)
		return (0);
	return (1);
}
