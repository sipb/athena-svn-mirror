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
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olcd.h,v $
 *      $Author: tjcoppet $
 */

/* Important files. */

extern char *OLC_DATABASE;
extern char *NF_PREFIX;
extern char *LOG_DIR;
extern char *BACKUP_FILE;
extern char *ERROR_LOG;
extern char *STATUS_LOG;
extern char *STDERR_LOG;
extern char *TOPIC_FILE;
extern char *USER_FILE;
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
#define SYSLOG_LEVEL LOG_LOCAL8
#endif SYSLOG

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
  char   realname[NAME_LENGTH];
  char   nickname[NAME_LENGTH];         
  char   title1[NAME_LENGTH];        /* title of user in OLC */
  char   title2[NAME_LENGTH];        /* title of consultant in OLC */
  char   machine[NAME_LENGTH];      /* user location */
  char   realm[NAME_LENGTH];
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
  char   title[NAME_LENGTH];
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
  char acl[NAME_LENGTH];
  char name[NAME_LENGTH];
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
} ACL;

/* Declarations of functions. */


/* OLC procedure declarations. */

extern olc_topic();             /* Change the current topic. */
extern olc_comment();           /* Insert a comment in the log. */
extern olc_describe();          /* Make more comments */
extern olc_done();              /* Mark a question done. */
extern olc_forward();           /* Forward a question. */
extern olc_list();              /* List current conversations. */
extern olc_mail();              /* Send mail to a user. */
extern olc_on();                /* Sign on to OLC. */
extern olc_off();               /* Sign off of OLC. */
extern olc_replay();            /* Replay the conversation. */
extern olc_send();              /* Send a message. */
extern olc_who();               /* Print user's name. */
extern olc_startup();           /* Start up an OLCR session. */
extern olc_show();              /* Show any new messages. */
extern olc_grab();              /* Grab a question on the queue. */
extern olc_cancel();            /* Cancel a question. */
extern olc_status();            /* Print user status information. */
extern olc_ask();               /* ask a question */
extern olc_chtopic();           /* change a topic */
extern olc_list_topics();       /* list topics */
extern olc_create_instance();   /* create a new instance */
extern olc_default_instance();
extern olc_motd();              /* retrieve the olc motd */
extern olc_dump();              /* debugging info */
extern olc_cancel();
extern olc_verify_topic();
extern olc_verify_instance();
extern olc_load_user();

/* Other external declarations. */

extern void backup_data();      /* Backup the current state. */

KNUCKLE *create_user();
KNUCKLE *create_knuckle();

void delete_user();
void delete_knuckle();
void init_user();
QUEUE_STATUS *get_status_info();

extern char *get_next_word();

/* System functions. */

extern char *malloc(), *realloc();


/* Global variables */

extern KNUCKLE          **Knuckle_List;
extern TOPIC            **Topic_List;
extern int              needs_backup;

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
                                (r->connected == t->connected)
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


