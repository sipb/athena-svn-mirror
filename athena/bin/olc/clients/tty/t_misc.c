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
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_misc.c,v $
 *	$Id: t_misc.c,v 1.6 1991-08-23 13:35:43 raek Exp $
 *	$Author: raek $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_misc.c,v 1.6 1991-08-23 13:35:43 raek Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_dump(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  int status;

  status = ODump(Request,type,file);
  if(status == SUCCESS)
    display_file(file);
  else
    status = handle_response(status,Request);

  return(status);
}


