/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUIDPred function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZCmpUIDP.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZCmpUIDP.c,v 1.1.1.1 2005-06-15 16:40:32 ghudson Exp $ */

#ifndef lint
static char rcsid_ZCompareUIDPred_c[] = "$Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZCmpUIDP.c,v 1.1.1.1 2005-06-15 16:40:32 ghudson Exp $";
#endif

#include "internal.h"

int ZCompareUIDPred(notice, uid)
    ZNotice_t *notice;
    void *uid;
{
    return (ZCompareUID(&notice->z_uid, (ZUnique_Id_t *) uid));
}

int ZCompareMultiUIDPred(notice, uid)
    ZNotice_t *notice;
    void *uid;
{
    return (ZCompareUID(&notice->z_multiuid, (ZUnique_Id_t *) uid));
}
