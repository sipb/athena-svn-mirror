/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the procedure for printing the version number.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: t_version.c,v 1.3 1999-03-06 16:48:13 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_version.c,v 1.3 1999-03-06 16:48:13 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_tty.h>
#include "version.h"

ERRCODE
t_version(Request)
     REQUEST *Request;
{
  char *output;
  int status;

  status = OVersion(Request,&output);
  printf("Server version: %s\n",output);
  printf("Client version: %s: %s\n",VERSION_STRING,VERSION_INFO);
  return(status);
}
