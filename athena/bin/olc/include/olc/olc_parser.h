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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_parser.h,v $
 *	$Id: olc_parser.h,v 1.5 1990-05-25 15:17:33 vanharen Exp $
 *	$Author: vanharen $
 */

#include <mit-copyright.h>
#include <olc/olc_tty.h>

char **handle_argument();


#define MAX_ARGS         20             /* Maximum number of arguments. */
#define MAX_ARG_LENGTH   80             /* Maximum length of an argument. */
#define MAX_COMMANDS    100             /* Maximum number of commands. */
#define NOT_UNIQUE      -2              /* Not a unique command match. */

/* These really don't return anything useful.  */
#if 0
typedef void (*Pfunction) (const char **);
#else
typedef int (*Pfunction)OPrototype((char **));
#endif

typedef struct tCOMMAND  {
        char            *command_name;          /* Name of the command. */
        Pfunction       command_function;       /* Function to execute. */
        char            *description;           /* Brief description. */
} COMMAND;

