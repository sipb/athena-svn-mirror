/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the primary command loop for olc and olcr.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_cmdloop.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_cmdloop.c,v 1.3 1990-01-17 05:41:53 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <signal.h>
#include <ctype.h>

int subsystem = 0;		/* (initial value) */
extern char **environ;

/*
 * Function:	command_loop() is the driver loop for olc and olcr.
 * Arguments:	Command_Table:	The command table for the appropriate
 *			program.  It contains command names and functions
 *			to be executed for each one.
 *		prompt:		The prompt to be used.
 * Returns:	Nothing.  Normally, it does not return, but exits via
 *			the "quit" function.
 * Notes:
 *	command_loop() loops forever, executing the following general steps:
 *		1.  Get an input line from the user.
 *		2.  Strip leading whitespace from the line.
 *		2.  If it is non-null, parse the command line into
 *			individual words.
 *		3.  Set up an array of pointers to those words.
 *		4.  Execute the command.
 */

command_loop(Command_Table, prompt)
     COMMAND Command_Table[];
     char *prompt;
{
  REQUEST Request;
  char command_line[COMMAND_LENGTH]; 
  char *comm_ptr;		
  char *arguments[MAX_ARGS];
  char arglist[MAX_ARGS][MAX_ARG_LENGTH];	
  int i,pid,ls=0;
  int (*func)();
  char buf[BUF_SIZE];
#ifdef CSH
  struct stat statbuf;
  char configfile[128];
#endif CSH
  
  subsystem = 1;
  
#ifdef CSH
  strcpy(configfile,(char *) getenv("HOME"));
  strcat(configfile,"/");
  strcat(configfile,".olc.cshrc");
  if(stat(configfile, &statbuf) == 0) 
    {
      arguments[0] = arglist[0];
      arguments[1] = (char *) NULL;
      strcpy(arglist[0],"csh");
      sh_main(1,arguments);
    } 
  else 
    {
#endif CSH

      while(1) 
	{
	  ls = 0;
	  fill_request(&Request);
	  comm_ptr = command_line;
	  set_prompt(&Request,buf,prompt);
	  get_prompted_input(buf, command_line);
	  while (isspace(*comm_ptr) && (*comm_ptr != '\0'))
	    comm_ptr++;
	  if (*comm_ptr == '\0')
	    continue;
	  if(*comm_ptr == '!')
	    {
	      comm_ptr++;
	      ls = TRUE;
	    }
	  parse_command_line(comm_ptr, arglist);
	  i = 0;
	  while (*arglist[i] != '\0') 
	    {
	      arguments[i] = arglist[i];
	      i++;
	    }
	  arguments[i] = (char *) NULL;
	  expand_arguments(&Request,arguments);
	  if(ls)
	    {
	      i = 0;
	      *command_line = '\0';
	      while(*arglist[i] != '\0') 
		{
		  strcat(command_line,arglist[i]);
		  strcat(command_line," ");
		  i++;
		}
	      system(command_line);
	    }
	  else
	    (void) do_command(Command_Table, arguments);
	}

#ifdef CSH
   }
#endif CSH
}

/*
 * Function:	do_command() executes the appropriate function based
 *			on the command line.
 * Arguments:	Command_Table:	The table matching names and functions.
 *		arguments:	The argument list to be passed to the function.
 * Returns:	SUCCESS	if command is successfully executed,
 *		ERROR if command is not defined,
 *		FAILURE if command name is not unique.
 * Notes:
 *	First, look up the command in the command table.  If it is not
 *	unique, return FAILURE.  If it is not a legal command, print an
 *	error message and return ERROR.  Otherwise, execute the appropriate
 *	function with the given argument list, and return SUCCESS.
 */

do_command(Command_Table, arguments)
     COMMAND Command_Table[];
     char *arguments[];
{
  int index;		    /* Index in the command table. */
	
#ifdef CSH
  if(*arguments[0] == '\\') /* want to escape olc command */
    return(-1);
#endif CSH

  if(string_eq(arguments[0],"answers")&&!OLC)
    strcpy(arguments[0],"stock");

  index = command_index(Command_Table, arguments[0], NULL);
  
  if (index == NOT_UNIQUE)
    return(FAILURE);
  else 
    if (index == ERROR) 
      {
	printf("The command '%s' is not defined.  ", arguments[0]);
	printf("Please use the 'help' request if\nyou need help.\n");

#ifdef CSH
	return(-1);
#else  CSH
	return(ERROR);
#endif CSH
      }
    else 
      {
	(*(Command_Table[index].command_function))(arguments);
	return(SUCCESS);
      }
}

/*
 * Function:	command_index() scans through a command table to find the
 *			index of the function associated with the given name.
 * Arguments:	Command_Table:	A table of names and corresponding functions.
 *		command_name:	Name of the command to be found.
 * Returns:	The index of the command if a single match is found,
 *		NOT_UNIQUE if more than one matche is found,
 * 		ERROR if no matches are found.
 * Notes:
 *	Loop through the command table, comparing the command name with the
 *	ones in the table.  We check only as many characters as the command
 *	name has, so any unique specifier can be used for a command.  Each
 *	time a match is found, record its index in an array, and increment
 *	a counter.  If more than one match is found, print the possible
 *	commands for the user.  If one match is found, return its index.
 *	Otherwise, return an ERROR.
 */

ERRCODE
command_index(Command_Table, command_name, ret)
     COMMAND Command_Table[];
     char *command_name;
     char *ret;
{
  int index;		      /* Index in table. */
  int matches[MAX_COMMANDS];  /* Matching commands. */
  int match_count;	      /* Number of matches. */
  int comm_length;	      /* Length of command. */
  
  index = 0;
  match_count = 0;
  if (command_name == (char *)NULL)
    return(ERROR);
	
  comm_length = strlen(command_name);
  while (Command_Table[index].command_name != (char *) NULL) 
    {
      if (!strncmp(command_name, Command_Table[index].command_name,
		   comm_length))
	matches[match_count++] = index;
      index++;
      if(ret != (char *) NULL)
	strcpy(ret, Command_Table[index].command_name);
    }
  
  if (match_count == 0)
    return(ERROR);
  else 
    if (match_count == 1)
      return(matches[0]);
    else 
      {
	printf("Could be one of:\n");
	for (index = 0; index < match_count; index++)
	  printf("\t%s\n", Command_Table[matches[index]].command_name);
	return(NOT_UNIQUE);
      }
}


expand_arguments(Request,arguments)
     REQUEST *Request;
     char **arguments;
{
  char **arg;
  int i;

  for(arg = arguments; *arg != (char *) NULL; ++arg)
    {
      for(i=0; (*arg)[i] != '\0'; i++)
	{
	  if((*arg)[i] == '$')
	    expand_variable(Request,&(*arg)[i]);
	}
    }
}

expand_variable(Request,var)
     REQUEST *Request;
     char *var;
{
  if(string_eq(var, "$cuser"))
    OGetConnectedUsername(Request, var);
  if(string_eq(var, "$chost"))
    OGetConnectedHostname(Request, var);
  if(string_eq(var, "$host"))
    OGetHostname(Request, var);
  if(string_eq(var, "$user"))
    OGetUsername(Request, var);
  if(string_eq(var, "$topic"))
    OGetTopic(&Request, var);
  if(string_eq(var, "$snyde"))
    strcpy(var,"Go away.");
}


set_prompt(Request,prompt,format)
     REQUEST *Request;
     char *prompt;
     char *format;
{
  char buf[BUF_SIZE];

  *prompt = '\0';
  while(*format != '\0')
    {
      if(*format == '%')
	{
	  ++format;
	  expand_variable(Request,(format = (char *) get_next_word(format)), buf);
	  strcat(prompt,buf);
	}
      else
	strncat(prompt, format, 1);
      ++format;
    }
}


