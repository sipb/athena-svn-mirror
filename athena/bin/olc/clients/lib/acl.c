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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: acl.c,v 1.9 1999-06-28 22:51:46 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: acl.c,v 1.9 1999-06-28 22:51:46 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

ERRCODE
OSetAcl(Request,acl)
     REQUEST *Request;
     char *acl;
{
  int fd;
  ERRCODE status;
 
  Request->request_type = OLC_CHANGE_ACL;
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
      write_text_to_fd(fd,acl);
    }
  
  read_response(fd,&status);
  close(fd);
  return(status);
}



ERRCODE
OListAcl(Request,acl, file)
     REQUEST *Request;
     char *acl;
     char *file;
{
  int fd;
  ERRCODE status;

  Request->request_type = OLC_LIST_ACL;

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
      write_text_to_fd(fd,acl);

  read_response(fd, &status);

  if(status == SUCCESS)
    read_text_into_file(fd,file);

  close(fd);
  return(status);
}

ERRCODE
OGetAccesses(Request, file)
     REQUEST *Request;
     char *file;
{
  int fd;
  ERRCODE status;

  Request->request_type = OLC_GET_ACCESSES;

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
    read_text_into_file(fd,file);

  close(fd);
  return(status);
}
