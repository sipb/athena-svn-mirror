/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/nl_requests.h,v 1.1 1991-03-11 13:45:39 lwvanels Exp $
 */

#ifdef KERBEROS
#define K_SERVICE "olc"
#endif

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
