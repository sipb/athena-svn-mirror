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
 *	$Id: p_instance.c,v 1.17 1999-07-30 18:27:34 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_instance.c,v 1.17 1999-07-30 18:27:34 ghudson Exp $";
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
  ERRCODE status = SUCCESS;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while (*arguments != NULL)
    {
      if(is_flag(*arguments,"-instance", 2) ||
	 is_flag(*arguments,"-change", 2))
	{
	  arguments++;
	  if((*arguments != NULL) && (*arguments[0] != '-'))
	    {
	      if (isnumber(*arguments) != SUCCESS)
		{
		  printf("Instance specified must be numeric; %s is not.\n",
			 *arguments);
		  status = ERROR;
		  break;
		}
	      instance = atoi(*arguments);
	      arguments++;
	    }
	  else
	    {
	      instance = -2;
	    }
	  continue;
	}

      status = handle_common_arguments(&arguments, &Request);
      if(status != SUCCESS)
	break;
    }

  if(status != SUCCESS)
    {
      fprintf(stderr, "Usage is: \tinstance [-instance <n>] [-change]\n"
	      "\t\t[<username> <instance id>]\n");
      return ERROR;
    }

  return t_instance(&Request,instance);
}
