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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: send.c,v 1.16 1999-01-22 23:12:12 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: send.c,v 1.16 1999-01-22 23:12:12 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


ERRCODE
OComment(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OSend(Request,OLC_COMMENT,file));
}


ERRCODE
OReply(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OSend(Request,OLC_SEND,file));
}

ERRCODE
OMail(Request,file)
     REQUEST *Request;
     char *file;
{
  return(OSend(Request,OLC_MAIL,file));
}


ERRCODE
OSend(Request,type,file)
     REQUEST *Request;
     char *file;
     int type;
{
  int fd;
  int response;
  int status;

  Request->request_type = type;

  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);
  
  if(is_option(Request->options,VERIFY)) {
    close(fd);
    return(response);
  }

  if(response == SUCCESS) {
    write_file_to_fd(fd,file);
    read_response(fd, &response);
  }
  
  (void) close(fd);
  return(response);
}
    

ERRCODE
OMailHeader(Request, file, username, realname, topic, destination, message)
     REQUEST *Request;
     char *file, *username, *realname, *topic, *destination, *message;
{
  FILE *fp;
  
  fp = fopen(file, "w");
  if(fp == NULL)
    return(ERROR);
  fprintf(fp, "To: \"%s\"  <%s@%s>\n", realname, username, destination);
  fprintf(fp, "cc: \n");
  fprintf(fp, "Subject: Your %s question about \"%s\"\n", client_service_name(),
	  topic);
  fprintf(fp, "--------\n\n");
  
  if(message != (char *) NULL)
    fprintf(fp,"Unseen messages:\n\n%s",message);

  if(fclose(fp) < 0)
    return(ERROR);
     
  return(SUCCESS);
}
