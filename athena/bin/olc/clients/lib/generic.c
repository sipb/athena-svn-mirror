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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/generic.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/generic.c,v 1.3 1989-08-04 11:19:50 tjcoppet Exp $";
#endif

#include <olc/olc.h>


ERRCODE
ORequest(Request,code)
     REQUEST *Request;
     int code;
{
  int fd;
  int status;
  
  Request->request_type = code;
  fd = open_connection_to_daemon();
  
  status = send_request(fd,Request);
  if(status)
    return(status);

  read_response(fd,&status);
  close(fd);
  return(status);
}

