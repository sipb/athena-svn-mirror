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
 *	$Id: messages.c,v 1.15 1999-06-28 22:51:50 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: messages.c,v 1.15 1999-06-28 22:51:50 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

ERRCODE
OReplayLog(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,(char **) NULL,OLC_REPLAY));
}


ERRCODE
OShowMessageIntoFile(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OGetMessage(Request,file,(char **) NULL,OLC_SHOW));
}

ERRCODE
OShowMessage(Request, buf)
     REQUEST *Request;
     char **buf;
{
  return(OGetMessage(Request,NULL,buf,OLC_SHOW));
}


ERRCODE
OGetMessage(Request,file,buf,code)
     REQUEST *Request;
     char *file;
     char **buf;
     int code;
{
  int fd;
  ERRCODE status;

  Request->request_type = code;

  status = open_connection_to_daemon(Request, &fd);
  if(status != SUCCESS)
    return(status);

  status = send_request(fd, Request);
  if(status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  read_response(fd,&status);
  
  if(status == SUCCESS)
    {
      if(buf == (char **) NULL)
	read_text_into_file(fd, file);
      else
	*buf = read_text_from_fd(fd);
    }
  close(fd);
  return(status);
}
