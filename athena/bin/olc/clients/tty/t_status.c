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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_status.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_status.c,v 1.2 1989-07-16 17:03:46 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_personal_status(Request)
     REQUEST *Request;
{
  int status;
  LIST *list;
  
  status = OListPerson(Request,&list);
  switch (status)
    {
    case SUCCESS:
      t_display_personal_status(Request,list);
      free(list);
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    default:
      status = handle_response(status, &Request);
      break;
    }

  return(status);
}


ERRCODE
t_display_personal_status(Request,list)
     REQUEST *Request;
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
  int count=0;

  if(list->ustatus ==  END_OF_LIST) 
    {
      printf("You don't exist, go away.\n");
      return(SUCCESS);
    }
  else
    for(l=list; l->ustatus != END_OF_LIST; ++l)
      count++;

#ifdef TEST
  printf("count: %d\n", count);
#endif TEST

  if(count == 1)
    {
      if(list->nseen >= 0)
	{
	  if(list->connected.uid >= 0)
	    printf("You are currently connected to %s %s (%s@%s)\n",
		   list->connected.title,list->connected.realname,
		   list->connected.username,list->connected.machine);
	  else
	    printf("You currently have %s \"%s\" question in the queue.\n",
		   article(list->topic),list->topic);
	}
      else
	{
	  if(list->connected.uid >= 0)
	    printf("You are connected to user %s (%s@%s)\n",
		   list->connected.realname,
		   list->connected.username,list->connected.machine);
	  else
	    printf("Answer some questions.\n");
	}
    }
  else
    {
      printf("Status for: %s (%s@%s)\n\n",list->user.realname,
	     list->user.username,list->user.machine);
      printf("   instance     status     connected party    topic\n");
      for(l=list; l->ustatus != END_OF_LIST; ++l)
	{
	  if(Request->requester.instance == l->user.instance)
	    printf("-> ");
	  else 
	    printf("   ");
	  if((l->nseen >=0) && (l->connected.uid >= 0))
	    {
	      get_status_string(l->ukstatus,cbuf,1);
	      printf("[%d]          %-10.10s %-9.9s (%d)       %s\n",
		     l->user.instance, cbuf, l->connected.username,
		     l->connected.instance,l->topic);
	    }
	  else
	    if((l->nseen >= 0) && (l->connected.uid <0))
	      {
		get_status_string(l->ukstatus,cbuf,1);
		printf("[%d]          %-10.10s %s   %s\n",    
		       l->user.instance, cbuf, "not connected",l->topic);
	      }
	    else
	      if((l->nseen < 0) && (l->connected.uid < 0))
		{
		  get_status_string(l->ukstatus,buf,0);
		  printf("[%d]         %-10.10s %s\n",    
			 l->user.instance, buf, "not connected");
		}
	      else
		{
		  printf("**unkown list entry***\n");
		}
	}
    }
  return(SUCCESS);
}




t_who(Request)
     REQUEST *Request;
{
  int status;
  LIST list;

  status = OWho(Request, &list);
  
  switch (status)
    { 
    case SUCCESS: 
      if(string_eq(Request->requester.username,list.user.username))
	{
	  if(list.connected.uid > 0)
	    printf("You are currently connected to %s (%s@%s)\n",
		   list.connected.realname, list.connected.username,
		   list.connected.machine);
	  else
	    {
	      if(list.nseen >= 0)
		printf("You are not connected to a consultant.\n");
	      else
		printf("You are not connected to a user.\n");
	    }
	}
      else
	{
	  if(list.connected.uid > 0)
	    printf("%s %s (%d) (%s@%s) is currently connected to %s %s (%d) (%s@%s).\n",
		   list.user.title, list.user.realname, 
		   list.user.instance,list.user.username,
		   list.user.machine, list.connected.title,
		   list.connected.realname,list.connected.instance,
		   list.connected.username,list.connected.machine);
	  else
	    {
	      if(list.nseen >=0)
		printf("%s %s (%d) (%s@%s) currently has %s %s in the queue.\n",
		       list.user.title,
		       list.user.realname, list.user.instance,
		       list.user.username,
		       list.user.machine, article(list.topic),
		       list.topic);
	      else
		printf("%s %s (%d) (%s@%s) is just hanging out.\n%s\n",
		       list.user.title,list.user.realname,
		       list.user.instance,list.user.username,
		       list.user.machine,happy_message());
	    }
	}
      status = SUCCESS; 
      break; 
    default:
      status = handle_response(status, &Request);
      break;
    } 	
  return(status);   
}


