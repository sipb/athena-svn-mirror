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
 *	$Id: motd.c,v 1.10 1999-01-22 23:12:10 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: motd.c,v 1.10 1999-01-22 23:12:10 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>


/*
 * Function:	OGetFile() 
 * Description: Gets the motd.
 * Returns:	ERRCODE
 */


ERRCODE
OGetFile(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  int fd;
  int status;

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
 * Function:	OChangeFile() 
 * Description: Changes the MOTD.
 * Returns:	ERRCODE
 */

ERRCODE
OChangeFile(Request, type, file)
     REQUEST *Request;
     int type;
     char *file;
{
  int fd;
  int status;
  
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


