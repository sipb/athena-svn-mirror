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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/misc.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/misc.c,v 1.3 1990-04-25 16:14:26 vanharen Exp $";
#endif

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
