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
 *	$Id: x_instance.c,v 1.6 1992-06-11 17:14:21 lwvanels Exp $
 *      $Author: lwvanels $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_instance.c,v 1.6 1992-06-11 17:14:21 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include "xolc.h"

ERRCODE
t_set_default_instance(Request)
     REQUEST *Request;
{
  int instance;  
  int status;

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





