/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUIDPred function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v $
 *	$Author: jtkohl $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v 1.4 1988-06-17 17:19:49 jtkohl Exp $ */

#ifndef lint
static char rcsid_ZCompareUIDPred_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZCmpUIDP.c,v 1.4 1988-06-17 17:19:49 jtkohl Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

int ZCompareUIDPred(notice, uid)
    ZNotice_t *notice;
    ZUnique_Id_t *uid;
{
    return (ZCompareUID(&notice->z_uid, uid));
}

int ZCompareMultiUIDPred(notice, uid)
    ZNotice_t *notice;
    ZUnique_Id_t *uid;
{
    return (ZCompareUID(&notice->z_multiuid, uid));
}
