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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_instance.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_instance.c,v 1.2 1989-07-16 17:04:18 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>


t_instance(Request,instance)
     REQUEST *Request;
     int instance;
{
  char buf[BUFSIZE];
  int status;

  if(!instance)
    {
      get_prompted_input("enter new instance (<return> to exit): ",buf);
      if(buf[0] == '\0')
	return(ERROR);
      instance = atoi(buf);
    }
  else
    if(instance < 0)
      {
	printf("You are %s (%d). %s.\n",Request->requester.username,Request->requester.instance,happy_message());
	return(SUCCESS);
      }

  while(1)
    {
      status = OVerifyInstance(&Request,instance);
      if(status == SUCCESS)
	{
	  User.instance = instance;
	  printf("You are now %s (%d)\n",User.username, User.instance);
	  return(SUCCESS);
	}
      else
	{  
	  printf("%s (%d) does not exist. Choose one of... \n",User.username, instance);
	  t_personal_status(Request);
	  get_prompted_input("enter new instance (<return> to exit): ",buf);
	  instance = atoi(buf);
	  if(buf[0] == '\0')
	    return(ERROR);
	}
    }
}





