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
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/x_motd.c,v 1.2 1989-07-31 15:14:52 vanharen Exp $";
#endif

#include "xolc.h"

/*extern Widget
  w_motd_scrl,
  w_motd_dlg;
*/

ERRCODE
x_get_motd(Request,type,file)
     REQUEST *Request;
     int type;
     char *file;
{
  int status;
  struct stat buf;
  Arg arg;
  char *motd;
  int fd;

  status = OGetMOTD(Request,type,file);
  
  switch(status)
    {
    case SUCCESS:
      stat(file, &buf);
      if ((motd = malloc((1 + buf.st_size) * sizeof(char))) == (char *) NULL)
	fprintf(stderr, "x_get_motd: unable to malloc space for motd.\n");
      if ((fd = open(file, O_RDONLY, 0)) < 0)
	fprintf(stderr, "x_get_motd: unable to open motd file for read.\n");
      if ((read(fd, motd, buf.st_size)) != buf.st_size)
	fprintf(stderr, "x_get_motd: unable to read motd correctly.\n");
      close(fd);
      motd[buf.st_size] = '\0';
/*
 * This part is gross.  I'm using this routine to set the motd for both a
 * dialog widget and a text widget, so I set them both here, and don't really
 * care which one I came here to do...  oh well.
 */
      XmTextSetString(w_motd_scrl, motd);

      XtSetArg(arg, XmNmessageString, XmStringLtoRCreate(motd, ""));
      XtSetValues(w_motd_dlg, &arg, 1);

      free(motd);
      break;

    default:
      XmTextSetString(w_motd_scrl, "An error has occurred.");

      XtSetArg(arg, XmNmessageString,
	       XmStringLtoRCreate("An error has occurred.", ""));
      XtSetValues(w_motd_dlg, &arg, 1);

      status = handle_response(Request,status);
      break;
    }

  return(status);
}
  
