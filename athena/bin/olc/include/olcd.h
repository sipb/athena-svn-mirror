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
 *	$Id: olcd.h,v 1.46 1994-08-21 18:22:20 cfields Exp $
 *	$Author: cfields $
 */

#include <mit-copyright.h>

#ifndef __olcd_h
#define __olcd_h __FILE__

#include <olc/lang.h>
#include <olc/olc.h>
#include <common.h>
#include <server_defines.h>
#include <server_structs.h>
#ifdef DBMALLOC
#include "/afs/athena.mit.edu/contrib/watchmaker/src/include/malloc.h"
#endif
/*
 * this ugliness is due to a (supposedly) standard-C compiler that
 * doesn't provide stdarg.h.
 */

#if defined(__STDC__) && !defined(__HIGHC__) && !defined(SABER)
/* Stupid High-C claims to be ANSI but doesn't have the include files.. */
/* Ditto for saber */
#include <stdarg.h>
#define HAS_STDARG
#endif


#define VERSION_INFO	"3.1"

/* useful macros */

#define is_allowed(u,a)         (u->permissions & a)
#define is_connected(k)         (k->connected != (KNUCKLE *) NULL)
#define has_question(k)         ((k != (KNUCKLE *) NULL) && \
				 (k->question != (QUESTION *) NULL))
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
#define is_connected_to(t,r)    (t != (KNUCKLE *) NULL) && \
                                (r != (KNUCKLE *) NULL) && \
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
int is_specialty P((USER *u , int topic ));
int is_topic P((int *topics , int code ));
void write_question_info P(( QUESTION *q ));

/* db.c */
int get_specialties P((USER *user ));
void get_acls P((USER *user ));
int load_db P((void ));
void load_user P((USER *user ));
int save_user_info P((USER *user ));

/* io.c */
ERRCODE read_request P((int fd , REQUEST *request ));
ERRCODE send_list P((int fd , REQUEST *request , LIST *list ));

/* list.c */
int list_knuckle P((KNUCKLE *knuckle , LIST *data ));
int list_user_knuckles P((KNUCKLE *knuckle , LIST **data , int *size ));
int list_redundant P((KNUCKLE *knuckle ));
int list_queue P((LIST **data , int *topics , int stati , char *name , int *size ));
void dump_list P((void ));

/* log.c */
void write_line_to_log P((FILE *log , char *line ));
void format_line_to_user_log P((FILE *log , char *line ));
void log_daemon P((KNUCKLE *knuckle , char *message ));
void log_message P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_mail P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_comment P((KNUCKLE *owner , KNUCKLE *sender , char *message , int is_private ));
void log_description P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
void log_long_description P((KNUCKLE *owner , KNUCKLE *sender , char *message ));
ERRCODE init_log P((KNUCKLE *knuckle , char *question , char *machinfo ));
ERRCODE terminate_log_answered P((KNUCKLE *knuckle ));
ERRCODE terminate_log_unanswered P((KNUCKLE *knuckle ));

/* motd.c */
void check_motd_timeout P((void ));
void set_motd_timeout P((KNUCKLE *requester ));
void log_motd P((char *username ));

/* notify.c */
ERRCODE write_message P((char *touser , char *tomachine , char *fromuser , char *frommachine , char *message ));
ERRCODE write_message_to_user P((KNUCKLE *k , char *message , int flags ));
ERRCODE olc_broadcast_message P((char *instance , char *message , char *code ));
void toggle_zephyr P((int toggle, int time));

/* olcd.c */
int main P((int argc , char **argv ));
int authenticate P((REQUEST *request , unsigned long addr ));
int get_kerberos_ticket P((void ));

/* requests_admin.c */
ERRCODE olc_load_user P((int fd , REQUEST *request ));
ERRCODE olc_dump P((int fd , REQUEST *request ));
ERRCODE olc_change_motd P((int fd , REQUEST *request ));
ERRCODE olc_change_hours P((int fd , REQUEST *request ));
ERRCODE olc_change_acl P((int fd , REQUEST *request ));
ERRCODE olc_list_acl P((int fd , REQUEST *request ));
ERRCODE olc_get_accesses P((int fd , REQUEST *request ));
ERRCODE olc_get_dbinfo P((int fd , REQUEST *request ));
ERRCODE olc_change_dbinfo P((int fd , REQUEST *request ));
ERRCODE olc_set_user_status P((int fd , REQUEST *request ));
ERRCODE olc_toggle_zephyr P((int fd , REQUEST *request ));

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
ERRCODE olc_list_services P((int fd , REQUEST *request ));
ERRCODE olc_motd P((int fd , REQUEST *request ));
ERRCODE olc_mail P((int fd , REQUEST *request ));
ERRCODE olc_startup P((int fd , REQUEST *request ));
ERRCODE olc_grab P((int fd , REQUEST *request ));
ERRCODE olc_get_hours P((int fd , REQUEST *request ));

/* statistics.c */
void write_ask_stats P((char *username, char *topic, char *machine, char
			*ask_by ));
void write_res_stats P((QUESTION *q));


/* syslog.c */
void log_error P((char *message ));
void log_zephyr_error P((char *message ));
void log_status P((char *message ));
void log_admin P((char *message ));
void log_debug P((char *message ));

/* utils.c */
void get_list_info P((KNUCKLE *k , LIST *data ));

/* version.c */
ERRCODE olc_version P((int fd , REQUEST *request ));


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
