/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for manipulating OLC data structures.
 *
 *	Win Treese
 *	Dan Morgan
 *	Bill Saphir
 *	MIT Project Athena
 *
 *	Ken Raeburn
 *	MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/utils.c,v $
 *	$Id: utils.c,v 1.10 1990-12-05 21:31:00 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/utils.c,v 1.10 1990-12-05 21:31:00 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

#include <strings.h>
#include <sys/types.h>		/* System type declarations. */
#include <sys/time.h>		/* System time definitions. */
#include <ctype.h>		/* character types */


void
get_user_status_string(u,status)
     USER *u;
     char *status;
{
  switch(u->status)
    {
    case ACTIVE:
      strcpy(status, "active");
      break;
    case LOGGED_OUT:
      strcpy(status, "logout");
      break;
    case MACHINE_DOWN:
      strcpy(status, "host down");
      break;
    default:
      strcpy(status, "unknown");
      break;
    }
}


void
get_status_string(k,status)
     KNUCKLE *k;
     char *status;
{
  *status = '\0';

printf("==> %d %d\n",is_busy(k), !is_busy(k));

  if(has_question(k))
    {
      if(k->question->owner == k)
	switch(k->status & QUESTION_STATUS)
	  {
	  case SERVICED:                  /* this is a bit! */
	    strcpy(status, "active");
	    break;
	  case PENDING:
	    strcpy(status, "pending");
	    break;
	  case NOT_SEEN:
	    strcpy(status, "unseen");
	    break;
	  case DONE:
	    strcpy(status, "done");
	    break;
	  case CANCEL:
	    strcpy(status, "cancel");
	    break;
	  default:
	    strcpy(status, "unknown");
	    break;
	  }
    }
  else
    {
      if(!is_busy(k))
	{
	  switch(k->status & SIGNED_ON)
	    {
	    case OFF:
	      strcpy(status, "off");
	      break;
	    case ON:
	      strcpy(status, "on");
	      break;
	    case FIRST:
	      strcpy(status, "sp1");
	      break;
	    case SECOND:
	      strcpy(status, "sp2");
	      break;
	    case DUTY:
	      strcpy(status, "dut");
	      break;
	    case URGENT:
	      strcpy(status, "urg");
	      break;
	    default:
	      strcpy(status, "unknown");
	      break;
	    }
	}
      else
	strcpy(status, "busy");
      
    }
  strcat(status, '\0');
}


void
get_list_info(k,data)
     KNUCKLE *k;
     LIST *data;
{ 
  data->user.uid = k->user->uid;
  data->user.instance = k->instance;
  data->ustatus = k->user->status;
  data->ukstatus = k->status;
  data->utime = k->timestamp;
  if(k->new_messages != (char *) NULL)
    data->umessage = TRUE;
  else
    data->umessage = FALSE;
  strcpy(data->user.username,k->user->username);
  strcpy(data->user.realname,k->user->realname);
  strcpy(data->user.machine,k->user->machine);
  strcpy(data->user.username,k->user->username);
  strcpy(data->user.title,k->title);

  if(is_connected(k))
    {
      data->connected.uid = k->connected->user->uid;
      data->connected.instance = k->connected->instance;
      data->cstatus = k->connected->user->status;
      data->ckstatus = k->connected->status;
      strcpy(data->connected.username,k->connected->user->username);
      strcpy(data->connected.realname,k->connected->user->realname);
      strcpy(data->connected.machine,k->connected->user->machine);
      strcpy(data->connected.title,k->connected->title);
      data->ctime = k->timestamp;
      if(k->new_messages != (char *) NULL)
	data->cmessage = TRUE;
      else
	data->cmessage = FALSE;
    }
  else 
    data->connected.uid = -1;

  if(has_question(k))
    {
      strncpy(data->topic, k->question->topic,TOPIC_SIZE);
      strncpy(data->note, k->question->note,NOTE_SIZE);
      data->nseen = k->question->nseen;
    }
  else 
    {
      data->nseen = -1;
      data->note[0] = '\0';
      data->topic[0] = '\0';
    }
}
