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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_motd.c,v $
 *	$Id: t_motd.c,v 1.9 1991-01-03 15:33:04 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_motd.c,v 1.9 1991-01-03 15:33:04 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_get_file(Request,type,file,display_opts)
     REQUEST *Request;
     int type, display_opts;
     char *file;
{
  int status;

  status = OGetFile(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      if(display_opts)
	display_file(file);
      else
	cat_file(file);
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  return(status);
}
  

ERRCODE
t_change_file(Request,type,file, editor, incflag)
     REQUEST *Request;
     int type;
     char *file;
     char *editor;
     int incflag;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OChangeFile(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      break;

    case PERMISSION_DENIED:
      fprintf(stderr, "You are not allowed to do that.\n");
      status = ERROR;
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  if(status != SUCCESS)
    return(status);

  unset_option(Request->options, VERIFY);
  if((editor != (char *) NULL) && incflag)
    {
/* This hack to pass through the right "get file" request number */
      int ort = Request->request_type;
      Request->request_type = incflag;
      status = OGetFile(Request,type,file);
      Request->request_type = ort;
      if(status != SUCCESS)
	printf("Error getting file, continuing...\n");
    }
    
  status = enter_message(file,editor);
  if(status)
    return(status);

  status = OChangeFile(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("Change succesful. \n");
      break;
      
          default:
      status =  handle_response(status, Request);
      break;
    }

  return(status);
}

  
