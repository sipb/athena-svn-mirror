/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for dealing with administrative functions
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: admin.c,v 1.2 1999-01-22 23:12:01 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: admin.c,v 1.2 1999-01-22 23:12:01 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>


/*
 * Function:	OTweakZephyr(&Request,what, how_long) 
 * Description: turns on/off zephyr use on the server;
 *		what = 1 means turn it off, for how_long minutes
 *		    (how_long = 0 means indefinitely, -1 = server default)
 *		what = 0 means turn it on; how_long ignored
 * Returns:	ERRCODE
 */


ERRCODE
OTweakZephyr(Request,what,how_long)
     REQUEST *Request;
     int what;
     int how_long;
{
  int fd;
  int status;

  if (what == 1) {
    Request->options = OFF_OPT;
  } else {
    Request->options = NO_OPT;
  }

  status = open_connection_to_daemon(Request, &fd);
  if(status)
    return(status);

  status = send_request(fd, Request);
  if(status) {
    close(fd);
    return(status);
  }

  read_response(fd, &status);  
 
  if ((status == SUCCESS) && (what == 1)) {
    status = write_int_to_fd(fd,how_long);
  }

  close(fd);
  return(status);
}
