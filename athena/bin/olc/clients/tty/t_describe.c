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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_describe.c,v $
 *	$Id: t_describe.c,v 1.9 1990-12-17 08:40:32 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_describe.c,v 1.9 1990-12-17 08:40:32 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
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

  if(dochnote)  {
    if (note == (char *) NULL)  {
      status = handle_response(ODescribe(Request, &list, "/dev/null", ""),
			       Request);
      if (status != SUCCESS)
	return(status);
      buf[0] = '\0';
      printf("Enter your note (%d chars max), default is:\n%15s[%-63.63s]\n",
	     NOTE_SIZE-1, " ", list.note);
      sprintf(mesg, "%16s", ">");
      get_prompted_input(mesg, buf);
      if (string_eq(buf, ""))
	return(SUCCESS);
      strncpy(notebuf,buf,NOTE_SIZE-1);
    }
    else
      strncpy(notebuf,note,NOTE_SIZE-1);

    set_option(Request->options,CHANGE_NOTE_OPT);
  }

  if(dochcomment)
    {
      status = enter_message(file,NULL);
      if(status != SUCCESS)
	return(status);
      set_option(Request->options,CHANGE_COMMENT_OPT);
    }
     
  notebuf[NOTE_SIZE-1] = '\0';
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
  char status1[NAME_SIZE];
  char status2[NAME_SIZE];
  
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
  
  
