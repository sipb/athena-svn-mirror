/* Copyright 1988, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: get_message.c,v 1.1 1999-12-08 22:06:44 danw Exp $";

#include "globalmessage.h"
void Usage(char *pname, char *errname);

#include <stdio.h>
#include <sys/types.h>
#include <syslog.h>
#include <com_err.h>

int main(int argc, char **argv)
{
  char **xargv = argv;
  int xargc = argc;
  Code_t status;
  char *message;
  int zephyr_p = 0, new_p = 0, login_p = 0;
  
  openlog(argv[0], LOG_PID, LOG_USER);
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

void Usage(char *pname, char *errname)
{
  fprintf(stderr, "%s <%s>: Usage: %s [-zephyr|-z] [-new|-n]\n",
	  pname, errname, pname);
}
