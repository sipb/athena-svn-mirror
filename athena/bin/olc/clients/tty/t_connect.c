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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_connect.c,v $
 *	$Id: t_connect.c,v 1.11 1990-05-26 11:59:34 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_connect.c,v 1.11 1990-05-26 11:59:34 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_grab(Request,flag,hold)
     REQUEST *Request;
     int flag;
     int hold;
{
  int status;
  int instance;
  
  instance = Request->requester.instance;

  if(flag)
    set_option(Request->options,SPLIT_OPT);

  status = OGrab(Request);
  switch (status)
    {
    case GRAB_ME:
      fprintf(stderr, "You cannot grab yourself in OLC.\n");
      status = NO_ACTION;
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,
              "You cannot grab this question.\n");
      status = NO_ACTION;
      break;

    case CONNECTED:
      fprintf(stderr, "You are connected to another user.\n");

    case HAS_QUESTION:
      printf("Your current instance is busy, creating another one for you.\n");
      return(t_grab(Request,TRUE,hold));
      break;

    case SUCCESS:
      printf("User grabbed.\n");
      status = SUCCESS;
      break;

    case ALREADY_CONNECTED:
      printf("Someone is already connected to %s [%d].\n",
             Request->target.username,Request->target.instance);
      status = NO_ACTION;
      break;

    case MAX_ANSWER:
      printf("You cannot answer any more questions simultaneously\n");
      printf("without forwarding one of your existing connections.\n");
      status = NO_ACTION;
      break;

    case NO_QUESTION:
      printf("%s [%d] does not have a question.\n",Request->target.username,
	     Request->target.instance);
      status = ERROR;
      break;

    case FAILURE:
    case ERROR:
      fprintf(stderr, "Error grabbing user.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  if((instance != Request->requester.instance) && !hold)
    {
      printf("You are now %s [%d].\n",Request->requester.username,
	   Request->requester.instance);
      User.instance =  Request->requester.instance;
    }
  return(status);
}




ERRCODE
t_forward(Request)
     REQUEST *Request;

{
  int status;
  int instance;

  t_describe(Request,NULL,NULL,TRUE,FALSE);
  instance = Request->requester.instance;
  status = OForward(Request);
  
  switch (status) 
    {
    case SUCCESS:
      printf("Question forwarded.  %s\n",
	     (is_option (Request->options, OFF_OPT)
	      ? "You have signed off OLC..."
	      : ""));

      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case CONNECTED:
      printf("Question forwarded.  You are now connected to another user.\n");
      status = SUCCESS;
      break;

    case SIGNED_OFF:
      printf("Question forwarded.  ");
      if(is_option(Request->options, OFF_OPT))
	printf("You have signed off OLC.\n");
      else
	printf("(You are not signed on to OLC.)\n");

      t_set_default_instance(Request);
      status = SUCCESS;
      break;

    case NOT_CONNECTED:
      fprintf(stderr,"You have no question to forward.\n");
      status = ERROR;
      break;

    case HAS_QUESTION:
      fprintf(stderr,"You cannot forward your own question.\n");
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "Unable to forward question.  Dunno why.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    printf("%s [%d] has been deactivated.  You are now %s [%d], again!\n",
	Request->requester.username,instance,
	Request->requester.username, Request->requester.instance);

  return(status);
}

