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

static const char rcsid[] = "$Id: view_message_by_zephyr.c,v 1.1 1999-12-08 22:06:46 danw Exp $";

#include "globalmessage.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <com_err.h>

void view_message_by_zephyr(char *message)
{
  char *whoami;
  char *ptr;
  
  whoami = getenv("ATHENA_USER");
  if (!whoami)
    whoami = getenv("USER");
  
  if(!whoami)
    whoami = getlogin();
  
  if(!whoami) {
    struct passwd *pw;
    pw = getpwuid(getuid());
    if(pw) {
      whoami = pw->pw_name;
    } else {
      fprintf(stderr,
	      "get_message: couldn't find username to send zephyr notice\n");
      exit(2);
    }
  }
  /* skip magic headers */
  ptr = strchr(message, '\n')+1;
  
  /* check that there is *something* after the headers */
  if(*ptr) {
    /* don't even fork... this just exits anyway... */
    execl("/usr/bin/zwrite",
	  "zwrite", "-d", "-q", "-n",  whoami, "-m", ptr, 0);
    /* put logging here in case the exec fails. */
    syslog(LOG_INFO, "GMS client execl of zwrite failed [%s]",
	   error_message(errno));
  }
}
