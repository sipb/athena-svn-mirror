/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZGetSubscriptions function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZGetSubs.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZGetSubs.c,v 1.5 1997-09-14 21:52:38 ghudson Exp $ */

#ifndef lint
static char rcsid_ZGetSubscriptions_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZGetSubs.c,v 1.5 1997-09-14 21:52:38 ghudson Exp $";
#endif

#include <internal.h>

#define min(a,b) ((a)<(b)?(a):(b))
	
Code_t ZGetSubscriptions(subscription, numsubs)
    ZSubscription_t *subscription;
    int *numsubs;
{
    int i;
	
    if (!__subscriptions_list)
	return (ZERR_NOSUBSCRIPTIONS);

    if (__subscriptions_next == __subscriptions_num)
	return (ZERR_NOMORESUBSCRIPTIONS);
	
    for (i=0;i<min(*numsubs, __subscriptions_num-__subscriptions_next);i++)
	subscription[i] = __subscriptions_list[i+__subscriptions_next];

    if (__subscriptions_num-__subscriptions_next < *numsubs)
	*numsubs = __subscriptions_num-__subscriptions_next;

    __subscriptions_next += *numsubs;
	
    return (ZERR_NONE);
}
