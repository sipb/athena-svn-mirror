/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZCompareUIDPred function.
 *
 *	Created by:	Robert French
 *
 *	$Id: ZCmpUIDP.c,v 1.7 1999-01-22 23:19:04 ghudson Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef lint
static char rcsid_ZCompareUIDPred_c[] = "$Id: ZCmpUIDP.c,v 1.7 1999-01-22 23:19:04 ghudson Exp $";
#endif

#include <internal.h>

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
