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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/connect.c,v 1.3 1989-08-04 11:16:56 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OGrab(Request)
     REQUEST *Request;
{
  int fd;
  int status;
  Request->request_type = OLC_GRAB;
  fd = open_connection_to_daemon();
  
  status = send_request(fd,Request);
  if(status)
    return(status);

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
