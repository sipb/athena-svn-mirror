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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_motd.c,v $
 *      $Author: vanharen $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_motd.c,v 1.5 1990-04-25 16:51:27 vanharen Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_get_motd(Request,type,file,display_opts)
     REQUEST *Request;
     int type, display_opts;
     char *file;
{
  int status;

  status = OGetMOTD(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      if(display_opts)
	display_file(file,TRUE);
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
t_change_motd(Request,type,file, editor, incflag)
     REQUEST *Request;
     int type;
     char *file;
     char *editor;
     int incflag;
{
  int status;

  set_option(Request->options, VERIFY);
  status = OChangeMOTD(Request,type,file);
  
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
      status = OGetMOTD(Request,type,file);
      if(status != SUCCESS)
	printf("Error getting motd, continuing...\n");
    }
    
  status = enter_message(file,editor);
  if(status)
    return(status);

  status = OChangeMOTD(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      printf("MOTD change succesful. \n");
      break;
      
          default:
      status =  handle_response(status, Request);
      break;
    }

  return(status);
}

  
