/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for resolving questions.
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
 *	$Id: p_resolve.c,v 1.16 1999-06-28 22:52:09 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_resolve.c,v 1.16 1999-06-28 22:52:09 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>


/*
 * Function:    do_olc_done() marks a question as resolved.
 * Arguments:   arguments:      The argument array from the parser.
 *                  arguments[1] may be "-off".
 * Returns:     An error code.
 * Notes:
 *      First, make sure that the consultant is connected to a user.
 *      If so, send an OLC_DONE request to the daemon, then handle the
 *      response.  If the first argument is "-off", sign the consultant
 *      off of OLC instead of connecting her to another user.  The
 *      consultant is prompted for a one-line description of the
 *      question to be used in the conversation log.
 */



ERRCODE
do_olc_done(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;
  char topic[TOPIC_SIZE];
  char title[LINE_SIZE];
  char *titleP = (char *) NULL;
  int check = 1;

  topic[0] = '\0';
  title[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if(is_flag(*arguments,"-topic",3))
	{
	  arguments++;
	  if(*arguments == (char *) NULL)
	    status = t_input_topic(&Request,topic,FALSE);
	  else
	    {
	      strncpy(topic,*arguments,TOPIC_SIZE);
	      status = t_input_topic(&Request,topic,FALSE);
	    }
	  
	  if(status!= SUCCESS)
	    return(status);

	  status = t_change_topic(&Request, topic);
	  if(status != SUCCESS)
	    {
	      printf("Error changing topic, question not resolved.\n");
	      return(status);
	    }
	  continue;
	}

      if(is_flag(*arguments,"-title",3))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    get_prompted_input("Title: ", title,LINE_SIZE,0);
	  else
	    strncpy(title,*arguments,LINE_SIZE);
	  continue;
	}

      if(is_flag(*arguments,"-off",2))
	{
	  set_option(Request.options,OFF_OPT);
	  continue;
	}

      if(is_flag(*arguments,"-nocheck",2))
	{
	  check = 0;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \tdone\n");
	  else
	    {
	      printf("Usage is: \tdone  [<username> <instance id>] ");
	      printf("[-off] ");
	      printf("[-title <title>]\n\t\t[-topic <topic>] ");
	      printf("[-instance <instance id>]\n");
	      printf("\t\t[-nocheck] ");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  if(title[0] != '\0')
    titleP = &title[0];
  
  status = t_done(&Request, titleP,check);
  return(status);
}






ERRCODE
do_olc_cancel(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;
  char title[LINE_SIZE];
  char *titleP = (char *) NULL;

  title[0] = '\0';

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if(is_flag(*arguments,"-title",3))
	{
	  ++arguments;
	  if(*arguments == (char *) NULL)
	    get_prompted_input("Title: ", title, LINE_SIZE,0);
	  else
	    strncpy(title,*arguments,LINE_SIZE);
	  continue;
	}

      if(is_flag(*arguments,"-off",2))
	{
	  set_option(Request.options,OFF_OPT);
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \tcancel\n");
	  else
	    {
	      printf("Usage is: \tcancel [<username> <instance id>] ");
	      printf("[-title <title>] [-off]\n");
	      printf("\t\t[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
        break;
    }

  if(title[0] != '\0')
      titleP = &title[0];
  status = t_cancel(&Request,titleP);
  return(status);
}



