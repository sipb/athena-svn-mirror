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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olcd.h,v $
 *	$Id: olcd.h,v 1.24 1990-12-12 15:08:23 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __olcd_h
#define __olcd_h __FILE__

#include <olc/lang.h>
#include <olc/olc.h>
#include <common.h>

/*
 * this ugliness is due to a (supposedly) standard-C compiler that
 * doesn't provide stdarg.h.
 */

#ifdef __STDC__
#ifndef __HIGHC__
#include <stdarg.h>
#define HAS_STDARG
#endif
#else
#include <varargs.h>
#endif


/* Important files. */

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
extern char *MOTD_TIMEOUT_FILE;
extern char *MACH_TRANS_FILE;
extern char *LIST_FILE_NAME;
extern char *LIST_TMP_NAME;
extern char *SPECIALTY_DIR;
extern char *ACL_DIR;

#ifdef KERBEROS
extern char K_INSTANCEbuf[];
extern char *SRVTAB_FILE;
#endif /* KERBEROS */

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

#define EMERGENCY_Q       1
#define ACTIVE_Q          2
#define INACTIVE_Q        4
#define PIT_Q             8

/* type classifiers */

#define IS_TOPIC               500
#define IS_SUBTOPIC            501

#ifdef LAVIN
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "TA"
#else
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "consultant"
#endif


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

typedef struct tdLIST
{
  char	*username;
  char	*machine;
  int	instance;
  int	ustatus;
  int	kstatus;
  char	*cusername;
  int	cinstance;
  int	cstatus;
  int	n_consult;
  char	*topic;
  long	timestamp;
  char	*note;
} D_LIST;

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
  char   nm_file[NAME_SIZE+6];
  int    new_messages;              /* new messages for this knuckle */
				    /* 0 = none, 1 = yes, -1 = unknown */
  
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

typedef struct tTRANS
{
  char orig[80];
  char trans[80];
} TRANS;

/* Global variables */

extern KNUCKLE          **Knuckle_List;
extern TOPIC            **Topic_List;
extern int              needs_backup;
extern PROC  Proc_List[];	/* OLC Proceedure Table */
extern ACL  Acl_List[];
extern int request_count;
extern int request_counts[OLC_NUM_REQUESTS];
extern long start_time;
extern char DaemonInst[];

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
# define        P(s) s
#else
# define P(s) ()
#endif

/* acl_files.c */
void acl_canonicalize_principal P((char *principal , char *canon ));
int acl_initialize P((char *acl_file , int perm ));
int acl_exact_match P((char *acl , char *principal ));
int acl_check P((char *acl , char *principal ));
int acl_add P((char *acl , char *principal ));
int acl_delete P((char *acl , char *principal ));

/* backup.c */
void reconnect_knuckles P((void ));
void backup_data P((void ));
void load_data P((void ));
void dump_data P((char *file ));

/* data_utils.c */
KNUCKLE *create_user P((PERSON *person ));
KNUCKLE *create_knuckle P((USER *user ));
int insert_knuckle P((KNUCKLE *knuckle ));
int insert_knuckle_in_user P((KNUCKLE *knuckle , USER *user ));
int insert_topic P((TOPIC *t ));
int get_topic_code P((char *t));
void delete_user P((USER *user ));
void delete_knuckle P((KNUCKLE *knuckle , int cont ));
int deactivate_knuckle P((KNUCKLE *knuckle ));
void init_user P((KNUCKLE *knuckle , PERSON *person ));
void init_dbinfo P((USER *user ));
int init_question P((KNUCKLE *k , char *topic , char *text , char *machinfo ));
int get_user P((PERSON *person , USER **user ));
int get_knuckle P((char *name , int instance , KNUCKLE **knuckle , int active ));
int match_knuckle P((char *name , int instance , KNUCKLE **knuckle ));
int find_knuckle P((PERSON *person , KNUCKLE **knuckle ));
int get_instance P((char *user , int *instance ));
int verify_instance P((KNUCKLE *knuckle , int instance ));
int connect_knuckles P((KNUCKLE *a , KNUCKLE *b ));
void disconnect_knuckles P((KNUCKLE *a , KNUCKLE *b ));
void free_new_messages P((KNUCKLE *knuckle ));
int match_maker P((KNUCKLE *knuckle ));
void new_message P((KNUCKLE *target , KNUCKLE *sender , char *message ));
int has_new_messages P((KNUCKLE *target ));
QUEUE_STATUS *get_status_info P((void ));
int verify_topic P((char *topic ));
int owns_question P((KNUCKLE *knuckle ));
int is_topic P((int *topics , int code ));

/* db.c */
int get_specialties P((USER *user ));
void get_acls P((USER *user ));
int load_db P((void ));
void load_user P((USER *user ));
int save_user_info P((USER *user ));

/* io.c */
ERRCODE read_request P((int fd , REQUEST *request ));
int send_list P((int fd , REQUEST *request , LIST *list ));
ERRCODE send_person P((int fd , PERSON *person ));

/* list.c */
int list_knuckle P((KNUCKLE *knuckle , LIST *data ));
int list_user_knuckles P((KNUCKLE *knuckle , LIST **data , int *size ));
int list_redundant P((KNUCKLE *knuckle ));
int list_queue P((LIST **data , int *topics , int stati , char *name , int *size ));
void dump_list P((void ));

/* log.c */
void write_line_to_log P((FILE *log , char *line ));
void format_line_to_user_log P((FILE *log , char *line ));
#ifndef __HIGHC__
char *vfmt P((char *format, va_list pvar ));
#endif
#ifdef HAS_STDARG
char *fmt P((char *format , ...));
#else
char *fmt P(());
#endif
void log_daemon P((KNUCKLE *knuckle , char *message ));
void log_message P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_mail P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_comment P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_description P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_long_description P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
ERRCODE init_log P((KNUCKLE *knuckle , char *question , char *machinfo ));
ERRCODE terminate_log_answered P((KNUCKLE *knuckle ));
ERRCODE terminate_log_unanswered P((KNUCKLE *knuckle ));

/* motd.c */
void check_motd_timeout P((void ));
void set_motd_timeout P((KNUCKLE *requester ));

/* notify.c */
ERRCODE write_message P((char *touser , char *tomachine , char *fromuser , char *frommachine , char *message ));
ERRCODE write_message_to_user P((KNUCKLE *k , char *message , int flags ));
ERRCODE olc_broadcast_message P((char *instance , char *message , char *code ));

/* olcd.c */
int main P((int argc , char **argv ));
int punt P((int sig ));
int authenticate P((REQUEST *request , unsigned long addr ));
int get_kerberos_ticket P((void ));

/* requests_admin.c */
ERRCODE olc_load_user P((int fd , REQUEST *request ));
ERRCODE olc_dump P((int fd , REQUEST *request ));
ERRCODE olc_dump_req_stats P((int fd , REQUEST *request ));
ERRCODE olc_dump_ques_stats P((int fd , REQUEST *request ));
ERRCODE olc_change_motd P((int fd , REQUEST *request ));
ERRCODE olc_change_acl P((int fd , REQUEST *request ));
ERRCODE olc_list_acl P((int fd , REQUEST *request ));
ERRCODE olc_get_accesses P((int fd , REQUEST *request ));
ERRCODE olc_get_dbinfo P((int fd , REQUEST *request ));
ERRCODE olc_change_dbinfo P((int fd , REQUEST *request ));

/* requests_olc.c */
ERRCODE olc_on P((int fd , REQUEST *request ));
ERRCODE olc_create_instance P((int fd , REQUEST *request ));
ERRCODE olc_get_connected_info P((int fd , REQUEST *request ));
ERRCODE olc_verify_instance P((int fd , REQUEST *request ));
ERRCODE olc_default_instance P((int fd , REQUEST *request ));
ERRCODE olc_who P((int fd , REQUEST *request ));
ERRCODE olc_done P((int fd , REQUEST *request ));
ERRCODE olc_cancel P((int fd , REQUEST *request ));
ERRCODE olc_ask P((int fd , REQUEST *request ));
ERRCODE olc_forward P((int fd , REQUEST *request ));
ERRCODE olc_off P((int fd , REQUEST *request ));
ERRCODE olc_send P((int fd , REQUEST *request ));
ERRCODE olc_comment P((int fd , REQUEST *request ));
ERRCODE olc_describe P((int fd , REQUEST *request ));
ERRCODE olc_replay P((int fd , REQUEST *request ));
ERRCODE olc_show P((int fd , REQUEST *request ));
ERRCODE olc_list P((int fd , REQUEST *request ));
ERRCODE olc_topic P((int fd , REQUEST *request ));
ERRCODE olc_chtopic P((int fd , REQUEST *request ));
ERRCODE olc_verify_topic P((int fd , REQUEST *request ));
ERRCODE olc_list_topics P((int fd , REQUEST *request ));
ERRCODE olc_motd P((int fd , REQUEST *request ));
ERRCODE olc_mail P((int fd , REQUEST *request ));
ERRCODE olc_startup P((int fd , REQUEST *request ));
ERRCODE olc_grab P((int fd , REQUEST *request ));

/* statistics.c */
void dump_request_stats P((char *file ));
void dump_question_stats P((char *file ));

/* syslog.c */
void log_error P((char *message ));
void log_zephyr_error P((char *message ));
void log_status P((char *message ));
void log_admin P((char *message ));
void log_debug P((char *message ));

/* utils.c */
void get_list_info P((KNUCKLE *k , LIST *data ));




/* other libraries */
/* Kerberos */
#ifdef KERBEROS
extern int krb_get_lrealm P((char *, int));
extern int krb_rd_req P((KTEXT, const char *, char *, u_long, AUTH_DAT *,
		       char *));
#endif /* KERBEROS */

#ifdef ZEPHYR
#include <zephyr/zephyr.h>
extern Code_t	ZInitialize P((void));
typedef int	(*ZPredFunc) P((ZNotice_t *, ZUnique_Id_t *));
typedef int	(*ZCertFunc) P((ZNotice_t *, char *, int, int*));
extern Code_t	ZSendNotice P((ZNotice_t *, ZCertFunc));
extern Code_t	ZIfNotice P((ZNotice_t *, struct sockaddr_in *,
			   ZPredFunc, char *));
extern void	ZFreeNotice P((ZNotice_t *));
#endif /* Zephyr */

#undef P

#endif
