/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFreeNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFreeNot.c,v $
 *	$Author: jtkohl $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFreeNot.c,v 1.2 1988-06-15 20:46:13 jtkohl Exp $ */

#ifndef lint
static char rcsid_ZFreeNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFreeNot.c,v 1.2 1988-06-15 20:46:13 jtkohl Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZFreeNotice(notice)
    ZNotice_t *notice;
{
    free(notice->z_packet);
}
