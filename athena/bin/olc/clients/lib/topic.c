/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with topics.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/topic.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/topic.c,v 1.4 1989-08-15 03:14:12 tjcoppet Exp $";
#endif


#include <olc/olc.h>


/*
 * Function:	OGetTopic() 
 * Description: Gets the current conversation topic specified in the Request.
 * Returns:	ERRCODE
 */


ERRCODE
OGetTopic(Request,topic)
     REQUEST *Request;
     char *topic;
{
  int fd;
  RESPONSE response;
  int status;
  char *buf;
  
  Request->request_type = OLC_TOPIC;
  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }
  read_response(fd, &response);  
 
  if(response == SUCCESS)
    {
      if ((buf = read_text_from_fd(fd)) != (char *)NULL)
	strncpy(topic,buf,TOPIC_SIZE);
      else
	response = FAILURE;
    }

  close(fd);
  return(response);
}



/*
 * Function:	OChangeTopic() 
 * Description: Changes the conversation topic specified in the Request.
 * Returns:	ERRCODE
 */

ERRCODE
OChangeTopic(Request, topic)
     REQUEST *Request;
     char *topic;
{
  int fd;
  RESPONSE response;
  int status;

  Request->request_type = OLC_CHANGE_TOPIC;
  if(*topic == '\0')
    return(ERROR);

  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);

  if(response == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd, &response);
    }

  close(fd);
  return(response);
}



/*
 * Function:	OListTopics() 
 * Description: Lists the topics known to the server.
 * Returns:	ERRCODE
 */

ERRCODE
OListTopics(Request,file)
     REQUEST *Request;
     char *file;
{
  int fd;
  RESPONSE response;
  int status;

  Request->request_type = OLC_LIST_TOPICS;

  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }
  read_response(fd, &response);

  if(response == SUCCESS)
    read_text_into_file(fd,file);
      
  close(fd);
  return(response);
}



/*
 * Function:	OVerifyTopic() 
 * Description: Verifies the conversation topic specified in the Request.
 * Returns:	ERRCODE
 */


ERRCODE
OVerifyTopic(Request,topic)
     REQUEST *Request;
     char *topic;
{
  int fd;
  RESPONSE response;
  int status;

  Request->request_type = OLC_VERIFY_TOPIC;
  
  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);

  if(response == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd,&response);
    }
  
  close(fd);
  return(response);
}




/*
 * Function:	OHelpTopic() 
 * Description: Verifies the conversation topic specified in the Request.
 * Returns:	ERRCODE
 */


ERRCODE
OHelpTopic(Request,topic, buf)      /*ARGSUSED*/
     REQUEST *Request;
     char *topic, *buf;
{
  int fd;
  RESPONSE response;
  int status;

  Request->request_type = OLC_HELP_TOPIC;
  fd = open_connection_to_daemon();
  
  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &response);

  if(response == SUCCESS)
    {
      write_text_to_fd(fd,topic);
      read_response(fd,&response);
      if(response == SUCCESS)
	buf = read_text_from_fd(fd);
    }
  
  close(fd);
  return(response);
}
