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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_ask.c,v $
 *      $Id: x_ask.c,v 1.6 1997-04-30 17:38:54 ghudson Exp $
 *      $Author: ghudson $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_ask.c,v 1.6 1997-04-30 17:38:54 ghudson Exp $";
#endif

#include <mit-copyright.h>

#include <fcntl.h>
#include <sys/param.h>
#include "xolc.h"

ERRCODE
x_ask(Request, topic, question)
     REQUEST *Request;
     char *topic;
     char *question;
{
  int status, fd;
  char file[MAXPATHLEN];
  char buf[BUFSIZ];

 try_again:
  set_option(Request->options,VERIFY);
  status = OAsk_buffer(Request,topic,NULL);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case SUCCESS:
      break;

    case INVALID_TOPIC:
      MuError("That topic is invalid.\n\nEither try again, or select a different topic and try again.");
      return(ERROR);
      
    case ERROR:
      MuError("An error has occurred while contacting server.\n\nPlease try again.");
      return(ERROR);

    case CONNECTED:
      MuError("You are already connected.");
      status = ERROR;
      break;

    case PERMISSION_DENIED:
      MuError("You are not allowed to ask OLC questions.\n\nDoes defeat the purpose of things, doesn't it?");
      status = ERROR;
      break;

    case MAX_ASK:
    case ALREADY_HAVE_QUESTION:
      MuError("You are already asking a question.");
      status = ERROR;
      break;

    case HAS_QUESTION:
      if (MuGetBoolean("Your current instance is busy, would you like to create\nanother instance to ask your question?",
		       "Yes", "No", NULL, TRUE))
	{
	  set_option(Request->options, SPLIT_OPT);
	  x_ask(Request, topic, question);
	  return(SUCCESS);
	}
      else
	status = NO_ACTION;
      break;
      
    case ALREADY_SIGNED_ON:
      MuError("You cannot be a user and consult in the same instance.");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      if (status == FAILURE)
	goto try_again;
      if (status != SUCCESS)
	  return(ERROR);
    }

  if(status!=SUCCESS)
    return(status);

  make_temp_name(file);
  fd = open(file, O_CREAT | O_WRONLY, 0644);
  if (fd < 0)
    {
      MuError("Unable to open temporary file for writing.");
      return(ERROR);
    }
  if (write(fd, question, strlen(question)) != strlen(question))
    {
      MuError("Error writing text to temporary file.");
      unlink(file);
      return(ERROR);
    }
  (void) close(fd);

  status = OAsk_file(Request,topic,file);

  (void) unlink(file);

  switch(status)
    {
    case NOT_CONNECTED:
      strcpy(buf,"There is no consultant currently available.  Your request\nwill be forwarded to the first available consultant.\n\nIf you would like to see answers to common questions,");
      strcat(buf,"\nclick on the \"stock answer browser\" button above.\n\nIf you find the answer to your question, click on the\n\"cancel\" button below, and your question will be\nremoved from the queue.");
      MuHelp(buf);
      status = SUCCESS;
      break;

    case CONNECTED:
      MuHelp("A consultant has received your question and\nis reviewing it now.");
      status = SUCCESS;
      break;

    default:
      status = handle_response(status, Request);
      if (status == FAILURE)
	goto try_again;
      break;
    }
  return(status);
}
