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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_ask.c,v 1.6 1990-04-25 16:38:26 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_ask(arguments)
     char **arguments;
{
  REQUEST  Request;
  int status = 0;
  char topic[TOPIC_SIZE];

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  topic[0]= '\0';

  if(arguments != (char **) NULL)
    {
      for (arguments++; *arguments != (char *) NULL; arguments++)
        {
          if(string_equiv(*arguments, "-topic", max(strlen(*arguments),2)))
            {
	      ++arguments;
              if(*arguments != (char *) NULL)
                {
                  (void) strcpy(topic,*arguments);
                  status = 1;
                }
	      else
		break;
            }
          else
            {
              arguments = handle_argument(arguments, &Request, &status);
	      if(status)
		return(ERROR);
              if(arguments == (char **) NULL) 
		{
		  if(OLC)
		    printf("Usage is: \task [-topic <topic>]\n");
		  else
		    {
		      printf("Usage is: \task [-topic <topic>] ");
		      printf("[<username> <instance id>]\n");
		      printf("\t\t[-instance <instance id]>\n");
		    }
		  
		  return(ERROR);
		}
              if(*arguments == (char *) NULL)   /* end of list */
                break;
            }
        }
    }

  if(!status)
    t_input_topic(&Request,topic,TRUE);

  status = t_ask(&Request,topic);
  if(OLC)
    {
      printf("\nSome other useful OLC commands are: \n\n");
      printf("\tsend  - send a message\n");
      printf("\tshow  - show new messages\n");
      printf("\tdone  - mark your question resolved\n");
      printf("\t?     - see entire listing of commands\n");
    }
  return(status);
}
