/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for ZPeekNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v 1.5 1987-08-01 15:29:47 rfrench Exp $ */

#ifndef lint
static char rcsid_ZPeekNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v 1.5 1987-08-01 15:29:47 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZPeekNotice(buffer,buffer_len,notice,from)
	ZPacket_t	buffer;
	int		buffer_len;
	ZNotice_t	*notice;
	struct		sockaddr_in *from;
{
	int len;
	Code_t retval;

	if ((retval = ZPeekPacket(buffer,buffer_len,&len,from)) !=
	    ZERR_NONE)
		return (retval);

	return (ZParseNotice(buffer,len,notice));
}
