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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/resolve.c,v $
 *	$Id: resolve.c,v 1.6 1990-05-26 11:57:10 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/resolve.c,v 1.6 1990-05-26 11:57:10 vanharen Exp $";
#endif

#include <mit-copyright.h>
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
  
