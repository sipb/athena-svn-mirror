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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/queue.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/queue.c,v 1.2 1989-07-06 22:03:03 tjcoppet Exp $";
#endif


#include <olc/olc.h>

ERRCODE
OListQueue(Request,list,queues,topics,stati)
     REQUEST *Request;
     LIST **list;
     char *queues;
     char *topics;
     char *stati;
{
  int fd;
  int status;
  int n;

  Request->request_type = OLC_LIST;
  fd = open_connection_to_daemon();
  send_request(fd, Request);
  read_response(fd, &status);

  if(status == SUCCESS)
    {
      read_int_from_fd(fd, &n);

#ifdef TEST
      printf("reading %d list elements\n",n);
#endif TEST

      *list = (LIST *) malloc((unsigned) (sizeof(LIST) * (n+1)));
      status = OReadList(fd, list,n);
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
  int status;
  int errflag = 0;

  l = *list;
  printf("list size: %d\n",size);
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
      printf("%s %d %s %d \n",l->user.username,l->user.instance,l->user.machine,l->ustatus);
      if(l->ustatus == END_OF_LIST)
	return(status);
      ++l;
    }
  l->ustatus = END_OF_LIST;
  return(status);
}

