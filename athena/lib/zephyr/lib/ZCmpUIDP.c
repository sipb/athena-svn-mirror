/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUIDPred function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v 1.2 1987-07-29 15:15:27 rfrench Exp $ */

#ifndef lint
static char rcsid_ZCompareUIDPred_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v 1.2 1987-07-29 15:15:27 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

int ZCompareUIDPred(notice,uid)
	ZNotice_t	*notice;
	ZUnique_Id_t	*uid;
{
	return (ZCompareUID(&notice->z_uid,uid));
}
