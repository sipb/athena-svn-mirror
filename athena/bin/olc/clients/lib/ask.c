/*
 * This file is part of the OLC On-Line Consulting System.
 * It makes the ask request to the server.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/ask.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/ask.c,v 1.3 1989-08-04 11:16:46 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OAsk(Request,topic,file)
     REQUEST *Request;
     char *topic;
     char *file;
{
  int fd;
  int status;
 
  Request->request_type = OLC_ASK;
  fd = open_connection_to_daemon();

  status = send_request(fd, Request);
  if(status)
    return(status);

  read_response(fd, &status);

  if(status == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd,&status);
    }
  
  if(is_option(Request->options,VERIFY) || status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  write_file_to_fd(fd,file);
  read_response(fd,&status);
  close(fd);
  return(status);
}
