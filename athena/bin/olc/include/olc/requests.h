/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions concerning requests to the daemon.
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
 *	$Id: requests.h,v 1.24 1999-03-06 16:48:34 ghudson Exp $
 */

#ifndef OLC__OLC_REQUESTS_H
#define OLC__OLC_REQUESTS_H

#include <mit-copyright.h>
#include <olc/structs.h>

#ifdef HAVE_KRB4
#include <krb.h>
#else /* don't HAVE_KRB4 */
/* Need this just for the structure size; allows some interopability between
   kerberized & non-kerberized clients/servers */
/* From krb.h */

#define         MAX_KTXT_LEN    1250
struct ktext {
    int     length;             /* Length of the text */
    unsigned char dat[MAX_KTXT_LEN];    /* The data itself */
    unsigned long mbz;          /* zero to catch runaway strings */
};

typedef struct ktext *KTEXT;
typedef struct ktext KTEXT_ST;
#endif /* don't HAVE_KRB4 */

/* request structure */

typedef struct tREQUEST 
{
  int	    version;             /* version of server client is expecting */
  int	    request_type;	 /* Type of request. */
  int	    options;		 /* Optional flags. */
  int       code;
  PERSON    requester; 		 /* Who sends the request. */
  PERSON    target;              /* Recipient of action */

/* Always leave space for ticket data; may just be ignored if not using */
/* kerberos- allows limited interoperability between auth & unauth */ 

  KTEXT_ST  kticket;             /* Kerberos authentication ticket */

} REQUEST;

/* IO_REQUEST is the structure to be sent over the net. In the future ther
 * may be several items convenient to contain in the REQUEST during
 * routing through the server but not necessary to send over.
 */

typedef struct tIO_REQUEST {
  char data[440];
/* The data is in the following format:

offset(dec)	name		type		meaning
----------------------------------------------------------------------------
0		version		net long	version
4		request_type	net long	type of request
8		options		net long	request options
12		code		net long	additional options
--- requester data ---
16		r_uid		net long	uid
20		r_instance	net long	instance
24		r_username	char[10]	username
34		r_realname	char[32]	"real" name
66		r_realm		char[40]	kerberos realm
106		r_inst		char[40]	kerberos instance
146		r_nickname	char[16]	gecos nickname
162		r_title		char[32]	title in OLC
194		r_machine	char[32]	machine name
226		r_pad		char[2]		padding (historical)
--- target data ---
228		t_uid		net long	uid
232		t_instance	net long	instance
236		t_username	char[10]	username
246		t_realname	char[32]	"real" name
278		t_realm		char[40]	kerberos realm
318		t_inst		char[40]	kerberos instance
358		t_nickname	char[16]	gecos nickname
374		t_title		char[32]	title in OLC
406		t_machine	char[32]	machine name
438		t_pad		char[2]		padding (historical)
*/
} IO_REQUEST;


typedef struct tIO_LIST
{
  char data[548];
/* The data is in the following format:
offset(dec)	name		type		meaning
----------------------------------------------------------------------------
0		ustatus		net long	user status
4		cstatus		net long	consultant status
8		ukstatus	net long	user "knuckle" status
12		ckstatus	net long	consultant "knuckle" status
16		utime		net long	time q. asked
20		ctime		net long	---
24		umessage	net long	---
28		cmessage	net long	---
32		nseen		net long	# con. consultants
36		topic		char[24]	topic of question
60		note		note[64]	description of question
--- user data ---
124		u_uid		net long	uid
128		u_instance	net long	instance
132		u_username	char[10]	username
142		u_realname	char[32]	"real" name
174		u_realm		char[40]	kerberos realm
214		u_inst		char[40]	kerberos instance
254		u_nickname	char[16]	gecos nickname
270		u_title		char[32]	title in OLC
302		u_machine	char[32]	machine name
334		u_pad		char[2]		padding (historical)
--- consultant data ---
336		c_uid		net long	uid
340		c_instance	net long	instance
344		c_username	char[10]	username
354		c_realname	char[32]	"real" name
386		c_realm		char[40]	kerberos realm
426		c_inst		char[40]	kerberos instance
466		c_nickname	char[16]	gecos nickname
482		c_title		char[32]	title in OLC
514		c_machine	char[32]	machine name
546		c_pad		char[2]		padding (historical)
*/
} IO_LIST;

/* request types */

#define UNKNOWN_REQUEST 200

/* Requests for the OLC processes */
/* it's going to be a fun day */

#define OLC_ON 	             201      /* Sign on */
#define OLC_OFF	             202      /* Sign off */
#define OLC_SEND             203      /* Send text to the user. */
#define OLC_REPLAY           204      /* Replay the conversation. */
#define OLC_LIST             207      /* List current conversations. */
#define OLC_GRAB             208      /* Grab a question. */
#define OLC_MAIL             209      /* Send mail to a user. */
#define OLC_TOPIC            210      /* Query/change topic of a question. */
#define OLC_COMMENT          211      /* Put a comment in the log. */
#define OLC_FORWARD          212      /* Forward a question to someone else. */
#define OLC_DONE             213      /* Done with this question. */
#define OLC_STARTUP          214      /* Start up an olc session. */
#define OLC_SHOW             215      /* Show any new messages. */
#define OLC_CANCEL           217      /* Cancel a question. */
#define OLC_DESCRIBE         218      /* Sing a song */
#define OLC_STATUS           220      /* Print OLC user status information */
#define OLC_LIST_TOPICS      221      /* List OLC topics */
#define OLC_ASK              222      /* ask a question */
#define OLC_MOTD             223      /* display motd */
#define OLC_CHANGE_MOTD      224      /* Change motd */
#define OLC_CHECK_USER       225      /* Check user status */
#define OLC_CHANGE_ACL       226      /* add user to acl */
#define OLC_USER_INFO        227      /* find out about user */
#define OLC_RESTART          229      /* restart server */
#define OLC_FLUSH_NM         230      /* flush new messages */
#define OLC_SET_TOPIC        231      /* change topic */
#define OLC_CREATE_INSTANCE  232      /* create instance */
#define OLC_DUMP             233      /* debug server */
#define OLC_WHO              234      /* find who */
#define OLC_VERIFY_INSTANCE  235      /* verify an instance */
#define OLC_HELP_TOPIC       236
#define OLC_VERIFY_TOPIC     237
#define OLC_CONNECTED        238
#define OLC_LOAD_USER        239
#define OLC_DEFAULT_INSTANCE 240
#define OLC_GET_DBINFO       241
#define OLC_SET_DBINFO       242
#define OLC_LIST_ACL         243
#define OLC_GET_ACCESSES     244
#define OLC_CHANGE_TOPICS    245

#define OLC_SET_USER_STATUS  246
#define OLC_GET_HOURS	     247
#define OLC_CHANGE_HOURS     248

#define OLC_UNUSED_1	     250      /* used to be dump_req_stats, now */
				      /* deprecated */
#define OLC_UNUSED_2	     251      /* used to be dump_ques_stats, now */
				      /* depricated */

#define OLC_VERSION	     252
#define OLC_TOGGLE_ZEPHYR    253
#define OLC_LIST_SERVICES    254

#define OLC_NUM_REQUESTS     53      /* number of possible requests */

/* Return values from daemon requests and functions. */

#define PERMISSION_DENIED	100
#define USER_EXISTS		101
#define USER_NOT_FOUND		102
#define ALREADY_SIGNED_ON	103
#define NOT_SIGNED_ON		104
#define RESTARTED		105
#define CONNECTED		106
#define NOT_CONNECTED		108
#define	SIGNED_OFF		109
#define	LISTING			110
#define KRB_FALIURE             111
#define ALREADY_HAVE_QUESTION   112
#define INVALID_TOPIC           113
#define INSTANCE_NOT_FOUND      114
#define EMPTY_LIST              115
#define ALREADY_CONNECTED       116
#define UNCHANGED               117
#define TARGET_NOT_FOUND        118
#define REQUESTER_NOT_FOUND     119
#define UNKNOWN_TOPIC           120
#define GRAB_ME                 121
#define HAS_QUESTION            122
#define NO_QUESTION             123
#define NAME_NOT_UNIQUE         124
#define SEND_INFO               125
#define OK                      126
#define MAX_ASK                 127
#define MAX_ANSWER              128
#define UNKNOWN_ACL             129
#define NO_MESSAGES             130
#define ERROR_NAME_RESOLVE      150
#define ERROR_CONNECT           151
#define ERROR_SLOC              152
#define END_OF_LIST             -1

/* Request options. */  /* should be broken up by request */

#define	NO_OPT             0	/* No options here. */
#define	OFF_OPT	           1<<1	/* Several commands -- sign consultant off. */
#define URGENT_OPT         1<<2	/* OLCR_ON -- Urgent questions only. */
#define OLC_DUTY_OPT       1<<3	/* OLCR_ON -- Consultant is on OLC duty. */
#define NEW_TOPIC_OPT      1<<4	/* OLC_SEND -- Parse a new topic. */
#define MAIL_SEND          1<<5	/* OLCR_MAIL -- Send the mail message. */
#define MAIL_COMPOSE       1<<6	
#define UNANS_OPT          1<<7	/* OLCR_FORWARD -- Send to unanswered log. */

#define VERIFY             1<<9
#define FORWARD_UNANSWERED 1<<10
#define ON_FIRST           1<<11
#define ON_SECOND          1<<12
#define ON_DUTY            1<<13
#define ON_URGENT          1<<14
#define OFF_FORCE          1<<15
#define TOPIC_VERIFY       1<<16
#define SPLIT_OPT          1<<17
#define CONNECTED_OPT      1<<18
#define NOFLUSH_OPT        1<<19
#define STATUS_ACTIVE      1<<20
#define STATUS_PENDING     1<<21
#define STATUS_UNSEEN      1<<22
#define STATUS_PICKUP      1<<23
#define STATUS_REFERRED    1<<24

#define PRIV_COMMENT_OPT   1<<24
#define CHANGE_COMMENT_OPT 1<<25
#define CHANGE_NOTE_OPT    1<<26

#define ADD_OPT            1<<20
#define DEL_OPT            1<<21
#define LIST_PERSONAL      1

/* other stuff */
#define NO_INSTANCE     0

/* Version numbers. */

#define	VERSION_1	1	/* 07 Aug 1986 */
#define	VERSION_2	2	/* 07 Jun 1988 */
#define VERSION_3	3	/* 07 Jun 1989 */
#define VERSION_4	4	/* 01 Oct 1989 */
#define VERSION_5	5	/* 16 Jul 1990 */

#define CURRENT_VERSION VERSION_5

#endif /* OLC__OLC_REQUESTS_H */
