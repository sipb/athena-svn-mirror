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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_send.c,v $
 *	$Id: x_send.c,v 1.3 1991-03-24 14:35:24 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_send.c,v 1.3 1991-03-24 14:35:24 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include <sys/param.h>
#include "xolc.h"


ERRCODE
x_reply(Request, message)    
     REQUEST *Request;
     char *message;
{
  int status, fd;
  char error[BUF_SIZE];
  char file[MAXPATHLEN];

  if (strlen(message) == 0)
    {
      MuErrorSync("You have not entered a message to send.\n\nClick the left mouse button in the text\narea and type your message, then try\nsending it again.");
      return(ERROR);
    }

  make_temp_name(file);
  if ((fd = open(file, O_CREAT | O_WRONLY, 0644)) < 0)
    {
      MuError("Unable to open temporary file for writing.");
      return(ERROR);
    }

  if (write(fd, message, strlen(message)) != strlen(message))
    {
      MuError("Error writing text to temporary file.");
      unlink(file);
      return(ERROR);
    }

  if (message[strlen(message) - 1] != '\n')
    if (write(fd, "\n", strlen("\n")) != strlen("\n"))
      {
	MuError("Error adding newline to temporary file.");
	unlink(file);
	return(ERROR);
      }

  (void) close(fd);

  set_option(Request->options, VERIFY);
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      sprintf(error, "You are not allowed to send to %s (%d).",
	      Request->target.username,
	      Request->target.instance);
      MuError(error);
      status = NO_ACTION;
      break;

    case ERROR:
      MuError("Unable to send message.");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      MuHelp("Message sent.");
      break;

    case NOT_CONNECTED:
      MuHelp("You are not currently connected to a consultant, but the\nnext one available will receive your message.");
      status = SUCCESS;
      break;

    case CONNECTED:
      MuHelp("Message sent.");
      status = SUCCESS;
      break;

    case PERMISSION_DENIED:
      sprintf(error, "You are no longer allowed to send to %s (%d).",
	      Request->target.username,
	      Request->target.instance);
      MuError(error);
      status = ERROR;
      break;

    case ERROR:
      MuError("Unable to send message.");
      status = ERROR;
      break;

    default:
      status =  handle_response(status, Request);
      break;
    }
  
  unlink(file);
  return(status);
}
