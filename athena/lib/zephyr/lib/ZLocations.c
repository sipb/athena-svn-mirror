/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSetLocation.c function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZLocations.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZLocations.c,v 1.2 1987-06-23 17:11:56 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZSetLocation()
{
	ZNotice_t notice;

	notice.z_kind = UNACKED;
	notice.z_port = 0;
	notice.z_class = LOGIN_CLASS;
	notice.z_class_inst = (char *)Z_GetSender();
	notice.z_opcode = LOGIN_USER_LOGIN;
	notice.z_sender = 0;
	notice.z_recipient = "";
	notice.z_message_len = 0;

	return (ZSendNotice(&notice,1));
}
