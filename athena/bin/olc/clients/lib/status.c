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
 *	$Id: status.c,v 1.14 1990-12-13 16:45:21 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/status.c,v 1.14 1990-12-13 16:45:21 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
extern long lc_time;
extern LIST list_cache;
#define LIST_LIFETIME 60

/*
ERRCODE
OGetConnectedPerson(Request,data)
     REQUEST *Request;
     PERSON *data;
{
  int fd;
  int status;

  Request->request_type = OLC_CONNECTED; 
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd,Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &status);
  if(status == SUCCESS)
    read_person(fd,data);
  close(fd);
  return(status);
}*/


OListPerson(Request,data)
     REQUEST *Request;
     LIST **data;
{
  int status;

  Request->options = LIST_PERSONAL;
  status = OListQueue(Request,data,"","","",0);
  return(status);
}


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
