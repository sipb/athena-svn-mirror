/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_message.c,v $
 * $Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_get_message_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_message.c,v 1.5 1997-04-22 00:37:54 ghudson Exp $";
#endif lint

#include "globalmessage.h"
void Usage();

#include <stdio.h>
#include <sys/types.h>
#ifndef ultrix
#include <syslog.h>
#else
#include <nsyslog.h>
#endif
char *error_message();


main (argc, argv)
     int argc;
     char *argv[];
{
  char **xargv = argv;
  int xargc = argc;
  Code_t status;
  char *message;
  int zephyr_p = 0, new_p = 0, login_p = 0;
  
#ifdef LOG_USER
  openlog(argv[0], LOG_PID, LOG_USER);
#else
  openlog(argv[0], LOG_PID);
#endif
  syslog(LOG_INFO, "GMS client started...");

  init_gms_err_tbl();

  /* Argument Processing:
   * 	-z or -zephyr: send the message as a zephyrgram.
   * 	-n or -new: only send if the message is newer
   */
  if(argc>3) {
    Usage(argv[0], "too many arguments");
    exit(1);
  }
  /* Only one valid argument: -zephyr or -z */
  while(--xargc) {
    xargv++;
    if((!strcmp(xargv[0],"-zephyr"))||(!strcmp(xargv[0],"-z"))) {
      zephyr_p = 1;
    } else if((!strcmp(xargv[0],"-new"))||(!strcmp(xargv[0],"-n"))) {
      new_p = 1;
    } else if((!strcmp(xargv[0],"-login"))||(!strcmp(xargv[0],"-l"))) {
      login_p = 1;
    } else {
      Usage(argv[0], xargv[0]);
      exit(1);
    }      
  }
  
  status = get_a_message(&message);
  if(!status) {
    /* check if the user has seen it already */
    status = check_viewable(message, new_p, !login_p);
    if(status) {
      syslog(LOG_INFO, "GMS not showing.");
      exit(0);
    }

    /* send it off if it passes the tests */
    if(zephyr_p) {
      view_message_by_zephyr(message);
    } else {
      view_message_by_tty(message);
    }
  } else {
    syslog(LOG_INFO, "GMS losing: %s", error_message(status));
  }

  exit(0);
}

void Usage(pname, errname)
     char *pname, *errname;
{
  fprintf(stderr, "%s <%s>: Usage: %s [-zephyr|-z] [-new|-n]\n",
	  pname, errname, pname);
}
