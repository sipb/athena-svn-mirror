/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC command parser.
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
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: olc_parser.h,v 1.16 1999-07-08 22:56:54 ghudson Exp $
 */


#ifndef __olc_olc_parser_h
#define __olc_olc_parser_h

/* These really don't return anything useful.  */
typedef ERRCODE (*Pfunction)(char **);

#include <mit-copyright.h>
#include <olc/olc_tty.h>

#define MAX_ARGS         20             /* Maximum number of arguments. */
#define MAX_ARG_LENGTH   80             /* Maximum length of an argument. */
#define MAX_COMMANDS    100             /* Maximum number of commands. */
#define NOT_UNIQUE      -2              /* Not a unique command match. */

typedef struct tCOMMAND  {
        char            *command_name;          /* Name of the command. */
        Pfunction       command_function;       /* Function to execute. */
        char            *description;           /* Brief description. */
} COMMAND;

/* p_acl.c */
ERRCODE do_olc_acl (char **arguments );

/* p_ask.c */
ERRCODE do_olc_ask (char **arguments );

/* p_cmdloop.c */
void command_loop (COMMAND Command_Table [], char *prompt );
ERRCODE do_command (COMMAND Command_Table [], char *arguments []);
ERRCODE command_index (COMMAND Command_Table [], char *command_name );
char *expand_variable (REQUEST *Request , char *var );
int expand_arguments (REQUEST *Request , char **arguments );
int set_prompt (REQUEST *Request , char *prompt , char *inprompt );
ERRCODE parse_command_line (char *command_line ,
			    char arguments [MAX_ARGS ][MAX_ARG_LENGTH ]);
void sigint_handler (int signal);

/* p_connect.c */
ERRCODE do_olc_grab (char **arguments );
ERRCODE do_olc_forward (char **arguments );

/* p_consult.c */
ERRCODE do_olc_on (char **arguments );
ERRCODE do_olc_off (char **arguments );

/* p_describe.c */
ERRCODE do_olc_describe (char **arguments );

/* p_instance.c */
ERRCODE do_olc_instance (char **arguments );

/* p_list.c */
ERRCODE do_olc_list (char **arguments );

/* p_local.c */
ERRCODE do_quit (char *arguments []);
ERRCODE do_olc_help (char *arguments []);
ERRCODE do_olc_list_cmds (char *arguments []);

/* p_messages.c */
ERRCODE do_olc_replay (char **arguments );
ERRCODE do_olc_show (char **arguments );

/* p_misc.c */
ERRCODE do_olc_load_user (char **arguments );
ERRCODE do_olc_dbinfo (char **arguments );

/* p_motd.c */
ERRCODE do_olc_motd (char **arguments );
ERRCODE do_olc_hours (char **arguments );

/* p_queue.c */
ERRCODE do_olc_queue (char **arguments );

/* p_resolve.c */
ERRCODE do_olc_done (char **arguments );
ERRCODE do_olc_cancel (char **arguments );

/* p_send.c */
ERRCODE do_olc_send (char **arguments );
ERRCODE do_olc_comment (char **arguments );
ERRCODE do_olc_mail (char **arguments );

/* p_status.c */
ERRCODE do_olc_status (char **arguments );
ERRCODE do_olc_who (char **arguments );
ERRCODE do_olc_version (char **arguments );

/* p_topic.c */
ERRCODE do_olc_topic (char **arguments );

/* p_utils.c */
ERRCODE handle_common_arguments(char ***args, REQUEST *req);
char **handle_argument (char **args , REQUEST *req , ERRCODE *status );

/* p_zephyr.c */
ERRCODE do_olc_zephyr (char **arguments );

#endif /* __olc_olc_parser_h */
