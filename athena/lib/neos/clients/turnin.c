/**********************************************************************
 * File Exchange turnin client
 *
 * $Author: miki $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/turnin.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/turnin.c,v 1.2 1994-03-22 14:42:25 miki Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include "fxmain.h"
#include <strings.h>

/*
 * turnin_arg checks to see if the current argument is -g author,
 * which is an option unique to the turnin program.
 */

/*ARGSUSED*/
int
turnin_arg(argc, argv, ip, p, flagp)
     int argc;
     char *argv[];
     int *ip;
     Paper *p;
     int *flagp;
{
  if (argv[*ip][0] == '-' && argv[*ip][1] == 'g') {
    p->type = GRADED;
    p->author = argv[++(*ip)];
    return(1);
  }
  return(0);
}

long
do_turnin(fxp, p, flags, filename)
     FX *fxp;
     Paper *p;
     int flags;
     char *filename;
{
  long code;

  /* If no filename is specified, use stdin if it's not a terminal. */
  if (!filename) {
    if (isatty(0)) return(ERR_USAGE);
    if (!p->filename) p->filename = "stdin";
    if (code = fx_send(fxp, p, stdin))
      strcpy(fxmain_error_context, "while sending from stdin");
    return(code);
  }

  /* If a filename is specified, send that file. */
  if (code = fx_send_file(fxp, p, filename))
    sprintf(fxmain_error_context, "while sending %s", filename);
  else if (flags & VERBOSE) {
    if (p->type == GRADED) printf("Returned %s to %s in %s.", filename,
				  full_name(p->author), fxp->name);
    else printf("Turned in %s to %s on %s.\n",
		filename, fxp->name, fxp->host);
  }
  return(code);
}

main(argc, argv)
     int argc;
     char *argv[];
{
  Paper turnin_paper;

  paper_clear(&turnin_paper);
  turnin_paper.type = TURNEDIN;
  if (fxmain(argc, argv,
	     "USAGE: %s [options] assignment filename\n",
	     &turnin_paper, turnin_arg, do_turnin)) exit(1);
  exit(0);
}
