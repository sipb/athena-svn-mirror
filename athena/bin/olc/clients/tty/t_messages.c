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
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_messages.c,v $
 *	$Id: t_messages.c,v 1.12 1990-05-26 11:59:58 vanharen Exp $
 *	$Author: vanharen $
 */

#ifndef lint
static const char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_messages.c,v 1.12 1990-05-26 11:59:58 vanharen Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

static const char *const default_sort_order[] = {
    "foo",
    "unconnected_consultants_last",
    "time",
    0,
};

ERRCODE
t_replay(Request, queues, topics, users, stati, file, display)
     REQUEST *Request;
     char *queues;
     char *topics;
     char *users;
     int stati;
     char *file;
     int display;
{
  int status, n;
  char c;
  LIST *list;
  LIST *l;

  if ((queues[0] == '\0') && (topics[0] == '\0') && (stati == 0))
    {
      status = OReplayLog(Request,file);
      switch (status)
	{
	case SUCCESS:
	  if (display)
	    status = display_file(file, TRUE);
	  break;

	case NOT_CONNECTED:
	  fprintf(stderr,
		  "%s [%d] does not have a question.\n",
		  Request->target.username, Request->target.instance);
	  break;

	case PERMISSION_DENIED:
	  fprintf(stderr, "You cannot replay log of %s [%d].\n",
		  Request->target.username, Request->target.instance);
	  break;
	  
	case ERROR:
	  fprintf(stderr, "Error replaying conversation.\n");
	  break;
	  
	case NAME_NOT_UNIQUE:
	  fprintf(stderr,
		  "The string \"%s\" is not unique.  Choose one of:\n",
		  Request->target.username);
	  strcpy(users, Request->target.username);
	  (void) t_list_queue(Request, (char *) NULL, (char) NULL, (char) NULL,
			      users, 0, 0, file, FALSE);
	  break;

	default:
	  status = handle_response(status, Request);
	  status = ERROR;
	  break;
	}
      return(status);
    }


  status = OListQueue(Request,&list,queues,topics,users,stati);
  OSortListByRule(list, default_sort_order);

  switch (status)
    {
    case SUCCESS:
      for(n = 0, l = list; l->ustatus != END_OF_LIST; l++, n++);

      for (l = list; l->ustatus != END_OF_LIST; l++)
	{
	  (void) strcpy(Request->target.username, l->user.username);
	  Request->target.instance = l->user.instance;
	  (void) t_replay(Request, (char *) NULL, (char *) NULL,
			  (char *) NULL, 0, file, display);
	  if (--n)
	    {
	      printf("========  Hit 'q' to quit, any other key ");
	      c = get_key_input("to continue.  ========");
	      printf("\n");
	      if ((c == 'q') || (c == 'Q'))
		{
		  free(list);
		  return(status);
		}
	    }
	}
      free(list);
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    case EMPTY_LIST:
      printf ("No questions match the given criteria.\n");
      status = SUCCESS;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}






ERRCODE
t_show_message(Request, file, display, connected, noflush)
     REQUEST *Request;
     char *file;
     int display;
     int connected;
     int noflush;
{
  int status;

  if(noflush)
    set_option(Request->options, NOFLUSH_OPT);

  if(connected)
    set_option(Request->options, CONNECTED_OPT);

  status = OShowMessageIntoFile(Request,file);
   
  switch (status)
    {
    case SUCCESS:
      if (display)
	display_file(file, TRUE);
      break;

    case NOT_CONNECTED:
      if(isme(Request))
	fprintf(stderr, "You are not connected.\n");
      else
	fprintf(stderr, "%s [%d] is not connected to anyone.\n",
		Request->target.username, Request->target.instance);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "Permission denied. \n");
      break;

    case NO_MESSAGES:
      printf("No new messages.\n");
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}


t_check_messages(Request)
     REQUEST *Request;
{
    int status;
    char file[NAME_SIZE];
    char prompt[BUF_SIZE];

    make_temp_name(file);
    status = OShowMessageIntoFile(Request,file);
    if (status == SUCCESS) {
	if(isme(Request))
	    strcpy (prompt, "You have");
	else {
	    strcpy (prompt, Request->target.username);
	    strcat (prompt, " has");
	}
	strcat (prompt, " unread message.  Display?  ");

	if (get_yn (prompt) == 'y')
	    display_file (file, TRUE);
    }

    return status;
}

t_check_connected_messages(Request)
     REQUEST *Request;
{
    int status;
    char file[NAME_SIZE];

    make_temp_name(file);
    set_option(Request->options,CONNECTED_OPT);
    status = OShowMessageIntoFile(Request,file);
    if(status == SUCCESS) {
	if (get_yn ("User has unread message.  Display?  ") == 'y')
	    display_file (file, TRUE);
    }
    unset_option(Request->options,CONNECTED_OPT);
    return status;
}
