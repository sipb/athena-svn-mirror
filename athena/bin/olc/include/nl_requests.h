/*
 * $Id: nl_requests.h,v 1.4 1999-01-22 23:13:26 ghudson Exp $
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>

/* Current version number */
#define VERSION 1

/* request defintions */
#define LIST_REQ		0
#define SHOW_NO_KILL_REQ	1
#define SHOW_KILL_REQ		2
#define REPLAY_KILL_REQ		3

/* Error definitions */

#define ERR_NO_SUCH_Q	-11
#define ERR_SERV	-12
#define ERR_NO_ACL	-13
#define ERR_OTHER_SHOW	-14
#define ERR_NOT_HERE	-15
#define ERR_MEM		-16

/* magic name & instance to get queue listing */
#define LIST_NAME	"qlist"
#define LIST_INSTANCE	-1
