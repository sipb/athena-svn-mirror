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

/* main() for the attach suite */

static const char rcsid[] = "$Id: suite.c,v 1.3.4.1 1999-11-09 16:11:18 ghudson Exp $";

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "attach.h"
#include "agetopt.h"

char *whoami;

int main(int argc, char **argv)
{
  int fd;
  sigset_t mask;

  /* First, a suid safety check. */
  fd = open("/dev/null", O_RDONLY);
  if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
    exit(1);
  close(fd);

  /* Block ^Z to prevent holding locks on the attachtab. */
  sigemptyset(&mask);
  sigaddset(&mask, SIGTSTP);
  sigaddset(&mask, SIGTTOU);
  sigaddset(&mask, SIGTTIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  if (argc > 1 && !strncmp(argv[1], "-P", 2))
    {
      whoami = argv[1] + 2;
      argv++;
      argc--;
    }
  else
    {
      whoami = strrchr(argv[0], '/');
      if (whoami)
	whoami++;
      else
	whoami = argv[0];
    }

  if (!strcmp(whoami, "add"))
    exit(add_main(argc, argv));
  else if (!strcmp(whoami, "attach"))
    exit(attach_main(argc, argv));
  else if (!strcmp(whoami, "attachandrun"))
    exit(attachandrun_main(argc, argv));
  else if (!strcmp(whoami, "detach"))
    exit(detach_main(argc, argv));
  else if (!strcmp(whoami, "fsid") || !strcmp(whoami, "nfsid"))
    exit(fsid_main(argc, argv));
  else if (!strcmp(whoami, "zinit"))
    exit(zinit_main(argc, argv));

  fprintf(stderr, "Not invoked with attach, detach, nfsid, fsid, zinit, add, or attachandrun!\n");
  exit(1);
}
