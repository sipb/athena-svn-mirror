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

/* This program checks a list of files against stat information
 * as produced by the os-statfiles program, and recreates regular
 * files, symlinks, and directories whose stat information does
 * not match the given data.  Files are recreated by copying from
 * a given root directory, presumably an Athena /os hierarchy.
 */

static const char rcsid[] = "$Id: os-checkfiles.c,v 1.1 1999-04-23 01:10:12 rbasch Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include "array.h"

#define MAXLINE ((PATH_MAX * 2) + 256)

char *progname;

char *osroot = "/os";
Array *exceptlist = NULL;
int noop = 0;
int verbose = 0;

void usage();

void do_file(const char *path, const struct stat *statp, mode_t mode,
	     off_t size, uid_t uid, gid_t gid, int flags, time_t since);

void do_symlink(const char *from, const struct stat *statp, const char *to);

void do_directory(const char *path, const struct stat *statp, mode_t mode,
		  uid_t uid, gid_t gid);

void nuke(const char *path, const struct stat *statp);

void nukedir(const char *path);

void copyfile(const char *from, const char *to);

void set_stat(const char *path, mode_t mode, uid_t uid, gid_t gid);

Array *make_list(const char *file);

int in_list(const Array *list, const char *what);

int compare_string(const void *p1, const void *p2);


void usage()
{
  fprintf(stderr, "Usage: %s [<options>] <statfile>\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "        -n               No-op mode\n");
  fprintf(stderr, "        -o <osroot>      OS root, default is /os\n");
  fprintf(stderr, "        -r <target>      Target root, default is /\n");
  fprintf(stderr, "        -t <file>        Timestamp file\n");
  fprintf(stderr, "        -v               Verbose output mode\n");
  fprintf(stderr, "        -x <file>        File containing exception list\n");
  exit(1);
}

int main(int argc, char **argv)
{
  const char *statfile = NULL;
  const char *timefile = NULL;
  const char *exceptfile = NULL;
  const char *target = "/";
  FILE *statfp;
  char inbuf[MAXLINE];
  char *path;
  char linkpath[PATH_MAX+1];
  struct stat sb, *statp;
  mode_t mode;
  uid_t uid;
  gid_t gid;
  off_t size;
  int c, len, flags;
  time_t since;

  progname = argv[0];

  while ((c = getopt(argc, argv, "no:r:t:vx:")) != EOF)
    {
      switch(c)
	{
	case 'n':
	  noop = 1;
	  break;
	case 'o':
	  osroot = optarg;
	  break;
	case 'r':
	  target = optarg;
	  break;
	case 't':
	  timefile = optarg;
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'x':
	  exceptfile = optarg;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  if (optind + 1 != argc)
    usage();

  statfile = argv[optind++];
  statfp = fopen(statfile, "r");
  if (statfp == NULL)
    {
      fprintf(stderr, "%s: Cannot open %s: %s\n", progname, statfile,
	      strerror(errno));
      exit(1);
    }

  if (stat(osroot, &sb) != 0 || !S_ISDIR(sb.st_mode))
    {
      fprintf(stderr, "%s: Invalid operating system root %s\n",
	      progname, osroot);
      exit(1);
    }

  /* If given a timestamp file against which to check, record its
   * modification time, otherwise just use the current time.
   */
  if (timefile != NULL)
    {
      if (stat(timefile, &sb) != 0)
	{
	  fprintf(stderr, "%s: Cannot stat %s: %s\n",
		  progname, timefile, strerror(errno));
	  exit(1);
	}
      since = sb.st_mtime;
    }
  else
    time(&since);

  if (exceptfile != NULL)
    exceptlist = make_list(exceptfile);

  /* Chdir to the target root. */
  if (chdir(target) != 0)
    {
      fprintf(stderr, "%s: Cannot chdir to %s: %s\n",
	      progname, target, strerror(errno));
      exit(1);
    }

  /* Main loop -- read entries from the stat file. */
  while (fgets(inbuf, sizeof(inbuf), statfp) != NULL)
    {
      len = strlen(inbuf);
      if (inbuf[len-1] != '\n')
	{
	  fprintf(stderr, "%s: Invalid entry '%s', aborting\n",
		  progname, inbuf);
	  exit(1);
	}
      inbuf[--len] = '\0';

      /* Get the entry path, always the last field in the line.
       * Skip it if it is in the exception list.
       * Note now whether we can stat it.
       */
      for (path = &inbuf[len-1]; path > &inbuf[0] && *path != ' '; --path);
      if (path <= &inbuf[0])
	{
	  fprintf(stderr, "%s: Invalid entry '%s', aborting\n",
		  progname, inbuf);
	  exit(1);
	}
      ++path;

      if (in_list(exceptlist, path))
	{
	  if (verbose)
	    printf("Skipping exception %s\n", path);
	  continue;
	}

      statp = (lstat(path, &sb) == 0 ? &sb : NULL);

      switch (inbuf[0])
	{
	case 'l':
	  if (sscanf(&inbuf[2], "%s", linkpath) != 1)
	    {
	      fprintf(stderr,
		      "%s: Invalid symlink entry '%s', aborting\n",
		      progname, inbuf);
	      exit(1);
	    }
	  do_symlink(path, statp, linkpath);
	  break;
	case 'f':
	  if (sscanf(&inbuf[2], "%lo %llu %u %u %x", &mode, &size,
		     &uid, &gid, &flags) != 5)
	    {
	      fprintf(stderr,
		      "%s: Invalid file entry '%s', aborting\n",
		      progname, inbuf);
	      exit(1);
	    }
	  do_file(path, statp, mode, size, uid, gid, flags, since);
	  break;
	case 'd':
	  if (sscanf(&inbuf[2], "%lo %u %u", &mode, &uid, &gid) != 3)
	    {
	      fprintf(stderr,
		      "%s: Invalid directory entry '%s', aborting\n",
		      progname, inbuf);
	      exit(1);
	    }
	  do_directory(path, statp, mode, uid, gid);
	  break;
	default:
	  fprintf(stderr, "%s: Unrecognized type '%c', aborting.\n",
		  progname, inbuf[0]);
	  exit(1);
	}
    }
}

void do_file(const char *path, const struct stat *statp, mode_t mode,
	     off_t size, uid_t uid, gid_t gid, int flags, time_t since)
{
  struct stat sb;
  char ospath[PATH_MAX+1];

  if (statp == NULL
      || !S_ISREG(statp->st_mode)
      || statp->st_mtime > since
      || statp->st_size != size)
    {
      printf("Replace regular file %s\n", path);
      if (!noop)
	{
	  /* Remove the existing path, if any. */
	  nuke(path, statp);

	  /* Copy the file into place. */
	  sprintf(ospath, "%s/%s", osroot, path);
	  copyfile(ospath, path);

	  /* Make sure new copy has expected size. */
	  if (lstat(path, &sb) != 0)
	    {
	      fprintf(stderr, "%s: Cannot stat %s after copy: %s\n",
		      progname, path, strerror(errno));
	      exit(1);
	    }
	  if (sb.st_size != size)
	    {
	      fprintf(stderr,
		      "%s: Size mismatch after copy of %s (%llu, %llu)\n",
		      progname, path, size, sb.st_size);
	      exit(1);
	    }

	  set_stat(path, mode, uid, gid);
	}
    }

  else if (statp->st_mode != mode
	   || statp->st_uid != uid
	   || statp->st_gid != gid)
    {
      /* Here to fix file permissions and ownership. */
      printf("Set permission/ownership of regular file %s\n", path);
      if (!noop)
	set_stat(path, mode, uid, gid);
    }

  return;
}

void do_symlink(const char *from, const struct stat *statp, const char *to)
{
  struct stat sb;
  char linkbuf[PATH_MAX+1];
  int len;
  int result = 0;

  if (statp != NULL && S_ISLNK(statp->st_mode))
    {
      len = readlink(from, linkbuf, sizeof(linkbuf));
      if (len > 0)
	{
	  linkbuf[len] = '\0';
	  if (strcmp(linkbuf, to) == 0)
	    result = 1;
	}
    }
  if (!result)
    {
      printf("Relink %s to %s\n", from, to);
      if (!noop)
	{
	  nuke(from, statp);
	  if (symlink(to, from) != 0)
	    {
	      fprintf(stderr, "%s: Cannot create symlink %s: %s\n",
		      progname, from, strerror(errno));
	      exit(1);
	    }
	}
    }
}

void do_directory(const char *path, const struct stat *statp, mode_t mode,
		  uid_t uid, gid_t gid)
{
  struct stat sb;

  if (statp == NULL || !S_ISDIR(statp->st_mode))
    {
      /* Path doesn't exist, or is not a directory. Recreate it. */
      printf("Recreate directory %s\n", path);
      if (!noop)
	{
	  if (statp != NULL)
	    nuke(path, statp);
	  if (mkdir(path, S_IRWXU) != 0)
	    {
	      fprintf(stderr, "%s: Cannot mkdir %s: %s\n",
		      progname, path, strerror(errno));
	      exit(1);
	    }
	}
    }
  else if (statp->st_mode == mode
	   && statp->st_uid == uid
	   && statp->st_gid == gid)
    return;

  /* Here when path is a directory, but needs permissions and ownership
   * to be set.
   */
  printf("Set permission/ownership of directory %s\n", path);
  if (!noop)
    set_stat(path, mode, uid, gid);
  return;
}

void nuke(const char *path, const struct stat *statp)
{
  if (statp == NULL)
    return;
  if (S_ISDIR(statp->st_mode))
    nukedir(path);
  else if (unlink(path) != 0)
    {
      fprintf(stderr, "%s: Cannot unlink %s: %s\n",
	      progname, path, strerror(errno));
      exit(1);
    }
  return;
}


void nukedir(const char *path)
{
  DIR *dirp;
  struct dirent *dp;
  char pathbuf[PATH_MAX+1];
  struct stat sb;

  dirp = opendir(path);
  if (dirp == NULL)
    {
      fprintf(stderr, "%s: Cannot open directory %s: %s\n",
	      progname, path, strerror(errno));
      exit(1);
    }
  while ((dp = readdir(dirp)) != NULL)
    {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
	continue;
      sprintf(pathbuf, "%s/%s", path, dp->d_name);
      if (lstat(pathbuf, &sb) != 0)
	{
	  fprintf(stderr, "%s: Cannot stat %s: %s\n",
		  progname, pathbuf, strerror(errno));
	  continue;
	}
      /* Recurse if we encounter a subdirectory. */
      if (S_ISDIR(sb.st_mode))
	nukedir(pathbuf);
      else if (unlink(pathbuf) != 0)
	{
	  fprintf(stderr, "%s: Cannot unlink %s: %s\n",
		  progname, pathbuf, strerror(errno));
	  exit(1);
	}
    }
  closedir(dirp);
  if (rmdir(path) != 0)
    {
      fprintf(stderr, "%s: Cannot rmdir %s: %s\n",
	      progname, path, strerror(errno));
      exit(1);
    }
  return;
}

/* Copy a file.  The target file must not exist. */
void copyfile(const char *from, const char *to)
{
  char buf[4096];
  FILE *infp, *outfp;
  int n, fd, status;

  infp = fopen(from, "r");
  if (infp == NULL)
    {
      fprintf(stderr, "%s: Cannot open %s: %s\n",
	      progname, from, strerror(errno));
      return;
    }
  fd = open(to, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    {
      fprintf(stderr, "%s: Cannot create %s: %s\n",
	      progname, to, strerror(errno));
      fclose(infp);
      exit(1);
    }
  outfp = fdopen(fd, "w");
  if (outfp == NULL)
    {
      fprintf(stderr, "%s: fdopen failed for %s: %s\n",
	      progname, to, strerror(errno));
      fclose(infp);
      close(fd);
      exit(1);
    }

  while ((n = fread(buf, 1, sizeof(buf), infp)) > 0)
    {
      if (fwrite(buf, 1, n, outfp) != n)
	{
	  fprintf(stderr, "%s: Write failed for %s: %s\n",
		  progname, to, strerror(errno));
	  fclose(infp);
	  fclose(outfp);
	  exit(1);
	}
    }
  status = ferror(infp);
  if ((fclose(infp) != 0) || (status != 0))
    {
      fprintf(stderr, "%s: Error reading %s: %s\n",
	      progname, from, strerror(errno));
      fclose(outfp);
      exit(1);
    }
  status = ferror(outfp);
  if ((fclose(outfp) != 0) || (status != 0))
    {
      fprintf(stderr, "%s: Error writing %s: %s\n",
	      progname, to, strerror(errno));
      exit(1);
    }
  return;
}

void set_stat(const char *path, mode_t mode, uid_t uid, gid_t gid)
{
  if (chmod(path, mode) != 0)
    {
      fprintf(stderr, "%s: Cannot chmod %s: %s\n",
	      progname, path, strerror(errno));
      exit(1);
    }
  if (chown(path, uid, gid) != 0)
    {
      fprintf(stderr, "%s: Cannot chown %s: %s\n",
	      progname, path, strerror(errno));
      exit(1);
    }
}

/* Read a file into an array, one element per line. */
Array *make_list(const char *file)
{
  FILE *f;
  Array *list;
  char buffer[MAXLINE];

  f = fopen(file, "r");
  if (f == NULL)
    return NULL;

  list = array_new();
  while (fgets(buffer, sizeof(buffer), f))
    {
      buffer[strlen(buffer) - 1] = '\0';
      array_add(list, strdup(buffer));
    }

  fclose(f);
  array_sort(list, compare_string);
  return list;
}

/* Check if the given string is an element in the given array.
 * Returns 1 if found in the array, 0 otherwise.
 */
int in_list(const Array *list, const char *what)
{
  int i;
  char *string;

  if (list == NULL)
    return 0;

  return (array_search(list, what) != NULL);
}

int compare_string(const void *p1, const void *p2)
{
  return strcmp(*((char **)p1), *((char **)p2));
}
