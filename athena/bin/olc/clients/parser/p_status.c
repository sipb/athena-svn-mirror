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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_status.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_status.c,v 1.4 1989-08-07 15:03:05 tjcoppet Exp $";
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
	  if(OLC)
	    fprintf(stderr,"Usage is: \tstatus\n");
	  else
	    fprintf(stderr,"Usage is: \tstatus [<username> <instance id>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_personal_status(&Request,FALSE);
  return(status);
}




do_olc_who(arguments)
     char  **arguments;
{
  int fd;
  REQUEST Request;
  int response;
  int status;
  char *msg;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  for (arguments++; *arguments != (char *)NULL; arguments++) 
    {
      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);
      if(arguments == (char **) NULL)   /* error */
	{
	  if(OLC)
	    fprintf(stderr,"Usage is: \twho\n");
	  else
	    fprintf(stderr,"Usage is: \twho< [<username> <instance id>]\n");
	  return(ERROR);
	}
      if(*arguments == (char *) NULL)   /* end of list */
	break;
    }

  status = t_who(&Request);
  return(status);
}


