/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSendList function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendList.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZSendList.c,v 1.1 1987-06-10 12:35:34 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZSendList(notice,list,nitems)
	ZNotice_t	*notice;
	char		*list[];
	int		nitems;
{
	Code_t retval;
	char *buffer;
	int len;

	buffer = (char *)malloc(BUFSIZ);
	if (!buffer)
		return (ZERR_NOMEM);

	if ((retval = ZFormatNoticeList(notice,list,nitems,buffer,
					 BUFSIZ,&len)) < 0) {
		free(buffer);
		return (retval);
	}

	retval = ZSendPacket(buffer,len);
	free(buffer);

	return (retval);
}
