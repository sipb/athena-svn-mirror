/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedure declarations for OLC.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/procs.h,v $
 *	$Id: procs.h,v 1.9 1991-02-25 16:34:27 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

/* Declarations of common functions. */

#include <olc/lang.h>

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

/* acl.c */
ERRCODE OSetAcl P((REQUEST *Request , char *acl ));
int OListAcl P((REQUEST *Request , char *acl , char *file ));
int OGetAccesses P((REQUEST *Request , char *file ));

/* ask.c */
ERRCODE OAsk P((REQUEST *Request , char *topic , char *file ));

/* connect.c */
ERRCODE OGrab P((REQUEST *Request ));
ERRCODE OForward P((REQUEST *Request ));

/* consult.c */
ERRCODE OSignOn P((REQUEST *Request ));
ERRCODE OSignOff P((REQUEST *Request ));

/* data.c */

/* db.c */
int OLoadUser P((REQUEST *Request ));
int OGetDBInfo P((REQUEST *Request , DBINFO *dbinfo ));
int OSetDBInfo P((REQUEST *Request , DBINFO *dbinfo ));

/* describe.c */
ERRCODE ODescribe P((REQUEST *Request , LIST *list , char *file , char *note ));

/* generic.c */
ERRCODE ORequest P((REQUEST *Request , int code ));

/* info.c */
ERRCODE OGetNewTopic P((REQUEST *Request , char *topic ));
ERRCODE OGetUsername P((REQUEST *Request , char *name ));
ERRCODE OGetConnectedUsername P((REQUEST *Request , char *name ));
ERRCODE OGetHostname P((REQUEST *Request , char *name ));
ERRCODE OGetConnectedHostname P((REQUEST *Request , char *name ));

/* init.c */
ERRCODE OInitialize P((void ));

/* instance.c */
ERRCODE OVerifyInstance P((REQUEST *Request , int *instance ));
int OGetDefaultInstance P((REQUEST *Request , int *instance ));

/* io.c */
ERRCODE send_request P((int fd , REQUEST *request ));
ERRCODE read_list P((int fd , LIST *list ));
ERRCODE open_connection_to_daemon P((REQUEST *request , int *fd ));

/* list.c */
ERRCODE OListQueue P((REQUEST *Request , LIST **list , char *queues , char *topics , char *users , int stati ));
int OReadList P((int fd , LIST **list , int size ));

/* messages.c */
ERRCODE OReplayLog P((REQUEST *Request , char *file ));
ERRCODE OShowMessageIntoFile P((REQUEST *Request , char *file ));
int OShowMessage P((REQUEST *Request , char **buf ));
ERRCODE OGetMessage P((REQUEST *Request , char *file , char **buf , int code ));
int OWaitForMessage P((REQUEST *Request , char *file , char *sender ));

/* misc.c */
int ODump P((REQUEST *Request , int type , char *file ));

/* motd.c */
ERRCODE OGetFile P((REQUEST *Request , int type , char *file ));
ERRCODE OChangeFile P((REQUEST *Request , int type , char *file ));

/* olc.c */
int main P((int argc , char **argv ));
int do_olc_init P((void ));

/* olc_stock.c */
ERRCODE do_olc_stock P((char **arguments ));

/* queue.c */
ERRCODE OListQueues P((REQUEST *Request , char *file ));
ERRCODE OChangeQueue P((REQUEST *Request , char *queue ));

/* resolve.c */
ERRCODE ODone P((REQUEST *Request , char *title ));
ERRCODE OCancel P((REQUEST *Request , char *title ));
ERRCODE OResolve P((REQUEST *Request , char *title , int flag ));

/* send.c */
ERRCODE OComment P((REQUEST *Request , char *file ));
ERRCODE OReply P((REQUEST *Request , char *file ));
ERRCODE OMail P((REQUEST *Request , char *file ));
ERRCODE OSend P((REQUEST *Request , int type , char *file ));
ERRCODE OMailHeader P((REQUEST *Request , char *file , char *username , char *realname , char *topic , char *destination , char *message ));

/* status.c */
int OListPerson P((REQUEST *Request , LIST **data ));
int OWho P((REQUEST *Request , LIST *data ));
int OGetStatusString P((int status , char *string ));
int OGetStatusCode P((char *string , int *status ));
int OGetUsername P((REQUEST *Request , char *username ));
int OGetHostname P((REQUEST *Request , char *hostname ));
int OGetConnectedUsername P((REQUEST *Request , char *username ));
int OGetConnectedHostname P((REQUEST *Request , char *hostname ));
int OVersion P((REQUEST *Request, char **vstring));

/* topic.c */
ERRCODE OGetTopic P((REQUEST *Request , char *topic ));
ERRCODE OChangeTopic P((REQUEST *Request , char *topic ));
ERRCODE OListTopics P((REQUEST *Request , char *file ));
ERRCODE OVerifyTopic P((REQUEST *Request , char *topic ));
ERRCODE OHelpTopic P((REQUEST *Request , char *topic , char *buf ));

/* utils.c */
int OFillRequest P((REQUEST *req ));
int fill_request P((REQUEST *req ));
int open_connection_to_mailhost P((void ));
int query_mailhost P((int s , char *name ));
int can_receive_mail P((char *name ));
ERRCODE call_program P((char *program , char *argument ));
void expand_hostname P((char *hostname , char *instance , char *realm ));
int sendmail P((char **smargs ));
int file_length P((char *file ));
char *zephyr_get_opcode P((char *class , char *instance ));
int zephyr_subscribe P((char *class , char *instance , char *recipient ));

/* System: */

#undef P
