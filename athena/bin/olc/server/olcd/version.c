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
 *	$Id: version.c,v 1.2 1999-01-22 23:14:35 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: version.c,v 1.2 1999-01-22 23:14:35 ghudson Exp $";
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
