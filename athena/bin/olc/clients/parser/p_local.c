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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_local.c,v $
 *	$Id: p_local.c,v 1.14 1992-02-14 19:11:36 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_local.c,v 1.14 1992-02-14 19:11:36 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
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

ERRCODE
do_quit(arguments)
     char *arguments[];
{
  REQUEST Request;
  LIST *list;
  int status;

  *arguments = (char *) NULL;
  
  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  status = OListPerson(&Request,&list);
  switch (status)
    {
    case SUCCESS:
      if(OLC)
	{
	  printf("To continue this question, just run this program again.  Remember, your\n");
	  printf("question is active until you use the 'done' or 'cancel' command.  It will be\n");
	  printf("stored until someone can answer it.  If you logout, someone may send you\n");
	  printf("mail.\n");
	}
 /* There is currently no way to tell, if you are connected, whether you */
 /* are connected to someone as a consultant or a user.  It would be */
 /* nice to warn consultants that they may still be connected to users...  */
      else
	{
#if 0
	  status = FALSE;
	  for(l=list; ((l->ustatus != END_OF_LIST) && (status == FALSE)); l++)
	    {
	      if ((l->nseen >= 0) && (l->connected.uid >= 0))
		status = TRUE;
	    }
	  if (status)
#endif
	    printf("Warning: you are still active in %s.  You may be signed on,\n", OLC_SERVICE_NAME);
            printf("connected to someone, or have a question of your own in the queue.\n");
	}

      free(list);
      break;

    case EMPTY_LIST:
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    default:
      fprintf(stderr, "An error occurered when trying to determine whether you were still\n");
      fprintf(stderr, "active in OLC:\n");
      status = handle_response(status, &Request);
      break;
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
  char help_filename[NAME_SIZE]; /* Name of help file. */
  int  ind;

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

      if ((ind = command_index(Command_Table, arguments[1])) == ERROR)
	{
	  printf("The command \"%s\" is not defined.  ", arguments[1]);
	  printf("For a list of commands, type \"?\".\n");
	  return(ERROR);
	}
      else
	if (ind == NOT_UNIQUE)
	  return(NOT_UNIQUE);

      (void) strcat(help_filename, Command_Table[ind].command_name);
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

