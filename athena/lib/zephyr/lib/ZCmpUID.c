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
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUID.c,v 1.1 1987-06-13 00:55:25 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr.h>

int ZCompareUID(uid1,uid2)
	ZUnique_Id_t *uid1,*uid2;
{
	return (!bcmp(uid1,uid2,sizeof ZUnique_Id_t));
}
