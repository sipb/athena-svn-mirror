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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_ask.c,v $
 *	$Id: t_ask.c,v 1.18 1992-02-06 17:06:43 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_ask.c,v 1.18 1992-02-06 17:06:43 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

ERRCODE
t_ask(Request,topic,q_file)
     REQUEST *Request;
     char *topic;
     char *q_file;
{
  int status;
  char file[NAME_SIZE];
  struct stat statbuf;
  int instance;
  
  instance = Request->requester.instance;
  set_option(Request->options,VERIFY);
  status = OAsk_buffer(Request,topic,NULL);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case SUCCESS:
      break; 

    case INVALID_TOPIC:
      fprintf(stderr, 
	      "Try olc again, but please use '?' to see a list of topics.\n");
      if(OLC)
	exit(1);
      else
	return(ERROR);
      break;

    case ERROR:
      fprintf(stderr, 
	 "An error has 1occurred while contacting server.  Please try again.\n");
      if(OLC)
	exit(1);
      else
	return(ERROR);
      break;

    case CONNECTED:
      fprintf(stderr,
              "You are already connected.\n");
      status = ERROR;
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,"You are not allowed to ask %s questions.\n",
	      OLC_SERVICE_NAME);
      fprintf(stderr,"Does defeat the purpose of things, doesn't it?\n");
      status = ERROR;
      if(OLC)
	exit(1);
      break;

    case MAX_ASK:
    case ALREADY_HAVE_QUESTION:
      fprintf(stderr,
              "You are already asking a question. \n");
      status = ERROR;
      break;

    case HAS_QUESTION:
      printf("Your current instance is busy, creating another one for you.\n");
      set_option(Request->options, SPLIT_OPT);
      t_ask(Request,topic,q_file);
      break;
      
    case ALREADY_SIGNED_ON:
      fprintf(stderr,
              "You cannot be a user and consult in the same instance.\n");
      status = ERROR;
      break;

    default:
      if((status = handle_response(status, Request))!=SUCCESS)
	{
	  if(OLC)
	    exit(1);
	  else
	    return(ERROR);
	}
      break;
    }

  if(status!=SUCCESS)
    return(status);

  if ((q_file == NULL) || (q_file[0] == '\0')) {
    make_temp_name(file);
    printf("Please enter your question.  ");
    printf("End with a ^D or '.' on a line by itself.\n");
    
    if (input_text_into_file(file) != SUCCESS)
      {
	fprintf(stderr,"An error occurred while reading your");
	fprintf(stderr," message; unable to continue.\n");
      }
    
    status = what_now(file, FALSE,NULL);
    if (status == ERROR)
      {
	fprintf(stderr,"An error occurred while reading your");
	fprintf(stderr," message; unable to continue.\n");
	(void) unlink(file);
	return(ERROR);
      }
    else
      if (status == NO_ACTION)
	{
	  printf("Your question has been cancelled.\n");
	  (void) unlink(file);
	  if(OLC)
	    exit(1);
	  return(SUCCESS);
	}
    
    (void) stat(file, &statbuf);
    if (statbuf.st_size == 0)
      {
	printf("You have not entered a question.\n");
	(void) unlink(file);
	if(OLC)
	  exit(1);
	return(SUCCESS);
      }
    status = OAsk_file(Request,topic,file);
    (void) unlink(file);
  }
  else {
    status = OAsk_file(Request,topic,q_file);
    (void) unlink(file);
  }

  switch(status)
    {
    case NOT_CONNECTED:
      printf("Your question will be forwarded to the first available ");
      printf("consultant.\n");
      status = SUCCESS;
      break;
    case CONNECTED:
      printf("A consultant is reviewing your question.\n");
      status = SUCCESS;
      break;
    default:
      status = handle_response(status, Request);
      break;
    }

  if ((instance != Request->requester.instance) &&
      (strcmp(Request->requester.username,Request->target.username) == 0))
    {
      printf("You are now %s [%d].\n",Request->requester.username,
	     Request->requester.instance);
      User.instance =  Request->requester.instance;
    }

  return(status);
}
