/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZLocateUser function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZLocateU.c,v $
 *	$Author: raeburn $
 *
 *	Copyright (c) 1987,1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZLocateU.c,v 1.21 1990-05-15 08:24:50 raeburn Exp $ */

#ifndef lint
static char rcsid_ZLocateUser_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZLocateU.c,v 1.21 1990-05-15 08:24:50 raeburn Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZLocateUser(user, nlocs)
    char *user;
    int *nlocs;
{
   return(ZNewLocateUser(user,nlocs,ZAUTH));
}
