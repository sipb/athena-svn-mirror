/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions common to all parts of OLC.
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
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_parser.h,v $
 *      $Author: tjcoppet $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_parser.h,v 1.1 1989-07-07 13:22:29 tjcoppet Exp $
 */

#include <olc/olc_tty.h>

char **handle_argument();


#define MAX_ARGS         20             /* Maximum number of arguments. */
#define MAX_ARG_LENGTH   80             /* Maximum length of an argument. */
#define MAX_COMMANDS    100             /* Maximum number of commands. */
#define NOT_UNIQUE      -2              /* Not a unique command match. */

typedef struct tCOMMAND  {
        char            *command_name;          /* Name of the command. */
        FUNCTION        command_function;       /* Function to execute. */
        char            *description;           /* Brief description. */
} COMMAND;

