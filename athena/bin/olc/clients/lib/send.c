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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/send.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/send.c,v 1.3 1989-08-04 11:21:20 tjcoppet Exp $";
#endif

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

  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    return(status);

  read_response(fd, &response);
  
  if(is_option(Request->options,VERIFY))
    return(response);
  
  if(response == SUCCESS)
    {
      write_file_to_fd(fd,file);
      read_response(fd, &response);
    }
  
  (void) close(fd);
  return(response);
}
    

ERRCODE
OMailHeader(Request,file,recipient,topic,destination)
     REQUEST *Request;
     char *file, *recipient, *topic, *destination;
{
  struct stat statbuf;
  FILE *fp;
  
      fp = fopen(file, "w");
      if(fp == NULL)
	return(ERROR);
      fprintf(fp, "To: %s@%s\n", recipient,destination);
      fprintf(fp, "cc: \n");
      fprintf(fp, "Subject: Your OLC question about %s\n", topic);
      fprintf(fp, "--------\n\n");
      
      if(fclose(fp) < 0)
	return(ERROR);
     
  return(SUCCESS);
}
