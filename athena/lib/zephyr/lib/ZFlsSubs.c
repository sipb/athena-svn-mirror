/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFlushSubscriptions function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFlsSubs.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFlsSubs.c,v 1.3 1991-06-20 08:49:17 jfc Exp $ */

#ifndef lint
static char rcsid_ZFlushSubscriptions_c[] = "$Id: ZFlsSubs.c,v 1.3 1991-06-20 08:49:17 jfc Exp $";
#endif

#include <zephyr/zephyr_internal.h>

Code_t ZFlushSubscriptions()
{
	register int i;
	
	if (!__subscriptions_list)
		return (ZERR_NONE);

	for (i=0;i<__subscriptions_num;i++) {
		free(__subscriptions_list[i].zsub_class);
		free(__subscriptions_list[i].zsub_classinst);
		free(__subscriptions_list[i].zsub_recipient);
	}
	
	free((char *)__subscriptions_list);

	__subscriptions_list = 0;
	__subscriptions_num = 0;

	return (ZERR_NONE);
}

