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
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/consult.c,v $
 *	$Id: consult.c,v 1.8 1990-07-16 08:14:14 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/consult.c,v 1.8 1990-07-16 08:14:14 lwvanels Exp $";
#endif

#include <mit-copyright.h>
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

