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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/messages.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/messages.c,v 1.2 1989-07-06 22:02:44 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OReplayLog(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,OLC_REPLAY));
}


ERRCODE
OShowMessage(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,OLC_SHOW));
}


ERRCODE
OGetMessage(Request,file,code)
     REQUEST *Request;
     char *file;
     int code;
{
  int fd;
  int status;

  Request->request_type = code;

  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd,&status);
  
  if(status == SUCCESS)
      read_text_into_file(fd, file);

  (void) close(fd);
  return(status);
}



OWaitForMessage(Request,file, sender)
     REQUEST *Request;
     char *file;
     char *sender;
{
  char *message;

  while(1)
    {
      message = (char *) zephyr_get_opcode("MESSAGE","PERSONAL");
      if(message == (char *) NULL)
	return(ERROR);
      if(!strncmp(message,"olc",3))
	{
	  strncpy(sender,message+4,LABEL_LENGTH);
	  free(message);
	  unsetenv("MORE");
	  return(OShowMessage(Request,file));
	}
      free(message);
    }	
}


