/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with topics.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/db.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/db.c,v 1.1 1989-11-17 14:17:00 tjcoppet Exp $";
#endif

#include <olc/olc.h>


OLoadUser(Request)
     REQUEST *Request;
{
  int status;
  int fd;

  Request->request_type = OLC_LOAD_USER;

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
  (void) close(fd);
  return(status);
}


OGetDBInfo(Request, dbinfo)
     REQUEST *Request;
     DBINFO *dbinfo;
{
  int status;
  int fd;

  Request->request_type = OLC_GET_DBINFO;

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
 
  if(status == SUCCESS)
    read_dbinfo(fd,dbinfo);

  (void) close(fd);
  return(status);
}


OSetDBInfo(Request, dbinfo)
     REQUEST *Request;
     DBINFO *dbinfo;
{
  int status;
  int fd;

  Request->request_type = OLC_SET_DBINFO;

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
 
  if(status == SUCCESS)
    send_dbinfo(fd,dbinfo);

  read_response(fd, &status); 
  (void) close(fd);
  return(status);
}
