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

static const char rcsid[] = "$Id: get_a_message.c,v 1.1 1999-12-08 22:06:44 danw Exp $";

#include "globalmessage.h"
#include <syslog.h>
#include <com_err.h>


/* return 0 status if the message is returned at all, else an error
 * code of some sort.
 */

Code_t get_a_message(char **buf)
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
