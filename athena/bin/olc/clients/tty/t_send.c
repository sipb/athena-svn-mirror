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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_send.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_send.c,v 1.3 1989-08-04 11:13:08 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


ERRCODE
t_reply(Request,file,editor)    
     REQUEST *Request;
     char *file, *editor;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to send to %s (%d).\n",
	      Request->target.username,
	      Request->target.instance);
      status = NO_ACTION;
      break;

    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);

  status = enter_message(file,editor);
  if(status)
    return(status);
  
  status = OReply(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("Message sent. \n");
      break;

    case NOT_CONNECTED:
      printf("You are not currently connected to a consultant but the next one\n");
      printf("available will receive your message.\n");
      status = ERROR;
      break;

    case CONNECTED:
      printf("Message sent.\n");
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are no longer allowed to send to %s (%d).\n",
	      Request->target.username,
	      Request->target.instance);
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "Unable to send message.\n");
      status = ERROR;
      break;

    default:
      status =  handle_response(status, Request);
      break;
    }
  
  return(status);
}






ERRCODE
t_comment(Request,file,editor)    
     REQUEST *Request;
     char *file, *editor;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OComment(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      fprintf(stderr,  "You are not allowed to comment in %s (%d)'s log.\n",
	      Request->target.username,
	      Request->target.instance);
      status = NO_ACTION;
      break;

    case ERROR:
      fprintf(stderr, "Unable to log comment.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);
  
  status = enter_message(file,editor);
  if(status)
    return(status);

  status = OComment(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("Comment Logged. \n");
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You no longer allowed to comment in %s (%d)'s log.\n",
	      Request->target.username,
	      Request->target.instance);
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr, "An error has occurred while logging comment. Try resending.\n");
      status = ERROR;
      break;

    default:
      status =  handle_response(status, Request);
      break;
    }  

  return(status);
}








ERRCODE
t_mail(Request,file,editor)
     REQUEST *Request;
     char *file, *editor;
{
  int status;
  char topic[TOPIC_SIZE];
  LIST *list;
  struct stat statbuf;

  set_option(Request->options, VERIFY);
  status = OMail(Request,file);
  unset_option(Request->options, VERIFY);

  switch(status)
    {
    case SUCCESS:
      status = OGetTopic(Request,topic);
      if(status != SUCCESS)
	{
	  fprintf(stderr,"warning: Unable to get topic for conversation.\n");
	  *topic = '\0';
	}
      status = OListPerson(Request,&list);
      if(status != SUCCESS)
	{
	  fprintf(stderr, 
		  "Unable to get status of conversation... exitting\n");
	  return(ERROR);
	}

      (void) OMailHeader(Request,file,list->connected.username,
			 topic,DEFAULT_MAILHUB);

      status = edit_message(file,editor);
      if(status == ERROR)
	{
	  fprintf(stderr,"Error editing file. . . aborting\n");
	  return(status);
	}
      else  
	if(status)
	  return(status);
      
      status = what_now(file,FALSE,NULL);
      if(status == ERROR)
        {
          fprintf(stderr,"An error occurred while reading your");
          fprintf(stderr," message; unable to continue.\n");
          unlink(file);
          return(ERROR);
        }
      else
       if(status)
         return(status);


      (void) stat(file, &statbuf);
      if (statbuf.st_size == 0) 
	{
	  printf("No message to send.\n");
	  return(NO_ACTION);
	}
      if (mail_message(Request->target.username, 
		       Request->requester.username,file) == SUCCESS)
	printf("Mail message sent.\n");
      else 
	{
	  printf("Error sending mail.\n");
	  return(ERROR);
	}
      break;

    case NOT_CONNECTED:
      printf("You are not connected.\n");
      return(ERROR);
    default:
      status = handle_response(status, Request);
      return(status);
    }

  status = OMail(Request,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    default:
      status = handle_response(status, Request);
      return(status);
    }  

  return(status);
}
