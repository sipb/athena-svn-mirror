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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_list.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_list.c,v 1.3 1989-11-17 14:58:27 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_list_queue(Request,sort,queues,topics,users,stati,comments,file,display)
     REQUEST *Request;
     char **sort;
     char *queues;
     char *topics;
     char *users;
     int stati;
     int comments;
     char *file;
     int display;
{
  int status;
  LIST *list;

  status = OListQueue(Request,&list,queues,topics,users,stati);
  switch (status)
    {
    case SUCCESS:
      OSortListByUTime(list);
      OSortListByRule(list,sort);
      t_display_list(list,comments,file);
      if(display == TRUE)
	display_file(file,TRUE);
      else
	cat_file(file);
      free(list);
      break;

    case ERROR:
      fprintf(stderr, "Error listing conversations.\n");
      break;

    case EMPTY_LIST:
      if(Request->options)
	printf("No questions match given status.\n");
      else
	printf("The queue is empty.\n");
      status = SUCCESS;
      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  return(status);
}


ERRCODE
t_display_list(list,comments,file)
     LIST *list;
     int comments;
     char *file;
{
  LIST *l;
  char uinstbuf[10];
  char cinstbuf[10];
  char ustatusbuf[32];
  char chstatusbuf[10];
  char nseenbuf[5];
  char cbuf[32];
  char buf[32];
  FILE *fp;

  if(list->ustatus ==  END_OF_LIST) 
    {
      printf("Empty list\n");
      return(SUCCESS);
    }

  fp = fopen(file,"w");
  if(fp == NULL)
    {
      fprintf(stderr,"t_display_list: unable to open temp. file %s.\n",file);
      return(ERROR);
    }

  fprintf(fp,"User          Status            Consultant                Status     Topic\n\n");
  for(l=list; l->ustatus != END_OF_LIST; ++l)
    {
#ifdef TEST
        if(l->connected.uid >=0)
	  fprintf(fp,"connect: %s status: %d\n",l->connected.username, l->ckstatus);
	if(l->nseen >= 0)
	  fprintf(fp,"question: %s \n",l->topic);
#endif TEST

      buf[0] = '\0';
      cbuf[0]= '\0';
      if((l->nseen >=0) && (l->connected.uid >= 0))
	{	  
	  OGetStatusString(l->ustatus,buf);
	  OGetStatusString(l->ukstatus,cbuf);
	  sprintf(ustatusbuf,"(%s/%s)",buf,cbuf);

	  OGetStatusString(l->ckstatus,buf);
	  sprintf(chstatusbuf,"(%-3.3s)",buf);
	  sprintf(uinstbuf,"[%d]",l->user.instance);
	  sprintf(cinstbuf,"[%d]",l->connected.instance);
	  sprintf(nseenbuf,"(%d)",l->nseen);
	  sprintf(cbuf,"%s@%s",l->connected.username,l->connected.machine);
	  fprintf(fp,"%-8.8s %-4.4s %-17.17s %-20.20s %-4.4s %-5.5s %-4.4s %-10.10s",
		 l->user.username, uinstbuf, ustatusbuf,
		 cbuf, cinstbuf,chstatusbuf,nseenbuf,l->topic);
	  if(comments && (l->note[0] != '\0'))
	    fprintf(fp,"\t[%-64.64s]",l->note);
	}
      else
	if((l->nseen >= 0) && (l->connected.uid <0))
	  {
	    OGetStatusString(l->ustatus,buf);
	    OGetStatusString(l->ukstatus,cbuf);
	    sprintf(ustatusbuf,"(%s/%s)",buf,cbuf);
	    sprintf(uinstbuf,"[%d]",l->user.instance);
	    sprintf(nseenbuf,"(%d)",l->nseen);
	    fprintf(fp,"%-8.8s %-4.4s %-17.17s                                 %-4.4s %-10.10s",
		   l->user.username, uinstbuf, ustatusbuf, nseenbuf, l->topic);
	    if(comments && (l->note[0] != '\0'))
	      fprintf(fp,"\t[%-64.64s]",l->note);
	  }
	else
	  if((l->nseen < 0) && (l->connected.uid < 0))
	    continue;
	  else
	    {
	      fprintf(fp,"**unkown list entry***");
	    }
	fprintf(fp,"\n");	
    }

  for(l=list; l->ustatus != END_OF_LIST; ++l)
    if((l->nseen < 0) && (l->connected.uid < 0))
      {
	OGetStatusString(l->ukstatus,buf);
	sprintf(chstatusbuf,"(%-3.3s)",buf);
	sprintf(cbuf,"%s@%s",l->user.username,l->user.machine);
	sprintf(cinstbuf,"[%d]",l->user.instance);
	fprintf(fp,"                                %-20.20s %-4.4s %-5.5s\n",
	       cbuf, cinstbuf, chstatusbuf);
      }
  fclose(fp);
  return(SUCCESS);
}



