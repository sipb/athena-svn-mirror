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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/list.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/list.c,v 1.2 1990-01-17 03:19:41 vanharen Exp $";
#endif


#include <olc/olc.h>

ERRCODE
OListQueue(Request,list,queues,topics,users,stati)
     REQUEST *Request;
     LIST **list;
     char *queues;
     char *topics;
     char *users;
     int stati;
{
  int fd;
  int status;
  int n;

  if(strlen(queues) > NAME_SIZE)
    return(ERROR);

  if(strlen(topics) > NAME_SIZE)
    return(ERROR);

  if(strlen(users) > NAME_SIZE)
    return(ERROR);

  Request->request_type = OLC_LIST;
  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status)
    {
      close(fd);
      return(status);
    }

  read_response(fd, &status);

  if(!is_option(Request->options,LIST_PERSONAL))
    {
      if(status == SUCCESS)
	{
	  write_text_to_fd(fd,queues);
	  write_text_to_fd(fd,topics);
	  write_text_to_fd(fd,users);
	  write_int_to_fd(fd,stati);
	}
      
      read_response(fd, &status);
    }

  if(status == SUCCESS)
    {
      read_int_from_fd(fd, &n);

#ifdef TEST
      printf("reading %d list elements\n",n);
#endif TEST
       
      if(!n)
        {
          *list = (LIST *) NULL;
          status = EMPTY_LIST;
        }
      else 
        {    
          *list = (LIST *) malloc((unsigned) (sizeof(LIST) * (n+1)));
          status = OReadList(fd, list,n);
        }
    }

  (void) close(fd);
  return(status);
}




OReadList(fd,list, size)
     int fd;
     LIST **list;
     int size;
{
  LIST *l;
  int i= 0;
  int status = 0;
  int errflag = 0;

  l = *list;
  for(i=0;i<size;i++)
    {
      status = read_list(fd, l);
      if(status == ERROR)
	{
	  i--;   ++errflag;
	  if(errflag > 3)
	    return(ERROR);
	  continue;
	}
      if(l->ustatus == END_OF_LIST)
	return(status);
      ++l;
    }
  l->ustatus = END_OF_LIST;
  return(status);
}

