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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/instance.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/instance.c,v 1.6 1989-11-17 14:17:51 tjcoppet Exp $";
#endif


#include <olc/olc.h>


ERRCODE
OVerifyInstance(Request,instance)
     REQUEST *Request;
     int *instance;
{
  int fd;
  int status;

  if(instance == (int *) NULL)
    return(ERROR);

  Request->request_type = OLC_VERIFY_INSTANCE;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd,Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  
  if(status == SUCCESS)
    {
      write_int_to_fd(fd,*instance);
      read_response(fd,&status);
    }

  if(status == OK)
    read_int_from_fd(fd,instance);

  close(fd);
  return(status);
}


OGetDefaultInstance(Request,instance)
     REQUEST *Request;
     int *instance;
{
  int status;
  int fd;

  Request->request_type = OLC_DEFAULT_INSTANCE;

  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd,Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  
  if(status == SUCCESS)
      read_int_from_fd(fd,instance);

  close(fd);
  return(status);
}
