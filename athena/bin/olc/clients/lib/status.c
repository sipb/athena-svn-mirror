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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/status.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/status.c,v 1.3 1989-07-16 17:04:48 tjcoppet Exp $";
#endif

#include <olc/olc.h>


ERRCODE
OGetConnectedPerson(Request,data)
     REQUEST *Request;
     PERSON *data;
{
  int fd;
  int status;

  Request->request_type = OLC_CONNECTED; 
  fd = open_connection_to_daemon();
  send_request(fd,Request);
  read_response(fd, &status);
  if(status == SUCCESS)
    read_person(fd,data);
  close(fd);
  return(status);
}


OListPerson(Request,data)
     REQUEST *Request;
     LIST **data;
{
  int status;
  
  Request->options = LIST_PERSONAL;
  status = OListQueue(Request,data);
  return(status);
}


OWho(Request,data)
     REQUEST *Request;
     LIST *data;
{
  int fd;
  int status;
  int n;
  
  Request->request_type = OLC_WHO;
  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd, &status);

  if(status == SUCCESS)
    status = OReadList(fd, &data, 1);

  (void) close(fd);
  return(status);
}
