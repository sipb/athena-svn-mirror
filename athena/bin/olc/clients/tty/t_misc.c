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
 *	$Id: t_misc.c,v 1.8 1999-03-06 16:48:10 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_misc.c,v 1.8 1999-03-06 16:48:10 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

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


