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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/connect.c,v $
 *	$Id: connect.c,v 1.6 1990-05-26 11:45:28 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/connect.c,v 1.6 1990-05-26 11:45:28 vanharen Exp $";
#endif

#include <mit-copyright.h>
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
