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

static const char rcsid[] = "$Id: larv__compose_packet.c,v 1.2.2.1 1998-09-24 13:54:49 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "larvnet.h"
#include "larv_private.h"

#define PATH_CONFIG "/etc/athena/rc.conf"

int larv__compose_packet(char *buf)
{
  int busy, len;
  char name[MAXHOSTNAMELEN + 1], line[BUFSIZ], *p, *arch;
  struct stat statbuf;
  FILE *fp;

  /* Determine whether the system is busy. */
  busy = (stat(LARVNET_PATH_BUSY, &statbuf) == 0);

  /* Determine the hostname. */
  if (gethostname(name, sizeof(name) - 1) == -1)
    strcpy(name, "unknown");
  name[sizeof(name) - 1] = 0;

  /* Determine the machine type. */
  fp = fopen(PATH_CONFIG, "r");
  if (fp)
    {
      while (fgets(line, sizeof(line), fp) != NULL)
	{
	  if (strncmp(line, "MACHINE=", 8) != 0)
	    continue;
	  arch = line + 8;
	  p = arch;
	  while (*p && !isspace(*p) && *p != ';')
	    p++;
	  *p = 0;
	  break;
	}
      fclose(fp);
    }
  else
    arch = "";

  /* Compute the length of the status packet and make sure we have space. */
  len = strlen(name) + strlen(arch) + 3;
  if (len > LARVNET_MAX_PACKET)
    return -1;

  /* Compose the status packet. */
  buf[0] = (busy) ? '1' : '0';
  strcpy(buf + 1, name);
  strcpy(buf + strlen(buf) + 1, arch);

  return len;
}
