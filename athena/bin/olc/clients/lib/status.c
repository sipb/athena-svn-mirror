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
 *	$Id: status.c,v 1.20 1999-06-28 22:51:52 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: status.c,v 1.20 1999-06-28 22:51:52 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
extern long lc_time;
extern LIST list_cache;
#define LIST_LIFETIME 60

ERRCODE
OListPerson(Request,data)
     REQUEST *Request;
     LIST **data;
{
  ERRCODE status;

  Request->options = LIST_PERSONAL;
  status = OListQueue(Request,data,"","","",0);
  return(status);
}


ERRCODE
OWho(Request,data)
     REQUEST *Request;
     LIST *data;
{
  int fd;
  ERRCODE status;

  if (((time(0) - lc_time) < LIST_LIFETIME)
      && (list_cache.user.instance == Request->requester.instance))
    {
      *data = list_cache;
      return(SUCCESS);
    }
  Request->request_type = OLC_WHO;
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
    {
      status = OReadList(fd, &data, 1);
      list_cache = *data;
      lc_time = time(0);
    }
  close(fd);
  return(status);
}

ERRCODE
OGetUsername(Request,username)
     REQUEST *Request;
     char *username;
{
  LIST list;
  ERRCODE status;

  status = OWho(Request, &list);
  if (status != SUCCESS)
    return(status);

  strcpy(username,list.user.username);
  return(SUCCESS);
}

ERRCODE
OGetHostname(Request,hostname)
     REQUEST *Request;
     char *hostname;
{
  LIST list;
  ERRCODE status;

  status = OWho(Request, &list);
  if (status != SUCCESS)
    return(status);

  strcpy(hostname,list.user.machine);
  return(SUCCESS);
}

ERRCODE
OGetConnectedUsername(Request,username)
     REQUEST *Request;
     char *username;
{
  LIST list;
  ERRCODE status;

  status = OWho(Request, &list);
  if (status != SUCCESS)
    return(status);

  strcpy(username,list.connected.username);
  return(SUCCESS);
}

ERRCODE
OGetConnectedHostname(Request,hostname)
     REQUEST *Request;
     char *hostname;
{
  LIST list;
  ERRCODE status;

  status = OWho(Request, &list);
  if (status != SUCCESS)
    return(status);

  strcpy(hostname,list.connected.machine);
  return(SUCCESS);
}

ERRCODE
OVersion(Request,vstring)
     REQUEST *Request;
     char **vstring;
{
  int fd;
  ERRCODE status;

  Request->request_type = OLC_VERSION;
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
    {
      *vstring = read_text_from_fd(fd);
    }
  close(fd);
  return(status);
}
