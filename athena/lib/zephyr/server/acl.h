/* This file is part of the Project Athena Zephyr Notification System.
 * It contains definitions for the ACL library
 *
 *	Created by:	John T. Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/acl.h,v $
 *	$Author: ghudson $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/acl.h,v 3.1 1997-09-14 21:54:06 ghudson Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#include <zephyr/mit-copyright.h>

#ifndef	__ACL__
#define	__ACL__

int acl_add __P((char *, char *));
int acl_check __P((char *, char *));
int acl_delete __P((char *, char *));
int acl_initialize __P((char *, int));
void acl_cache_reset __P((void));

#endif /* __ACL__ */

