/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_describe.c,v $
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_describe.c,v 1.1 1989-08-04 11:12:03 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_describe(Request,file,note,dochnote,dochcomment)
     REQUEST *Request;
     char *file;
     char *note;
     int dochnote;
     int dochcomment;
{
  LIST list;
  int status;
  char buf[BUF_SIZE];
  char mesg[BUF_SIZE];
  char notebuf[NOTE_SIZE];

  if(dochnote && (note == (char *) NULL))
    {
      buf[0] = '\0';
      sprintf(mesg, "Enter your note (%d max chars):\n> ",NOTE_SIZE);
      get_prompted_input(mesg, buf);
      strncpy(notebuf,buf,NOTE_SIZE-1);
     }
  else
    if(dochnote)
      strncpy(notebuf,note,NOTE_SIZE-1);

  if(dochnote)
    set_option(Request->options,CHANGE_NOTE_OPT);

  if(dochcomment)
    {
      status = enter_message(file,NULL);
      if(status != SUCCESS)
	return(status);
      set_option(Request->options,CHANGE_COMMENT_OPT);
    }
     
  status = ODescribe(Request,&list,file,notebuf);
  
  switch(status)
    {
    case SUCCESS:
      printf("Question description change successful.\n");
      break;

    case OK:
      t_display_description(&list,file);
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  return(status);
}
  

t_display_description(list,file)
     LIST *list;
     char *file;
{
  char status1[NAME_LENGTH];
  char status2[NAME_LENGTH];
  
  printf("\nUser:   %s %s (%s@%s)\n",list->user.title,
	 list->user.realname, list->user.username,
	 list->user.machine);
  OGetStatusString(list->ustatus,status1);
  OGetStatusString(list->ukstatus,status2);
  printf("Status: %s/%s\n",status1,status2);
  
  if(list->connected.uid >= 0)
    {
      printf("Connected to:  %s %s (%s@%s)\n",list->connected.title,
	 list->connected.realname, list->connected.username,
	 list->connected.machine);
    }

  if(list->nseen >= 0)
    {
      printf("Topic:  %s\t\t#connected: %d\n",list->topic, list->nseen);
      printf("Note:   %s\n",list->note);
    }
  
  if(file_length(file) > 0)
      {
	printf("\nAdditional comments:\n");
	cat_file(file);
      }
}
  
  
