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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_instance.c,v $
 *	$Id: p_instance.c,v 1.7 1990-05-26 12:11:08 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_instance.c,v 1.7 1990-05-26 12:11:08 vanharen Exp $";
#endif

#include <mit-copyright.h>
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
