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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_connect.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_connect.c,v 1.2 1989-07-13 12:08:18 tjcoppet Exp $";
#endif


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
  int status;
  int flag = 0;
  int hold = 0;

  if(fill_request(&Request) != SUCCESS)
     return(ERROR);

  for(++arguments; *arguments != (char *) NULL; arguments++)
    {
      if(string_equiv(*arguments,"-create_new_instance",
		      max(strlen(*arguments),2)))
	{
	  flag = 1;
          continue;
	}
      if(string_equiv(*arguments,"-do_not_change_instance",
		      max(strlen(*arguments),2)))
	{
	  hold = 1;
          continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   
	{
	  fprintf(stderr,"Usage is: \tgrab  [<username> <instance id>] [-create_new_instance] [-do_not_change_instance]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
        break;
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
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
	
  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      if (string_equiv("-o", *arguments,max(strlen(arguments),2)))
	set_option(Request.options,FORWARD_OFF);
      else if(string_equiv("-un",arguments,max(strlen(arguments),2)))
	set_option(Request.options, FORWARD_UNANSWERED);
      else 
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status)
	    return(ERROR);
	  if(arguments == (char **) NULL)   /* error */
	    {
	      fprintf(stderr,"Usage is: \tforward  [<username> <instance id>] [-off] [-unanswered]\n");
	      return(ERROR);
	    }
	  if(*arguments == (char *) NULL)   /* end of list */
	    break;    
	}
    }

  status = t_forward(&Request);
  return(status);
}

