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
 *	$Id: t_admin.c,v 1.3 1999-03-06 16:48:07 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_admin.c,v 1.3 1999-03-06 16:48:07 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

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
