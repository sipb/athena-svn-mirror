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
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/connect.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/connect.c,v 1.5 1989-11-17 14:16:30 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OGrab(Request)
     REQUEST *Request;
{
  int fd;
  int status;

  Request->request_type = OLC_GRAB;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd,Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  if(status == SUCCESS)
      read_int_from_fd(fd,&(Request->requester.instance));

  close(fd);
  return(status);
}

ERRCODE
OForward(Request)
     REQUEST *Request;
{
  return(ORequest(Request,OLC_FORWARD));
}
