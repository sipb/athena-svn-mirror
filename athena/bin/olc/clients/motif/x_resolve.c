/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_resolve.c,v $
 *      $Id: x_resolve.c,v 1.3 1991-03-24 14:30:00 lwvanels Exp $
 *      $Author: lwvanels $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_resolve.c,v 1.3 1991-03-24 14:30:00 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include "xolc.h"
#include "data.h"

void
x_done(Request)
     REQUEST *Request;
{
  int status;
  char title[TITLE_SIZE];
  char error[BUF_SIZE];
  int off = 0;
  int instance;

  title[0] = '\0';

  instance = Request->requester.instance;
  set_option(Request->options,VERIFY);
  status = ODone(Request,title);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case SEND_INFO:
      if (! MuGetString("Please enter a title for this conversation:\n",
			title, TITLE_SIZE, NULL))
	return;
      break;

    case OK:
      if (! MuGetBoolean(DONE_MESSAGE, "Yes", "No", NULL, FALSE))
	return;
      break;

    case PERMISSION_DENIED:
      sprintf(error, "You are not allowed to resolve %s's question.",
	      Request->target.username);
      MuError(error);
      return;

    case NO_QUESTION:
    case NOT_CONNECTED:
      MuError("You do not have a question to resolve.");
      return;
      
    default:
      status = handle_response(status, Request);
      if(status != SUCCESS)
	return;
      break;
    }

  status = ODone(Request,title);

  switch(status)
    {
    case SIGNED_OFF:
      MuHelp("Question resolved.");
      status = SUCCESS;
      break;

    case CONNECTED:
      MuHelp("Question resolved.  You have been connected to another user.");
      status = SUCCESS;
      break;

    case OK:
      MuHelp("The consultant has been notified that you are finished with your question.\n\nThank you for using OLC!");
     if(OLC)
       {
	 exit(0);
       }
      
      break;

    case SUCCESS:
      exit(0);
      break;

    case PERMISSION_DENIED:
      sprintf(error, "You are not allowed to resolve %s's question.",
              Request->target.username);
      MuError(error);
      return;

    case ERROR:
      sprintf(error, "An error has occurred.  The question\nhas not been marked resolved.");
      MuError(error);
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    {
      sprintf(error, "%s (%d) has been deactivated.  You are %s (%d).",
	      Request->requester.username, instance,
	      Request->requester.username, Request->requester.instance);
      MuHelp(error);
    }

  return;
}
  

ERRCODE
x_cancel(Request)
     REQUEST *Request;
{
  int status;
  char title[TITLE_SIZE];
  char error[BUF_SIZE];
  int instance;

  title[0] = '\0';

  instance = Request->requester.instance;
  Request->request_type = OLC_CANCEL;

  if (OLC)
    {
      if (! MuGetBoolean(CANCEL_MESSAGE, "Yes", "No", NULL, FALSE))
	return(NO_ACTION);
      else
	status = OCancel(Request,"Cancelled Question");
    }
  else
    if (! MuGetBoolean("Are you sure you want to cancel this question?",
		       "Yes", "No", NULL, FALSE))
      return(NO_ACTION);
    else
      status = OCancel(Request,"Cancelled Question");

  switch(status)
    {
    case SUCCESS:
      if (OLC)
	{
	  exit(0);
	}
	  
      MuHelp("Question cancelled.");
      
      status = SUCCESS;
      break;

    case OK:
      MuHelp("Your question has been cancelled.\n\nThank you for using OLC!");
      if(OLC)
         exit(0);

      break;

    case SIGNED_OFF:
      MuHelp("Question cancelled.");

      status = SUCCESS;
      break;

    case CONNECTED:
      MuHelp("You have been connected to another user.");
      status = SUCCESS;
      break;

    case ERROR:
      MuErrorSync("An error has occurred.  The question has not been cancelled.");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(instance != Request->requester.instance)
    {
      sprintf(error, "%s (%d) has been deactivated.  You are now %s (%d).",
	      Request->requester.username, instance,
	      Request->requester.username, Request->requester.instance);
      MuHelp(error);
    }

  return(status);
}
