/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for resolving questions.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/resolve.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/resolve.c,v 1.2 1989-07-06 22:03:12 tjcoppet Exp $";
#endif

#include <olc/olc.h>


ERRCODE 
ODone(Request,title)
     REQUEST *Request;
     char *title;
{
  return(OResolve(Request,title,OLC_DONE));
}

ERRCODE 
OCancel(Request,title)
     REQUEST *Request;
     char *title;
{
  return(OResolve(Request,title,OLC_CANCEL));
}


ERRCODE
OResolve(Request,title,flag)
     REQUEST *Request;
     char *title;
     int flag;
{
  int fd;
  int status;
  
  Request->request_type = flag;
  
  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd, &status);

  if(is_option(Request->options,VERIFY))
    {
      close(fd);
      return(status);
    }
  
  if(status == SEND_INFO)
    {
      write_text_to_fd(fd,title);
      read_response(fd, &status);
    }
  
  (void) close(fd);
  return(status);
}
  
