/* Copyright 1999 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: syncupdate.c,v 1.4 2001-10-23 21:59:02 amb Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef linux
#include <libgen.h>
#endif

#ifndef S_IAMB
#define S_IAMB 0x1FF	/* Access bits part of struct stat st_mode field */
#endif

static void copy(const char *source, const char *tempfile);
static void open_and_sync(const char *filename);
static void usage(void);
#ifdef linux
static void open_dir_and_sync(const char *filename);
#endif

int main(int argc, char **argv)
{
  int c;
  char *tempfile = NULL, *source, *dest;

  while ((c = getopt(argc, argv, "c:")) != EOF)
    {
      switch (c)
	{
	case 'c':
	  tempfile = optarg;
	  break;
	case '?':
	  usage();
	}
    }
  argc -= optind;
  argv += optind;
  if (argc != 2)
    usage();
  source = argv[0];
  dest = argv[1];

  /* Set up for the move. */
  if (tempfile)
    {
      copy(source, tempfile);
      source = tempfile;
    }
  else
    open_and_sync(source);

  if (rename(source, dest) == -1)
    {
      fprintf(stderr, "syncupdate: can't rename %s to %s: %s\n",
	      source, dest, strerror(errno));
      exit(1);
    }

#ifdef linux
  open_dir_and_sync(dirname(dest));
#endif

  exit(0);
}

static void copy(const char *source, const char *tempfile)
{
  int sfd, tfd, count, pos, n;
  struct stat statbuf;
  unsigned char buf[8192];

  /* Open the source file and get its status information. */
  sfd = open(source, O_RDONLY, 0);
  if (sfd == -1)
    {	
      fprintf(stderr, "syncupdate: can't read %s: %s\n", source,
	      strerror(errno));
      exit(1);
    }
  if (fstat(sfd, &statbuf) == -1)
    {
      fprintf(stderr, "syncupdate: can't stat %s: %s\n", source,
	      strerror(errno));
      exit(1);
    }

  /* Remove, open, and set mode on the temp file. */
  if (unlink(tempfile) == -1 && errno != ENOENT)
    {
      fprintf(stderr, "syncupdate: can't remove %s: %s\n", tempfile,
	      strerror(errno));
      exit(1);
    }
  tfd = open(tempfile, O_RDWR | O_CREAT | O_EXCL, 0);
  if (tfd == -1)
    {
      fprintf(stderr, "syncupdate: can't create %s: %s\n", tempfile,
	      strerror(errno));
      exit(1);
    }
  if (fchmod(tfd, statbuf.st_mode & S_IAMB) == -1)
    {
      fprintf(stderr, "syncupdate: can't fchmod %s: %s\n", tempfile,
	      strerror(errno));
      unlink(tempfile);
      exit(1);
    }

  /* Now copy and fsync. */
  while ((count = read(sfd, buf, sizeof(buf))) > 0)
    {
      for (pos = 0; pos < count; pos += n)
	{
	  n = write(tfd, buf + pos, count - pos);
	  if (n == -1)
	    {
	      fprintf(stderr, "syncupdate: can't write to %s: %s\n", tempfile,
		      strerror(errno));
	      unlink(tempfile);
	      exit(1);
	    }
	}
    }
  if (count == -1)
    {
      fprintf(stderr, "syncupdate: can't read from %s: %s\n", source,
	      strerror(errno));
    }

  if (fsync(tfd) == -1)
    {
      fprintf(stderr, "syncupdate: can't fsync %s: %s\n", tempfile,
	      strerror(errno));
      unlink(tempfile);
      exit(1);
    }

  close(sfd);
  close(tfd);
}

static void open_and_sync(const char *filename)
{
  int fd;

  fd = open(filename, O_RDWR, 0);
  if (fd == -1)
    {
      fprintf(stderr, "syncupdate: can't open %s: %s\n", filename,
	      strerror(errno));
      exit(1);
    }

  if (fsync(fd) == -1)
    {
      fprintf(stderr, "syncupdate: can't fsync %s: %s\n", filename,
	      strerror(errno));
      exit(1);
    }

  close(fd);
}

#ifdef linux
/* Despite the implication of the man page that fsync() will only work on
   RDWR file descriptors, it also works on directories open RDONLY.
   Since linux fsync() doesn't flush directory entries, force the issue. */
static void open_dir_and_sync(const char *filename)
{
  int fd;

  fd = open(filename, O_RDONLY, 0);
  if (fd == -1)
    {
      fprintf(stderr, "syncupdate: can't open %s: %s\n", filename,
	      strerror(errno));
      exit(1);
    }

  fsync(fd);
  close(fd);
}
#endif

static void usage(void)
{
  fprintf(stderr, "Usage: syncupdate [-c tempfile] source dest\n");
  exit(1);
}
