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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_messages.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_messages.c,v 1.6 1990-01-17 02:40:25 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_replay(Request,file, display, sort)
     REQUEST *Request;
     char *file;
     int display;
     int sort;
{
  int status;
  char cmd[LINE_SIZE];

  status = OReplayLog(Request,file);
  
  switch (status)
    {
    case SUCCESS:
      if(sort)
	{
	  sprintf(cmd,"/usr/bin/sort %s -o %s",file,file);
	  system(cmd);
	}
      if (display)
	status = display_file(file, TRUE);
      break;

    case NOT_CONNECTED:
      fprintf(stderr,
	"%s (%d) does not have a question.\n",
	 Request->target.username, Request->target.instance);
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You cannot replay log of %s (%d).\n", 
	      Request->target.username, Request->target.instance);
       break;

    case ERROR:
      fprintf(stderr, "Error replaying conversation.\n");
       break;

    default:
      status = handle_response(status, Request);
      status = ERROR;
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
	fprintf(stderr, "%s (%d) is not connected to anyone.\n",
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
  char buf[BUF_SIZE];

  make_temp_name(file);
  status = OShowMessageIntoFile(Request,file);
  if(status == SUCCESS)
    {
      if(isme(Request))
	printf("You have ");
      else
	printf("%s has ", Request->target.username);

      *buf = '\0';
      get_prompted_input("unread messages. Display? ", buf);
      if(*buf == 'y')
	display_file(file, TRUE);
    }

  return(status);
}

t_check_connected_messages(Request)
     REQUEST *Request;
{
  int status;
  char file[NAME_SIZE];
  char buf[BUF_SIZE];

  make_temp_name(file);
  set_option(Request->options,CONNECTED_OPT);
  status = OShowMessageIntoFile(Request,file);
  if(status == SUCCESS)
    {
      *buf = '\0';
      get_prompted_input("User has unread messages. Display? ", buf);
      if(*buf == 'y')
	display_file(file, TRUE);
    }
  unset_option(Request->options,CONNECTED_OPT);
  return(status);
}
