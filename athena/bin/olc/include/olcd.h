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
 *	$Id: olcd.h,v 1.51 1999-06-29 21:31:06 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __olcd_h
#define __olcd_h __FILE__

#include <olc/lang.h>
#include <olc/olc.h>
#include <common.h>
#include <server_defines.h>
#include <server_structs.h>
#include <sys/param.h>

#ifdef STDC_HEADERS
#include <stdarg.h>
#endif

#define VERSION_INFO	"3.2"

/* useful macros */

#define is_allowed(u,a)         (u->permissions & a)
#define is_connected(k)         (k->connected != NULL)
#define has_question(k)         ((k != NULL) && \
				 (k->question != NULL))
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
#define is_connected_to(t,r)    (t != NULL) && \
                                (r != NULL) && \
                                (r->connected) && (t->connected) && \
                                (r->connected == t) && (t->connected == r)

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


/* acl_files.c */
void acl_canonicalize_principal (char *principal , char *canon );
int acl_initialize (char *acl_file , int perm );
int acl_exact_match (char *acl , char *principal );
int acl_check (char *acl , char *principal );
int acl_add (char *acl , char *principal );
int acl_delete (char *acl , char *principal );

/* backup.c */
void ensure_consistent_state (void );
void reconnect_knuckles (void );
char *fget_line (char **buf, size_t *size, FILE *fp);
ERRCODE expect_long (long *val, char *lead,
		     char **buf, size_t *size, FILE *fp);
ERRCODE expect_str_fixwid (char *dst, size_t dstlen, char *lead,
			   char **buf, size_t *size, FILE *fp);
void backup_data (void);
void load_data (void);
/* backup-bin.c */
void binary_backup_data (void);
void binary_load_data (void);
/* backup-dump.c */
ERRCODE dump_data (char *file);
ERRCODE undump_data (char *file);

/* data_utils.c */
KNUCKLE *create_user (PERSON *person );
KNUCKLE *create_knuckle (USER *user );
int insert_knuckle (KNUCKLE *knuckle );
int insert_knuckle_in_user (KNUCKLE *knuckle , USER *user );
int insert_topic (TOPIC *t );
int get_topic_code (char *t);
void delete_user (USER *user );
void delete_knuckle (KNUCKLE *knuckle , int cont );
int deactivate_knuckle (KNUCKLE *knuckle );
void init_user (KNUCKLE *knuckle , PERSON *person );
void init_dbinfo (USER *user );
int init_question (KNUCKLE *k , char *topic , char *text , char *machinfo );
int get_user (PERSON *person , USER **user );
int get_knuckle (char *name , int instance , KNUCKLE **knuckle , int active );
int match_knuckle (char *name , int instance , KNUCKLE **knuckle );
int find_knuckle (PERSON *person , KNUCKLE **knuckle );
int get_instance (char *user , int *instance );
int verify_instance (KNUCKLE *knuckle , int instance );
int connect_knuckles (KNUCKLE *a , KNUCKLE *b );
void disconnect_knuckles (KNUCKLE *a , KNUCKLE *b );
void free_new_messages (KNUCKLE *knuckle );
int match_maker (KNUCKLE *knuckle );
void new_message (KNUCKLE *target , KNUCKLE *sender , char *message );
int has_new_messages (KNUCKLE *target );
QUEUE_STATUS *get_status_info (void );
int verify_topic (char *topic );
int owns_question (KNUCKLE *knuckle );
int is_specialty (USER *u , int topic );
int is_topic (int *topics , int code );
void write_question_info ( QUESTION *q );

/* db.c */
int get_specialties (USER *user );
void get_acls (USER *user );
int load_db (void );
void load_user (USER *user );
int save_user_info (USER *user );

/* io.c */
ERRCODE read_request (int fd , REQUEST *request );
ERRCODE send_list (int fd , REQUEST *request , LIST *list );

/* list.c */
int list_knuckle (KNUCKLE *knuckle , LIST *data );
int list_user_knuckles (KNUCKLE *knuckle , LIST **data , int *size );
int list_redundant (KNUCKLE *knuckle );
int list_queue (LIST **data , int *topics,
		int stati , char *name , int *size );
void dump_list (void );

/* log.c */
void write_line_to_log (FILE *log , char *line );
void format_line_to_user_log (FILE *log , char *line );
void log_daemon (KNUCKLE *knuckle , char *message );
void log_message (KNUCKLE *owner , KNUCKLE *sender , char *message );
void log_mail (KNUCKLE *owner , KNUCKLE *sender , char *message );
void log_comment (KNUCKLE *owner , KNUCKLE *sender,
		  char *message , int is_private );
void log_description (KNUCKLE *owner , KNUCKLE *sender , char *message );
void log_long_description (KNUCKLE *owner , KNUCKLE *sender,
			   char *message );
ERRCODE init_log (KNUCKLE *knuckle , char *question , char *machinfo );
ERRCODE terminate_log_answered (KNUCKLE *knuckle );
ERRCODE terminate_log_unanswered (KNUCKLE *knuckle );

/* motd.c */
void check_motd_timeout (void );
void set_motd_timeout (KNUCKLE *requester );
void log_motd (char *username );

/* notify.c */
ERRCODE write_message (char *touser , char *tomachine ,
		       char *fromuser , char *frommachine , char *message );
ERRCODE write_message_to_user (KNUCKLE *k , char *message , int flags );
ERRCODE olc_broadcast_message (char *instance , char *message , char *code );
void toggle_zephyr (int toggle, int time);

/* olcd.c */
int main (int argc , char **argv );
int authenticate (REQUEST *request , unsigned long addr );
int get_kerberos_ticket (void );

/* requests_admin.c */
ERRCODE olc_load_user (int fd , REQUEST *request );
ERRCODE olc_dump (int fd , REQUEST *request );
ERRCODE olc_change_motd (int fd , REQUEST *request );
ERRCODE olc_change_hours (int fd , REQUEST *request );
ERRCODE olc_change_acl (int fd , REQUEST *request );
ERRCODE olc_list_acl (int fd , REQUEST *request );
ERRCODE olc_get_accesses (int fd , REQUEST *request );
ERRCODE olc_get_dbinfo (int fd , REQUEST *request );
ERRCODE olc_change_dbinfo (int fd , REQUEST *request );
ERRCODE olc_set_user_status (int fd , REQUEST *request );
ERRCODE olc_toggle_zephyr (int fd , REQUEST *request );

/* requests_olc.c */
ERRCODE olc_on (int fd , REQUEST *request );
ERRCODE olc_create_instance (int fd , REQUEST *request );
ERRCODE olc_get_connected_info (int fd , REQUEST *request );
ERRCODE olc_verify_instance (int fd , REQUEST *request );
ERRCODE olc_default_instance (int fd , REQUEST *request );
ERRCODE olc_who (int fd , REQUEST *request );
ERRCODE olc_done (int fd , REQUEST *request );
ERRCODE olc_cancel (int fd , REQUEST *request );
ERRCODE olc_ask (int fd , REQUEST *request );
ERRCODE olc_forward (int fd , REQUEST *request );
ERRCODE olc_off (int fd , REQUEST *request );
ERRCODE olc_send (int fd , REQUEST *request );
ERRCODE olc_comment (int fd , REQUEST *request );
ERRCODE olc_describe (int fd , REQUEST *request );
ERRCODE olc_replay (int fd , REQUEST *request );
ERRCODE olc_show (int fd , REQUEST *request );
ERRCODE olc_list (int fd , REQUEST *request );
ERRCODE olc_topic (int fd , REQUEST *request );
ERRCODE olc_chtopic (int fd , REQUEST *request );
ERRCODE olc_verify_topic (int fd , REQUEST *request );
ERRCODE olc_list_topics (int fd , REQUEST *request );
ERRCODE olc_list_services (int fd , REQUEST *request );
ERRCODE olc_motd (int fd , REQUEST *request );
ERRCODE olc_mail (int fd , REQUEST *request );
ERRCODE olc_startup (int fd , REQUEST *request );
ERRCODE olc_grab (int fd , REQUEST *request );
ERRCODE olc_get_hours (int fd , REQUEST *request );

/* statistics.c */
void write_ask_stats (char *username, char *topic, char *machine, char
			*ask_by );
void write_res_stats (QUESTION *q);

/* syslog.c */
void init_logs (void);
void log_error (const char *fmt, ...);
void log_zephyr_error (const char *fmt, ...);
void log_status (const char *fmt, ...);
void log_admin (const char *fmt, ...);
void log_debug (const char *fmt, ...);
void log_error_string (const char *msg);

/* utils.c */
void get_list_info (KNUCKLE *k , LIST *data );

/* version.c */
ERRCODE olc_version (int fd , REQUEST *request );


/* other libraries */
#ifdef HAVE_ZEPHYR
#include <zephyr/zephyr.h>
#endif /* HAVE_ZEPHYR */

#endif /*__olcd_h*/
