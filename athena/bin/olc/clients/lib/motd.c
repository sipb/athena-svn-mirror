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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/motd.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/motd.c,v 1.1 1989-07-06 21:47:31 tjcoppet Exp $";
#endif


#include <olc/olc.h>


/*
 * Function:	OGetMOTD() 
 * Description: Gets the motd.
 * Returns:	ERRCODE
 */


ERRCODE
OGetMOTD(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  int fd;
  RESPONSE response;

  Request->request_type = OLC_MOTD;
  Request->options = type;
  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd, &response);  
 
  if(response == SUCCESS)
    read_text_into_file(fd,file);

  close(fd);
  return(response);
}



/*
 * Function:	OChangeMotd() 
 * Description: Changes the MOTD.
 * Returns:	ERRCODE
 */

ERRCODE
OChangeMOTD(Request, type, file)
     REQUEST *Request;
     int type;
     char *file;
{
  int fd;
  RESPONSE response;
  
  Request->request_type = OLC_CHMOTD;
  Request->options = type;

  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd, &response);

  if(response == SUCCESS)
    {
      write_file_to_fd(fd,file);
      read_response(fd, &response);
    }

  close(fd);
  return(response);
}


