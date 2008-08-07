/* Copyright 2008 by the Massachusetts Institute of Technology.
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

/* busytest.c - Simple diagnostic program to query busy information
 * from an Athena machine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUSYPOLL 49154

int main(int argc, char **argv)
{
  struct hostent *host;
  struct sockaddr_in sin;
  int s, count, i;
  char dummy = 0, buf[1024], *name, *arch;
  socklen_t len;

  /* Verify and resolve the hostname argument. */
  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s hostname\n", argv[0]);
      exit(1);
    }
  host = gethostbyname(argv[1]);
  if (!host)
    {
      fprintf(stderr, "Could not resolve hostname %s: %s\n", argv[1],
	      strerror(errno));
      exit(1);
    }

  /* Construct the address. */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, host->h_addr, sizeof(sin.sin_addr));
  sin.sin_port = htons(BUSYPOLL);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    {
      fprintf(stderr, "Could not create socket: %s\n", strerror(errno));
      exit(1);
    }

  sendto(s, &dummy, 1, 0, (struct sockaddr *) &sin, sizeof(sin));
  printf("Packet sent to %s, waiting for response...\n", argv[1]);
  count = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &sin,
		   &len);
  if (len < 0)
    {
      fprintf(stderr, "Error receiving response: %s\n", strerror(errno));
      exit(1);
    }
  printf("Response received from %s\n", inet_ntoa(sin.sin_addr));
  if (len == 0)
    {
      fprintf(stderr, "Empty packet\n");
      exit(1);
    }
  buf[count] = 0;
  name = buf + 1;
  arch = name + strlen(name) + 1;
  if (arch >= buf + count)
    {
      fprintf(stderr, "Invalid packet\n");
      for (i = 0; i < count; i++)
	fprintf(stderr, "%03d: %03d (%c)\n", i, buf[i], buf[i]);
      exit(1);
    }
  printf("Busy state: %s\n", (buf[0] == '1') ? "busy" :
	 (buf[0] == '0') ? "free" : "invalid");
  printf("Hostname: %s\n", name);
  printf("Arch: %s\n", arch);
  return 0;
}
