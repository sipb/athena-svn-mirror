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
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Id: x_instance.c,v 1.9 1999-06-28 22:51:57 ghudson Exp $
 */

#ifndef lint
static char rcsid[]= "$Id: x_instance.c,v 1.9 1999-06-28 22:51:57 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include "xolc.h"

ERRCODE
t_set_default_instance(Request)
     REQUEST *Request;
{
  int instance;  
  ERRCODE status;

 try_again:
  status = OGetDefaultInstance(Request,&instance);
  switch(status)
    {
    case SUCCESS:
      User.instance = instance;
      Request->target.instance = instance;
      Request->requester.instance = instance;
      break;
    default:
      if (handle_response(status, Request) == FAILURE)
	goto try_again;
      break;
    }
  return(status);
}





