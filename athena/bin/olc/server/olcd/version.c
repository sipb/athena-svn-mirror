/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for handling requests from the olc
 * client.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/version.c,v $
 *	$Id: version.c,v 1.1 1991-10-29 14:11:34 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/version.c,v 1.1 1991-10-29 14:11:34 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>
#include "version.h"

ERRCODE
olc_version(fd,request)
     int fd;
     REQUEST *request;
{
  char vtext[256];

  sprintf(vtext,"%s: %s", VERSION_INFO,SERVER_VERSION_STRING);
  send_response(fd,SUCCESS);
  write_text_to_fd(fd,vtext);
  return(SUCCESS);
}
