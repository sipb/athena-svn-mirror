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
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/parser/p_instance.c,v 1.1 1989-07-06 22:08:44 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

do_olc_instance(arguments)
     char **arguments;
{
  REQUEST Request;
  char buf[LINE_LENGTH];
  int instance = -1;
  int status;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  if(*++arguments != NULL)
    {
      if(string_equiv(*arguments,"-instance",
		      max(strlen(*arguments),2)))
	{
	  if(*(++arguments) != (char *) NULL)
	    instance = atoi(*arguments);
	  else
	    instance = -1;
	}
      else
	{
	  fprintf(stderr,"The argument \"%s\" is invalid.\n",*arguments);
	  fprintf(stderr,"Usage is: \tinstance [-instance <n>].\n");
	  return(ERROR);
	}
    }
  else
    instance = -1;

  return(t_instance(&Request,instance));
}
