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
 *	$Id: p_queue.c,v 1.14 1999-06-28 22:52:09 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_queue.c,v 1.14 1999-06-28 22:52:09 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

ERRCODE
do_olc_queue(arguments)
     char **arguments;
{
  REQUEST Request;
  char queue[NAME_SIZE];
  ERRCODE status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(*++arguments != NULL)
    {
      if(is_flag(*arguments,"-queue", 2) || is_flag(*arguments,"-change", 2))
	{
          ++arguments;
	  if(*arguments != (char *) NULL)
	    strncpy(queue,*arguments, NAME_SIZE);
/*	  else
	    t_input_queue(&Request,queue,FALSE);*/
	}
      else
	{
	  arguments = handle_argument(arguments, &Request, &status);
	  if(status != SUCCESS)
	    return(ERROR);

	  if(arguments == (char **) NULL)
	    {
	      printf("The argument \"%s\" is invalid.\n",*arguments);
	      printf("Usage is: \tqueue [-queue <queue>] [-change]\n");
	    }
	  return(ERROR);
	}
    }
  else
    return(t_queue(&Request, (char *) NULL));
      

  return(t_queue(&Request, queue));
}