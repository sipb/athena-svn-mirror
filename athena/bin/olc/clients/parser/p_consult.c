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
 *	$Id: p_consult.c,v 1.9 1999-06-28 22:52:07 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_consult.c,v 1.9 1999-06-28 22:52:07 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

/*
 * Function:	do_olc_on() signs a consultant on to the OLC system.
 * Arguments:	arguments:	The argument array from the parser.
 *		   arguments[1] may be "olc" or "urgent".
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Send a OLC_ON request to the daemon, and evaluate the responses
 *	from the daemon.  If the first argument is "olc", the consultant
 *	is working OLC duty and is given highest priority when selecting
 *	a consultant to be connected to a user.  If the first argument is
 *	"urgent", the consultant will be given a question only if no one
 *	else is available.
 *      "duty" is a synonym for "olc".
 */

ERRCODE
do_olc_on(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

   for (arguments++; *arguments != (char *) NULL; arguments++) 
    {
      if (is_flag(*arguments, "-first",2))
	set_option(Request.options,ON_FIRST);
      else if (is_flag(*arguments, "-duty",2))
	set_option(Request.options,ON_DUTY);
      else if (is_flag(*arguments, "-second",2))
	set_option(Request.options,ON_SECOND);
      else if (is_flag(*arguments, "-urgent",2))
	set_option(Request.options,ON_URGENT);
      else 
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status != SUCCESS)
	    return(ERROR);
	  if(arguments == (char **) NULL)   /* error */
	    {
	      fprintf(stderr,"Usage is: \ton [-first] [-duty] [-second] ");
	      fprintf(stderr,"[-urgent]\n");
              return(ERROR);
	    }
	  if(*arguments == (char *) NULL)   /* end of list */
	    break;
	}
    }

  status = t_sign_on(&Request,0,0);
  return(status);
}
	    
	
  


/*
 * Function:    do_olc_off() signs a consultant off of OLC.
 * Arguments:   arguments:      Argument array from the parser.
 * Returns:     An error code.
 * Notes:
 *      Send an OLC_OFF request to the daemon and handle the response.
 */

ERRCODE
do_olc_off(arguments)
     char **arguments;
{
  REQUEST Request;
  ERRCODE status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *) NULL; arguments++)
    {
      if (is_flag(*arguments, "-force",2))
        set_option(Request.options,OFF_FORCE);
      else
        {
          arguments=handle_argument(arguments, &Request,&status);
	  if(status != SUCCESS)
	    return(ERROR);
          if(arguments == (char **) NULL)   /* error */
	    {
	      fprintf(stderr,"Usage is: \toff [-force]\n");
	      return(ERROR);
	    }
          if(*arguments == (char *) NULL)   /* end of list */
            break;
        }
    }

  status = t_olc_off(&Request);
  return(status);
}

