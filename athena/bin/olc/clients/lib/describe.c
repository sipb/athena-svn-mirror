/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *	$Id: describe.c,v 1.9 1999-06-28 22:51:48 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: describe.c,v 1.9 1999-06-28 22:51:48 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>



ERRCODE
ODescribe(Request,list,file,note)
     REQUEST *Request;
     LIST *list;
     char *file;
     char *note;
{
  int fd;
  ERRCODE response;
  ERRCODE another_response;
  ERRCODE status;

  Request->request_type = OLC_DESCRIBE;

  status = open_connection_to_daemon(Request, &fd);
  if(status != SUCCESS)
    return(status);

  status = send_request(fd, Request);
  if(status != SUCCESS)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);  
 
  if(response == SUCCESS)
    {
      if(is_option(Request->options, CHANGE_NOTE_OPT))
	write_text_to_fd(fd,note);
      if(is_option(Request->options, CHANGE_COMMENT_OPT))
	write_file_to_fd(fd,file);
    }
  else
    if(response == OK)
      {
	read_list(fd, list);
	read_text_into_file(fd,file);
      }

  else
    {
      close(fd);
      return(response);
    }

  read_response(fd,&another_response);
  close(fd);

  if(another_response == SUCCESS)
    return(response);
  else
    return(another_response);
}


