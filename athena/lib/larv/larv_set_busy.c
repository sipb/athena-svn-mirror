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

static const char rcsid[] = "$Id: larv_set_busy.c,v 1.1 1998-08-25 03:26:59 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <hesiod.h>
#include "larvnet.h"
#include "larv.h"
#include "larv_private.h"

int larv_set_busy(int busy)
{
  struct servent *serv;
  struct hostent *host;
  struct sockaddr_in sin;
  unsigned short port;
  int fd, s, len;
  void *ctx;
  char **vec, **vp, buf[LARVNET_MAX_PACKET];

  /* Create or unlink the busy file, as appropriate. */
  if (busy)
    {
      fd = open(LARVNET_PATH_BUSY, O_CREAT, S_IRWXU);
      if (fd < 0)
	return -1;
      else
	close(fd);
    }
  else
    {
      if (unlink(LARVNET_PATH_BUSY) < 0 && errno != ENOENT)
	return -1;
    }

  /* Compose the status packet.  It's important that we do this step
   * after creating or unlinking the busy file, since
   * larv__compose_packet() will check whether the busy file exists.
   */
  len = larv__compose_packet(buf);
  if (len == -1)
    return -1;

  serv = getservbyname("larvnet", "udp");
  port = (serv) ? serv->s_port : htons(LARVNET_FALLBACK_PORT);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s == -1)
    return -1;

  if (hesiod_init(&ctx) != 0)
    {
      close(s);
      return -1;
    }

  vec = hesiod_resolve(ctx, "larvnet", "sloc");
  if (!vec)
    {
      hesiod_end(ctx);
      close(s);
      return (errno == ENOENT) ? 0 : -1;
    }

  /* Send the status packet to each cview service location record. */
  for (vp = vec; *vp; vp++)
    {
      host = gethostbyname(*vp);
      if (!host || host->h_addrtype != AF_INET
	  || host->h_length != sizeof(sin.sin_addr))
	continue;
      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_port = port;
      memcpy(&sin.sin_addr, host->h_addr, sizeof(sin.sin_addr));
      sendto(s, buf, len, 0, (struct sockaddr *) &sin, sizeof(sin));
    }

  hesiod_free_list(ctx, vec);
  hesiod_end(ctx);
  close(s);
  return 0;
}
