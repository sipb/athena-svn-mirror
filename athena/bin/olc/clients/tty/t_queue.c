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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_queue.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_queue.c,v 1.2 1989-07-16 17:03:13 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_list_queue(Request,queues,topics,stati)
     REQUEST *Request;
     char *queues;
     char *topics;
     char *stati;
{
  int status;
  LIST *list;
  
  status = OListQueue(Request,&list,queues,topics,stati);
  switch (status)
    {
    case SUCCESS:
      t_display_list(list);
      free(list);
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    case EMPTY_LIST:
      printf("The queue is empty.\n");
      status = SUCCESS;
      break;

    default:
      status = handle_response(status, &Request);
      break;
    }

  return(status);
}


ERRCODE
t_display_list(list)
          LIST *list;
{
  LIST *l;
  char uinstbuf[10];
  char cinstbuf[10];
  char ustatusbuf[32];
  char chstatusbuf[10];
  char nseenbuf[5];
  char cbuf[32];
  char buf[32];

  if(list->ustatus ==  END_OF_LIST) 
    {
      printf("Empty list\n");
      return(SUCCESS);
    }

  printf("User          Status           Consultant                Status     Topic\n\n");
  for(l=list; l->ustatus != END_OF_LIST; ++l)
    {
      buf[0] = '\0';
      cbuf[0]= '\0';
      if((l->nseen >=0) && (l->connected.uid >= 0))
	{	  
	  get_user_status_string(l->ustatus,buf);
	  get_status_string(l->ukstatus,cbuf,1);
	  sprintf(ustatusbuf,"(%s/%s)",buf,cbuf);

	  get_status_string(l->ckstatus,buf,0);
	  sprintf(chstatusbuf,"(%s)",buf);
	  sprintf(uinstbuf,"[%d]",l->user.instance);
	  sprintf(cinstbuf,"[%d]",l->connected.instance);
	  sprintf(nseenbuf,"(%d)",l->nseen);
	  sprintf(cbuf,"%s@%s",l->connected.username,l->connected.machine);
	  printf("%-8.8s %-4.4s %-16.16s %-20.20s %-4.4s %-5.5s %-4.4s %-11.11s\n",
		 l->user.username, uinstbuf, ustatusbuf,
		 cbuf, cinstbuf,chstatusbuf,nseenbuf,l->topic);
	}
      else
	if((l->nseen >= 0) && (l->connected.uid <0))
	  {
	    get_user_status_string(l->ustatus,buf);
	    get_status_string(l->ukstatus,cbuf,1);
	    sprintf(ustatusbuf,"(%s/%s)",buf,cbuf);
	    sprintf(uinstbuf,"[%d]",l->user.instance);
	    sprintf(nseenbuf,"(%d)",l->nseen);
	    printf("%-8.8s %-4.4s %-16.16s                                 %-4.4s %-11.11s\n",
		   l->user.username, uinstbuf, ustatusbuf, nseenbuf, l->topic);
	  }
	else
	  if((l->nseen < 0) && (l->connected.uid < 0))
	    {
	      get_status_string(l->ukstatus,buf,0);
	      sprintf(chstatusbuf,"(%s)",buf);
	      sprintf(cbuf,"%s@%s",l->connected.username,l->connected.machine);
	      sprintf(cinstbuf,"[%d]",l->connected.instance);
	      printf("                               %-20.20s %-4.4s %-5.5s\n",
		     cbuf, cinstbuf, chstatusbuf);
	    }
	  else
	    {
	      printf("**unkown list entry***\n");
	    }
    }
  return(SUCCESS);
}



get_user_status_string(status,string)
     int status;
     char *string;
{
  switch(status)
    {
    case ACTIVE:
      strcpy(string, "active");
      break;
    case LOGGED_OUT:
      strcpy(string, "logout");
      break;
    case MACHINE_DOWN:
      strcpy(string, "host down");
      break;
    default:
      strcpy(status, "unknown");
      break;
    }
}

get_status_string(status,string,dir)
     int status;
     char *string;
     int dir;
{
  
  if(dir)
  {
    switch(status)
      {
      case SERVICED:          
	strcpy(string, "active");
	break;
      case PENDING:
	strcpy(string, "pending");
	break;
      case NOT_SEEN:
	strcpy(string, "unseen");
	break;
      case DONE:
	strcpy(string, "done");
	break;
      case CANCEL:
	strcpy(string, "cancel");
	break;
      default:
	strcpy(string, "unknown");
	break;
      }
  }
  else
    {
      switch(status & SIGNED_ON)
	{
	case OFF:
	  strcpy(string, "off");
	  break;
	case ON:
	  strcpy(string, "on");
	  break;
	case FIRST:
	  strcpy(string, "sp1");
	  break;
	case SECOND:
	  strcpy(string, "sp2");
	  break;
	case DUTY:
	  strcpy(string, "dut");
	  break;
	case URGENT:
	  strcpy(string, "urg");
	  break;
	default:
	  strcpy(string, "unknown");
	  break;
	}
    }
  strcat(string, '\0');
}

