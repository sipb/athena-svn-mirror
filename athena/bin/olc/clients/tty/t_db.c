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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_db.c,v $
 *	$Id: t_db.c,v 1.7 1992-01-10 19:57:56 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_db.c,v 1.7 1992-01-10 19:57:56 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>


ERRCODE
t_load_user(Request)
     REQUEST *Request;
{
  int status;

  status = OLoadUser(Request);
  if(status == SUCCESS)
    printf("ok\n");
  else
    handle_response(status,Request);

  return(status);
}

ERRCODE
t_dbinfo(Request,file)
     REQUEST *Request;
     char *file;
{
  DBINFO dbinfo;
  int status;

  status = OGetDBInfo(Request,&dbinfo);
  if(status == SUCCESS)
    {
      printf("asker attributes:    %s %d\n",dbinfo.title1, dbinfo.max_ask);
      printf("answerer attributes: %s %d\n",dbinfo.title2, dbinfo.max_answer);
    }
  else
    status = handle_response(status,Request);

  return(status);
}

ERRCODE
t_change_dbinfo(Request)
     REQUEST *Request;
{
  DBINFO dbinfo;
  int status;
  char buf[BUF_SIZE];
  char mesg[BUF_SIZE];

  status = OGetDBInfo(Request,&dbinfo);
  if(status != SUCCESS)
    {
      status = handle_response(status,Request);
      fprintf(stderr,"Error querying.\n");
      return(ERROR);
    }

  sprintf(mesg, "title [%s]: ",dbinfo.title1);
  buf[0] = '\0';
  get_prompted_input(mesg,buf,BUF_SIZE,0);
  if(buf[0] != '\0')
    {
      strncpy(dbinfo.title1, buf,TITLE_SIZE);
      dbinfo.title1[TITLE_SIZE-1] = '\0';
    }
  
  sprintf(mesg, "# questions allowed to ask [%d]: ",dbinfo.max_ask);
  buf[0] = '\0';
  get_prompted_input(mesg,buf,BUF_SIZE,0);
  if(buf[0] != '\0')
    {
      if(atoi(buf) > 0)
	dbinfo.max_ask = atoi(buf);
    }

  sprintf(mesg, "title [%s]: ",dbinfo.title2);
  buf[0] = '\0';
  get_prompted_input(mesg,buf,BUF_SIZE,0);
  if(buf[0] != '\0')
    {
      strncpy(dbinfo.title2, buf,TITLE_SIZE);
      dbinfo.title2[TITLE_SIZE-1] = '\0';
    }

  sprintf(mesg, "# questions allowed to answer [%d]: ",dbinfo.max_answer);
  buf[0] = '\0';
  get_prompted_input(mesg,buf,BUF_SIZE,0);
  if(buf[0] != '\0')
    {
      if(atoi(buf) > 0)
	dbinfo.max_answer = atoi(buf);
    }

  status = OSetDBInfo(Request, &dbinfo);
  status = handle_response(status,Request);
  return(status);
}
