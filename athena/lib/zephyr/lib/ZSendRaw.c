/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSendRawNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendRaw.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendRaw.c,v 1.1 1987-06-20 19:22:02 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZSendRawNotice(notice)
	ZNotice_t	*notice;
{
	Code_t retval;
	char *buffer;
	int len;

	buffer = (char *)malloc(Z_MAXPKTLEN);
	if (!buffer)
		return (ENOMEM);

	if ((retval = ZFormatRawNotice(notice,buffer,Z_MAXPKTLEN,&len)) !=
	    ZERR_NONE) {
		free(buffer);
		return (retval);
	}

	retval = ZSendPacket(buffer,len);
	free(buffer);

	return (retval);
}
