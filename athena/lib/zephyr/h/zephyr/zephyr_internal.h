/* This file is part of the Project Athena Zephyr Notification System.
 * It contains internal definitions for the client library.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/h/zephyr/zephyr_internal.h,v $
 *	$Author: jtkohl $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/h/zephyr/zephyr_internal.h,v 1.7 1987-07-02 10:48:16 jtkohl Exp $ */

#ifndef __ZINTERNAL_H__
#define __ZINTERNAL_H__

#include <zephyr/zephyr.h>
#include <strings.h>			/* for strcpy, etc. */
#include <sys/types.h>			/* for time_t, uid_t, etc */

struct _Z_InputQ {
	struct		_Z_InputQ *next;
	struct		_Z_InputQ *prev;
	int		packet_len;
	struct		sockaddr_in from;
	ZPacket_t	packet;
};

extern struct _Z_InputQ *__Q_Head, *__Q_Tail;
extern int __Q_Length;

extern int __Zephyr_open;
extern int __HM_set;
extern int __Zephyr_server;

extern ZLocations_t *__locate_list;
extern int __locate_num;
extern int __locate_next;

extern int krb_err_base;

extern char *malloc();
extern time_t time();
extern long random();

#endif !__ZINTERNAL_H__
