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
 *	$Id: queue.c,v 1.13 1999-03-06 16:47:39 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: queue.c,v 1.13 1999-03-06 16:47:39 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>


/*
 * Function:	OListQueues() 
 * Description: Lists the queues known to the service.
 * Returns:	ERRCODE
 */

ERRCODE
OListQueues(Request,file)
     REQUEST *Request;
     char *file;
{
  int fd;
  RESPONSE response;
  int status;
/*
  Request->request_type = OLC_LIST_QUEUES;
*/
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
 * Function:	OVerifyQueue() 
 * Description: Verifies the conversation queue specified.
 * Returns:	ERRCODE
 */


ERRCODE
OChangeQueue(Request,queue)
     REQUEST *Request;
     char *queue;
{
  int fd;
  RESPONSE response;
  int status;
/*
  Request->request_type = OLC_CHANGE_QUEUE;
  */
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
      write_text_to_fd(fd,queue);
      read_response(fd,&response);
    }
  
  close(fd);
  return(response);
}


