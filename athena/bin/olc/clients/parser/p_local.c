/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_local.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_local.c,v 1.1 1989-11-17 14:07:41 tjcoppet Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

extern COMMAND *Command_Table;		
extern char *HELP_FILE, *HELP_DIR, *HELP_EXT;

/*
 * Function:	do_quit() exits from OLC.
 * Arguments:	arguments:	The argument array from the parser.
 * Returns:	Doesn't return.
 * Notes:
 *	Exits from OLC with status 0.
 */

do_quit(arguments)
     char *arguments[];
{
  REQUEST *Request;
  LIST list;
  int status;
#ifdef lint
  *arguments = (char *) NULL;
#endif lint
  
    
  if(OLC)
    {
      printf("To continue this question, just run this program again. Remember, your\n");
      printf("question is active until you use the 'done' or 'cancel' command.  It will be\n");
      printf("stored until someone can answer it. If you logout, someone may send you\n");
      printf("mail.\n");
    }
  exit(0);
}

/*
 * Function:	do_olc_help() prints information about olc commands.
 * Arguments:	arguments:	The argument array from the parser.
 *			arguments[1]:	Help topic.  If this is NULL,
 *			print the general help file.
 * Returns:	An error code.
 * Notes:
 *	If there is no argument, a brief explanation of olc is printed.
 *	Otherwise, the filename of the appropriate help file is constructed,
 *	and then printed on the terminal.
 */

ERRCODE
do_olc_help(arguments)
	char *arguments[];
{
  char help_filename[NAME_LENGTH]; /* Name of help file. */
  char ret[LINE_LENGTH];

  if (arguments[1] == (char *)NULL) 
    {
      (void) strcpy(help_filename, HELP_DIR);
      (void) strcat(help_filename, "/");
      (void) strcat(help_filename, HELP_FILE);
    }
  else 
    {
      (void) strcpy(help_filename, HELP_DIR);
      (void) strcat(help_filename, "/");
      
      if (command_index(Command_Table, arguments[1], ret) == -1) 
	{
	  printf("The command \"%s\" is not defined.  ", ret);
	  printf("For a list of commands, type \"?\"\n");
	  return(ERROR);
	}
      else 
	(void) strcat(help_filename, ret);
    }
  (void) strcat(help_filename, HELP_EXT);
  return(display_file(help_filename));
}


/*
 * Function:	do_olc_list_cmds() prints out a list of commands and
 *			brief descriptions of them.
 * Arguments:	arguments:	The argument array from the parser.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Loop through the command table printing the command name and
 *	the corresponding description.
 */

ERRCODE
do_olc_list_cmds(arguments)
     char *arguments[];
{
  int i;

  for (i = 0; Command_Table[i].command_name != (char *) NULL; i++)
    printf("%s%s%s\n", Command_Table[i].command_name,
           strlen(Command_Table[i].command_name) > 7 ? "\t" : "\t\t",
	   Command_Table[i].description);
  return(SUCCESS);
}

