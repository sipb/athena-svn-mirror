/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with administrative commands
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_admin.c,v $
 *	$Id: t_admin.c,v 1.1 1991-11-05 14:07:58 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_admin.c,v 1.1 1991-11-05 14:07:58 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

ERRCODE
t_toggle_zephyr(Request,what,how_long)
     REQUEST *Request;
     int what, how_long;
{
  int status;

  status = OTweakZephyr(Request,what,how_long);
  
  switch(status)
    {
    case SUCCESS:
      break;

    default:
      status = handle_response(status,Request);
      break;
    }

  return(status);
}
