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
 *	$Id: olc_tty.h,v 1.20 1999-06-28 22:52:34 ghudson Exp $
 */

#include <mit-copyright.h>

#define DEFAULT_EDITOR "/usr/athena/bin/emacs"
#define NO_EDITOR      "NO_EDITOR"
#define DEFAULT_PAGER  "more"

/* t_acl.c */
ERRCODE t_set_acl (REQUEST *Request , char *acl , int flag );
ERRCODE t_list_acl (REQUEST *Request , char *acl , char *file );
ERRCODE t_get_accesses (REQUEST *Request , char *file );

/* t_ask.c */
ERRCODE t_ask (REQUEST *Request , char *topic , char *q_file);

/* t_admin.c */
ERRCODE t_toggle_zephyr (REQUEST *Request, int what, int how_long);

/* t_connect.c */
ERRCODE t_grab (REQUEST *Request , int flag , int hold );
ERRCODE t_forward (REQUEST *Request );

/* t_consult.c */
ERRCODE t_sign_on (REQUEST *Request , int flag , int hold );
ERRCODE t_olc_off (REQUEST *Request );

/* t_data.c */

/* t_db.c */
ERRCODE t_load_user (REQUEST *Request );
ERRCODE t_dbinfo (REQUEST *Request , char *file );
ERRCODE t_change_dbinfo (REQUEST *Request );

/* t_describe.c */
ERRCODE t_describe (REQUEST *Request , char *file , char *note ,
		    int dochnote , int dochcomment );
int t_display_description (LIST *list , char *file );

/* t_getline.c */
void gl_init (int scrn_wdth);
void gl_cleanup (void);
void gl_char_cleanup (void);
void gl_char_init (void);
char *getline (char *prompt, int add_to_hist);

/* t_instance.c */
ERRCODE t_instance (REQUEST *Request , int instance );
ERRCODE t_set_default_instance (REQUEST *Request );

/* t_list.c */
ERRCODE t_list_queue (REQUEST *Request , char **sort , char *queues ,
		      char *topics , char *users , int stati ,
		      int comments , char *file , int display );
void output_status_header (FILE *file , const char *status );
ERRCODE t_display_list (LIST *list , int comments , char *file );

/* t_messages.c */
ERRCODE t_replay (REQUEST *Request , char *queues , char *topics ,
		  char *users , int stati , char *file , int display );
ERRCODE t_show_message (REQUEST *Request , char *file , int display ,
			int connected , int noflush );
ERRCODE t_check_messages (REQUEST *Request );
ERRCODE t_check_connected_messages (REQUEST *Request );

/* t_motd.c */
ERRCODE t_get_file (REQUEST *Request , int type , char *file ,
		    int display_opts );
ERRCODE t_change_file (REQUEST *Request , int type , char *file ,
			 char *editor , int incflag, int clearflag );

/* t_queue.c */
ERRCODE t_queue (REQUEST *Request , char *queue );

/* t_resolve.c */
ERRCODE t_done (REQUEST *Request , char *title, int check );
ERRCODE t_cancel (REQUEST *Request , char *title );

/* t_send.c */
ERRCODE t_reply (REQUEST *Request , char *file , char *editor );
ERRCODE t_comment (REQUEST *Request , char *file , char *editor );
ERRCODE t_mail (REQUEST *Request , char *file , char *editor ,
		char **smargs , int check , int noedit, int header);

/* t_status.c */
ERRCODE t_personal_status (REQUEST *Request , int chart );
ERRCODE t_display_personal_status (REQUEST *Request , LIST *list , int chart );
ERRCODE t_who (REQUEST *Request );
ERRCODE t_input_status (REQUEST *Request , char *string );
int get_user_status_string (int status , char *string );
int get_status_string (int status , char *string );
int t_pp_stati (void );

/* t_topic.c */
ERRCODE t_input_topic (REQUEST *Request , char *topic , int flags );
ERRCODE t_list_topics (REQUEST *Request , char *file , int display );
ERRCODE t_verify_topic (REQUEST *Request , char *topic );
ERRCODE t_get_topic (REQUEST *Request , char *topic );
ERRCODE t_change_topic (REQUEST *Request , char *topic );

/* t_utils.c */
int is_flag(char *string, char *flag, int minimum);
ERRCODE display_file (char *filename );
ERRCODE cat_file (char *file );
ERRCODE enter_message (char *file , char *editor );
ERRCODE input_text_into_file (char *filename );
ERRCODE verify_terminal (void );
char get_key_input (char *text );
int raw_mode (void );
int cooked_mode (void );
ERRCODE handle_response (int response , REQUEST *req );
ERRCODE get_prompted_input (char *prompt , char *buf, int buflen,
			      int add_to_history); 
int get_yn (char *prompt );
ERRCODE what_now (char *file , int edit_first , char *editor );
ERRCODE edit_message (char *file , char *editor );
ERRCODE mail_message (char *user , char *consultant , char *msgfile ,
		      char **args );
char *happy_message (void );
char *article (char *word );

/* t_version.c */
ERRCODE t_version (REQUEST *Request );
