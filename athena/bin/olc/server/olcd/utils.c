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
 *	$Id: utils.c,v 1.13 1995-05-14 01:09:20 cfields Exp $
 *	$Author: cfields $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/utils.c,v 1.13 1995-05-14 01:09:20 cfields Exp $";
#endif
#endif

#include <mit-copyright.h>



#include <strings.h>
#include <sys/types.h>		/* System type declarations. */
#include <sys/time.h>		/* System time definitions. */
#include <ctype.h>		/* character types */
#include <olcd.h>

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
  if(has_new_messages(k))
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
      if(has_new_messages(k))
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
