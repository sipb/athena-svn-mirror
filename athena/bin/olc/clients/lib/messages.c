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
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/messages.c,v 1.6 1990-01-17 03:19:43 vanharen Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OReplayLog(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,(char **) NULL,OLC_REPLAY));
}


ERRCODE
OShowMessageIntoFile(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,(char **) NULL,OLC_SHOW));
}


OShowMessage(Request, buf)
     REQUEST *Request;
     char **buf;
{
  return(OGetMessage(Request,NULL,buf,OLC_SHOW));
}


ERRCODE
OGetMessage(Request,file,buf,code)
     REQUEST *Request;
     char *file;
     char **buf;
     int code;
{
  int fd;
  int status;

  Request->request_type = code;

  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  
  if(status == SUCCESS)
    {
      if(buf == (char **) NULL)
	read_text_into_file(fd, file);
      else
	*buf = read_text_from_fd(fd);
    }
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
	  strncpy(sender,message+4,LABEL_SIZE);
	  free(message);
	  unsetenv("MORE");
	  return(OShowMessageIntoFile(Request,file));
	}
      free(message);
    }	
}


