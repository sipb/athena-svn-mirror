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
 *	$Id: p_connect.c,v 1.14 1999-06-28 22:52:07 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_connect.c,v 1.14 1999-06-28 22:52:07 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

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
  ERRCODE status;
  int flag = 0;
  int hold = 0;

  if(fill_request(&Request) != SUCCESS)
     return(ERROR);

  arguments++;
  while(*arguments != (char *) NULL)
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

      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);

      arguments += num_of_args;		/* HACKHACKHACK */
      
      if(arguments == (char **) NULL)   
	{
	  printf("Usage is: \tgrab  [<username> <instance id>] ");
	  printf("[-create_new_instance]\n");
	  printf("\t\t[-do_not_change_instance] [-instance <instance id>]\n");
	  return(ERROR);
	}
    }

  status = t_grab(&Request,flag,hold);
  return(status);
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
  ERRCODE status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
	
  arguments++;
  while(*arguments != (char *)NULL)
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
	  if(status != SUCCESS)
	    return(status);
	  if(*arguments != (char *) NULL)
	    arguments++;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status != SUCCESS)
	return(ERROR);

      arguments += num_of_args;		/* HACKHACKHACK */

      if(arguments == (char **) NULL)   /* error */
	{
	  printf("Usage is: \tforward  [<username> <instance id>] ");
	  printf("[-status <status>]\n\t\t[-off] [-unanswered] ");
	  printf("[-instance <instance id>]\n");
	  return(ERROR);
	}
    }

  status = t_forward(&Request);
  return(status);
}



