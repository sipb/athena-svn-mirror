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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_motd.c,v $
 *      $Author: vanharen $
 */


#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_motd.c,v 1.4 1989-10-11 16:30:55 vanharen Exp $";
#endif

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
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

  MuSetCursors(wl, 2, XC_watch);
  status = OGetMOTD(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:

      if (stat(file, &statbuf))
	{
	  fprintf(stderr, "motd: unable to stat motd file.\n");
	  MuError("motd: unable to stat motd file.");
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return(ERROR);
	}
	  
      if ((motd = malloc((1 + statbuf.st_size) * sizeof(char)))
	  == (char *) NULL)
	{
	  fprintf(stderr, "x_get_motd: unable to malloc space for motd.\n");
	  MuError("x_get_motd: unable to malloc space for motd.");
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return(ERROR);
	}

      if ((fd = open(file, O_RDONLY, 0)) < 0)
	{
	  MuError("x_get_motd: unable to open motd file for read.");
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return(ERROR);
	}

      if ((read(fd, motd, statbuf.st_size)) != statbuf.st_size)
	{
	  MuError("x_get_motd: unable to read motd correctly.");
	  MuSetCursors(wl, 2, XC_top_left_arrow);
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
	  XmTextSetString(w_motd_scrl, motd);
	  if (statbuf.st_size == 0)
	    XmTextSetString(w_motd_scrl, "There is no Message Of The Day right now.\nYou may want to check again later.");
	}
      close(fd);
      free(motd);

      break;

    default:
      status = handle_response(status, Request);
      break;
    }

  MuSetCursors(wl, 2, XC_top_left_arrow);
  return(status);
}
