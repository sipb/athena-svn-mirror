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
 *	$Id: p_status.c,v 1.11 1999-01-22 23:12:50 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_status.c,v 1.11 1999-01-22 23:12:50 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
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
do_olc_status(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \tstatus\n");
	  else
	    {
	      printf("Usage is: \tstatus [<username> <instance id>] ");
	      printf("[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_personal_status(&Request,FALSE);
  return(status);
}



ERRCODE
do_olc_who(arguments)
     char  **arguments;
{
  REQUEST Request;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \twho\n");
	  else
	    {
	      printf("Usage is: \twho< [<username> <instance id>] ");
	      printf("[-instance <instance id>]\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_who(&Request);
  return(status);
}

ERRCODE
do_olc_version(arguments)
     char  **arguments;
{
  REQUEST Request;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(client_is_user_client())
	    printf("Usage is: \tversion\n");
	  else
	    {
	      printf("Usage is: \tversion\n");
	    }
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_version(&Request);
  return(status);
}


