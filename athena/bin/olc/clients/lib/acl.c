/*
 * This file is part of the OLC On-Line Consulting System.
 * It makes the ask request to the server.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/acl.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/acl.c,v 1.1 1989-11-17 14:15:59 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OSetAcl(Request,acl)
     REQUEST *Request;
     char *acl;
{
  int fd;
  int status;
 
  Request->request_type = OLC_CHANGE_ACL;
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
      write_text_to_fd(fd,acl);
      read_response(fd,&status);
    }
  
  read_response(fd,&status);
  close(fd);
  return(status);
}




OListAcl(Request,acl, file)
     REQUEST *Request;
     char *acl;
     char *file;
{
  int fd;
  int status;

  Request->request_type = OLC_LIST_ACL;

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
      write_text_to_fd(fd,acl);

  read_response(fd, &status);

  if(status == SUCCESS)
    read_text_into_file(fd,file);

  close(fd);
  return(status);
}


OGetAccesses(Request, file)
     REQUEST *Request;
     char *file;
{
  int fd;
  int status;

  Request->request_type = OLC_GET_ACCESSES;

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
    read_text_into_file(fd,file);

  (void) close(fd);
  return(status);
}
