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
 *      MIT Project Athena
 *
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/requests.h,v $
 *      $Author: tjcoppet $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/requests.h,v 1.1 1989-07-07 13:22:39 tjcoppet Exp $
 */

/* request structure */

typedef struct tREQUEST 
{
  int	    version;             /* version of server client is expecting */
  int	    request_type;	 /* Type of request. */
  int	    options;		 /* Optional flags. */
  int       code;
  PERSON    requester; 		 /* Who sends the request. */
  PERSON    target;              /* Recipient of action */

#ifdef KERBEROS
  KTEXT_ST  kticket;             /* Kerberos authentication ticket */
#endif KERBEROS
} REQUEST;

/* IO_REQUEST is the structure to be sent over the net. In the future ther
 * may be several items convenient to contain in the REQUEST during
 * routing through the server but not necessary to send over.
 */

typedef struct tIO_REQUEST 
{
  int	    version;             /* version of server client is expecting */
  int	    request_type;	 /* Type of request. */
  int	    options;		 /* Optional flags. */
  int       code;
  PERSON    requester; 		 /* Who sends the request. */
  PERSON    target;              /* Recipient of action */
} IO_REQUEST;

/* request types */

#define UNKNOWN_REQUEST 200

/* Requests for the OLC processes */
/* it's going to be a fun day */

#define OLC_ON 	            201	     /* Sign on */
#define OLC_OFF	            202	     /* Sign off */
#define OLC_SEND            203	     /* Send text to the user. */
#define OLC_REPLAY          204	     /* Replay the conversation. */
#define OLC_LIST            207	     /* List current conversations. */
#define OLC_GRAB            208	     /* Grab a question. */
#define OLC_MAIL            209	     /* Send mail to a user. */
#define OLC_TOPIC           210	     /* Query or change topic of a question. */
#define OLC_COMMENT         211	     /* Put a comment in the log. */
#define OLC_FORWARD         212	     /* Forward a question to someone else. */
#define OLC_DONE            213	     /* Done with this question. */
#define OLC_STARTUP         214	     /* Start up an olc session. */
#define OLC_SHOW            215	     /* Show any new messages. */
#define OLC_CANCEL          217	     /* Cancel a question. */
#define OLC_ACCESS          218      /* Allow access. NOT IMPLEMENTED. */
#define OLC_NOACCESS        219	     /* Disallow access. NOT IMPLEMENTED. */
#define OLC_STATUS          220	     /* Print OLC user status information */
#define OLC_LIST_TOPICS     221      /* List OLC topics */
#define OLC_ASK             222      /* ask a question */
#define OLC_MOTD            223      /* display motd */
#define OLC_CHMOTD          224      /* Change motd */
#define OLC_CHECK_USER      225      /* Check user status */
#define OLC_ADD_PRINCIPAL   225      /* add user to acl */
#define OLC_DEL_PRINCIPAL   226      /* delete user from acl */
#define OLC_USER_INFO       227      /* find out about user */
#define OLC_RESTART         229      /* restart server */
#define OLC_FLUSH_NM        230      /* flush new messages */
#define OLC_CHTOPIC         231      /* change topic */
#define OLC_CREATE_INSTANCE 232      /* create instance */
#define OLC_DUMP            233      /* debug server */
#define OLC_WHO             234      /* find who */
#define OLC_VERIFY_INSTANCE 235      /* verify an instance */
#define OLC_HELP_TOPIC      236
#define OLC_VERIFY_TOPIC    237
#define OLC_CONNECTED       238

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
#define END_OF_LIST             -1

/* Request options. */  /* should be broken up by request */

#define	NO_OPT             0	/* No options here. */
#define	OFF_OPT	           1	/* Several commands -- sign consultant off. */
#define URGENT_OPT         2	/* OLCR_ON -- Urgent questions only. */
#define OLC_DUTY_OPT       4	/* OLCR_ON -- Consultant is on OLC duty. */
#define NEW_TOPIC_OPT      8	/* OLC_SEND -- Parse a new topic. */
#define MAIL_SEND          16	/* OLCR_MAIL -- Send the mail message. */
#define MAIL_COMPOSE       32	
#define UNANS_OPT          64	/* OLCR_FORWARD -- Send to unanswered log. */
#define FORWARD_OFF        128	/* OLCR_OFF et al -- Sign off forcibly. */
#define VERIFY             256
#define FORWARD_UNANSWERED 512
#define ON_FIRST           1024
#define ON_SECOND          2048
#define ON_DUTY            4096
#define ON_URGENT          8192
#define OFF_FORCE          16384
#define TOPIC_VERIFY       32768
#define SPLIT_OPT          65536
#define CONNECTED_OPT      131072
#define NOFLUSH_OPT        262144

#define LIST_PERSONAL      1

/* other stuff */
#define NO_INSTANCE     0

/* Version numbers. */

#define	VERSION_1	1	/* 07 Aug 1986 */
#define	VERSION_2	2	/* 07 Jun 1988 */
#define VERSION_3       3

#define	CURRENT_VERSION	VERSION_3




