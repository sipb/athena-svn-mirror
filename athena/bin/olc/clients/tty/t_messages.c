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
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_messages.c,v 1.4 1989-08-22 13:54:24 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_replay(Request,file, display)
     REQUEST *Request;
     char *file;
     int display;
{
  int status;

  status = OReplayLog(Request,file);
  
  switch (status)
    {
    case SUCCESS:
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

  status = OShowMessage(Request,file);
   
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

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}
