/* Copyright 2000 by the Massachusetts Institute of Technology.
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

/* attachandrun - attach a locker and run a program out of it.
 * Intended mainly for use by scripts.
 */

static const char rcsid[] = "$Id: attachandrun.c,v 1.2 2000-04-14 20:53:52 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <athdir.h>

static char *run_attach(const char *locker);
static char *read_from_pipe(int fd);
static void try_shell_exec(char *path, int argc, char **argv);
static void *emalloc(size_t size);
static void *erealloc(void *ptr, size_t size);
static void usage(void);

static const char *progname;

int main(int argc, char **argv)
{
  const char *locker, *program;
  char *locker_path, **paths, *prog_path;
  int check = 0;

  progname = strrchr(argv[0], '/');
  progname = (progname == NULL) ? argv[0] : progname;

  /* Process arguments.  Leave argc/argv specifying the arguments for
   * the command to run.
   */
  if (argc >= 2 &&
      (strcmp(argv[1], "--check") == 0 || strcmp(argv[1], "-c") == 0))
    {
      if (argc < 3)
	usage();

      check = 1;
      argc--;
      argv++;
    }
  else if (argc < 4)
    usage();
  locker = argv[1];
  program = argv[2];
  argc -= 3;
  argv += 3;

  /* Run attach and get the path to run. */
  locker_path = run_attach(locker);
  paths = athdir_get_paths(locker_path, "bin", NULL, NULL, NULL, NULL, 0);
  if (paths == NULL)
    {
      fprintf(stderr, "%s: can't find a binary directory in %s\n",
	      progname, locker_path);
      exit(1);
    }
  prog_path = emalloc(strlen(paths[0]) + 1 + strlen(program) + 1);
  sprintf(prog_path, "%s/%s", paths[0], program);
  athdir_free_paths(paths);

  /* If we're just doing a check, do that now and exit. */
  if (check)
    exit((access(prog_path, X_OK) == 0) ? 0 : 1);

  /* Run the command. */
  execv(prog_path, argv);
  if (errno == ENOEXEC)
    try_shell_exec(prog_path, argc, argv);
  fprintf(stderr, "%s: failure to exec %s: %s\n", progname, program,
	  strerror(errno));
  exit(1);
}

/* Run attach -p <lockername> and return the locker path attach prints. */
static char *run_attach(const char *locker)
{
  pid_t pid;
  int fds[2], status;
  char *locker_path;

  if (pipe(fds) == -1)
    {
      fprintf(stderr, "%s: pipe failed: %s\n", progname, strerror(errno));
      exit(1);
    }

  pid = fork();
  if (pid == -1)
    {
      fprintf(stderr, "%s: fork failed: %s\n", progname, strerror(errno));
      exit(1);
    }
  else if (pid == 0)
    {
      /* In the child, run attach with stdout directed at the write
       * side of the pipe.
       */
      close(fds[0]);
      dup2(fds[1], STDOUT_FILENO);
      if (fds[1] != STDOUT_FILENO)
	close(fds[1]);
      execl("/bin/athena/attach", "attach", "-p", locker, (char *) NULL);
      fprintf(stderr, "%s: execl failed: %s\n", progname, strerror(errno));
      _exit(1);
    }

  /* In the parent, read the locker path from the pipe. */
  close(fds[1]);
  locker_path = read_from_pipe(fds[0]);
  close(fds[0]);
  waitpid(pid, &status, 0);
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    exit(1);
  if (!*locker_path)
    {
      fprintf(stderr, "%s: attach failed to output locker path\n", progname);
      exit(1);
    }
  return locker_path;
}

static char *read_from_pipe(int fd)
{
  char *buf;
  int count, pos, sz;

  sz = BUFSIZ;
  buf = emalloc(sz + 1);
  pos = 0;
  while (1)
    {
      count = read(fd, buf + pos, sz - pos);
      if (count == -1)
	{
	  fprintf(stderr, "%s: read from pipe failed: %s\n", progname,
		  strerror(errno));
	  exit(1);
	}
      else if (count == 0)
	{
	  buf[pos] = 0;
	  if (pos > 0 && buf[pos - 1] == '\n')
	    buf[pos - 1] = 0;
	  return buf;
	}
      pos += count;
      if (pos == sz)
	{
	  sz += BUFSIZ;
	  buf = erealloc(buf, sz + 1);
	}
    }
}

/* Try to feed a text file to the shell by hand.  On error, errno will
 * be the error value from open() or read() or execv(), or ENOEXEC if
 * the file looks binary, or ENOMEM if we couldn't allocate memory for
 * an argument list.
 */
static void try_shell_exec(char *path, int argc, char **argv)
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

static void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    {
      fprintf(stderr, "%s: malloc size %lu failed", progname,
	      (unsigned long) size);
      exit(1);
    }
  return ptr;
}

static void *erealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    {
      fprintf(stderr, "%s: realloc size %lu failed", progname,
	      (unsigned long) size);
      exit(1);
    }
  return ptr;
}

static void usage(void)
{
  fprintf(stderr, "Usage: %s [--check] locker program argv0 [argv1...]\n",
	  progname);
  exit(1);
}
