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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_topic.c,v $
 *	$Id: t_topic.c,v 1.12 1992-02-06 17:06:43 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_topic.c,v 1.12 1992-02-06 17:06:43 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

/*
 * Function:	input_topic() reads a question topic from the standard input.
 * Arguments:	topic:	Space to put the topic.
 * Returns:	ERRCODE
 */

ERRCODE
t_input_topic(Request,topic,flags)
     REQUEST *Request;
     char *topic;
     int flags;
{
  char buf[LINE_SIZE];
  char file[NAME_SIZE];
  char *bufP;		        
  int loop = 0;
  int status;
  int fudge = 0;

  *buf = '\0';
  
  if(flags)
    {
      printf("Please type a one-word topic for your question.");
      printf("  Type ? for a list of\n");
      printf("available topics or ^C to exit.\n\n");
    }

  if(*topic != '\0');
     fudge = 1;

  while(loop < 5)
    {
      *buf = '\0';
      while (*buf == '\0') 
	{
	  bufP = buf;
          if(!fudge)
       	     (void) get_prompted_input("Topic: ", buf, LINE_SIZE,0);
          else
           {
             strcpy(buf,topic);
             fudge = 0;
           } 

	  if (*bufP == '\0') 
	    {
	      *buf = '\0';
	      continue;
	    }
	  while(*bufP == '\\')
	    ++bufP;
	  (void) strcpy(topic, bufP);
	  uncase(topic);
	}
      
      if(string_eq(topic,"?"))
	{
	  make_temp_name(file);
	  t_list_topics(Request,file,TRUE);
	  unlink(file);
	  continue;
	}

      if(string_eq(topic,"help"))
	{
	  printf("Choose a topic, type '?' for a list of available topics.\n");
	  continue;
	}

      if((status = t_verify_topic(Request,topic)) == SUCCESS)
	return(SUCCESS);
      
      if(status == INVALID_TOPIC)
	printf("Type '?' for a list of topics.\n");
      else
	return(ERROR);
      
      ++loop;
    }

  return(ERROR);
}


ERRCODE
t_list_topics(Request, file, display)
     REQUEST *Request;
     char *file;
     int display;
{
  int status;

  status = OListTopics(Request,file);
  switch(status)
    {
    case SUCCESS:
      if(display)
	display_file(file);
      break;

    case ERROR:
      fprintf(stderr,"Cannot list %s topics\n", OLC_SERVICE_NAME);
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }
  return(status);
}

ERRCODE
t_verify_topic(Request, topic)
     REQUEST *Request;
     char *topic;
{
  int status;

  status = OVerifyTopic(Request,topic);
  switch(status)
    {
    case SUCCESS:
      break;

    case INVALID_TOPIC:
      printf("%s is not a valid topic.\n",topic);
      break;

    default:
      status = handle_response(status, Request);
      break;
    } 
  return(status);
}


ERRCODE
t_get_topic(Request, topic)
     REQUEST *Request;
     char *topic;
{
  int status;

  status = OGetTopic(Request,topic);
  
  switch(status)
    {
    case SUCCESS:
      printf("Current topic is \"%s\".\n", topic);
      status = SUCCESS;
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to see %s's [%d] topic.\n",
	      Request->target.username, Request->target.instance);
      status = ERROR;
      break;

    case FAILURE:
      fprintf(stderr, "Unable to get topic.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }
  
  return(status);
}


ERRCODE
t_change_topic(Request,topic)
     REQUEST *Request;
     char *topic;
{
  int status;

  status = OChangeTopic(Request,topic);
  switch(status)
    {
    case SUCCESS:
      printf("Topic changed to %s.\n",topic);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to change %s's [%d] topic.\n",
	      Request->target.username, Request->target.instance);
      status = ERROR;
      break;

    case ERROR:
      fprintf(stderr,
	      "Topic \"%s\" is not defined. Try 'topic ?' (it won't bite)\n",
	      topic);
      break;

    default:
      status = handle_response(status, Request);
      break;
    }
  
  return(status);
}
