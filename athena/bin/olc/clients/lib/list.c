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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: list.c,v 1.11 1999-01-22 23:12:08 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: list.c,v 1.11 1999-01-22 23:12:08 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
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

  if((queues != NULL) && (strlen(queues) > NAME_SIZE))
    return(ERROR);

  if((topics != NULL) && (strlen(topics) > NAME_SIZE))
    return(ERROR);

  if((users != NULL) && (strlen(users) > NAME_SIZE))
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

  if (status == PERMISSION_DENIED)
    return(status);

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
      if (read_int_from_fd(fd, &n) != SUCCESS)
	return(ERROR);

#ifdef TEST
      printf("reading %d list elements\n",n);
#endif /* TEST */
       
      if(!n)
        {
          *list = (LIST *) NULL;
          status = EMPTY_LIST;
        }
      else 
        {    
          *list = (LIST *) malloc((unsigned) (sizeof(LIST) * (n+1)));
	  if (*list == (LIST *) NULL) {
	    fprintf(stderr,"Unable to allocate memory to list queue\n");
	    return(ERROR);
	  }
          status = OReadList(fd, list,n);
        }
    }

  (void) close(fd);
  return(status);
}



ERRCODE
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
      if(status == FATAL)
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
  if (!l)
    return(ERROR);
  l->ustatus = END_OF_LIST;
  return(status);
}

