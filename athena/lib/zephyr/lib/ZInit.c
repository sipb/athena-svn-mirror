/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZInitialize function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZInit.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZInit.c,v 1.3 1987-06-13 16:20:24 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZInitialize()
{
	init_ezef_err_tbl();

	return (ZERR_NONE);
}
