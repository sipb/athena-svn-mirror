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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_instance.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_instance.c,v 1.6 1990-04-25 16:41:32 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_parser.h>

do_olc_instance(arguments)
     char **arguments;
{
  REQUEST Request;
  int instance = -1;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  while (*arguments != (char *) NULL)
    {
      arguments++;

      if(string_equiv(*arguments,"-instance", max(strlen(*arguments),2)) ||
	 string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
          ++arguments;
	  if(*arguments != (char *) NULL)
	    instance = atoi(*arguments);
	  else
	    instance = -2;
	  continue;
	}

      arguments = handle_argument(arguments, &Request, &status);
      if(status)
	return(ERROR);

      if(arguments == (char **) NULL)
	{
	  printf("Usage is: \tinstance [-instance <n>] [-change]\n");
	  printf("\t\t[<username> <instance id>]\n");
	  return(ERROR);
	}
    }

  return(t_instance(&Request,instance));
}
