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
 *	$Id: t_queue.c,v 1.14 1999-01-22 23:13:04 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_queue.c,v 1.14 1999-01-22 23:13:04 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_queue(Request,queue)
     REQUEST *Request;
     char *queue;
{
  int status;

  
  if(queue == (char *) NULL)
    {
      printf("You are %s [%d]. %s.\n",Request->requester.username,Request->requester.instance,happy_message());
      return(SUCCESS);
    }

  status = OChangeQueue(Request,queue);
  switch(status)
    {
    case SUCCESS:
      printf("You are now in the %s queue.\n",queue);		 
      break;
/*    case INVALID_QUEUE:
      printf("The queue \"%s\" does not exist.\n", queue);
      break;*/
    case PERMISSION_DENIED:
      printf("You are not a participant in the \"%s\" queue.\n",queue);
      break;
    default:
      status = handle_response(status, Request);
      break;
    }
  return(status);
}



 
