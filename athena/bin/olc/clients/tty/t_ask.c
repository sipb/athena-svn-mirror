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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_ask.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_ask.c,v 1.3 1989-07-16 17:02:34 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

ERRCODE
t_ask(Request,topic)
     REQUEST *Request;
     char *topic;
{
  int status;
  char file[NAME_LENGTH];
  struct stat statbuf;
  char buf[BUFSIZE];

  set_option(Request->options,VERIFY);
  status = OAsk(Request,topic,NULL);
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
	 "An error has occurred while contacting server. Please try again.\n");
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
      fprintf(stderr,"You are not allowed to ask OLC questions.\n");
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
      printf("You already have a question, would you like to create\n");
      get_prompted_input("another instance to ask another question? ",buf);
      if(string_equiv(buf,"yes",1))
	set_option(Request->options, SPLIT_OPT);
      else
	status = NO_ACTION;
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


  status = OAsk(Request,topic,file);

  (void) unlink(file);

  switch(status)
    {
    case NOT_CONNECTED:
      printf("\nThere is no consultant currently");
      printf(" available. Your");
      printf("request will be forwarded\nto the first available");
      printf(" consultant.\n");
      printf("\nIf you would like to see answers to common questions,");
      printf(" please type\n\"answers\" at the olc> prompt. If you find");
      printf(" the answer to your question, type\n\"cancel\" at the olc> ");
      printf("prompt.\n");
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
  return(status);
}
