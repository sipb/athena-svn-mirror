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
 *	$Id: t_status.c,v 1.28 1999-06-28 22:52:18 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_status.c,v 1.28 1999-06-28 22:52:18 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_personal_status(Request,chart)
     REQUEST *Request;
     int chart;
{
  ERRCODE status;
  LIST *list;
  
  status = OListPerson(Request,&list);
  switch (status)
    {
    case SUCCESS:
      OSortListByUInstance(list);
      t_display_personal_status(Request,list,chart);
      free(list);
      break;

    case EMPTY_LIST:
      if(isme(Request))
	printf("You are not doing anything in %s.\n", client_service_name());
      else
	printf("%s %s [%d] (%s@%s) is not doing anything in %s.\n",
	       cap(Request->target.title), Request->target.realname, 
	       Request->target.instance, Request->target.username,
	       Request->target.machine, client_service_name());
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}


ERRCODE
t_display_personal_status(Request,list,chart)
     REQUEST *Request;
     LIST *list;
     int chart;
{
  LIST *l;
  char cbuf[32];
  char buf[32];
  int count=0;

  if(list->ustatus ==  END_OF_LIST) 
    {
      printf("Unable to get your listing.\n");
      return(SUCCESS);
    }
  else
    for(l=list; l->ustatus != END_OF_LIST; ++l)
      count++;

  if((count == 1) && (list->user.instance == 0) && !chart)
    {
      if(list->nseen >= 0)
	{
	  if(list->connected.uid >= 0)
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s %s [%d] (%s@%s) is ",
		       cap(list->user.title), list->user.realname, 
		       list->user.instance, list->user.username,
		       list->user.machine);

	      printf("currently connected to %s %s [%d] (%s@%s).\n",
		     list->connected.title, list->connected.realname,
		     list->connected.instance,
		     list->connected.username, list->connected.machine);
	    }		
	  else
	    {
	      if(isme(Request))
		printf("You currently have ");
	      else
		printf("%s %s [%d] (%s@%s) has ",
		       cap(list->user.title), list->user.realname, 
		       list->user.instance, list->user.username,
		       list->user.machine);

	      printf("%s \"%s\" question in the queue.\n",
		   article(list->topic),list->topic);
	      if(client_is_user_client())
		printf("You are waiting to be connected to a %s.\n",
		       client_default_consultant_title());
	    }
	}
      else
	{
	  if(list->connected.uid >= 0)
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s %s [%d] (%s@%s) is ",
		       cap(list->user.title), list->user.realname, 
		       list->user.instance, list->user.username,
		       list->user.machine);
	      
	      printf("connected to %s %s [%d] (%s@%s).\n",
		     list->connected.title, list->connected.realname,
		     list->connected.instance,
		     list->connected.username, list->connected.machine);
	    }
	  else
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s %s [%d] (%s@%s) is ",
		       cap(list->user.title), list->user.realname, 
		       list->user.instance, list->user.username,
		       list->user.machine);

	      printf("signed on to %s!\n", client_service_name());
	    }
	}
    }
  else
    {
      printf("Status for: %s (%s@%s)\n\n",list->user.realname,
	     list->user.username,list->user.machine);
      printf("   Instance   NM     Status     Connected to        Topic\n");
      for(l=list; l->ustatus != END_OF_LIST; ++l)
	{
	  if(isme(Request)
	     && (Request->requester.instance == l->user.instance))
	    printf("-> ");
	  else 
	    printf("   ");

	  if((l->nseen >=0) && (l->connected.uid >= 0))
	    {
	      OGetStatusString(l->ukstatus,cbuf);
	      printf("[%d]%s       %c      %-10.10s %-8.8s[%d]%s        %s\n",
		     l->user.instance, l->user.instance > 9 ? "" : " ",
		     l->umessage ? '*' : ' ', cbuf, l->connected.username,
		     l->connected.instance,
		     l->connected.instance > 9 ? "" : " ",
		     l->topic);
	    }
	  else
	    if((l->nseen >= 0) && (l->connected.uid <0))
	      {
		OGetStatusString(l->ukstatus,cbuf);
		printf("[%d]%s       %c      %-10.10s %s       %s\n",    
		       l->user.instance, l->user.instance > 9 ? "" : " ",
		       l->umessage ? '*' : ' ',
		       cbuf, "not connected",l->topic);
	      }
	    else
	      if((l->nseen < 0) && (l->connected.uid < 0))
		{
		  OGetStatusString(l->ukstatus,buf);
		  printf("[%d]%s       %c      %-10.10s %s\n",    
			 l->user.instance, l->user.instance > 9 ? "" : " ",
			 l->umessage ? '*' : ' ',
			 buf, "not connected");
		}
	      else
		{
		  printf("**unkown list entry***\n");
		}
	}
    }
  return(SUCCESS);
}



ERRCODE
t_who(Request)
     REQUEST *Request;
{
  ERRCODE status;
  LIST list;

  status = OWho(Request, &list);
 
  switch (status)
    { 
    case SUCCESS: 
      if(string_eq(Request->requester.username,list.user.username))
	{
	  if(list.connected.uid >= 0)
	    printf("You are currently connected to %s %s [%d] (%s@%s).\n",
                   list.connected.title, list.connected.realname,
		   list.connected.instance, list.connected.username,
		   list.connected.machine);
	  else
	    {
	      if(list.nseen >= 0)
		printf("You are not connected to a %s.\n",
		       client_default_consultant_title());
	      else
		printf("You are not connected to a user.\n");
	    }
	}
      else
	{
	  if(list.connected.uid > 0)
	    printf("%s %s [%d] (%s@%s) is currently connected to %s %s [%d] (%s@%s).\n",
		   cap(list.user.title), list.user.realname,
		   list.user.instance, list.user.username,
		   list.user.machine, list.connected.title,
		   list.connected.realname, list.connected.instance,
		   list.connected.username, list.connected.machine);
	  else
	    {
	      if(list.nseen >=0)
		printf("%s %s [%d] (%s@%s) currently has %s \"%s\" question in the queue.\n",
		       cap(list.user.title), list.user.realname,
		       list.user.instance, list.user.username,
		       list.user.machine, article(list.topic),
		       list.topic);
	      else
		printf("%s %s [%d] (%s@%s) is just hanging out.\n%s.\n",
		       cap(list.user.title), list.user.realname,
		       list.user.instance, list.user.username,
		       list.user.machine, happy_message());
	    }
	}
      status = SUCCESS; 
      break; 
    default:
      status = handle_response(status,Request);
      break;
    } 	
  return(status);   
}



ERRCODE
t_input_status(Request,string)
     REQUEST *Request;
     char *string;
{
  char buf[BUF_SIZE];
  int state = 0;

  if(string != (char *) NULL)
    strcpy(buf,string);
  else 
    buf[0] = '\0';

  while(1)
    {
      if(buf != '\0')
	{
	  if(is_flag(buf, "pit", 3))
	    state = STATUS_PICKUP;
	  else if(is_flag(buf, "referred", 1))
	    state = STATUS_REFERRED;
	  else if(is_flag(buf, "active", 1))
	    state = STATUS_ACTIVE;
	  else if(is_flag(buf, "pickup", 3))
	    state = STATUS_PICKUP;
	  else if(is_flag(buf, "pending", 2))
	    state = STATUS_PENDING;
	  else if(is_flag(buf, "unseen", 1))
	    state = STATUS_UNSEEN;
	}
      if(state)
	{
	  set_option(Request->options,state);
	  return(SUCCESS);
	}

      printf("Forward status may be one of...\n");
      printf("\t\tactive\n");
      printf("\t\tunseen\n");
      printf("\t\tpending\n");
      printf("\t\treferred\n");
      printf("\t\tpickup\n");
     
      buf[0] = '\0'; 
      get_prompted_input("enter new status (<return> to exit): ",buf,
			 BUF_SIZE,0);
      if(buf[0] == '\0')
	return(ERROR);
    }
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
      strcpy(string, "unknown");
      break;
    }
}

get_status_string(status,string)
     int status;
     char *string;
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
      case REFERRED:
	strcpy(string, "refer");
	break;
      case PICKUP:
	strcpy(string, "pickup");
	break;
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
  strcat(string, '\0');
}


t_pp_stati()
{
  int ind = 0;
  int i = 0;

  while (Status_Table[ind].status != UNKNOWN_STATUS)
    {
      printf("\t\t%-10s",Status_Table[ind].label);
      ind++;
      ++i;
      if(i > 2)
	{
	  i = 0;
	  printf("\n");
	}
    }
  printf("\n");
}