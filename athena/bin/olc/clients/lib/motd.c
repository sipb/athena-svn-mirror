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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/motd.c,v $
 *	$Id: motd.c,v 1.7 1990-07-16 08:16:03 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/motd.c,v 1.7 1990-07-16 08:16:03 lwvanels Exp $";
#endif

#include <mit-copyright.h>
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
  int status;

  Request->request_type = OLC_MOTD;
  set_option(Request->options, type);
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

  close(fd);
  return(status);
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
  int status;
  
  Request->request_type = OLC_CHANGE_MOTD;
  set_option(Request->options, type);

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

  if(is_option(Request->options, VERIFY))
    return(status);

  if(status == SUCCESS)
    {
      write_file_to_fd(fd,file);
      read_response(fd, &status);
    }

  close(fd);
  return(status);
}


