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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/misc.c,v $
 *	$Id: misc.c,v 1.4 1990-05-26 11:56:24 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/misc.c,v 1.4 1990-05-26 11:56:24 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

ODump(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  int status;
  int fd;

  if ((type == OLC_DUMP)
      || (type == OLC_DUMP_REQ_STATS)
      || (type == OLC_DUMP_QUES_STATS))
    Request->request_type = type;
  else
    Request->request_type = OLC_DUMP;

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
