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

/* This is attachandrun, used to attach a locker and run a program in it. */

static const char rcsid[] = "$Id: attachandrun.c,v 1.1 1999-02-26 23:13:01 danw Exp $";

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <athdir.h>
#include <locker.h>

void usage(void);
void try_shell_exec(char *path, int argc, char **argv);

char *whoami;

int main(int argc, char **argv)
{
  locker_context context;
  locker_attachent *at;
  uid_t uid = getuid();
  char **found, *path, *mountpoint;

  whoami = strrchr(argv[0], '/');
  if (whoami)
    whoami++;
  else
    whoami = argv[0];

  if (argc < 4)
    usage();

  if (locker_init(&context, uid, NULL, NULL))
    exit(1);

  if (locker_attach(context, argv[1], NULL, LOCKER_AUTH_DEFAULT, 0, NULL, &at))
    exit(1);
  mountpoint = strdup(at->mountpoint);
  if (!mountpoint)
    {
      fprintf(stderr, "%s: Out of memory.\n", whoami);
      exit(1);
    }
  locker_free_attachent(context, at);
  locker_end(context);

  if (setuid(uid))
    {
      fprintf(stderr, "%s: setuid call failed: %s\n", whoami, strerror(errno));
      exit(1);
    }

  found = athdir_get_paths(mountpoint, "bin", NULL, NULL, NULL, NULL, 0);
  if (found)
    {
      path = malloc(strlen(found[0]) + strlen(argv[2]) + 2);
      if (!path)
	{
	  fprintf(stderr, "%s: Can't malloc while preparing for exec\n",
		  whoami);
	  exit(1);
	}

      sprintf(path, "%s/%s", found[0], argv[2]);
      athdir_free_paths(found);

      execv(path, argv + 3);

      /* Feed text files to the shell by hand. */
      if (errno == ENOEXEC)
	try_shell_exec(path, argc - 3, argv + 3);

      fprintf(stderr, "%s: failure to exec %s: %s\n", whoami, argv[2],
	      strerror(errno));
      free(path);
      exit(1);
    }
  else
    {
      fprintf(stderr, "%s: couldn't find a binary directory in %s\n",
	      whoami, mountpoint);
      exit(1);
    }
}

/* Try to feed a text file to the shell by hand.  On error, errno will
 * be the error value from open() or read() or execv(), or ENOEXEC if
 * the file looks binary, or ENOMEM if we couldn't allocate memory for
 * an argument list.
 */
void try_shell_exec(char *path, int argc, char **argv)
{
  int i, count, fd, err;
  unsigned char sample[128];
  char **arg;

  /* First we should check if the file looks binary.  Open the file. */
  fd = open(path, O_RDONLY, 0);
  if (fd == -1)
    return;

  /* Read in a bit of data from the file. */
  count = read(fd, sample, sizeof(sample));
  err = errno;
  close(fd);
  if (count < 0)
    {
      errno = err;
      return;
    }

  /* Look for binary characters in the first line. */
  for (i = 0; i < count; i++)
    {
      if (sample[i] == '\n')
	break;
      if (!isspace(sample[i]) && !isprint(sample[i]))
	{
	  errno = ENOEXEC;
	  return;
	}
    }

  /* Allocate space for a longer argument list. */
  arg = malloc((argc + 2) * sizeof(char *));
  if (!arg)
    {
      errno = ENOMEM;
      return;
    }

  /* Set up the argument list. Copy in the argument part of argv
   * including the terminating NULL. argv[0] is lost, unfortunately.
   */
  arg[0] = "/bin/sh";
  arg[1] = path;
  memcpy(arg + 2, argv + 1, argc * sizeof(char *));

  execv(arg[0], arg);
  free(arg);
  return;
}

void usage(void)
{
  fprintf(stderr, "Usage: attachandrun locker program argv0 [argv1...]\n");
  exit(1);
}
