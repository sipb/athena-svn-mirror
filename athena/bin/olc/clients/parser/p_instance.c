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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_instance.c,v 1.15 1999-03-06 16:48:00 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_instance.c,v 1.15 1999-03-06 16:48:00 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <ctype.h>

ERRCODE
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
      if (!*arguments) break;

      if(string_equiv(*arguments,"-instance", max(strlen(*arguments),2)) ||
	 string_equiv(*arguments,"-change", max(strlen(*arguments),2)))
	{
          ++arguments;
	  if(*arguments != (char *) NULL) {
	    if (!isdigit(**arguments)) {
	      printf("Instance specified must be numeric; %s is not.\n",
		     *arguments);
	      return(ERROR);
	    }
	    instance = atoi(*arguments);
	  }
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
