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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_topic.c,v $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_topic.c,v 1.4 1991-04-18 21:52:08 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <Xm/List.h>

#include "xolc.h"

TOPIC TopicTable[256];

ERRCODE
x_list_topics(Request, file)
     REQUEST *Request;
     char *file;
{
  int status;
  FILE *infile;
  char inbuf[BUF_SIZE];
  int i = 0;

  status = OListTopics(Request,file);
  switch(status)
    {
    case SUCCESS:
      infile = fopen(file, "r");
      i = 0;
      while (fgets(inbuf, BUF_SIZE, infile) != NULL)
	{
	  inbuf[strlen(inbuf) - 1] = (char) '\0';
	  sscanf(inbuf, "%s", TopicTable[i].topic);
	  i++;
	  AddItemToList(w_list, inbuf);
	}
      fclose(infile);
      break;
      
    case ERROR:
      MuError("Cannot list OLC topics.");
      status = ERROR;
      break;
      
    default:
      status = handle_response(status, Request);
      break;
    }
  return(status);
}
