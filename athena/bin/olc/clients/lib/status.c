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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/status.c,v $
 *	$Id: status.c,v 1.17 1991-08-23 12:57:42 raek Exp $
 *	$Author: raek $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/status.c,v 1.17 1991-08-23 12:57:42 raek Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
extern long lc_time;
extern LIST list_cache;
#define LIST_LIFETIME 60

ERRCODE
OListPerson(Request,data)
     REQUEST *Request;
     LIST **data;
{
  int status;

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
  int status;

  if (((time(0) - lc_time) < LIST_LIFETIME)
      && (list_cache.user.instance == Request->requester.instance))
    {
      *data = list_cache;
      return(SUCCESS);
    }
  Request->request_type = OLC_WHO;
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
    {
      status = OReadList(fd, &data, 1);
      list_cache = *data;
      lc_time = time(0);
    }
  (void) close(fd);
  return(status);
}

ERRCODE
OGetUsername(Request,username)
     REQUEST *Request;
     char *username;
{
  LIST list;
  int status;

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
  int status;

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
  int status;

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
  int status;

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
  int status;

  Request->request_type = OLC_VERSION;
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
    {
      *vstring = read_text_from_fd(fd);
    }
  (void) close(fd);
  return(status);
}
