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
 *	$Id: p_ask.c,v 1.18 1999-07-30 18:25:41 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_ask.c,v 1.18 1999-07-30 18:25:41 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <sys/param.h>

ERRCODE
do_olc_ask(arguments)
     char **arguments;
{
  REQUEST  Request;
  ERRCODE status = SUCCESS;
  char topic[TOPIC_SIZE];
  char file[MAXPATHLEN];
  int i;

  topic[0]= '\0';
  file[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if(is_flag(*arguments, "-topic", 2))
	{
	  arguments++;
	  if((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      strcpy(topic,*arguments);
	      arguments++;
	      continue;
	    }
	  else
	    {
	      status = ERROR;
	      break;
	    }
	}
      if(is_flag(*arguments, "-file",2))
	{
	  arguments++;
	  if((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      strcpy(file, *arguments);
	      arguments++;
	      continue;
	    } 
	  else
	    {
	      status = ERROR;
	      break;
	    }
	}
      status = handle_common_arguments(&arguments, &Request);
      if(status == SUCCESS)
	continue; 
      else
	break;
    }

  if(status != SUCCESS)
    {
      if(client_is_user_client())
	fprintf(stderr, "Usage is: \task [-topic <topic>]\n");
      else
	{
	  fprintf(stderr, "Usage is: \task [-topic <topic>] "
		  "[<username> <instance id>]\n"
		  "\t\t[-instance <instance id]>\n"
		  "\t\t[-file <filename>]\n");
	}
      return ERROR;
    }

  if(topic[0] == '\0')
    t_input_topic(&Request,topic,TRUE);

  status = t_ask(&Request,topic,file);

  if(client_is_user_client())
    {
      printf("\nSome other useful %s commands are: \n\n", client_service_name());
      printf("\tsend  - send a message\n");
      printf("\tshow  - show new messages\n");
      printf("\tdone  - mark your question resolved\n");
      printf("\tquit  - exit %s, leaving your question active\n",
	     client_service_name());
      if (client_has_hours())
	printf("\thours - Find hours %s is staffed\n", client_service_name());
      printf("\t?     - see entire listing of commands\n");
    }
  return status;
}
