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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_connect.c,v 1.16 1999-07-30 18:26:09 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_connect.c,v 1.16 1999-07-30 18:26:09 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:    do_olc_grab() grabs a pending question.
 * Arguments:   arguments:      The argument array from the parser.
 * Returns:     SUCCESS or ERROR.
 * Notes:
 */

ERRCODE
do_olc_grab(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status = SUCCESS;
  int flag = 0;
  int hold = 0;

  if(fill_request(&Request) != SUCCESS)
     return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if(is_flag(*arguments,"-create_new_instance", 2))
	{
	  flag = 1;
	  arguments++;
	  continue;
	}

      if(is_flag(*arguments,"-do_not_change_instance",2))
	{
	  hold = 1;
	  arguments++;
          continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	{
	  break;
	}
    }

  if(status != SUCCESS)
    {
      fprintf(stderr,"Usage is: \tgrab [<username> <instance id>] "
	      "[-create_new_instance]\n"
	      "\t\t[-do_not_change_instance] [-instance <instance id>]\n");
      return ERROR;
    }

  status = t_grab(&Request,flag,hold);
  return status;
}

/*
 * Function:	do_olc_forward() forwards the question to another consultant.
 * Arguments:	arguments:	The argument array from the parser.
 *		    arguments[1] may be "-unans(wered)" or "-off".
 * Returns:	An error code.
 * Notes:
 *	First, we send an OLC_FORWARD request to the daemon.  This should
 *	put the question back on the queue for another consultant to answer.
 *	If the first argument is "-unanswered", the question is forwarded
 *	directly tothe log as an unanswered question.  If the first
 *	argument is "-off", the consultant is signed off of OLC instead of
 *	being connected to another user.  These two arguments may not be
 *	used at the same time.
 */

ERRCODE
do_olc_forward(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status = SUCCESS;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if (is_flag(*arguments, "-off", 2))
	{
	  set_option(Request.options, OFF_OPT);
	  arguments++;
	  continue;
	}

      if(is_flag(*arguments, "-unanswered", 2))
	{
	  set_option(Request.options, FORWARD_UNANSWERED);
	  arguments++;
	  continue;
	}

      if(is_flag(*arguments,"-status",2))
	{
	  arguments++;
	  status = t_input_status(&Request, *arguments);
	  if(*arguments != NULL)
	    arguments++; /* Argument eaten by t_input_status(). */
	  if(status != SUCCESS)
	    break;
	  continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)   /* error */
    {
      fprintf(stderr,
	      "Usage is: \tforward  [<username> <instance id>] "
	      "[-status <status>]\n\t\t[-off] [-unanswered] "
	      "[-instance <instance id>]\n");
      return ERROR;
    }

  status = t_forward(&Request);
  return(status);
}



