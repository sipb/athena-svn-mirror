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
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: resolve.c,v 1.10 1999-03-06 16:47:39 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: resolve.c,v 1.10 1999-03-06 16:47:39 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

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
  
