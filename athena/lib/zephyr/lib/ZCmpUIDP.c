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
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v 1.1 1987-06-29 02:28:51 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

int ZCompareUIDPred(notice,uid)
	ZNotice_t	*notice;
	ZUnique_Id_t	*uid;
{
	return (ZCompareUID(&notice->z_uid,uid));
}
