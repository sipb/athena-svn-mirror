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
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v 1.3 1987-06-23 16:58:51 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZPeekNotice(buffer,buffer_len,notice,auth,from)
	ZPacket_t	buffer;
	int		buffer_len;
	ZNotice_t	*notice;
	int		*auth;
	struct		sockaddr_in *from;
{
	int len;
	Code_t retval;

	if ((retval = ZPeekPacket(buffer,buffer_len,&len,from)) !=
	    ZERR_NONE)
		return (retval);

	return (ZParseNotice(buffer,len,notice,auth,from));
}
