/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSendNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendNot.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendNot.c,v 1.2 1987-06-12 16:59:23 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZSendNotice(notice)
	ZNotice_t	*notice;
{
	Code_t retval;
	char *buffer;
	int len;

	buffer = (char *)malloc(BUFSIZ);
	if (!buffer)
		return (ZERR_NOMEM);

	len = BUFSIZ;

	if ((retval = ZFormatNotice(notice,buffer,BUFSIZ,&len)) < 0) {
		free(buffer);
		return (retval);
	}

	retval = ZSendPacket(buffer,len);
	free(buffer);

	return (retval);
}
