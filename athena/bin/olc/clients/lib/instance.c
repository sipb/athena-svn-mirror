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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/instance.c,v $
 *	$Id: instance.c,v 1.9 1990-11-13 14:28:23 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/instance.c,v 1.9 1990-11-13 14:28:23 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
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
