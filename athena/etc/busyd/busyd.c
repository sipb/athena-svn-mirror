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

static const char rcsid[] = "$Id: busyd.c,v 1.1 1998-08-25 03:31:14 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <larv.h>

int main(int argc, char **argv)
{
  struct sockaddr_in sin;
  int sz = sizeof(sin);
  char dummy;

  /* We were run (we hope) from inetd.  Read the request packet to
   * clear it from the queue.
   */
  if (recvfrom(0, &dummy, 1, 0, (struct sockaddr *) &sin, &sz) < 0)
    return 1;

  /* Now send the response. */
  if (larv_send_status(0) == -1)
    return 1;

  return 0;
}
