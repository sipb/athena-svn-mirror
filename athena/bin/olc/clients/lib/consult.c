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
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/consult.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/consult.c,v 1.3 1989-08-04 11:18:53 tjcoppet Exp $";
#endif

#include <olc/olc.h>

ERRCODE
OSignOn(Request)
     REQUEST *Request;
{
  return(ORequest(Request,OLC_ON));
}

ERRCODE
OSignOff(Request)
     REQUEST *Request;
{
  return(ORequest(Request,OLC_OFF));
}

