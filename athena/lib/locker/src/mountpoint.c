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

/* This file is part of liblocker. It deals with canonicalizing and
 * creating mountpoints, and the associated security issues.
 */

static const char rcsid[] = "$Id: mountpoint.c,v 1.11 2001-08-16 14:27:52 ghudson Exp $";

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "locker.h"
#include "locker_private.h"

/* Include <sys/param.h> for MAXPATHLEN (and MAXSYMLINKS). We assume
 * that if some future OS doesn't have a maximum path length then
 * they'll also provide replacements for getcwd() and readlink() that
 * don't require the caller to guess how large a buffer to provide.
 */
#include <sys/param.h>

#define MODE_MASK (S_IRWXU | S_IRWXG | S_IRWXO)
#define LOCKER_DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define BAD_MODE_BITS (S_IWGRP | S_IWOTH)

static int mountpoint_mkdir(locker_context context, locker_attachent *at,
			    char *path);
static int mountpoint_rmdir(locker_context context, locker_attachent *at,
			    char *path);

static int get_dirlock(locker_context context, locker_attachent *at,
		       int type);
static void release_dirlock(locker_context context, locker_attachent *at);

/* Canonicalize a path and optionally make sure that it doesn't pass
 * through any mountpoints or user-writable directories.  *pathp must
 * contain an allocated string, which may be freed and replaced with
 * another allocated string containing the canonicalized path.  If
 * buildfromp is not NULL, *buildfromp is set to an allocated string
 * containing the first ancestor directory of the canonicalized path
 * which doesn't exist, or to NULL if the canonicalized path already
 * exists.
 */
int locker__canonicalize_path(locker_context context, int check,
			      char **pathp, char **buildfromp)
{
  char *path, *buildfrom = NULL, *cur, *end, *p;
  struct stat st;
  int nlinks = 0, status = 0;
  dev_t last_dev = 0;
  uid_t uid = geteuid();

  path = *pathp;

  /* If it doesn't start with '/', prepend cwd. */
  if (path[0] != '/')
    {
      char cwdbuf[MAXPATHLEN], *newpath;

      if (!getcwd(cwdbuf, sizeof(cwdbuf)))
	{
	  locker__error(context, "Could not retrieve current working "
			"directory:\n%s.\n", strerror(errno));
	  free(path);
	  *pathp = NULL;
	  return LOCKER_EBADPATH;
	}
      if (!strcmp(cwdbuf, "/"))
	*cwdbuf = '\0';		/* Avoid generating "//foo". */

      newpath = malloc(strlen(cwdbuf) + strlen(path) + 2);
      if (!newpath)
	{
	  free(path);
	  *pathp = NULL;
	  locker__error(context, "Out of memory canonicalizing mountpoint.\n");
	  return LOCKER_ENOMEM;
	}

      sprintf(newpath, "%s/%s", cwdbuf, path);
      free(path);
      path = newpath;

      /* Don't need to canonicalize cwd part, so skip that. */
      cur = path + strlen(cwdbuf) + 1;
    }
  else
    cur = path + 1;

  /* Remove trailing slash. */
  p = strrchr(path, '/');
  if (p && !*(p + 1) && p > path)
    *p = '\0';

  if (uid != context->user)
    seteuid(context->user);

  /* Expand symlinks. */
  do
    {
      end = strchr(cur, '/');
      if (end)
	*end = '\0';

      /* cur points to the character after a '/'. Everything before cur is
       * canonicalized. If cur points to the final component, end is NULL.
       * Otherwise, end points to a '\0' that is covering the next '/'
       * after cur.
       */

      /* Handle special cases. */
      if (!*cur || !strcmp(cur, "."))
	{
	  /* "." can't be final path component (since we might not have
	   * wanted to resolve the final path component, and now it's
	   * too late.
	   */
	  if (!end)
	    {
	      locker__error(context, "Path cannot end with \".\" (%s).\n",
			    path);
	      status = LOCKER_EBADPATH;
	      goto cleanup;
	    }

	  /* Copy the next component over this one. */
	  strcpy(cur, end + 1);
	  continue;
	}
      else if (!strcmp(cur, ".."))
	{
	  /* ".." can't be final path component for similar reasons
	   * to above.
	   */
	  if (!end)
	    {
	      locker__error(context, "%s: Path cannot end with \"..\" (%s).\n",
			    path);
	      status = LOCKER_EBADPATH;
	      goto cleanup;
	    }

	  /* Remove this component and the previous one. */
	  cur -= 2;
	  while (cur > path && *cur != '/')
	    cur--;

	  if (cur < path)
	    cur++;		/* "/.." == "/" */

	  cur++;                /* Leave a trailing "/", because end just
				 * got its leading "/" stripped off
				 */

	  /* Copy the next component over the previous one. */
	  strcpy(cur, end + 1);
	  continue;
	}

      /* Don't resolve the final component unless we need to check it. */
      if (!end && (check != LOCKER_CANON_CHECK_ALL))
	break;

      /* Check if current component is a symlink. */
      if (lstat(path, &st) < 0)
	{
	  if (errno == ENOENT)
	    {
	      /* The rest of the path doesn't exist. */
	      buildfrom = strdup(path);
	      if (!buildfrom)
		{
		  locker__error(context, "Out of memory canonicalizing "
				"mountpoint.\n");
		  status = LOCKER_ENOMEM;
		  goto cleanup;
		}
	      if (end)
		*end = '/';
	      break;
	    }
	  else
	    {
	      locker__error(context, "Could not canonicalize path:\n"
			    "%s for %s\n", strerror(errno), path);
	      status = LOCKER_EBADPATH;
	      goto cleanup;
	    }
	}
      else if (!end && !S_ISDIR(st.st_mode))
	{
	  locker__error(context, "Final path component is not a directory "
			"in \"%s\".\n", path);
	  status = LOCKER_EBADPATH;
	  goto cleanup;
	}
      else if (S_ISLNK(st.st_mode))
	{
	  char linkbuf[MAXPATHLEN], *newpath;
	  int len;

	  if (++nlinks > MAXSYMLINKS)
	    {
	      locker__error(context, "Too many levels of symlinks "
			    "while canonicalizing path \"%s\".\n",
			    path);
	      status = LOCKER_EBADPATH;
	      goto cleanup;
	    }

	  len = readlink(path, linkbuf, sizeof(linkbuf));
	  if (len == -1)
	    {
	      locker__error(context, "Could not canonicalize mountpoint:\n"
			    "%s while reading symlink %s\n", strerror(errno),
			    path);
	      status = LOCKER_EBADPATH;
	      goto cleanup;
	    }

	  if (linkbuf[0] == '/')
	    {
	      /* It's absolute, so replace existing path with it. */
	      newpath = malloc(len + strlen(end + 1) + 2);
	      if (!newpath)
		{
		  locker__error(context, "Out of memory canonicalizing "
				"mountpoint.\n");
		  status = LOCKER_ENOMEM;
		  goto cleanup;
		}
	      sprintf(newpath, "%.*s/%s", len, linkbuf, end + 1);
	      free(path);
	      path = newpath;
	      /* And start over from the top. */
	      cur = path + 1;
	    }
	  else
	    {
	      /* Add this in to existing path. */
	      *cur = '\0';
	      newpath = malloc(strlen(path) + len + strlen(end + 1) + 3);
	      if (!newpath)
		{
		  locker__error(context, "Out of memory canonicalizing "
				"mountpoint.\n");
		  status = LOCKER_ENOMEM;
		  goto cleanup;
		}
	      sprintf(newpath, "%s%.*s/%s", path, len, linkbuf, end + 1);
	      /* cur is effectively unchanged. */
	      cur = newpath + (cur - path);
	      free(path);
	      path = newpath;
	    }
	}
      else
	{
	  if (check != LOCKER_CANON_CHECK_NONE)
	    {
	      /* Check that we can build in this directory. */
	      if (last_dev && st.st_dev != last_dev)
		{
		  locker__error(context, "Cannot attach locker on %s:\n"
				"directory %s is not on root filesystem.\n",
				*pathp, path);
		  status = LOCKER_EBADPATH;
		  goto cleanup;
		}
	      else if (st.st_uid != 0)
		{
		  locker__error(context, "Cannot attach locker on %s:\n"
				"directory %s is not owned by root.\n",
				*pathp, path);
		  status = LOCKER_EBADPATH;
		  goto cleanup;
		}
	      else if (st.st_mode & BAD_MODE_BITS)
		{
		  locker__error(context, "Cannot attach locker on %s:\n"
				"directory %s is group/other writable.\n",
				*pathp, path);
		  status = LOCKER_EBADPATH;
		  goto cleanup;
		}

	      last_dev = st.st_dev;
	    }

	  /* Replace *end, update cur. */
	  if (end)
	    {
	      *end = '/';
	      cur = end + 1;
	    }
	}
    }
  while (end);

  if (check != LOCKER_CANON_CHECK_NONE)
    {
      int len = strlen(context->attachtab);
      if (!strncmp(context->attachtab, path, len) &&
	  (path[len] == '\0' || path[len] == '/'))
	{
	  locker__error(context, "Cannot attach locker on %s:\n"
			"directory passes through attachtab directory.\n",
			*pathp, path);
	  status = LOCKER_EBADPATH;
	  goto cleanup;
	}
    }

  /* Remove trailing slash. */
  p = strrchr(path, '/');
  if (p && !*(p + 1) && p > path)
    *p = '\0';

  *pathp = path;
  if (buildfromp)
    *buildfromp = buildfrom;
  else
    free(buildfrom);

cleanup:
  if (uid != context->user)
    seteuid(uid);
  if (status)
    {
      free(path);
      *pathp = NULL;
      free(buildfrom);
    }
  return status;
}

/* Create any directories needed to attach something to the named
 * mountpoint. Record the fact that we created the directory so that
 * we know to delete it later.
 */
int locker__build_mountpoint(locker_context context, locker_attachent *at)
{
  char *q;
  int status;

  status = get_dirlock(context, at, F_RDLCK);
  if (status)
    return status;

  if (at->buildfrom)
    {
      /* Create any remaining directories. */
      q = at->mountpoint + (strrchr(at->buildfrom, '/') - at->buildfrom);
      while (!status)
	{
	  q = strchr(q + 1, '/');
	  if (q)
	    *q = '\0';
	  else if (!(at->fs->flags & LOCKER_FS_NEEDS_MOUNTDIR))
	    break;
	  status = mountpoint_mkdir(context, at, at->mountpoint);
	  if (q)
	    *q = '/';
	  else
	    break;
	}
    }

  /* We do not release the dirlock now, to guarantee that no one
   * else deletes our directories before we're done mounting
   * the filesystem. It will be released by locker_free_attachent().
   */

  return status;
}

/* Remove any directories we created as part of attaching a locker */
int locker__remove_mountpoint(locker_context context, locker_attachent *at)
{
  char *p, *q;
  int status;

  p = strrchr(at->mountpoint, '/');

  /* If no '/', it must be a MUL locker. */
  if (!p)
    return LOCKER_SUCCESS;

  status = get_dirlock(context, at, F_WRLCK);
  if (status)
    return status;

  status = mountpoint_rmdir(context, at, at->mountpoint);
  if (status == LOCKER_ENOENT)
    status = LOCKER_SUCCESS;
  while (!status && (q = p) && (q != at->mountpoint))
    {
      *q = '\0';
      p = strrchr(at->mountpoint, '/');
      status = mountpoint_rmdir(context, at, at->mountpoint);
      *q = '/';
    }

  release_dirlock(context, at);

  if (status == LOCKER_ENOENT || status == LOCKER_EMOUNTPOINTBUSY)
    return LOCKER_SUCCESS;
  else
    return status;
}

/* Unlock the mountpoint created by locker__build_mountpoint(). */
void locker__put_mountpoint(locker_context context, locker_attachent *at)
{
  release_dirlock(context, at);
}

/* Lock the dirlock file. type should be F_RDLCK if you are creating
 * directories and F_WRLCK if you are deleting them. A process that creates
 * directories should hold onto the lock until it has actually finished
 * trying to attach the locker, so that some other detach procedure doesn't
 * remove those directories out from under it.
 */
static int get_dirlock(locker_context context, locker_attachent *at, int type)
{
  int status;
  char *lock;
  struct flock fl;
  mode_t o_umask;

  lock = locker__attachtab_pathname(context, LOCKER_LOCK, ".dirlock");
  if (!lock)
    return LOCKER_ENOMEM;

  o_umask = umask(0);
  at->dirlockfd = open(lock, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
  free(lock);
  umask(o_umask);
  if (at->dirlockfd < 0)
    {
      at->dirlockfd = 0;
      locker__error(context, "%s: Could not %s mountpoint:\n"
		    "%s while opening directory lock file.\n", at->name,
		    at->attached ? "remove" : "create", strerror(errno));
      return LOCKER_EATTACHTAB;
    }
  fl.l_type = type;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = 0;
  status = fcntl(at->dirlockfd, F_SETLKW, &fl);
  if (status < 0)
    {
      close(at->dirlockfd);
      at->dirlockfd = 0;
      locker__error(context, "%s: Could not %s mountpoint:\n"
		    "%s while locking directory lock file.\n", at->name,
		    at->attached ? "remove" : "create", strerror(errno));
      return LOCKER_EATTACHTAB;
    }

  return LOCKER_SUCCESS;
}

static void release_dirlock(locker_context context, locker_attachent *at)
{
  close(at->dirlockfd);
  at->dirlockfd = 0;
}

static int mountpoint_mkdir(locker_context context, locker_attachent *at,
			    char *path)
{
  char *file;
  struct stat st;
  mode_t omask;
  int status;

  file = locker__attachtab_pathname(context, LOCKER_DIRECTORY, path);
  if (!file)
    {
      locker__error(context, "Out of memory building mountpoint.\n");
      return LOCKER_ENOMEM;
    }

  if (access(file, F_OK) != 0 || lstat(path, &st) != 0)
    {
      int fd;

      /* Need to create the dirfile and the directory. If we get killed
       * in between the two steps, we'd rather have a dirfile that
       * doesn't correspond to a directory than a directory that we
       * don't know is our fault. So do the dirfile first.
       */

      fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (fd == -1)
	{
	  free(file);
	  locker__error(context, "Could not create directory data file "
			"%s:\n%s.\n", file, strerror(errno));
	  return LOCKER_EATTACHTAB;
	}
      close(fd);

      /* Make directory. */
      omask = umask(0);
      status = mkdir(path, LOCKER_DIR_MODE);
      umask(omask);
      if (status == -1)
	{
	  if (errno == EEXIST)
	    {
	      locker__error(context, "%s: Directory should not exist: %s\n",
			    at->name, path);
	    }
	  else
	    {
	      locker__error(context, "%s: Could not create directory %s:\n"
			    "%s.\n", at->name, path, strerror(errno));
	    }
	  unlink(file);
	  free(file);
	  return LOCKER_EMOUNTPOINT;
	}
    }
  else
    {
      if (!S_ISDIR(st.st_mode) || st.st_uid != 0 ||
	  (st.st_mode & MODE_MASK) != LOCKER_DIR_MODE)
	{
	  locker__error(context, "%s: Directory \"%s\" has changed.\n",
			at->name, path);
	  unlink(file);
	  free(file);
	  return LOCKER_EMOUNTPOINT;
	}
    }

  free(file);
  return LOCKER_SUCCESS;
}

static int mountpoint_rmdir(locker_context context, locker_attachent *at,
			    char *path)
{
  char *file;
  struct stat st;

  file = locker__attachtab_pathname(context, LOCKER_DIRECTORY, path);
  if (!file)
    {
      locker__error(context, "Out of memory removing mountpoint.\n");
      return LOCKER_ENOMEM;
    }

  if (access(file, F_OK) != 0)
    {
      free(file);
      return LOCKER_ENOENT;
    }

  /* Make sure directory exists and looks like we built it. */
  if (lstat(path, &st) == -1)
    {
      locker__error(context, "%s: Could not check directory %s:\n%s.\n",
		    at->name, path, strerror(errno));
      unlink(file);
      free(file);
      return LOCKER_EMOUNTPOINT;
    }

  if (!S_ISDIR(st.st_mode) || st.st_uid != 0 ||
      (st.st_mode & MODE_MASK) != LOCKER_DIR_MODE)
    {
      locker__error(context, "%s: Directory \"%s\" has changed.\n",
		    at->name, path);
      unlink(file);
      free(file);
      return LOCKER_EMOUNTPOINT;
    }

  if (rmdir(path) == -1)
    {
      free(file);
      if (errno == ENOTEMPTY || errno == EEXIST)
	return LOCKER_EMOUNTPOINTBUSY;
      locker__error(context, "%s: Could not remove mountpoint component "
		    "%s:\n%s.\n", at->name, path, strerror(errno));
      return LOCKER_EMOUNTPOINT;
    }

  unlink(file);
  free(file);
  return LOCKER_SUCCESS;
}
