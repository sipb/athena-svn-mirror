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
 *	$Id: db.c,v 1.8 1999-06-28 22:51:48 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: db.c,v 1.8 1999-06-28 22:51:48 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

ERRCODE
OLoadUser(Request)
     REQUEST *Request;
{
  ERRCODE status;
  int fd;

  Request->request_type = OLC_LOAD_USER;

  status = open_connection_to_daemon(Request, &fd); 
  if(status != SUCCESS)
    return(status);

  status = send_request(fd, Request);
  if(status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &status); 
  close(fd);
  return(status);
}

ERRCODE
OGetDBInfo(Request, dbinfo)
     REQUEST *Request;
     DBINFO *dbinfo;
{
  ERRCODE status;
  int fd;

  Request->request_type = OLC_GET_DBINFO;

  status = open_connection_to_daemon(Request, &fd);
  if(status != SUCCESS)
    return(status);

  status = send_request(fd, Request);
  if(status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &status);
 
  if(status == SUCCESS)
    read_dbinfo(fd,dbinfo);

  close(fd);
  return(status);
}

ERRCODE
OSetDBInfo(Request, dbinfo)
     REQUEST *Request;
     DBINFO *dbinfo;
{
  ERRCODE status;
  int fd;

  Request->request_type = OLC_SET_DBINFO;

  status = open_connection_to_daemon(Request, &fd);
  if(status != SUCCESS)
    return(status);

  status = send_request(fd, Request);
  if(status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &status);
 
  if(status == SUCCESS)
    send_dbinfo(fd,dbinfo);

  read_response(fd, &status); 
  close(fd);
  return(status);
}
