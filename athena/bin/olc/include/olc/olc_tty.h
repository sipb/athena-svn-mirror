/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains some random definitions for the tty library.
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
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_tty.h,v $
 *	$Id: olc_tty.h,v 1.14 1992-01-10 20:02:48 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

extern int OLC, OLCR, OLCA;

#define DEFAULT_EDITOR "/usr/athena/emacs"
#define NO_EDITOR	"NO_EDITOR"
#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

/* t_acl.c */
ERRCODE t_set_acl P((REQUEST *Request , char *acl , int flag ));
ERRCODE t_list_acl P((REQUEST *Request , char *acl , char *file ));
ERRCODE t_get_accesses P((REQUEST *Request , char *file ));

/* t_ask.c */
ERRCODE t_ask P((REQUEST *Request , char *topic , char *q_file));

/* t_admin.c */
ERRCODE t_toggle_zephyr P((REQUEST *Request, int what, int how_long));

/* t_connect.c */
ERRCODE t_grab P((REQUEST *Request , int flag , int hold ));
ERRCODE t_forward P((REQUEST *Request ));

/* t_consult.c */
ERRCODE t_sign_on P((REQUEST *Request , int flag , int hold ));
ERRCODE t_olc_off P((REQUEST *Request ));

/* t_data.c */

/* t_db.c */
ERRCODE t_load_user P((REQUEST *Request ));
ERRCODE t_dbinfo P((REQUEST *Request , char *file ));
ERRCODE t_change_dbinfo P((REQUEST *Request ));

/* t_describe.c */
ERRCODE t_describe P((REQUEST *Request , char *file , char *note , int dochnote , int dochcomment ));
int t_display_description P((LIST *list , char *file ));

/* t_getline.c */
void gl_init P((int scrn_wdth));
void gl_cleanup P((void));
void gl_char_cleanup P((void));
void gl_char_init P((void));
char *getline P((char *prompt, int add_to_hist));

/* t_instance.c */
ERRCODE t_instance P((REQUEST *Request , int instance ));
ERRCODE t_set_default_instance P((REQUEST *Request ));

/* t_list.c */
ERRCODE t_list_queue P((REQUEST *Request , char **sort , char *queues , char *topics , char *users , int stati , int comments , char *file , int display ));
void output_status_header P((FILE *file , const char *status ));
ERRCODE t_display_list P((LIST *list , int comments , char *file ));

/* t_messages.c */
ERRCODE t_replay P((REQUEST *Request , char *queues , char *topics , char *users , int stati , char *file , int display ));
ERRCODE t_show_message P((REQUEST *Request , char *file , int display , int connected , int noflush ));
ERRCODE t_check_messages P((REQUEST *Request ));
ERRCODE t_check_connected_messages P((REQUEST *Request ));

/* t_misc.c */
ERRCODE t_dump P((REQUEST *Request , int type , char *file ));

/* t_motd.c */
ERRCODE t_get_file P((REQUEST *Request , int type , char *file , int display_opts ));
ERRCODE t_change_file P((REQUEST *Request , int type , char *file , char *editor , int incflag ));

/* t_queue.c */
ERRCODE t_queue P((REQUEST *Request , char *queue ));

/* t_resolve.c */
ERRCODE t_done P((REQUEST *Request , char *title, int check ));
ERRCODE t_cancel P((REQUEST *Request , char *title ));

/* t_send.c */
ERRCODE t_reply P((REQUEST *Request , char *file , char *editor ));
ERRCODE t_comment P((REQUEST *Request , char *file , char *editor ));
ERRCODE t_mail P((REQUEST *Request , char *file , char *editor , char
		  **smargs , int check , int noedit, int header));

/* t_status.c */
ERRCODE t_personal_status P((REQUEST *Request , int chart ));
ERRCODE t_display_personal_status P((REQUEST *Request , LIST *list , int chart ));
ERRCODE t_who P((REQUEST *Request ));
ERRCODE t_input_status P((REQUEST *Request , char *string ));
int get_user_status_string P((int status , char *string ));
int get_status_string P((int status , char *string ));
int t_pp_stati P((void ));

/* t_topic.c */
ERRCODE t_input_topic P((REQUEST *Request , char *topic , int flags ));
ERRCODE t_list_topics P((REQUEST *Request , char *file , int display ));
ERRCODE t_verify_topic P((REQUEST *Request , char *topic ));
ERRCODE t_get_topic P((REQUEST *Request , char *topic ));
ERRCODE t_change_topic P((REQUEST *Request , char *topic ));

/* t_utils.c */
ERRCODE display_file P((char *filename ));
ERRCODE cat_file P((char *file ));
ERRCODE enter_message P((char *file , char *editor ));
ERRCODE input_text_into_file P((char *filename ));
ERRCODE verify_terminal P((void ));
char get_key_input P((char *text ));
int raw_mode P((void ));
int cooked_mode P((void ));
ERRCODE handle_response P((int response , REQUEST *req ));
ERRCODE get_prompted_input P((char *prompt , char *buf, int buflen,
			      int add_to_history)); 
int get_yn P((char *prompt ));
ERRCODE what_now P((char *file , int edit_first , char *editor ));
int edit_message P((char *file , char *editor ));
ERRCODE mail_message P((char *user , char *consultant , char *msgfile , char **args ));
char *happy_message P((void ));
char *article P((char *word ));

/* t_version.c */
ERRCODE t_version P((REQUEST *Request ));

#undef P

