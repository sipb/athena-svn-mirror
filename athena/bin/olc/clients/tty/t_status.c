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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_status.c,v 1.8 1989-11-17 14:12:35 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_personal_status(Request,chart)
     REQUEST *Request;
     int chart;
{
  int status;
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
	printf("You are not doing anything in OLC.\n");
      else
	printf("%s (%d) is not doing anything in OLC.\n",
	       Request->target.username, Request->target.instance);
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

#ifdef TEST
  printf("count: %d\n", count);
#endif TEST

  if((count == 1) && (list->user.instance == 0) && !chart)
    {
      if(list->nseen >= 0)
	{
	  if(list->connected.uid >= 0)
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s %s is ",list->user.title, list->user.username);

	      printf("currently connected to %s %s (%s@%s)\n",
		   list->connected.title,list->connected.realname,
		     list->connected.username,list->connected.machine);
	    }		
	  else
	    {
	      if(isme(Request))
		printf("You currently have ");
	      else
		printf("%s %s has ",list->user.title, list->user.username);

	      printf("%s \"%s\" question in the queue.\n",
		   article(list->topic),list->topic);
	      if(OLC)
		printf("You are waiting to be connected to a consultant.\n");
	    }
	}
      else
	{
	  if(list->connected.uid >= 0)
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s is ",list->user.username);
	      
	      printf("connected to %s %s (%s@%s)\n",
		     list->connected.title,list->connected.realname,
		     list->connected.username,list->connected.machine);
	    }
	  else
	    {
	      if(isme(Request))
		printf("You are ");
	      else
		printf("%s is ",list->user.username);
	      printf("signed on to OLC!\n");
	    }
	}
    }
  else
    {
      printf("Status for: %s (%s@%s)\n\n",list->user.realname,
	     list->user.username,list->user.machine);
      printf("   instance   nm     status     connected party     topic\n");
      for(l=list; l->ustatus != END_OF_LIST; ++l)
	{
	  if(Request->requester.instance == l->user.instance)
	    printf("-> ");
	  else 
	    printf("   ");
	  if((l->nseen >=0) && (l->connected.uid >= 0))
	    {
	      OGetStatusString(l->ukstatus,cbuf);
	      printf("[%d]%s       %c      %-10.10s %-9.9s (%d)%s      %s\n",
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
	  if(list.connected.uid >= 0)
	    printf("You are currently connected to %s %s (%s@%s)\n",
                   list.connected.title,
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
		   cap(list.user.title), list.user.realname, 
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
      status = handle_response(status,Request);
      break;
    } 	
  return(status);   
}




t_input_status(Request,string)
     REQUEST *Request;
     char *string;
{
  char buf[BUFSIZE];
  int status = 0;
  int state = 0;

  if(string != (char *) NULL)
    strcpy(buf,string);
  else 
    buf[0] = '\0';

  while(!status)
    {
      if(buf != '\0')
	{
	  if(string_equiv(buf,
		       "pit",max(strlen(buf),2)))
	    state = STATUS_PICKUP;
	  else
	    if(string_equiv(buf,
			 "referred",max(strlen(buf),2)))
	      state = STATUS_REFERRED;
	    else
	      if(string_equiv(buf,
			   "active",max(strlen(buf),2)))
		state = STATUS_ACTIVE;
	      else
		if(string_equiv(buf,
			     "pickup",max(strlen(buf),2)))
		  state = STATUS_PICKUP;
		else
		  if(string_equiv(buf,
			       "pending",max(strlen(buf),2)))
		    state = STATUS_ACTIVE;
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
      get_prompted_input("enter new status (<return> to exit): ",buf);
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
  int index = 0;
  int i = 0;

  while (Status_Table[index].status != UNKNOWN_STATUS)
    {
      printf("\t\t%-10s",Status_Table[index].label);
      index++;
      ++i;
      if(i > 2)
	{
	  i = 0;
	  printf("\n");
	}
    }
  printf("\n");
}

