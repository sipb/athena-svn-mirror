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
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: connect.c,v 1.9 1999-01-22 23:12:02 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: connect.c,v 1.9 1999-01-22 23:12:02 ghudson Exp $";
#endif
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
