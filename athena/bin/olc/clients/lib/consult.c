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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: consult.c,v 1.11 1999-03-06 16:47:35 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: consult.c,v 1.11 1999-03-06 16:47:35 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

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

