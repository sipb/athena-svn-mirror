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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/db.c,v $
 *	$Id: db.c,v 1.5 1991-08-23 12:57:22 raek Exp $
 *	$Author: raek $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/db.c,v 1.5 1991-08-23 12:57:22 raek Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

ERRCODE
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

ERRCODE
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

ERRCODE
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
