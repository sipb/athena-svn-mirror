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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: topic.c,v 1.9 1999-01-22 23:12:14 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: topic.c,v 1.9 1999-01-22 23:12:14 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
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
  int status;
  LIST data;

  status = OWho(Request,&data);
  if(status == SUCCESS)      
    strcpy(topic,data.topic);
  return(status);
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

  Request->request_type = OLC_SET_TOPIC;
  if(*topic == '\0')
    return(ERROR);

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
