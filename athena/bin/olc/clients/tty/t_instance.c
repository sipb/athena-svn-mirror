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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_instance.c,v $
 *	$Id: t_instance.c,v 1.12 1990-07-16 08:08:59 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_instance.c,v 1.12 1990-07-16 08:08:59 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>


t_instance(Request,instance)
     REQUEST *Request;
     int instance;
{
  char buf[BUF_SIZE];
  int status;

  if(instance == -2)
    {
take:
      buf[0] = '\0';
      get_prompted_input("enter new instance (<return> to exit): ",buf);
      if(buf[0] == '\0')
	return(ERROR);
      if(isnumber(buf) != SUCCESS)
	{
	  printf("Instance id \"%s\" is not a number.\n",buf);
	  goto take;
	}
      instance = atoi(buf);
    }
  else
    if(instance == -1)
      {
	printf("You are %s [%d]. %s.\n",Request->requester.username,Request->requester.instance,happy_message());
	return(SUCCESS);
      }

  Request->requester.instance = 0;
  Request->target.instance = 0;
  while(1)
    {
      status = OVerifyInstance(Request,&instance);
      if((status == SUCCESS) || (status == OK))
	{
	  User.instance = instance;
	  Request->requester.instance = instance;
	  printf("You are now %s [%d].\n",User.username, User.instance);
	  t_who(Request);
	  return(SUCCESS);
	}
      
      else
	{  
	  printf("%s [%d] does not exist. Your status is... \n\n",User.username, instance);
	  t_personal_status(Request,TRUE);
take2:
	  buf[0] = '\0';
	  get_prompted_input("enter new instance (<return> to exit): ",buf);
	  if(isnumber(buf) != SUCCESS)
	    {
	      printf("Instance id \"%s\" is not a number.\n",buf);
	      goto take2;
	    }
	  instance = atoi(buf);
	  if(buf[0] == '\0')
	    {
	      User.instance = Request->requester.instance;
	      return(ERROR);
	    }
	}
    }
}



t_set_default_instance(Request)
     REQUEST *Request;
{
  int instance;  
  int status;

  status = OGetDefaultInstance(Request,&instance);
  switch(status)
    {
    case SUCCESS:
      User.instance = instance;
      Request->target.instance = instance;
      Request->requester.instance = instance;
      break;
    default:
      handle_response(status, Request);
      break;
    }
  return(status);
}





