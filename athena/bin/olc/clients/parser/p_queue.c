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
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_queue.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_queue.c,v 1.5 1989-08-08 14:34:45 tjcoppet Exp $";
#endif


#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:	do_olc_list() displays the current questions and their
 *			status.
 * Arguments:	arguments:	The argument array from the parser.
 * Returns:	An error code.
 * Notes:
 *	Send an OLC_LIST request to the daemon and read back the text,
 *	if the request is successful.
 */

ERRCODE
do_olc_list(arguments)
     char **arguments;
{
  REQUEST Request;
  int  status;
  int  stati = 0;
  char queues[NAME_LENGTH];
  char users[NAME_LENGTH];
  char topics[NAME_LENGTH];
  char instances[NAME_LENGTH];
  int comments = 0;
  int i,mask;
  
  queues[0] = '\0';
  users[0] = '\0';
  instances[0] = '\0';
  topics[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if(string_equiv(*arguments,"-queue",max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      fprintf(stderr,
		      "You must specify a queue after the -queue option.\n");
	      return(ERROR);
	    }
	 
	  for(i=0; *arguments != (char *) NULL; arguments++)
	    {
	      if(strlen(*arguments) >= (NAME_LENGTH -i))
		fprintf(stderr,"Too many queues specified. Continuing...\n");
	      else
		{
		  strcat(queues," ");
		  strncat(queues,*arguments,NAME_LENGTH-1);
		  break;
		}
	      if(*(arguments+1)[0] == '-')
		break;
	    }
	}
      else
	if(string_equiv(*arguments,"-status",max(strlen(*arguments),2)))
	  {
	    ++arguments;
	    if(*arguments == (char *) NULL)
	      OGetStatusCode(*arguments, &stati);
	    else
	      for(i=0; *arguments != (char *) NULL; arguments++)
		{
		  OGetStatusCode(*arguments, &mask);
		  if(mask == 0)
		    {
		      printf("Invalid status label specified. Choose one of...\n");
		      t_pp_stati();
		      return(ERROR);
		    }
		  else
		    stati |= mask;
		  if(*(arguments+1)[0] == '-')
		    break;
		}
	  }
	else
	  if(string_equiv(*arguments,"-user",max(strlen(*arguments),2)))
	    {
	      ++arguments;
	        if(*arguments == (char *) NULL)
		{
		  fprintf(stderr,
			"You must specify something after the -user option.\n");
		  return(ERROR);
		}
	       strncpy(users,*arguments,NAME_LENGTH-1);
	    }
	  else
	    if(string_equiv(*arguments,"-topic",max(strlen(*arguments),2)))
	      {
		++arguments;
		if(*arguments == (char *) NULL)
		  {
		    fprintf(stderr,
			    "You must specify a topic after the -topic option.\n");
		    return(ERROR);
		  }
		for(i=0; *arguments != (char *) NULL; arguments++)
		  {
		    if(strlen(*arguments) >= (NAME_LENGTH -i))
		      fprintf(stderr,
			      "Too many topics specified. Continuing...\n");
		    else
		      {
			strncpy(topics,*arguments,NAME_LENGTH-1);
			break;
		    }
		    if(*(arguments+1)[0] == '-')
		      break;
		}
	    }
      
      /* 
       * some strange way is going to have to be devised to specify
       * multiple targets
       */
	  else
	    if(string_equiv(*arguments,"-comments",max(strlen(*arguments),2))||
	       string_equiv(*arguments,"-long",max(strlen(*arguments),2)))
	      comments = TRUE;
	    else
	      {
		arguments = handle_argument(arguments, &Request, &status);
		if(status)
		  return(ERROR);
	      }

      if(arguments == (char **) NULL)   /* error */
	{
	  fprintf(stderr,"Usage is: \tlist [-display] [-queue <queues>] ");
	  fprintf(stderr,"[-topic <topic>]\n\t\t[-status <statuses>] ");
	  fprintf(stderr,"[-comments] ");
	  fprintf(stderr,"[<username pattern>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_list_queue(&Request,queues,topics,users,stati,comments);
  return(status);
}
