/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUID function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v 1.5 1988-05-17 21:21:16 rfrench Exp $ */

#ifndef lint
static char rcsid_ZCompareUID_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v 1.5 1988-05-17 21:21:16 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

int ZCompareUID(uid1, uid2)
    ZUnique_Id_t *uid1, *uid2;
{
    return (!bcmp((char *)uid1, (char *)uid2, sizeof (*uid1)));
}
