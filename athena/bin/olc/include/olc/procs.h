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
 *	$Id: procs.h,v 1.22 1999-06-28 22:52:34 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __olc_procs_h
#define __olc_procs_h

/* Declarations of common functions. */

#include <olc/lang.h>

/* acl.c */
ERRCODE OSetAcl (REQUEST *Request , char *acl );
ERRCODE OListAcl (REQUEST *Request , char *acl , char *file );
ERRCODE OGetAccesses (REQUEST *Request , char *file );

/* admin.c */
ERRCODE OTweakZephyr (REQUEST *Request , int what, int how_long);

/* ask.c */
ERRCODE OAsk_buffer (REQUEST *Request , char *topic , char *buf );
ERRCODE OAsk_file (REQUEST *Request , char *topic , char *file );

/* connect.c */
ERRCODE OGrab (REQUEST *Request );
ERRCODE OForward (REQUEST *Request );

/* consult.c */
ERRCODE OSignOn (REQUEST *Request );
ERRCODE OSignOff (REQUEST *Request );

/* data.c */

/* db.c */
ERRCODE OLoadUser (REQUEST *Request );
ERRCODE OGetDBInfo (REQUEST *Request , DBINFO *dbinfo );
ERRCODE OSetDBInfo (REQUEST *Request , DBINFO *dbinfo );

/* describe.c */
ERRCODE ODescribe (REQUEST *Request , LIST *list , char *file , char *note );

/* generic.c */
ERRCODE ORequest (REQUEST *Request , int code);

/* info.c */
ERRCODE OGetNewTopic (REQUEST *Request , char *topic );
ERRCODE OGetUsername (REQUEST *Request , char *name );
ERRCODE OGetConnectedUsername (REQUEST *Request , char *name );
ERRCODE OGetHostname (REQUEST *Request , char *name );
ERRCODE OGetConnectedHostname (REQUEST *Request , char *name );

/* init.c */
ERRCODE OInitialize (void );

/* instance.c */
ERRCODE OVerifyInstance (REQUEST *Request , int *instance );
ERRCODE OGetDefaultInstance (REQUEST *Request , int *instance );

/* io.c */
ERRCODE send_request (int fd , REQUEST *request );
ERRCODE read_list (int fd , LIST *list );
ERRCODE open_connection_to_daemon (REQUEST *request , int *fd );
ERRCODE open_connection_to_named_daemon (REQUEST *request , int *fd,
					   char *hostname );
ERRCODE open_connection_to_nl_daemon (int *fd);

/* list.c */
ERRCODE OListQueue (REQUEST *Request , LIST **list , char *queues ,
		    char *topics , char *users , int stati );
ERRCODE OReadList (int fd , LIST **list , int size );

/* messages.c */
ERRCODE OReplayLog (REQUEST *Request , char *file );
ERRCODE OShowMessageIntoFile (REQUEST *Request , char *file );
ERRCODE OShowMessage (REQUEST *Request , char **buf );
ERRCODE OGetMessage (REQUEST *Request , char *file , char **buf , int code );

/* motd.c */
ERRCODE OGetFile (REQUEST *Request , int type , char *file );
ERRCODE OChangeFile (REQUEST *Request , int type , char *file );

/* nl.c */
ERRCODE nl_get_qlist (int fd, char **buf, int *buflen, int *outlen);
ERRCODE nl_get_log (int fd, char **buf, int *buflen, char *username,
		    int instance, int *outlen);
ERRCODE nl_get_nm (int fd, char **buf, int *buflen, char *username,
		   int instance, int nuke, int *outlen);

/* olc.c */
int main (int argc , char **argv );
ERRCODE do_olc_init (void );

/* olc_stock.c */
ERRCODE do_olc_stock (char **arguments );

/* queue.c */
ERRCODE OListQueues (REQUEST *Request , char *file );
ERRCODE OChangeQueue (REQUEST *Request , char *queue );

/* resolve.c */
ERRCODE ODone (REQUEST *Request , char *title );
ERRCODE OCancel (REQUEST *Request , char *title );
ERRCODE OResolve (REQUEST *Request , char *title , int flag );

/* send.c */
ERRCODE OComment (REQUEST *Request , char *file );
ERRCODE OReply (REQUEST *Request , char *file );
ERRCODE OMail (REQUEST *Request , char *file );
ERRCODE OSend (REQUEST *Request , int type , char *file );
ERRCODE OMailHeader (REQUEST *Request , char *file , char *username ,
		     char *realname , char *topic , char *destination ,
		     char *message );

/* status.c */
ERRCODE OListPerson (REQUEST *Request , LIST **data );
ERRCODE OWho (REQUEST *Request , LIST *data );
ERRCODE OGetUsername (REQUEST *Request , char *username );
ERRCODE OGetHostname (REQUEST *Request , char *hostname );
ERRCODE OGetConnectedUsername (REQUEST *Request , char *username );
ERRCODE OGetConnectedHostname (REQUEST *Request , char *hostname );
ERRCODE OVersion (REQUEST *Request, char **vstring);

/* topic.c */
ERRCODE OGetTopic (REQUEST *Request , char *topic );
ERRCODE OChangeTopic (REQUEST *Request , char *topic );
ERRCODE OListTopics (REQUEST *Request , char *file );
ERRCODE OVerifyTopic (REQUEST *Request , char *topic );
ERRCODE OHelpTopic (REQUEST *Request , char *topic , char *buf );

/* utils.c */
ERRCODE OFillRequest (REQUEST *req );
ERRCODE fill_request (REQUEST *req );
ERRCODE open_connection_to_mailhost (void );
ERRCODE query_mailhost (int s , char *name );
ERRCODE can_receive_mail (char *name );
ERRCODE call_program (char *program , char *argument );
void expand_hostname (char *hostname , char *instance , char *realm );
int sendmail (char **smargs );
int file_length (char *file );

/* System: */
#endif /* __olc_procs_h */
