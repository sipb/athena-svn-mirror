/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_a_message.c,v $
 * $Author: vrt $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_get_a_message_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_a_message.c,v 1.2 1994-04-30 13:17:25 vrt Exp $";
#endif lint

#include "globalmessage.h"


/* return 0 status if the message is returned at all, else an error
 * code of some sort.
 */
#ifndef ultrix
#include <syslog.h>
#else
#include <nsyslog.h>
#endif

Code_t get_a_message(buf)
     char **buf;
{
  Code_t status;
  int size;
  char **server;

  status = get_servername(&server);
  if(!status) {
    status = get_message_from_server(buf, &size, server[0]);
  }

  if(status){
    syslog(LOG_INFO, "GMS get server failed, %s", error_message(status));
  }

  if(!status) {
    status = put_fallback_file(*buf, size, GMS_MESSAGE_NAME);
    /* but we don't really care what it is, the message is ok */
    if(status)
      syslog(LOG_INFO, "GMS put failed, %s", error_message(status));
    return(0);
  }

  status = get_fallback_file(buf, &size, GMS_MESSAGE_NAME);
  if(status) {
    syslog(LOG_INFO, "GMS put failed, %s", error_message(status));
    /* we have no message at all... */
    return(status);
  } else {
    return(0);
  }
}
