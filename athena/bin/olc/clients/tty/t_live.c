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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_live.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_live.c,v 1.1 1989-07-06 22:05:44 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


ERRCODE
t_live(Request,file)
     REQUEST *Request;
     char *file;
{
  int status;
  struct stat statbuf;	
  char sender[LABEL_LENGTH];
  int port;

  set_option(Request->options, VERIFY);
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;
    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to send to %s (%d).\n",
	      Request->target.username,
	      Request->target.instance);
      status = NO_ACTION;
      break;
    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;
    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);
  ZInitialize(); 
  port = 0;
  ZOpenPort(&port);
  zephyr_subscribe("message","personal",Request->requester.username);

  while(1)
    {
      status = OWaitForMessage(Request,file,sender);
      
      switch(status)
	{
	case SUCCESS:
	  printf("%s: ", sender);
	  fflush(stdout);
	  display_file(file);
	  break;
	case NOT_CONNECTED:
	  fprintf(stderr, "You are not connected nor have a question.\n");
	  break;
	case PERMISSION_DENIED:
	  fprintf(stderr, "Permission denied. \n");
	  break;
	default:
	  status = handle_response(status, &Request);
	  break;
	}

      if(status != SUCCESS)
	return(status);
      
    }
}

