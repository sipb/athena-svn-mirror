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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_queue.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_queue.c,v 1.9 1990-02-15 18:14:29 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>


t_queue(Request,queue)
     REQUEST *Request;
     char *queue;
{
  char buf[BUF_SIZE];
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



 
