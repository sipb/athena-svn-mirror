/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dumping information from the daemon to a
 *   client.
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
 *	$Id: misc.c,v 1.10 1999-06-28 22:51:50 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: misc.c,v 1.10 1999-06-28 22:51:50 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

ERRCODE
ODump(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  ERRCODE status;
  int fd;

  if ((type == OLC_DUMP)
      || (type == OLC_DUMP_REQ_STATS)
      || (type == OLC_DUMP_QUES_STATS))
    Request->request_type = type;
  else
    Request->request_type = OLC_DUMP;

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