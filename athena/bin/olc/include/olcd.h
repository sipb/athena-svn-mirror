/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC daemon.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olcd.h,v $
 *	$Id: olcd.h,v 1.19 1990-07-16 10:07:02 vanharen Exp $
 *	$Author: vanharen $
 */

#include <mit-copyright.h>

#ifndef __olcd_h
#define __olcd_h __FILE__

#include <olc/lang.h>

/* Important files. */

extern char *DONE_DIR;
extern char *LOG_DIR;
extern char *BACKUP_FILE;
extern char *BACKUP_TEMP;
extern char *ERROR_LOG;
extern char *REQ_STATS_LOG;
extern char *QUES_STATS_LOG;
extern char *STATUS_LOG;
extern char *ADMIN_LOG;
extern char *STDERR_LOG;
extern char *DATABASE_FILE;
extern char *TOPIC_FILE;
extern char *MOTD_FILE;
extern char *SPECIALTY_DIR;
extern char *ACL_DIR;

#ifdef KERBEROS
extern char K_INSTANCEbuf[];
extern char *SRVTAB_FILE;
#endif KERBEROS

/* system defines */

#define NOW                    (time((time_t *)NULL))
#define DAEMON_TIME_OUT        10
#define MAX_CACHE_SIZE         500

#ifdef SYSLOG
#define SYSLOG_LEVEL LOG_LOCAL6
#endif

/* for notifications */

#define OLCD_TIMEOUT    10

/* random internal flags */

#define NULL_FLAG       0
#define FORWARD         1	
#define NOTIFY          2	
#define UNANSWERED      3       /* Question was never answered. */
#define ANSWERED        4       /* Question was answered. */
#define PING            5
#define NO_RESPOND      6

/* Additional size constants. */

#define DB_LINE         1000    /* maximum length of a database line.*/
#define MAX_SEEN        81      /* Maximum number of consultants to */
#define SPEC_SIZE       10                     

/* priority queues */

#define EMERGENCY_Q       1     /* she's having a baby */
#define ACTIVE_Q          2     /* alive and well */
#define INACTIVE_Q        4     /* runaway */
#define PIT_Q             8     /* the ish list */

/* type classifiers */

#define IS_TOPIC               500
#define IS_SUBTOPIC            501


#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "consultant"

/* OLCD data definitions */

typedef struct tUSER 
{
  struct tKNUCKLE **knuckles;       /* all user instances */
  int    uid;                       /* user id */
  char   username[LOGIN_SIZE];      /* user name */
  char   realname[NAME_SIZE];
  char   nickname[NAME_SIZE];         
  char   title1[NAME_SIZE];        /* title of user in OLC */
  char   title2[NAME_SIZE];        /* title of consultant in OLC */
  char   machine[NAME_SIZE];      /* user location */
  char   realm[NAME_SIZE];
  int    specialties[SPEC_SIZE];    /* Specialty list. */
  int    no_specialties;
  int    permissions;
  int    status;                    /* status of the user 
                                        (logout, idle, etc) */
  int    no_knuckles;               /* number of current connections */
  int    max_ask;                   /* maximum allowable connections */
  int    max_answer;
} USER;


typedef struct tKNUCKLE
{
  struct tQUESTION *question;        /* question */
  struct tKNUCKLE *connected;        /* connected user */
  struct tUSER *user;                /* central user */
  char   title[NAME_SIZE];
  int    instance;                   
  int    priority;
  int    queue;
  long   timestamp;                  /* specific to type */
  int    status;                     /* status of this instance 
                                        (on priorities, pending, etc..) */
  char   cusername[LOGIN_SIZE];
  int    cinstance;
  char   *new_messages;              /* new messages from this connection */
} KNUCKLE;

typedef struct tQUESTION
{
  struct tKNUCKLE *owner;
  char  logfile[NAME_SIZE];          /* Name of the logfile. */
  long  logfile_timestamp;           /* timestamp used for logfile caching */
  int   seen[MAX_SEEN];              /* UIDs of users who have seen 
                                        this question */
  int   nseen;                       /* Number who have seen it. */
  char  topic[TOPIC_SIZE];           /* topic of this question. */
  int   topic_code;                  /* number version of the above */
  char  title[NAME_SIZE];            /* Title for log. */
  char  note[NOTE_SIZE];
  char  comment[COMMENT_SIZE];
} QUESTION;

typedef struct tTOPIC
{
  char acl[NAME_SIZE];
  char name[NAME_SIZE];
  int  value;
  struct tTOPIC **subtopic;
} TOPIC;

typedef struct tOLC_PROC  
{
  int   proc_code;      /* Request code. */
  FUNCTION olc_proc;    /* Procedure to execute. */
  char  *description;   /* What it does. */
} PROC;

/* OLC status structure. */

typedef struct tQUEUE_STATUS	
{
  int   consultants;    /* Number of visible consultants. */
  int   invisible;      /* Number of invisible consultants; */
  int   busy;           /* Number of busy consultants. */
  int   waiting;        /* Number of waiting users. */
} QUEUE_STATUS;


typedef struct t_ACL
{
  int code;
  char *file;
  char *name;
} ACL;

/* Declarations of functions. */


/* OLC procedure declarations. */

#ifdef __STDC__
extern ERRCODE olc_topic(int fd, REQUEST *request, int auth);             /* Change the current topic. */
extern ERRCODE olc_comment(int fd, REQUEST *request, int auth);           /* Insert a comment in the log. */
extern ERRCODE olc_describe(int fd, REQUEST *request, int auth);          /* Make more comments */
extern ERRCODE olc_done(int fd, REQUEST *request, int auth);              /* Mark a question done. */
extern ERRCODE olc_forward(int fd, REQUEST *request, int auth);           /* Forward a question. */
extern ERRCODE olc_list(int fd, REQUEST *request, int auth);              /* List current conversations. */
extern ERRCODE olc_mail(int fd, REQUEST *request, int auth);              /* Send mail to a user. */
extern ERRCODE olc_on(int fd, REQUEST *request, int auth);                /* Sign on to OLC. */
extern ERRCODE olc_off(int fd, REQUEST *request, int auth);               /* Sign off of OLC. */
extern ERRCODE olc_replay(int fd, REQUEST *request, int auth);            /* Replay the conversation. */
extern ERRCODE olc_send(int fd, REQUEST *request, int auth);              /* Send a message. */
extern ERRCODE olc_who(int fd, REQUEST *request, int auth);               /* Print user's name. */
extern ERRCODE olc_startup(int fd, REQUEST *request, int auth);           /* Start up an OLCR session. */
extern ERRCODE olc_show(int fd, REQUEST *request, int auth);              /* Show any new messages. */
extern ERRCODE olc_grab(int fd, REQUEST *request, int auth);              /* Grab a question on the queue. */
extern ERRCODE olc_cancel(int fd, REQUEST *request, int auth);            /* Cancel a question. */
extern ERRCODE olc_status();            /* Print user status information. */
extern ERRCODE olc_ask(int fd, REQUEST *request, int auth);               /* ask a question */
extern ERRCODE olc_chtopic(int fd, REQUEST *request, int auth);           /* change a topic */
extern ERRCODE olc_list_topics(int fd, REQUEST *request, int auth);       /* list topics */
extern ERRCODE olc_create_instance(int fd, REQUEST *request, int auth);   /* create a new instance */
extern ERRCODE olc_default_instance(int fd, REQUEST *request, int auth);
extern ERRCODE olc_motd(int fd, REQUEST *request, int auth);              /* retrieve the olc motd */
extern ERRCODE olc_dump(int fd, REQUEST *request, int auth);              /* debugging info */
extern ERRCODE olc_dump_req_stats(int fd, REQUEST *request, int auth);    /* "profiling" info */
extern ERRCODE olc_dump_ques_stats(int fd, REQUEST *request, int auth);   /* stats about questions */
extern ERRCODE olc_cancel(int fd, REQUEST *request, int auth);
extern ERRCODE olc_verify_topic(int fd, REQUEST *request, int auth);
extern ERRCODE olc_verify_instance(int fd, REQUEST *request, int auth);
extern ERRCODE olc_load_user(int fd, REQUEST *request, int auth);
extern ERRCODE olc_change_motd(int fd, REQUEST *request, int auth);
extern ERRCODE olc_change_acl(int fd, REQUEST *request, int auth);
extern ERRCODE olc_get_dbinfo(int fd, REQUEST *request, int auth);
extern ERRCODE olc_list_acl(int fd, REQUEST *request, int auth);
extern ERRCODE olc_change_dbinfo(int fd, REQUEST *request, int auth);
extern ERRCODE olc_get_accesses(int fd, REQUEST *request, int auth);

/* Other external declarations. */

extern void backup_data(void);	/* Backup the current state. */

KNUCKLE *create_user(PERSON *person);
KNUCKLE *create_knuckle(USER *user);

void delete_user(USER *user);
void delete_knuckle(KNUCKLE *knuckle, int cont);
void init_user(KNUCKLE *knuckle, PERSON *person);
QUEUE_STATUS *get_status_info(void);

extern char *fmt (const char *, ...);


#else /* __STDC__ */

extern olc_topic();		/* Change the current topic. */
extern olc_comment();		/* Insert a comment in the log. */
extern olc_describe();		/* Make more comments */
extern olc_done();		/* Mark a question done. */
extern olc_forward();		/* Forward a question. */
extern olc_list();		/* List current conversations. */
extern olc_mail();		/* Send mail to a user. */
extern olc_on();		/* Sign on to OLC. */
extern olc_off();		/* Sign off of OLC. */
extern olc_replay();		/* Replay the conversation. */
extern olc_send();		/* Send a message. */
extern olc_who();		/* Print user's name. */
extern olc_startup();		/* Start up an OLCR session. */
extern olc_show();		/* Show any new messages. */
extern olc_grab();		/* Grab a question on the queue. */
extern olc_cancel();		/* Cancel a question. */
extern olc_status();		/* Print user status information. */
extern olc_ask();		/* ask a question */
extern olc_chtopic();		/* change a topic */
extern olc_list_topics();	/* list topics */
extern olc_create_instance();	/* create a new instance */
extern olc_default_instance();
extern olc_motd();		/* retrieve the olc motd */
extern olc_dump();		/* debugging info */
extern olc_dump_req_stats();	/* "profiling" info */
extern olc_dump_ques_stats();	/* stats about questions */
extern olc_cancel();
extern olc_verify_topic();
extern olc_verify_instance();
extern olc_load_user();
extern olc_change_motd();
extern olc_change_acl();
extern olc_get_dbinfo();
extern olc_list_acl();
extern olc_change_dbinfo();
extern olc_get_accesses();

/* Other external declarations. */

extern void backup_data();	/* Backup the current stats. */

KNUCKLE *create_user();
KNUCKLE *create_knuckle();

void delete_user();
void delete_knuckle();
void init_user();
QUEUE_STATUS *get_status_info();

extern char *fmt ();

#endif /* __STDC__ */



/* Global variables */

extern KNUCKLE          **Knuckle_List;
extern TOPIC            **Topic_List;
extern int              needs_backup;
extern PROC  Proc_List[];	/* OLC Proceedure Table */
extern int request_count;
extern int request_counts[OLC_NUM_REQUESTS];
extern long start_time;

/* useful macros */

#define is_allowed(u,a)         (u->permissions & a)
#define is_connected(k)         k->connected
#define has_question(k)         k->question
#define is_signed_on(k)         (k->status & SIGNED_ON)
#define sign_on(k,code)         k->status |= code
#define sign_off(k)             k->status = 0
#define is_logout(k)            (k->user->status & LOGGED_OUT)
#define is_pitted(k)            (k->queue  & PIT_Q)
#define is_busy(k)              (k->status & BUSY)
#define add_status(k,s)         k->status |= s
#define sub_status(k,s)         k->status &= ~s
#define set_status(k,s)         k->status = s
#define is_active(k)            ((k->status) || (k->connected))
#define deactivate(k)           k->status = 0;
#define is_me(r,t)              (r->user == t->user)
#define is_connected_to(t,r)    (r->connected) && (t->connected) && \
                                (r->connected == t) && (t->connected == r)
#define is_specialty(user,code) is_topic(user->specialties,code)

#define ON_ACL                  1<<1
#define MONITOR_ACL             1<<2
#define OLC_ACL                 1<<3
#define CONSULT_ACL             1<<4
#define GRAB_ACL                1<<5
#define GRESOLVE_ACL            1<<6
#define GASK_ACL                1<<7
#define GCOMMENT_ACL            1<<8
#define GMESSAGE_ACL            1<<9
#define ADMIN_ACL               1<<10
#define GCHTOPIC_ACL            1<<11
#define MOTD_ACL                1<<12


#ifdef __STDC__
/* These declarations should go elsewhere!!! */

extern int	get_knuckle (char *, int, KNUCKLE **, int);
extern int	insert_knuckle (KNUCKLE *);
extern int	insert_knuckle_in_user (KNUCKLE *, USER *);
extern void	init_dbinfo (USER *);
extern void	load_user (USER *);
extern int	verify_topic (char *);
extern ERRCODE	init_log (KNUCKLE *, const char *, const char *);
extern int	is_topic (int *, int);
extern int	owns_question (KNUCKLE *);
extern int	insert_topic (TOPIC *);
extern void	get_list_info (KNUCKLE *, LIST *);
extern int	load_db (void);
extern void	load_data (void);
extern ERRCODE	read_request (int, REQUEST *);
extern int	authenticate (REQUEST *, unsigned long);
extern int	find_knuckle (PERSON *, KNUCKLE **);
extern int	get_user (PERSON *, USER **);
extern void	dump_data (const char *);
extern void	dump_request_stats (const char *);
extern void	dump_question_stats (const char *);
extern int	save_user_info (USER *);
extern int	match_knuckle (char *, int, KNUCKLE **);
extern int	match_maker (KNUCKLE *);
extern int	send_person (int, PERSON *);
extern int	verify_instance (KNUCKLE *, int);
extern int	get_instance (char *, int *);
extern int	send_list (int, REQUEST *, LIST *);
extern ERRCODE	terminate_log_answered (KNUCKLE *);
extern ERRCODE	terminate_log_unanswered (KNUCKLE *);
extern void	new_message (char **, KNUCKLE *, char *);
extern int	list_user_knuckles (KNUCKLE *, LIST **, int *);
extern int	list_queue (int, LIST **, int, int *, int, char *, int *);
extern int	connect_knuckles (KNUCKLE *, KNUCKLE *);
extern int	init_question (KNUCKLE *, char *, char *, char *);
extern void	free_new_messages (KNUCKLE *);

/* notifications */
extern ERRCODE	write_message_to_user (KNUCKLE *, char *, int);
extern ERRCODE	olc_broadcast_message (const char *, const char *,
				       const char *);

/* user/question logs */
extern void	log_message (const KNUCKLE *, const KNUCKLE *, const char *);
extern void	log_comment (const KNUCKLE *, const KNUCKLE *, const char *);
extern void	log_description (const KNUCKLE *, const KNUCKLE *,
				 const char *);
extern void	log_mail (const KNUCKLE *, const KNUCKLE *, const char *);
extern void	log_daemon (const KNUCKLE *, const char *);

/* OLCD system logs */
extern void	log_error (const char *message);
extern void	log_status (const char *);
extern void	log_admin (const char *);
extern void	log_debug (const char *);

#if is_cplusplus
extern "C" {
#endif

    extern char *get_next_word();

    /* other libraries */
    /* Kerberos */
#ifdef KERBEROS
    extern int krb_get_lrealm (char *, int);
    extern int krb_rd_req (KTEXT, const char *, char *, long, AUTH_DAT *,
			   const char *);
    extern int dest_tkt (void);
    extern int krb_get_svc_in_tkt (const char *, const char *, const char *,
				   const char *, const char *, int,
				   const char *);

#endif

    /* Zephyr */
#if defined (ZEPHYR) && defined (ZVERSIONHDR)
    extern Code_t	ZInitialize (void);
    /* This is inconsistent between Code_t and int for return types, but
     * that's the way it goes...  */
    typedef int	(*OZMagicFunction) (ZNotice_t *, char *, int, int *);
    extern Code_t	ZSendNotice (ZNotice_t *, OZMagicFunction);
    extern Code_t	ZIfNotice (ZNotice_t *, struct sockaddr_in *,
				   OZMagicFunction, char *);
    extern void	ZFreeNotice (ZNotice_t *);
#endif /* Zephyr */

    /* Acl library */
    extern int	acl_check (char *, char *);
    extern int	acl_add (char *, char *);
    extern int	acl_delete (char *, char *);

#if is_cplusplus
};
#endif

#else /* __STDC__ */
extern int	get_knuckle ();
extern int	insert_knuckle ();
extern int	insert_knuckle_in_user ();
extern void	init_dbinfo ();
extern void	load_user ();
extern int	verify_topic ();
extern ERRCODE	init_log ();
extern int	is_topic ();
extern int	owns_question ();
extern int	insert_topic ();
extern void	get_list_info ();
extern int	load_db ();
extern void	load_data ();
extern ERRCODE	read_request ();
extern int	authenticate ();
extern int	find_knuckle ();
extern int	get_user ();
extern void	dump_data ();
extern void	dump_request_stats ();
extern void	dump_question_stats ();
extern int	save_user_info ();
extern int	match_knuckle ();
extern int	match_maker ();
extern int	send_person ();
extern int	verify_instance ();
extern int	get_instance ();
extern int	send_list ();
extern ERRCODE	terminate_log_answered ();
extern ERRCODE	terminate_log_unanswered ();
extern void	new_message ();
extern int	list_user_knuckles ();
extern int	list_queue ();
extern int	connect_knuckles ();
extern int	init_question ();
extern void	free_new_messages ();

/* notifications */
extern ERRCODE	write_message_to_user ();
extern ERRCODE	olc_broadcast_message ();

/* user/question logs */
extern void	log_message ();
extern void	log_comment ();
extern void	log_description ();
extern void	log_mail ();
extern void	log_daemon ();

/* OLCD system logs */
extern void	log_error ();
extern void	log_status ();
extern void	log_admin ();
extern void	log_debug ();

extern char *get_next_word();

/* other libraries */
/* Kerberos */
#ifdef KERBEROS
extern int krb_get_lrealm ();
extern int krb_rd_req ();
extern int dest_tkt ();
extern int krb_get_svc_in_tkt ();

#endif

/* Zephyr */
#if defined (ZEPHYR) && defined (ZVERSIONHDR)
extern Code_t	ZInitialize ();
/* This is inconsistent between Code_t and int for return types, but
 * that's the way it goes...  */
typedef int	(*OZMagicFunction) ();
extern Code_t	ZSendNotice ();
extern Code_t	ZIfNotice ();
extern void	ZFreeNotice ();
#endif /* Zephyr */

/* Acl library */
extern int	acl_check ();
extern int	acl_add ();
extern int	acl_delete ();

#endif /* __STDC__ */
#endif /* __olcd_h */
