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
 *	$Id: olc_parser.h,v 1.14 1999-01-22 23:13:43 ghudson Exp $
 */


#ifndef __olc_olc_parser_h
#define __olc_olc_parser_h

/* These really don't return anything useful.  */
typedef ERRCODE (*Pfunction)OPrototype((char **));

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

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

/* p_acl.c */
ERRCODE do_olc_acl P((char **arguments ));

/* p_ask.c */
ERRCODE do_olc_ask P((char **arguments ));

/* p_cmdloop.c */
void command_loop P((COMMAND Command_Table [], char *prompt ));
ERRCODE do_command P((COMMAND Command_Table [], char *arguments []));
ERRCODE command_index P((COMMAND Command_Table [], char *command_name ));
char *expand_variable P((REQUEST *Request , char *var ));
int expand_arguments P((REQUEST *Request , char **arguments ));
int set_prompt P((REQUEST *Request , char *prompt , char *inprompt ));
ERRCODE parse_command_line P((char *command_line , char arguments [MAX_ARGS ][MAX_ARG_LENGTH ]));

/* p_connect.c */
ERRCODE do_olc_grab P((char **arguments ));
ERRCODE do_olc_forward P((char **arguments ));

/* p_consult.c */
ERRCODE do_olc_on P((char **arguments ));
ERRCODE do_olc_off P((char **arguments ));

/* p_describe.c */
ERRCODE do_olc_describe P((char **arguments ));

/* p_instance.c */
ERRCODE do_olc_instance P((char **arguments ));

/* p_list.c */
ERRCODE do_olc_list P((char **arguments ));

/* p_local.c */
ERRCODE do_quit P((char *arguments []));
ERRCODE do_olc_help P((char *arguments []));
ERRCODE do_olc_list_cmds P((char *arguments []));

/* p_messages.c */
ERRCODE do_olc_replay P((char **arguments ));
ERRCODE do_olc_show P((char **arguments ));

/* p_misc.c */
ERRCODE do_olc_load_user P((char **arguments ));
ERRCODE do_olc_dbinfo P((char **arguments ));

/* p_motd.c */
ERRCODE do_olc_motd P((char **arguments ));
ERRCODE do_olc_hours P((char **arguments ));

/* p_queue.c */
ERRCODE do_olc_queue P((char **arguments ));

/* p_resolve.c */
ERRCODE do_olc_done P((char **arguments ));
ERRCODE do_olc_cancel P((char **arguments ));

/* p_send.c */
ERRCODE do_olc_send P((char **arguments ));
ERRCODE do_olc_comment P((char **arguments ));
ERRCODE do_olc_mail P((char **arguments ));

/* p_status.c */
ERRCODE do_olc_status P((char **arguments ));
ERRCODE do_olc_who P((char **arguments ));
ERRCODE do_olc_version P((char **arguments ));

/* p_topic.c */
ERRCODE do_olc_topic P((char **arguments ));

/* p_utils.c */
char **handle_argument P((char **args , REQUEST *req , int *status ));

/* p_zephyr.c */
ERRCODE do_olc_zephyr P((char **arguments ));

#undef P

#endif /* __olc_olc_parser_h */
