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

/* This file is part of liblocker. It implements the attachtab directory,
 * which contains files corresponding to attached (or formerly attached)
 * lockers.
 */

static const char rcsid[] = "$Id: attachtab.c,v 1.1 1999-02-26 19:04:47 danw Exp $";

#include "locker.h"
#include "locker_private.h"

#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/* Used internally by locker_iterate_attachtab */
struct locker__dirent {
  char *name;
  time_t ctime;
};

static int delete_attachent(locker_context context, locker_attachent *at);
static int get_attachent(locker_context context, char *name,
			 char *mountpoint, int create,
			 locker_attachent **atp);
static int read_attachent(locker_context context, int kind,
			  char *name, locker_attachent **atp);
static int lock_attachtab(locker_context context, int *lock);
static void unlock_attachtab(locker_context context, int lock);
static int compare_locker__dirents(const void *a, const void *b);

/* Public interface to locker__lookup_attachent. */
int locker_read_attachent(locker_context context, char *name,
			  locker_attachent **atp)
{
  return locker__lookup_attachent(context, name, NULL, 0, atp);
}

/* Look up a locker by filesys name and return an attachent for it. If
 * "which" is 0, only return an already-attached locker. Otherwise,
 * "which" is the index of the Hesiod filesys entry for "name" to look
 * up. (The caller can keep calling with successively higher values of
 * "which" until it succeeds in attaching one, or
 * locker__lookup_attachent returns a LOCKER_LOOKUP_FAILURE() status.
 *
 * name can be NULL iff mountpoint is non-NULL and which is 0.
 */
int locker__lookup_attachent(locker_context context, char *name,
			     char *mountpoint, int which,
			     locker_attachent **atp)
{
  int status, i, lock;
  locker_attachent *at;
  char **descs, *desc, *p;
  void *cleanup;
  struct locker_ops *fs;

  status = lock_attachtab(context, &lock);
  if (status != LOCKER_SUCCESS)
    return status;

  if (which < 2)
    {
      /* Look for a pre-attached locker. */
      at = NULL;
      status = get_attachent(context, name, mountpoint, 0, &at);
      if (status != LOCKER_ENOTATTACHED)
	{
	  unlock_attachtab(context, lock);
	  if (status == LOCKER_SUCCESS)
	    *atp = at;
	  return status;
	}
      else if (!which)
	{
	  unlock_attachtab(context, lock);
	  locker__error(context, "%s: Not attached.\n",
			name ? name : mountpoint);
	  return status;
	}
    }

  /* It's not attached. Look up the locker description. */
  status = locker_lookup_filsys(context, name, &descs, &cleanup);
  if (status)
    {
      unlock_attachtab(context, lock);
      return status;
    }

  if (!descs[1])
    {
      /* If only one description, it won't have an order field. */
      if (which == 1)
	desc = descs[0];
      else
	desc = NULL;
    }
  else
    {
      /* Check each filesystem description until we find the one we want. */
      for (i = 0, desc = NULL; descs[i]; i++)
	{
	  p = strrchr(descs[i], ' ');
	  if (!p)
	    {
	      unlock_attachtab(context, lock);
	      locker_free_filesys(context, descs, cleanup);
	      locker__error(context, "%s: Could not parse locker "
			    "description \"%s\".\n", name, descs[i]);
	      return LOCKER_EPARSE;
	    }

	  if (atoi(p + 1) == which)
	    {
	      desc = descs[i];
	      break;
	    }
	}
    }

  if (!desc)
    {
      unlock_attachtab(context, lock);
      locker_free_filesys(context, descs, cleanup);
      return LOCKER_ENOENT;
    }

  fs = locker__get_fstype(context, desc);
  if (!fs)
    {
      unlock_attachtab(context, lock);
      locker__error(context, "%s: Unknown locker type in description "
		    "\"%s\".\n", name, desc);
      locker_free_filesys(context, descs, cleanup);
      return LOCKER_EPARSE;
    }

  status = fs->parse(context, name, desc, mountpoint, &at);
  locker_free_filesys(context, descs, cleanup);
  if (status)
    {
      unlock_attachtab(context, lock);
      return status;
    }

  if (mountpoint)
    status = get_attachent(context, NULL, at->mountpoint, which != 0, &at);
  else
    status = get_attachent(context, name, at->mountpoint, which != 0, &at);
  unlock_attachtab(context, lock);
  if (status == LOCKER_SUCCESS)
    *atp = at;
  else
    locker_free_attachent(context, at);

  return status;
}

/* Look up a locker by explicit description and return an attachent
 * for it. If "create" is 0, only return an already-attached locker.
 */
int locker__lookup_attachent_explicit(locker_context context, char *type,
				      char *desc, char *mountpoint,
				      int create, locker_attachent **atp)
{
  struct locker_ops *fs;
  locker_attachent *at;
  int status, lock;

  fs = locker__get_fstype(context, type);
  if (!fs)
    {
      locker__error(context, "%s: Unknown locker type \"%s\".\n", desc, type);
      return LOCKER_EPARSE;
    }

  status = fs->parse(context, NULL, desc, mountpoint, &at);
  if (status)
    return status;

  status = lock_attachtab(context, &lock);
  if (status != LOCKER_SUCCESS)
    {
      locker_free_attachent(context, at);
      return status;
    }
  status = get_attachent(context, NULL, at->mountpoint, create, &at);
  unlock_attachtab(context, lock);
  if (status == LOCKER_SUCCESS)
    *atp = at;
  else
    locker_free_attachent(context, at);

  return status;
}

/* Delete an attachent file and its corresponding symlink, if any. */
static int delete_attachent(locker_context context, locker_attachent *at)
{
  int status, lock;
  char *path;

  status = lock_attachtab(context, &lock);
  if (status != LOCKER_SUCCESS)
    return status;

  if (at->flags & LOCKER_FLAG_NAMEFILE)
    {
      path = locker__attachtab_pathname(context, LOCKER_NAME, at->name);
      if (!path)
	{
	  locker__error(context, "Out of memory deleting attachent.\n");
	  status = LOCKER_ENOMEM;
	  goto cleanup;
	}
      if (unlink(path) == -1 && errno != ENOENT)
	{
	  locker__error(context, "Could not delete file %s:\n%s.\n",
			path, strerror(errno));
	  status = LOCKER_EATTACHTAB;
	  goto cleanup;
	}
      free(path);
    }

  path = locker__attachtab_pathname(context, LOCKER_MOUNTPOINT,
				    at->mountpoint);
  if (!path)
    {
      locker__error(context, "Out of memory deleting attachent.\n");
      status = LOCKER_ENOMEM;
      goto cleanup;
    }
  if (unlink(path) == -1)
    {
      locker__error(context, "Could not delete file %s:\n%s.\n",
		    path, strerror(errno));
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }
  free(path);

  unlock_attachtab(context, lock);
  return LOCKER_SUCCESS;

cleanup:
  free(path);
  unlock_attachtab(context, lock);
  return status;
}

/* Free a locker_attachent and unlock any locks associated with it. */
void locker_free_attachent(locker_context context, locker_attachent *at)
{
  if (at->mountpoint_file)
    {
      fclose(at->mountpoint_file);
      if (!at->attached)
	delete_attachent(context, at);
    }
  if (at->dirlockfd)
    close(at->dirlockfd);
  free(at->name);
  free(at->mountpoint);
  free(at->buildfrom);
  free(at->hostdir);
  free(at->owners);

  if (at->next)
    locker_free_attachent(context, at->next);

  free(at);
}


/* Iterate through all attachent entries, calling the "test" function
 * on each one, and then calling the "act" function on all entries for
 * which the "test" function returns true. Holds an exclusive lock on
 * the attachtab lock file through the duration of the call. Only
 * returns an error if it was unable to begin iterating through the
 * entries.
 */
int locker_iterate_attachtab(locker_context context,
			     locker_callback test, void *testarg,
			     locker_callback act, void *actarg)
{
  int status, lock;
  char *path;
  DIR *dir;
  struct dirent *entry;
  locker_attachent *at;
  struct locker__dirent *files = NULL;
  int nfiles = 0, filessize = 0, i;
  struct stat st;

  status = lock_attachtab(context, &lock);
  if (status)
    return status;

  path = locker__attachtab_pathname(context, LOCKER_MOUNTPOINT, "");
  if (!path)
    {
      unlock_attachtab(context, lock);
      locker__error(context, "Out of memory reading attachtab.\n");
      return LOCKER_ENOMEM;
    }

  dir = opendir(path);
  if (!dir)
    {
      unlock_attachtab(context, lock);
      locker__error(context, "Could not read attachtab:\n%s opening "
		    "directory %s\n", strerror(errno), path);
      free(path);
      return LOCKER_EATTACHTAB;
    }

  while ((entry = readdir(dir)))
    {
      if (entry->d_name[0] == '.')
	continue;

      if (nfiles >= filessize)
	{
	  struct locker__dirent *newfiles;
	  int newfilessize;

	  newfilessize = filessize ? 2 * filessize : 10;
	  newfiles = realloc(files, (newfilessize) * sizeof(*files));
	  if (!newfiles)
	    {
	      status = LOCKER_ENOMEM;
	      break;
	    }
	  files = newfiles;
	  filessize = newfilessize;
	}

      files[nfiles].name = malloc(strlen(path) + strlen(entry->d_name) + 1);
      if (!files[nfiles].name)
	{
	  status = LOCKER_ENOMEM;
	  break;
	}
      sprintf(files[nfiles].name, "%s%s", path, entry->d_name);
      
      if (stat(files[nfiles].name, &st) != 0)
	{
	  locker__error(context, "%s while reading attachtab entry\n\"%s\".\n",
			strerror(errno), files[nfiles].name);
	  status = LOCKER_EATTACHTAB;
	  break;
	}

      files[nfiles++].ctime = st.st_ctime;
    }
  closedir(dir);
  free(path);

  if (status != LOCKER_SUCCESS)
    {
      if (status == LOCKER_ENOMEM)
	locker__error(context, "Out of memory reading attachtab.\n");

      goto cleanup;
    }

  /* Sort the entries. */
  qsort(files, nfiles, sizeof(*files), compare_locker__dirents);

  /* Now run through the callbacks. */
  for (i = 0; i < nfiles; i++)
    {
      status = read_attachent(context, LOCKER_FULL_PATH, files[i].name, &at);
      if (status == LOCKER_SUCCESS)
	{
	  if (!test || test(context, at, testarg))
	    act(context, at, actarg);
	  locker_free_attachent(context, at);
	}
    }
  status = LOCKER_SUCCESS;

cleanup:
  for (i = 0; i < nfiles; i++)
    free(files[i].name);
  free(files);

  unlock_attachtab(context, lock);
  return status;
}

static int compare_locker__dirents(const void *a, const void *b)
{
  const struct locker__dirent *da = a, *db = b;

  return (da->ctime < db->ctime) ? -1 : (da->ctime > db->ctime) ? 1 : 0;
}


/* Fetch the desired attachtab entry into a struct attachtab. name is
 * the locker name and mountpoint is its mountpoint. If create is non-0
 * an empty attachtab entry will be created if none exists.
 *
 * name may be NULL. mountpoint may be NULL if name is not. *atp should
 * be a partially-filled in attachent if create is true. Otherwise it
 * can be partially-filled in, or NULL.
 */
static int get_attachent(locker_context context, char *name,
			 char *mountpoint, int create,
			 locker_attachent **atp)
{
  locker_attachent *at = NULL;
  int fd = 0;
  FILE *fp = NULL;
  int status;
  char *path = NULL;
  struct flock fl;

  /* Try opening an existing file */
  if (mountpoint)
    status = read_attachent(context, LOCKER_MOUNTPOINT, mountpoint, &at);
  else
    status = read_attachent(context, LOCKER_NAME, name, &at);

  if (status == LOCKER_SUCCESS)
    {
      /* If caller passed in both mountpoint and name, make sure they
       * both matched.
       */
      if (mountpoint && name && strcmp(name, at->name))
	{
	  locker__error(context, "%s: %s is already attached on %s.\n",
			name, at->name, mountpoint);
	  locker_free_attachent(context, at);
	  return LOCKER_EMOUNTPOINTBUSY;
	}

      /* Otherwise, if no attachent was passed in, we've won. */
      if (!*atp)
	{
	  *atp = at;
	  return status;
	}

      /* Make sure the found attachent matches the passed-in attachent. */
      if (strcmp(at->name, (*atp)->name) ||
	  strcmp(at->mountpoint, (*atp)->mountpoint) ||
	  strcmp(at->hostdir, (*atp)->hostdir) ||
	  at->fs != (*atp)->fs)
	{
	  locker__error(context, "%s: %s is already attached on %s.\n",
			(*atp)->name, at->name, at->mountpoint);
	  locker_free_attachent(context, at);
	  return LOCKER_EMOUNTPOINTBUSY;
	}

      /* Free the passed-in attachent and return the found one. */
      locker_free_attachent(context, *atp);
      *atp = at;
      return LOCKER_SUCCESS;
    }
  else if (status != LOCKER_ENOTATTACHED)
    return status;

  /* If we were looking for something already attached, we lose. (But
   * don't print an error: this might not be fatal.)
   */
  if (!create)
    return LOCKER_ENOTATTACHED;

  path = locker__attachtab_pathname(context, LOCKER_MOUNTPOINT, mountpoint);
  if (!path)
    return LOCKER_ENOMEM;

  fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1)
    {
      locker__error(context, "Could not create attachtab file %s:\n%s.\n",
		    path, strerror(errno));
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }

  fp = fdopen(fd, "r+");
  if (!fp)
    {
      locker__error(context, "Could not create attachtab file %s:\n%s.\n",
		    path, strerror(errno));
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = 0;
  status = fcntl(fileno(fp), F_SETLKW, &fl);
  if (status < 0)
    {
      locker__error(context, "Could not lock attachtab file %s:\n%s.\n",
		    path, strerror(errno));
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }

  if (name)
    {
      char *npath;
      npath = locker__attachtab_pathname(context, LOCKER_NAME, name);
      if (!npath)
	{
	  status = LOCKER_ENOMEM;
	  goto cleanup;
	}
      if (symlink(path, npath) < 0)
	{
	  locker__error(context, "Could not symlink %s\n to %s: %s.\n",
			path, npath, strerror(errno));
	  free(npath);
	  status = LOCKER_EATTACHTAB;
	  goto cleanup;
	}
      free(npath);
      (*atp)->flags |= LOCKER_FLAG_NAMEFILE;
    }

  free(path);

  (*atp)->mountpoint_file = fp;
  return LOCKER_SUCCESS;

cleanup:
  if (fp || fd > 0)
    {
      unlink(path);
      if (fp)
	fclose(fp);
      else
	close(fd);
    }
  if (path)
    free(path);
  if (at)
    free(at);
  return status;
}

/* The format of a mountpoint file is:
 *
 * version (currently 1)
 * hesiod or explicit name
 * mountpoint
 * fstype
 * IP addr of host (or 0.0.0.0)
 * path to locker
 * auth mode
 * flags
 * # of owners:owner uid:owner uid:...
 *
 */

/* Copy data from a locker_attachent into the corresponding file. */
void locker__update_attachent(locker_context context, locker_attachent *at)
{
  int i;

  if (at->attached)
    {
      rewind(at->mountpoint_file);
      fprintf(at->mountpoint_file, "1\n%s\n%s\n%s\n%s\n%s\n%c\n%d\n",
	      at->name, at->mountpoint, at->fs->name, inet_ntoa(at->hostaddr),
	      at->hostdir, at->mode, at->flags);

      fprintf(at->mountpoint_file, "%d", at->nowners);
      for (i = 0; i < at->nowners; i++)
	fprintf(at->mountpoint_file, ":%ld", (long)at->owners[i]);
      fprintf(at->mountpoint_file, "\n");
      fflush(at->mountpoint_file);
    }
  else
    {
      ftruncate(fileno(at->mountpoint_file), 0);
      rewind(at->mountpoint_file);
    }
}

/* Read the named attachtab file into a locker_attachent. */
static int read_attachent(locker_context context, int kind, char *name,
			  locker_attachent **atp)
{
  FILE *fp;
  char *path, *pmem = NULL, *buf = NULL, *p, *q;
  int bufsize, status, i;
  struct flock fl;
  locker_attachent *at;
  struct stat st1, st2;

  if (kind != LOCKER_FULL_PATH)
    {
      path = pmem = locker__attachtab_pathname(context, kind, name);
      if (!path)
	return LOCKER_ENOMEM;
    }
  else
    path = name;

  /* Need to open it read/write so we can get an F_WRLCK. */
  fp = fopen(path, "r+");
  if (!fp)
    {
      if (errno == ENOENT)
	{
	  free(pmem);
	  return LOCKER_ENOTATTACHED;
	}

      locker__error(context, "Could not open attachtab file %s:\n%s.\n",
		    path, strerror(errno));
      free(pmem);
      return LOCKER_EATTACHTAB;
    }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = 0;
  status = fcntl(fileno(fp), F_SETLKW, &fl);
  if (status < 0)
    {
      fclose(fp);
      locker__error(context, "Could not lock attachtab file %s:\n%s.\n",
		    path, strerror(errno));
      free(pmem);
      return LOCKER_EATTACHTAB;
    }

  /* The file might have been deleted while we were waiting on the lock.
   * Worse yet, someone else might have created a new one after the one
   * we're waiting for was deleted, but before the deleter gave up its
   * lock. So check first that it's there, and second that it's still
   * the right file.
   */
  if (stat(path, &st1) == -1)
    {
      fclose(fp);
      if (errno == ENOENT)
	{
	  free(pmem);
	  return LOCKER_ENOTATTACHED;
	}
      else
	{
	  locker__error(context, "Could not stat attachent file %s:\n%s.\n",
			path, strerror(errno));
	  free(pmem);
	  return LOCKER_EATTACHTAB;
	}
    }
  if (fstat(fileno(fp), &st2) == -1)
    {
      locker__error(context, "Could not fstat attachent file %s:\n%s.\n",
		    path, strerror(errno));
      free(pmem);
      return LOCKER_EATTACHTAB;
    }
  free(pmem);
  if (st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino)
    {
      fclose(fp);
      return LOCKER_ENOTATTACHED;
    }

  /* Start building the attachent. */
  at = locker__new_attachent(context, NULL);
  if (!at)
    {
      fclose(fp);
      return LOCKER_ENOMEM;
    }

  at->mountpoint_file = fp;

  /* Read version number. */
  status = locker__read_line(fp, &buf, &bufsize);

  /* Check if that failed because the file is empty. */
  if (status == LOCKER_EOF)
    {
      status = LOCKER_ENOTATTACHED;
      goto cleanup2;
    }
  else if (status != LOCKER_SUCCESS)
    goto cleanup;

  /* OK. It's attached. Return to checking the version number. */
  at->attached = 1;
  if (atoi(buf) != 1)
    {
      locker__error(context, "Could not parse attachtab file %s:\n"
		    "unknown version number \"%s\".\n", name, buf);
      status = LOCKER_EATTACHTAB;
      goto cleanup2;
    }

  /* Read locker name. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->name = strdup(buf);
  if (!at->name)
    {
      status = LOCKER_ENOMEM;
      goto cleanup;
    }

  /* Read mountpoint. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->mountpoint = strdup(buf);
  if (!at->mountpoint)
    {
      status = LOCKER_ENOMEM;
      goto cleanup;
    }

  /* Read fstype. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->fs = locker__get_fstype(context, buf);
  if (!at->fs)
    {
      status = LOCKER_ENOMEM;
      goto cleanup;
    }

  /* Read hostaddr. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->hostaddr.s_addr = inet_addr(buf);

  /* Read hostdir. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->hostdir = strdup(buf);
  if (!at->hostdir)
    {
      status = LOCKER_ENOMEM;
      goto cleanup;
    }

  /* Read mode */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->mode = buf[0];

  /* Read flags */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->flags = atoi(buf);

  /* Read list of owners. */
  if ((status = locker__read_line(fp, &buf, &bufsize)) != LOCKER_SUCCESS)
    goto cleanup;
  at->nowners = strtol(buf, &p, 10);
  if (p == buf || (*p && *p != ':'))
    {
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }
  at->owners = malloc(at->nowners * sizeof(uid_t));
  if (!at->owners)
    {
      status = LOCKER_ENOMEM;
      goto cleanup;
    }

  for (i = 0, p++; i < at->nowners && p; i++, p = q + 1)
    {
      at->owners[i] = strtol(p, &q, 10);
      if (p == q || (*q && *q != ':'))
	{
	  status = LOCKER_EATTACHTAB;
	  goto cleanup;
	}
    }
  if (i < at->nowners)
    {
      status = LOCKER_EATTACHTAB;
      goto cleanup;
    }

  free(buf);

  /* If this is a MUL locker, use mul_parse to read in the other
   * attachents.
   */
  if (!strcmp(at->fs->name, "MUL"))
    {
      locker_attachent *atm;

      status = at->fs->parse(context, at->name, at->mountpoint, NULL, &atm);
      if (status)
	goto cleanup;

      /* Swap the parsed attachent's MUL chain into the one we read off
       * disk, and then dispose of the parsed one.
       */
      at->next = atm->next;
      atm->next = NULL;
      locker_free_attachent(context, atm);
    }

  *atp = at;
  return LOCKER_SUCCESS;

cleanup:
  if (status == LOCKER_EATTACHTAB)
    {
      locker__error(context, "Invalid line in attachtab file %s:%s\n",
		    name, buf);
    }
  else if (status == LOCKER_EMOUNTPOINTBUSY)
    locker__error(context, "%s: Mountpoint busy.\n", at->mountpoint);
  else if (status == LOCKER_ENOMEM)
    locker__error(context, "Out of memory reading attachtab file.\n");
  else
    {
      locker__error(context, "Unexpected %s while reading attachtab "
		    "file %s.\n", status == LOCKER_EOF ? "end of file" :
		    "file error", name);
      status = LOCKER_EATTACHTAB;
    }
cleanup2:
  free(buf);
  locker_free_attachent(context, at);
  return status;
}

/* Lock the attachtab directory. This must be done in order to:
 *  - create an attachent
 *  - delete an attachent
 *  - create a symlink from mountpoints/ to lockers/
 *  - prevent other processes from doing the above, to guarantee
 *    a consistent view of the attachtab.
 */
static int lock_attachtab(locker_context context, int *lock)
{
  int lockfd, status;
  char *path;
  struct flock fl;

  /* If the attachtab is already locked (because we're reading a
   * subcomponent of a MUL locker, for instance), just record an
   * additional lock and return success.
   */
  if (context->locks)
    {
      context->locks++;
      *lock = -1;
      return LOCKER_SUCCESS;
    }

  path = locker__attachtab_pathname(context, LOCKER_LOCK, ".lock");
  if (!path)
    {
      locker__error(context, "Out of memory in lock_attachtab.\n");
      return LOCKER_ENOMEM;
    }

  lockfd = open(path, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
  free (path);
  if (lockfd < 0)
    {
      locker__error(context, "Could not open attachtab lock file: %s.\n",
		    strerror(errno));
      return LOCKER_EATTACHTAB;
    }

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = fl.l_len = 0;
  status = fcntl(lockfd, F_SETLKW, &fl);
  if (status < 0)
    {
      close(lockfd);
      locker__error(context, "Could not lock attachtab: %s.\n",
		    strerror(errno));
      return LOCKER_EATTACHTAB;
    }

  context->locks++;
  *lock = lockfd;
  return LOCKER_SUCCESS;
}

static void unlock_attachtab(locker_context context, int lock)
{
  if (--context->locks == 0)
    close(lock);
}

static char *kpath[] = {
  ".", /* LOCK */
  "locker", /* NAME */
  "mountpoint", /* MOUNTPOINT */
  "directory", /* DIRECTORY */
};

char *locker__attachtab_pathname(locker_context context,
				 int kind, char *name)
{
  int len;
  char *path, *s, *d;

  len = strlen(context->attachtab) + strlen(kpath[kind]) + strlen(name) + 3;
  for (path = name; *path; path++)
    {
      if (*path == '/' || *path == '_')
	len++;
    }

  path = malloc(len);
  if (!path)
    {
      locker__error(context, "Out of memory reading attachent.\n");
      return NULL;
    }

  sprintf(path, "%s/%s/", context->attachtab, kpath[kind]);
  d = path + strlen(path);

  for (s = name; *s; s++)
    {
      switch (*s)
	{
	case '/':
	  *d++ = '_';
	  *d++ = '=';
	  break;

	case '_':
	  *d++ = '_';
	  /* fall through */

	default:
	  *d++ = *s;
	}
    }
  *d = '\0';

  return path;
}

locker_attachent *locker__new_attachent(locker_context context,
					struct locker_ops *type)
{
  locker_attachent *at;

  at = malloc(sizeof(locker_attachent));
  if (!at)
    {
      locker__error(context, "Out of memory creating attachent.\n");
      return NULL;
    }

  memset(at, 0, sizeof(locker_attachent));
  at->name = at->mountpoint = at->hostdir = at->buildfrom = NULL;
  at->owners = NULL;
  at->mountpoint_file = NULL;
  at->next = NULL;

  at->fs = type;
  return at;
}
