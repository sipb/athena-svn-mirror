/* Copyright 1998 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: larv_send_status.c,v 1.1 1998-08-25 03:26:54 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include "larvnet.h"
#include "larv.h"
#include "larv_private.h"

int larv_send_status(int fd)
{
  char buf[LARVNET_MAX_PACKET];
  int len;

  len = larv__compose_packet(buf);
  if (len == -1)
    return -1;

  return send(fd, buf, len, 0);
}
