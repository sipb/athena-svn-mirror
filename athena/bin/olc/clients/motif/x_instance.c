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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_instance.c,v $
 *	$Id: x_instance.c,v 1.4 1991-03-24 14:33:52 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_instance.c,v 1.4 1991-03-24 14:33:52 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include "xolc.h"

t_set_default_instance(Request)
     REQUEST *Request;
{
  int instance;  
  int status;

  status = OGetDefaultInstance(Request,&instance);
  switch(status)
    {
    case SUCCESS:
      User.instance = instance;
      Request->target.instance = instance;
      Request->requester.instance = instance;
      break;
    default:
      handle_response(status, Request);
      break;
    }
  return(status);
}





