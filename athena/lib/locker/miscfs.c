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

/* This file is part of liblocker. It implements UFS and ERR lockers. */

static const char rcsid[] = "$Id: miscfs.c,v 1.2 1999-03-29 17:33:23 danw Exp $";

#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "locker.h"
#include "locker_private.h"

static int ufs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp);
static int ufs_auth(locker_context context, locker_attachent *at,
		    int mode, int op);
static int ufs_zsubs(locker_context context, locker_attachent *at);

struct locker_ops locker__ufs_ops = {
  "UFS",
  LOCKER_FS_NEEDS_MOUNTDIR,
  ufs_parse,
  locker__mount,
  locker__unmount,
  ufs_auth,
  ufs_zsubs
};

static int err_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **at);

struct locker_ops locker__err_ops = {
  "ERR",
  0,
  err_parse,
  NULL,
  NULL,
  NULL,
  NULL
};

static int ufs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp)
{
  locker_attachent *at;
  char *p, *dup = NULL, *lasts = NULL;
  int status;

  at = locker__new_attachent(context, &locker__ufs_ops);
  if (!at)
    return LOCKER_ENOMEM;

  if (!name)
    {
      at->name = strdup(desc);
      at->hostdir = strdup(desc);
      if (mountpoint)
	at->mountpoint = strdup(mountpoint);
      else
	at->mountpoint = strdup(LOCKER_UFS_MOUNT_DIR);
      at->mode = LOCKER_AUTH_READWRITE;

      if (!at->name || !at->hostdir || !at->mountpoint)
	goto mem_error;
    }
  else
    {
      /* A Hesiod UFS description (if we had any) would look like:
       * UFS /dev/dsk/c0t0d0s2 w /u1
       */

      at->name = strdup(name);
      if (!at->name)
	goto mem_error;

      dup = strdup(desc);
      if (!dup)
	goto mem_error;

      /* Skip "UFS". */
      if (!strtok_r(dup, " ", &lasts))
	goto parse_error;

      /* Hostdir */
      at->hostdir = strtok_r(NULL, " ", &lasts);
      if (!at->hostdir)
	goto parse_error;
      at->hostdir = strdup(at->hostdir);
      if (!at->hostdir)
	goto mem_error;

      /* Mount mode */
      p = strtok_r(NULL, " ", &lasts);
      if (!p || *(p + 1))
	goto parse_error;

      switch (*p)
	{
	case 'r':
	  at->mode = LOCKER_AUTH_READONLY;
	  break;
	case 'w':
	  at->mode = LOCKER_AUTH_READWRITE;
	  break;
	case 'n':
	  at->mode = LOCKER_AUTH_READWRITE;
	  break;
	default:
	  locker__error(context, "%s: Unrecognized mount option '%c' in "
			"description:\n%s\n", name, *p, desc);
	  status = LOCKER_EPARSE;
	  goto cleanup;
	}

      /* Mountpoint */
      p = strtok_r(NULL, " ", &lasts);
      if (!p)
	goto parse_error;
      if (mountpoint)
	at->mountpoint = strdup(mountpoint);
      else
	at->mountpoint = strdup(at->mountpoint);
      if (!at->mountpoint)
	goto mem_error;

      free(dup);
      dup = NULL;
    }

  status = locker__canonicalize_path(context, 1, &(at->mountpoint),
				     &(at->buildfrom));
  if (status != LOCKER_SUCCESS)
    goto cleanup;

  *atp = at;
  return LOCKER_SUCCESS;

mem_error:
  locker__error(context, "Out of memory parsing locker description.\n");
  status = LOCKER_ENOMEM;
  goto cleanup;

parse_error:
  locker__error(context, "Could not parse locker description "
		"\"%s\".\n", desc);
  status = LOCKER_EPARSE;

cleanup:
  free(dup);
  locker_free_attachent(context, at);
  return status;
}

static int ufs_auth(locker_context context, locker_attachent *at,
		    int mode, int op)
{
  return LOCKER_SUCCESS;
}

static int ufs_zsubs(locker_context context, locker_attachent *at)
{
  return LOCKER_SUCCESS;
}


static int err_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp)
{
  if (name)
    locker__error(context, "%s: %s\n", name, desc + 4);
  else
    locker__error(context, "%s\n", desc);
  return LOCKER_EATTACH;
}
