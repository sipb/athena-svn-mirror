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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_queue.c,v 1.1 1989-07-06 22:09:02 tjcoppet Exp $";
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
  char stati[NAME_LENGTH];
  char queues[NAME_LENGTH];
  char users[NAME_LENGTH];
  char topics[NAME_LENGTH];
  char instances[NAME_LENGTH];
  int i;
  
  stati[0] = '\0';
  queues[0] = '\0';
  users[0] = '\0';
  instances[0] = '\0';

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
		  strncat(queues,*arguments);
		}
	      if(*(arguments+1)[0] == '-')
		break;
	    }
	}

      if(string_equiv(*arguments,"-status",max(strlen(*arguments),2)))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    {
	      fprintf(stderr,
		      "You must specify a status after the -status option.\n");
	      return(ERROR);
	    }
	 
	  for(i=0; *arguments != (char *) NULL; arguments++)
	    {
	      if(strlen(*arguments) >= (NAME_LENGTH -i))
		fprintf(stderr,"Too many stati specified. Continuing...\n");
	      else
		{
		  strcat(stati," ");
		  strncat(stati,*arguments);
		}
	      if(*(arguments+1)[0] == '-')
		break;
	    }
	}

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
		fprintf(stderr,"Too many topics specified. Continuing...\n");
	      else
		{
		  strcat(topics," ");
		  strncat(topics,*arguments);
		}
	      if(*(arguments+1)[0] == '-')
		break;
	    }
	}

      /* 
       * some strange way is going to have to be devised to specify
       * multiple targets
       */

      arguments = handle_argument(arguments, &Request, &status);
      if(!status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  fprintf(stderr,"Usage is: \tlist [-display] [-queue <queues>] ");
	  fprintf(stderr,"[-topic <topics>] [-status <statuses>]\n");
	  fprintf(stderr,"\t\t[<username> <instance id> ...]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_list_queue(&Request,queues,topics,stati);
  return(status);
}
