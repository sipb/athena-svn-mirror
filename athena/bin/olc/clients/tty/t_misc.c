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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_misc.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_misc.c,v 1.1 1989-11-17 14:11:30 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <olc/olc_tty.h>





t_dump(Request,file)
     REQUEST *Request;
     char *file;
{
  int status;

  status = ODump(Request,file);
  if(status == SUCCESS)
    display_file(file,TRUE);
  else
    status = handle_response(status,Request);

  return(status);
}


