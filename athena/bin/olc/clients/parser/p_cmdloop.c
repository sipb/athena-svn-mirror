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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_cmdloop.c,v $
 *	$Id: p_cmdloop.c,v 1.16 1991-09-10 11:27:40 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_cmdloop.c,v 1.16 1991-09-10 11:27:40 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#if defined(__STDC__) && !defined(ibm032)
#include <stdlib.h>
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <signal.h>
#include <ctype.h>
#if defined(_AUX_SOURCE) || defined(_POSIX_SOURCE)
#include <time.h>
#endif

int subsystem = 0;		/* (initial value) */
extern char **environ;
extern char *wday[]; 
extern char *month[];

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

void
command_loop(Command_Table, prompt)
     COMMAND Command_Table[];
     char *prompt;
{
  REQUEST Request;
  char command_line[COMMAND_LENGTH]; 
  char *comm_ptr;		
  char *arguments[MAX_ARGS];
  char arglist[MAX_ARGS][MAX_ARG_LENGTH];	
  int i,ls=0;
  char buf[BUF_SIZE];
  subsystem = 1;
  
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
  return;
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

ERRCODE
do_command(Command_Table, arguments)
     COMMAND Command_Table[];
     char *arguments[];
{
  int ind;		    /* Index in the command table. */
  int status;

  ind = command_index(Command_Table, arguments[0]);
  
  if (ind == NOT_UNIQUE)
    return(FAILURE);
  else 
    if (ind == ERROR) 
      {
	printf("The command '%s' is not defined.  ", arguments[0]);
	printf("Please use the 'help' request if\nyou need help.\n");
	return(ERROR);
      }
    else 
      {
	status = (*(Command_Table[ind].command_function))(arguments);
	return(status);
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
command_index(Command_Table, command_name)
     COMMAND Command_Table[];
     char *command_name;
{
  int ind;		      /* Index in table. */
  int matches[MAX_COMMANDS];  /* Matching commands. */
  int match_count;	      /* Number of matches. */
  int comm_length;	      /* Length of command. */
  
  ind = 0;
  match_count = 0;
  if (command_name == (char *)NULL)
    return(ERROR);
	
  comm_length = strlen(command_name);
  while (Command_Table[ind].command_name != (char *) NULL) 
    {
      if (!strncmp(command_name, Command_Table[ind].command_name,
		   comm_length))
	matches[match_count++] = ind;
      ind++;
    }
  
  if (match_count == 0)
    return(ERROR);
  else 
    if (match_count == 1)
      return(matches[0]);
    else 
      {
	printf("Could be one of:\n");
	for (ind = 0; ind < match_count; ind++)
	  printf("\t%s\n", Command_Table[matches[ind]].command_name);
	return(NOT_UNIQUE);
      }
}


static char *local[] =
{
  "inst", "username", "realname",
  "machine", "user", /** "nickname", "title", **/
};
#define NUML (sizeof(local)/sizeof(char *))

static char *server[] = 
{
  "cinst", "cusername", "crealname",
  "cmachine", "cuser",
  "ustatus", "cstatus", "nseen", "topic",
  /** "cnickname", "ctitle", **/
};
#define NUMS (sizeof(server)/sizeof(char *))

static char *timestuff[] =
{
  "dayname", "monthname", "MON", "mon", "DAY", "day", "YEAR", "year",
  "milhour", "HOUR", "hour", "min", "sec", "AMPM", "ampm",
};
#define NUMT (sizeof(timestuff)/sizeof(char *))

char *
expand_variable(Request,var)
     REQUEST *Request;
     char *var;
{
  int uinfo, sinfo, tinfo;
  long time_now;
  struct tm *time_info;
  char buf[BUF_SIZE];
  LIST list;

  for (uinfo=0; uinfo < NUML; uinfo++)
    {
      if (string_eq(var, local[uinfo]))
	  {
	    break;
	  }
    }
  if (uinfo != NUML)
    {
      switch(uinfo)
	{
	case 0:			/* instance */
	  sprintf(buf, "%d", User.instance);
	  break;
	case 1:			/* username */
	  sprintf(buf, "%s", User.username);
	  break;
	case 2:			/* realname */
	  sprintf(buf, "%s", User.realname);
	  break;
	case 3:			/* machine */
	  sprintf(buf, "%s", User.machine);
	  break;
	case 4:			/* USER (username@machine) */
	  sprintf(buf, "%s@%s", User.username, User.machine);
	  break;
#if 0
/** Can't do these, yet. **/
	case 5:			/* nickname */
	  sprintf(buf, "%s", Request->requester.nickname);
	  break;
	case 6:			/* title */
	  sprintf(buf, "%s", Request->requester.title);
	  break;
#endif
	default:		/* this shouldn't happen, but just in case */
	  strcpy(buf, "ERROR");
	}
      return(buf);
    }

  for (sinfo=0; sinfo < NUMS; sinfo++)
    {
      if (string_eq(var, server[sinfo]))
	  {
	    break;
	  }
    }
  if (sinfo != NUMS)
    {
      OWho(Request, &list);
      switch(sinfo)
	{
	case 0:			/* c_instance */
	  if (list.connected.uid == -1)
	    strcpy(buf, "");
	  else
	    sprintf(buf, "%d", list.connected.instance);
	  break;
	case 1:			/* c_username */
	  if (list.connected.uid == -1)
	    strcpy(buf, "");
	  else
	    sprintf(buf, "%s", list.connected.username);
	  break;
	case 2:			/* c_realname */
	  sprintf(buf, "%s", list.connected.realname);
	  break;
	case 3:			/* c_machine */
	  sprintf(buf, "%s", list.connected.machine);
	  break;
	case 4:			/* c_user (username@machine) */
	  if (list.connected.uid == -1)
	    strcpy(buf, "");
	  else
	    sprintf(buf, "%s@%s", list.connected.username,
		    list.connected.machine);
	  break;
	case 5:			/* ustatus */
	  OGetStatusString(list.ustatus, buf);
	  break;
	case 6:			/* cstatus */
	  OGetStatusString(list.cstatus, buf);
	  break;
	case 7:			/* nseen */
	  if (list.nseen == -1)
	    strcpy(buf, "");
	  else
	    sprintf(buf, "%d", list.nseen);
	  break;
	case 8:			/* topic */
	  sprintf(buf, "%s", list.topic);
	  break;
#if 0
	  /** Can't do these, yet. **/
	case 9:			/* nickname */
	  sprintf(buf, "%s", Request->requester.nickname);
	  break;
	case 10:		/* title */
	  sprintf(buf, "%s", Request->requester.title);
	  break;
#endif
	default:		/* this shouldn't happen, but just in case */
	  strcpy(buf, "ERROR");
	}
      return(buf);
    }

/*  
  if(string_eq(var, "cuser"))
    OGetConnectedUsername(Request, var);
  if(string_eq(var, "chost"))
    OGetConnectedHostname(Request, var);
  if(string_eq(var, "host"))
    OGetHostname(Request, var);
  if(string_eq(var, "user"))
    OGetUsername(Request, var);
  if(string_eq(var, "topic"))
    OGetTopic(&Request, var);
*/

/*
 *  Display any requested time strings...
 */

  for (tinfo=0; tinfo < NUMT; tinfo++)
    {
      if (string_eq(var, timestuff[tinfo]))
	  {
	    break;
	  }
    }

  if (tinfo != NUMT)
    {
      (void) time(&time_now);
      time_info = localtime(&time_now);

      switch(tinfo)
	{
	case 0:			/* dayname */
	  sprintf(buf, "%s", wday[time_info->tm_wday]);
	  break;
	case 1:			/* monthname */
	  sprintf(buf, "%s", month[time_info->tm_mon]);
	  break;
	case 2:			/* MON (month number) */
	  sprintf(buf, "%2.2d", time_info->tm_mon + 1);
	  break;
	case 3:			/* mon (month #, with leading space) */
	  sprintf(buf, "%d", time_info->tm_mon + 1);
	  break;
	case 4:			/* DAY (day of month) */
	  sprintf(buf, "%2.2d", time_info->tm_mday);
	  break;
	case 5:			/* day (day of month, with leading space*/
	  sprintf(buf, "%d", time_info->tm_mday);
	  break;
	case 6:			/* YEAR (full year number) */
	  sprintf(buf, "%4d", time_info->tm_year + 1900);
	  break;
	case 7:			/* year (year number, century srtipped) */
	  sprintf(buf, "%2d", time_info->tm_year);
	  break;
	case 8:			/* milhour (24 hour time) */
	  sprintf(buf, "%2.2d", time_info->tm_hour);
	  break;
	case 9:			/* HOUR (hour number) */
	  sprintf(buf, "%2.2d", ((time_info->tm_hour > 12)
				 ? (time_info->tm_hour - 12)
				 : ((time_info->tm_hour == 0)
				    ? 12
				    : time_info->tm_hour)));
	  break;
	case 10:		/* hour (hour number, with leading  space) */
	  sprintf(buf, "%d", ((time_info->tm_hour > 12)
			       ? (time_info->tm_hour - 12)
			       : ((time_info->tm_hour == 0)
				  ? 12
				  : time_info->tm_hour)));
	  break;
	case 11:		/* min (minute) */
	  sprintf(buf, "%2.2d", time_info->tm_min);
	  break;
	case 12:		/* sec (seconds) */
	  sprintf(buf, "%2.2d", time_info->tm_sec);
	  break;
	case 13:		/* AMPM ("AM" or "PM") */
	  sprintf(buf, "%s", (time_info->tm_hour) > 12 ? "PM" : "AM");
	  break;
	case 14:		/* ampm ("am" or "pm") */
	  sprintf(buf, "%s", (time_info->tm_hour) > 12 ? "pm" : "am");
	  break;
	default:		/* this shouldn't happen, but just in case */
	  strcpy(buf, "ERROR");
	}
      return(buf);
    }
  return("UNKNOWN");
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

set_prompt(Request, prompt, inprompt)
     REQUEST *Request;
     char *prompt;
     char *inprompt;
{
  char buf[BUF_SIZE];
  char format[BUF_SIZE];
  char variable[BUF_SIZE];
  char *buf2;

  format[0] = '\0';
  variable[0] = '\0';
  *prompt = '\0';
  while(*inprompt != '\0')
    {
      if (*inprompt == '%')
	{
	  if (*(inprompt+1) == '%')
	    {
	      strcat(prompt, "%");
	      inprompt += 2;
	    }
	  else
	    {
	      while(!isalpha(*inprompt))
		{
		  strncat(format, inprompt, 1);
		  ++inprompt;
		}
	      strcat(format, "s");
	      inprompt = (char *) get_next_word(inprompt, variable, IsAlpha);
	      buf2 = expand_variable(Request, variable);
	      sprintf(buf, format, buf2);
	      strcat(prompt, buf);
	      strcpy(format, "");
	    }
	}
      else
	{
	  strncat(prompt, inprompt, 1);
	  ++inprompt;
	}
    }
}

/*
 * Function: parse_command_line() parses a command line into individual words.
 * Arguments:	command_line:	A pointer to the command line string.
 *		arguments:	A pointer to an array that will contain the
 *				parsed arguments.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Get each word in the command line and set up a pointer to it.
 *      Kludge it so phrases can be quoted.
 */

ERRCODE
parse_command_line(command_line, arguments)
     char *command_line;
     char arguments[MAX_ARGS][MAX_ARG_LENGTH];
{
  char *current;		/* Current place in the command line.*/
  int argcount;		        /* Running counter of arguments. */
  char *get_next_word();	/* Get the next word in a line. */
  char buf[BUF_SIZE];
  char *bufP;
  int quote = 0;
  int first = 1;

  current = command_line;
  argcount = 0;
  while ((current = get_next_word(current, buf, NotWhiteSpace))
	 != (char *) NULL)
    {
      if(buf[0] == '\"')
	{
	  quote = TRUE;
	  bufP = &buf[1];
	}
      else
	bufP = &buf[0];

      if(quote)
	{
	  if(bufP[strlen(bufP)-1] == '\"')
	    {
	      bufP[strlen(bufP)-1] = '\0';
	     (void) strcat(arguments[argcount],bufP);
	      quote = FALSE;
	      first = 1;
	      ++argcount;
	    }
	  else
	    {
	      if(first)
		{
		  (void) strcpy(arguments[argcount],bufP);
		  first = 0;
		}
	      else
		(void) strcat(arguments[argcount],bufP);

	      (void) strcat(arguments[argcount]," ");
	    }
	}
      else
	{
	  (void) strcpy(arguments[argcount],bufP);
	  argcount++;
	}
    }
  
  *arguments[argcount] = '\0';
  return(SUCCESS);
}

