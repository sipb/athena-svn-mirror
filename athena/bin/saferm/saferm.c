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

/*
 * saferm
 *
 * Running as root, remove and optionally zero files in /tmp that
 * may be owned by anybody, avoiding any possible race conditions
 * resulting in security problems.
 */

static char rcsid[] = "$Id: saferm.c,v 1.4 1998-09-30 21:10:38 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

char *program_name;

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Safely open a file with intent to zero. */
int safe_open(char *filename, struct stat *file_stat)
{
  int file;
  struct stat name_stat;

  /* Try to open the file we think we want to zero. */
  file = open(filename, O_WRONLY);
  if (file == -1)
    {
      fprintf(stderr, "%s: open(%s) for writing failed: %s\n",
	      program_name, filename, strerror(errno));
      return -1;
    }

  /* Stat the file we actually opened. */
  if (fstat(file, file_stat) == -1)
    {
      fprintf(stderr, "%s: fstat(%s) failed: %s\n",
	      program_name, filename, strerror(errno));
      close(file);
      return -1;
    }

  /* Make sure it's a regular file. */
  if (!S_ISREG(file_stat->st_mode))
    {
      fprintf(stderr, "%s: %s is %s\n",
	      program_name, filename,
	      (S_ISDIR(file_stat->st_mode) 
	       ? "a directory"
	       : "not a regular file"));
      close(file);
      return -1;
    }

  /* Stat the file we think we want to zero. */
  if (lstat(filename, &name_stat) == -1)
    {
      fprintf(stderr, "%s: lstat(%s) failed: %s\n",
	      program_name, filename, strerror(errno));
      close(file);
      return -1;
    }

  /* We must be sure that we are not zeroing a hard link or symbolic
   * link to something like the password file. To do this, we must
   * verify that the link count on the file we zero is equal to 1, and
   * that we did not follow a symbolic link to reach that file.
   *
   * Simply lstat()ing the file before or after we open it is subject to
   * lose in a race condition, as there is no way to know that the file we
   * opened is the same file as the one we lstat()ed. Simply fstat()ing
   * the file we actually open can't tell us whether we followed a symbolic
   * link to get there, and also can't give us a reliable link count also
   * due to race conditions.
   *
   * But if we both lstat() and fstat() and compare the inode numbers from
   * the two cases, we can verify that the file we have open, at the time
   * of our lstat(), existed in a unique location identified by the pathname
   * we used to open it.
   *
   * So first, verify that our two stats got the same file.
   */
  if (file_stat->st_dev != name_stat.st_dev
      || file_stat->st_ino != name_stat.st_ino)
    {
      fprintf(stderr, "%s: %s and file opened are not identical\n",
	      program_name, filename);
      close(file);
      return -1;
    }

  /* Now verify that the link count is 1. Note that file_stat may not be
   * used here, otherwise we could be beaten by a race condition:
   *
   *   1. The attacker creates /tmp/passwd as a symlink to /etc/passwd.
   *   2. We open /tmp/passwd, which follows the symlink, and so we have
   *      fstat()ed /etc/passwd. The link count as of this stat is 1.
   *   3. The attacker removes the /tmp/passwd symlink and replaces it
   *      with a hard link to /etc/passwd.
   *   4. We lstat() /tmp/passwd. The link count now is 2.
   */
  if (name_stat.st_nlink != 1)
    {
      fprintf(stderr, "%s: multiple links for file %s, not zeroing\n",
	      program_name, filename);
      close(file);
      return -1;
    }

  /* Now we know it is safe to muck with the file we opened. */
  return file;
}

/* Zero the file represented by the passed file descriptor. Assumes
 * that the file pointer is already at the beginning of the file, and
 * file_stat points to a stat of the file.
 */
int zero(int file, struct stat *file_stat)
{
  char zeroes[BUFSIZ];
  int written = 0, status;

  memset(zeroes, 0, BUFSIZ);

  while (written < file_stat->st_size)
    {
      status = write(file, zeroes, MIN(BUFSIZ, file_stat->st_size - written));
      if (status == -1)
	{
	  fprintf(stderr, "%s: write() failed: %s\n",
		  program_name, strerror(errno));
	  return -1;
	}
      written += status;
    }

  fsync(file);
  return 0;
}

/* Return non-zero if it is safe to unlink NAME. */
int safe_to_unlink(char *name)
{
  struct stat st;
  
  if (lstat(name, &st) < 0)
    {
      fprintf(stderr, "%s: %s: %s\n", program_name, name, strerror(errno));
      return 0;
    }
  
  if (S_ISDIR(st.st_mode))
    {
      fprintf(stderr, "%s: %s is a directory\n", program_name, name);
      return 0;
    }

  return 1;
}

/* path is the full pathname to a file. We cd into the directory
 * containing the file, following no symbolic links. A pointer to just
 * the filename is returned on success (a substring of path), and NULL
 * is returned on failure.
 */
char *safe_cd(char *path)
{
  char *start_ptr, *end_ptr, *path_buf;
  struct stat above, below;

  if (path == NULL || *path == 0)
    return NULL;

  /* Make a copy of the path to be removed, since we're going to modify it. */
  path_buf = malloc(strlen(path) + 1);
  if (path_buf == NULL)
    {
      fprintf(stderr, "%s: out of memory\n", program_name);
      return NULL;
    }
  strcpy(path_buf, path);

  /* Step through each path component and cd into it, verifying
   * that we wind up where we expect to.
   *
   * strchr()ing from start_ptr + 1 handles the absolute path case
   * as well as beginning to address the double-/ case.
   */
  start_ptr = path_buf;
  while (end_ptr = strchr(start_ptr + 1, '/'))
    {
      /* Include any remaining extra /'s at the end of this component. */
      while (*(end_ptr + 1) == '/')
	end_ptr++;

      *end_ptr = 0;

      if (lstat(start_ptr, &above) == -1)
	{
	  fprintf(stderr, "%s: lstat of \"%s\" failed: %s\n", program_name,
		  path_buf, strerror(errno));
	  free(path_buf);
	  return NULL;
	}

      if (!S_ISDIR(above.st_mode))
	{
	  fprintf(stderr, "%s: final component of \"%s\" is not a directory\n",
		  program_name, path_buf);
	  free(path_buf);
	  return NULL;
	}

      if (chdir(start_ptr) == -1)
	{
	  fprintf(stderr, "%s: chdir to \"%s\" failed: %s\n", program_name,
		  path_buf, strerror(errno));
	  free(path_buf);
	  return NULL;
	}

      if (stat(".", &below) == -1)
	{
	  fprintf(stderr, "%s: stat of \"%s\" failed: %s\n", program_name,
		  path_buf, strerror(errno));
	  free(path_buf);
	  return NULL;
	}

      if (above.st_dev != below.st_dev || above.st_ino != below.st_ino)
	{
	  fprintf(stderr, "%s: final component of \"%s\" changed during "
		  "traversal\n", program_name, path_buf);
	  free(path_buf);
	  return NULL;
	}

      /* Advance to next component. */
      start_ptr = end_ptr + 1;

      /* Restore the / we nulled for diagnostics. */
      *end_ptr = '/';
    }

  free(path_buf);
  return (start_ptr - path_buf) + path;
}

/* Safely remove and optionally zero a file. -1 is returned for any
 * level of failure, such as zeroing failed but unlinking succeeded.
 */
int safe_rm(char *path, int zero_file)
{
  int file, status = 0;
  struct stat file_stat;
  char *name;

  name = safe_cd(path);
  if (name == NULL)
    return -1;

  /* Do the unlink before the actual zeroing so that the appearance
   * is atomic.
   */

  if (zero_file)
    {
      file = safe_open(name, &file_stat);
      if (file == -1)
	status = -1;
    }

  if (safe_to_unlink(name) && unlink(name) == -1)
    {
      fprintf(stderr, "%s: error unlinking %s: %s\n", program_name,
	      path, strerror(errno));
      status = -1;
    }

  if (zero_file && file != -1)
    {
      if (zero(file, &file_stat) == -1)
	status = -1;
      close(file);
    }

  return status;
}

/* Safely remove a directory. -1 is returned for any level of failure. */
int safe_rmdir(char *path, int quietflag)
{
  char *name;

  name = safe_cd(path);
  if (name == NULL)
    return -1;

  if (rmdir(name) == -1)
    {
      /* System V systems return EEXIST when a directory isn't empty
       * but hack the rmdir command to display a better error message.
       * Well, we can do that too.
       */
      if (errno == EEXIST)
	errno = ENOTEMPTY;

      if (!quietflag || errno != ENOTEMPTY)
	{
	  fprintf(stderr, "%s: error removing directory %s: %s\n",
		  program_name, path, strerror(errno));
	}
      return -1;
    }

  return 0;
}

int main(int argc, char **argv)
{
  int c, zeroflag = 0, dirflag = 0, quietflag = 0, errflag = 0;
  int dir, status = 0;

  /* Set up our program name. */
  if (argv[0] != NULL)
    {
      program_name = strrchr(argv[0], '/');
      if (program_name != NULL)
        program_name++;
      else
        program_name = argv[0];
    }
  else
    program_name = "saferm";

  while ((c = getopt(argc, argv, "dqz")) != EOF)
    {
      switch(c)
	{
	case 'd':
	  dirflag = 1;
	  break;
	case 'q':
	  quietflag = 1;
	  break;
	case 'z':
	  zeroflag = 1;
	  break;
	case '?':
	  errflag = 1;
	  break;
	}
    }

  if (errflag || optind == argc || (dirflag && zeroflag))
    {
      fprintf(stderr, "usage: %s [-q] [-d|-z] filename [filename ...]\n",
	      program_name);
      exit(1);
    }

  /* Remember our current directory in case we are being passed
   * relative paths.
   */
  dir = open(".", O_RDONLY);
  if (dir == -1)
    {
      fprintf(stderr, "%s: error opening current directory: %s\n",
	      program_name, strerror(errno));
      exit(1);
    }

  while (optind < argc)
    {
      /* If we're removing a relative path, be sure to cd to where
       * we started.
       */
      if (argv[optind][0] != '/')
	{
	  if (fchdir(dir) == -1)
	    {
	      fprintf(stderr, "%s: error cding to initial directory: %s\n",
		      program_name, strerror(errno));
	      exit(1);
	    }
	}

      if (dirflag && safe_rmdir(argv[optind], quietflag) == -1)
	status = 1;
      else if (!dirflag && safe_rm(argv[optind], zeroflag) == -1)
	status = 1;

      optind++;
    }

  exit(status);
}
