/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZParseNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZParseNot.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZParseNot.c,v 1.12 1987-08-01 15:29:31 rfrench Exp $ */

#ifndef lint
static char rcsid_ZParseNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZParseNot.c,v 1.12 1987-08-01 15:29:31 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZParseNotice(buffer,len,notice)
	ZPacket_t	buffer;
	int		len;
	ZNotice_t	*notice;
{
	extern int ZCheckAuthentication();

	return (Z_InternalParseNotice(buffer,len,notice));
}
