/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUID function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v $
 *	$Author: probe $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v 1.7 1993-09-24 16:19:19 probe Exp $ */

#ifndef lint
static char rcsid_ZCompareUID_c[] = "$Id: ZCmpUID.c,v 1.7 1993-09-24 16:19:19 probe Exp $";
#endif

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

int ZCompareUID(uid1, uid2)
    ZUnique_Id_t *uid1, *uid2;
{
    return (!_BCMP((char *)uid1, (char *)uid2, sizeof (*uid1)));
}
