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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/instance.c,v 1.4 1989-08-08 10:40:09 tjcoppet Exp $";
#endif


#include <olc/olc.h>


ERRCODE
OVerifyInstance(Request,instance)
     REQUEST *Request;
     int instance;
{
  int fd;
  int status;

  Request->request_type = OLC_VERIFY_INSTANCE;
  fd = open_connection_to_daemon();
  
  status = send_request(fd,Request);
  if(status)
    return(status);

  read_response(fd,&status);
  
  if(status == SUCCESS)
    {
      write_int_to_fd(fd,instance);
      read_response(fd,&status);
    }
  
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

  fd = open_connection_to_daemon();
  
  status = send_request(fd,Request);
  if(status)
    return(status);

  read_response(fd,&status);
  
  if(status == SUCCESS)
      read_int_from_fd(fd,instance);

  close(fd);
  return(status);
}
