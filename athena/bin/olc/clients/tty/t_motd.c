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
 *      $Author: tjcoppet $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_motd.c,v 1.2 1989-08-04 11:12:35 tjcoppet Exp $";
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
	display_file(file,FALSE);
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  return(status);
}
  
