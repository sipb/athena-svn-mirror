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

static const char rcsid[] = "$Id: busyd.c,v 1.2.6.1 2000-09-23 19:33:27 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <larv.h>

/* Reject queries from the echo, daytime, chargen, time, domain, and
 * busyd ports.
 */
static int reject[] = { 7, 13, 19, 37, 53, 49154 };
#define NREJECT (sizeof(reject) / sizeof(*reject))

int main(int argc, char **argv)
{
  struct sockaddr_in sin;
  int sz = sizeof(sin), i;
  char dummy;

  /* We were run (we hope) from inetd.  Read the request packet to
   * clear it from the queue.
   */
  if (recvfrom(0, &dummy, 1, 0, (struct sockaddr *) &sin, &sz) < 0)
    return 1;

  /* Reject queries from ports of well-known services which we could
   * get into a loop with.
   */
  for (i = 0; i < NREJECT; i++)
    {
      if (ntohs(sin.sin_port) == reject[i])
	return 1;
    }

  /* Now send the response. */
  if (larv_send_status(0, (struct sockaddr *) &sin, sz) == -1)
    return 1;

  return 0;
}
