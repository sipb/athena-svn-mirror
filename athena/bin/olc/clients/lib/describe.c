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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/describe.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/describe.c,v 1.2 1989-08-15 03:13:09 tjcoppet Exp $";
#endif


#include <olc/olc.h>



ERRCODE
ODescribe(Request,list,file,note)
     REQUEST *Request;
     LIST *list;
     char *file;
     char *note;
{
  int fd;
  RESPONSE response;
  int another_response;
  int status;

  Request->request_type = OLC_DESCRIBE;

  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);  
 
  if(response == SUCCESS)
    {
      if(is_option(Request->options, CHANGE_NOTE_OPT))
	write_text_to_fd(fd,note);
      if(is_option(Request->options, CHANGE_COMMENT_OPT))
	write_file_to_fd(fd,file);
    }
  else
    if(response == OK)
      {
	read_list(fd, list);
	read_text_into_file(fd,file);
      }

  else
    {
      close(fd);
      return(response);
    }

  read_response(fd,&another_response);
  close(fd);

  if(another_response == SUCCESS)
    return(response);
  else
    return(another_response);
}


