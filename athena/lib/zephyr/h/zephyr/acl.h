/* This file is part of the Project Athena Zephyr Notification System.
 * It contains definitions for the ACL library
 *
 *	Created by:	John T. Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/h/zephyr/acl.h,v $
 *	$Author: jtkohl $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/h/zephyr/acl.h,v 1.1 1987-06-25 08:50:00 jtkohl Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#include <zephyr/mit-copyright.h>

#ifndef	__ACL__
#define	__ACL__
extern int acl_check(), acl_add(), acl_delete(), acl_initialize();
#endif	__ACL__
