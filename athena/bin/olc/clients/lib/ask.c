/*
 * This file is part of the OLC On-Line Consulting System.
 * It makes the ask request to the server.
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
 *	$Id: ask.c,v 1.21 1999-03-06 16:47:34 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: ask.c,v 1.21 1999-03-06 16:47:34 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>

ERRCODE
OAsk_buffer(Request,topic,buf)
     REQUEST *Request;
     char *topic;
     char *buf;
{
  int fd;
  int status;
  FILE *f = NULL;
  char s[BUF_SIZE], machinfo[BUF_SIZE];
 
  /* Start this early so that things aren't blocked on it later */

#ifdef MACHTYPE_PATH
  /* Open the pipe early, so the data is already written when we need it. */
  if (! (is_option(Request->options,VERIFY)))
    {
      f = popen(MACHTYPE_PATH " -c -d -M -v -L", "r");
    }
#endif /* MACHTYPE_PATH */

  Request->request_type = OLC_ASK;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    {
      if (f)
	pclose(f);
      return(status);
    }

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      if (f)
	pclose(f);
      return(status);
    }

  read_response(fd, &status);

  if(status == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd,&status);
    }
  
  if(is_option(Request->options,VERIFY) || status != SUCCESS)
    {
      close(fd);
      if (f)
	pclose(f);
      return(status);
    }

  write_text_to_fd(fd,buf);
  read_response(fd,&status);

  if (status != SUCCESS)
    {
      close(fd);
      if (f)
	pclose(f);
      return(status);
    }

  machinfo[0] = '\0';
  if (f)
    {
      while (fgets(s, BUF_SIZE, f) != NULL)
	{
	  strncat(machinfo, s, strlen(s) - 1);
	  strcat(machinfo, ", ");
	}
      machinfo[strlen(machinfo) - 2] = '\0';
    }
  else
    {
      strcpy(machinfo,"no machine information is available.\n");
    }

  write_text_to_fd(fd, machinfo);

  if (f)
    pclose(f);

  read_response(fd, &status);

  if ((status == CONNECTED) || (status == NOT_CONNECTED))
    read_int_from_fd(fd,&(Request->requester.instance));
  close(fd);
  return(status);
}

ERRCODE
OAsk_file(Request,topic,file)
     REQUEST *Request;
     char *topic;
     char *file;
{
  char *buf = NULL;
  ERRCODE status;

  if (! (is_option(Request->options,VERIFY))) {
    status = read_file_into_text(file,&buf);
    if (status != SUCCESS)
      return(status);
  }

  status = OAsk_buffer(Request,topic,buf);
  if (buf != NULL) {
    free(buf);
  }
  return(status);
}
