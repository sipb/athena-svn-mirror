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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/ask.c,v $
 *	$Id: ask.c,v 1.7 1990-05-26 11:44:29 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/ask.c,v 1.7 1990-05-26 11:44:29 vanharen Exp $";
#endif

#include <mit-copyright.h>
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
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

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
  if ((status == CONNECTED) || (status == NOT_CONNECTED))
    read_int_from_fd(fd,&(Request->requester.instance));
  close(fd);
  return(status);
}
