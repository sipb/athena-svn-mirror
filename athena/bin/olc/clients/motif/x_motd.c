/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with motd's.
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
 *      Copyright (c) 1989,1991 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_motd.c,v $
 *      $Id: x_motd.c,v 1.7 1991-08-12 13:43:19 lwvanels Exp $
 *      $Author: lwvanels $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_motd.c,v 1.7 1991-08-12 13:43:19 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <Xm/Text.h>

#include "xolc.h"

ERRCODE
x_get_motd(Request,type,file,dialog)
     REQUEST *Request;
     int type;
     char *file;
     int dialog;
{
  int status;
  struct stat statbuf;
  Arg arg;
  char *motd;
  int fd;

  WAIT_CURSOR;

  Request->request_type = OLC_MOTD;
  status = OGetFile(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:

      if (stat(file, &statbuf))
	{
	  MuError("motd: unable to stat motd file.");
	  STANDARD_CURSOR;
	  return(ERROR);
	}
	  
      if ((motd = malloc((1 + statbuf.st_size) * sizeof(char)))
	  == (char *) NULL)
	{
	  MuError("x_get_motd: unable to malloc space for motd.");
	  STANDARD_CURSOR;
	  return(ERROR);
	}

      if ((fd = open(file, O_RDONLY, 0)) < 0)
	{
	  close(fd);
	  free(motd);
	  MuError("x_get_motd: unable to open motd file for read.");
	  STANDARD_CURSOR;
	  return(ERROR);
	}

      if ((read(fd, motd, statbuf.st_size)) != statbuf.st_size)
	{
	  close(fd);
	  free(motd);
	  MuError("x_get_motd: unable to read motd correctly.");
	  STANDARD_CURSOR;
	  return(ERROR);
	}
      
      motd[statbuf.st_size] = '\0';
      if (dialog)
	{
	  XtSetArg(arg, XmNmessageString, MotifString(motd));
	  if (statbuf.st_size == 0)
	    XtSetArg(arg, XmNmessageString, MotifString("There is no Message Of The Day right now.\nYou may want to check again later."));
	  XtSetValues(w_motd_dlg, &arg, 1);
	}
      else
	{
	  if (statbuf.st_size == 0)
	    XmTextSetString(w_motd_scrl, "There is no Message Of The Day right now.\nYou may want to check again later.");
	  else
	    XmTextSetString(w_motd_scrl, motd);
	}
      close(fd);
      free(motd);

      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  STANDARD_CURSOR;
  return(status);
}
