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

/* This file is part of liblocker. It deals with miscellaneous
 * public locker operations besides attaching and detaching.
 */

static const char rcsid[] = "$Id: misc.c,v 1.3 1999-10-30 19:32:51 danw Exp $";

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hesiod.h>

#include "locker.h"
#include "locker_private.h"

int locker_auth(locker_context context, char *filesystem, int op)
{
  int status;
  locker_attachent *at;

  status = locker_read_attachent(context, filesystem, &at);
  if (status)
    return status;

  status = at->fs->auth(context, at, LOCKER_AUTH_DEFAULT, op);

  locker_free_attachent(context, at);
  return status;
}

int locker_zsubs(locker_context context, char *filesystem)
{
  int status;
  locker_attachent *at;

  status = locker_read_attachent(context, filesystem, &at);
  if (status)
    return status;

  status = at->fs->zsubs(context, at);

  locker_free_attachent(context, at);
  return status;
}

/* See if the owner list for the locker contains the uid pointed to by
 * ownerp. (For use with locker_iterate_attachtab.)
 */
int locker_check_owner(locker_context context, locker_attachent *at,
		       void *ownerp)
{
  uid_t owner;
  int i;

  owner = *(uid_t *)ownerp;

  for (i = 0; i < at->nowners; i++)
    {
      if (at->owners[i] == owner)
	return 1;
    }
  return 0;
}

/* See if the locker is on the host pointed to by addrp. If *addrp is
 * 0.0.0.0, always returns true. (For use with
 * locker_iterate_attachtab.)
 */
int locker_check_host(locker_context context, locker_attachent *at,
		      void *addrp)
{
  struct in_addr *addr = addrp;
  return !addr->s_addr || at->hostaddr.s_addr == addr->s_addr;
}


/* Look up a locker description (in Hesiod or attach.conf). */
int locker_lookup_filsys(locker_context context, char *name, char ***descs,
			  void **cleanup)
{
  char *conffs;

  /* Look for a locker defined in attach.conf. */
  conffs = locker__fs_data(context, context->filesystem, NULL, name);
  if (conffs)
    {
      *descs = malloc(2 * sizeof(char *));
      if (!*descs)
	{
	  locker__error(context, "Out of memory in looking up filesystem.\n");
	  return LOCKER_ENOMEM;
	}
      (*descs)[0] = conffs;
      (*descs)[1] = NULL;
      *cleanup = NULL;

      return LOCKER_SUCCESS;
    }
  else
    {
      /* Otherwise, look for a locker in Hesiod--which might be an fsgroup. */
      *descs = hesiod_resolve(context->hes_context, name, "filsys");
      if (!*descs && errno == ENOENT)
	{
	  locker__error(context, "%s: Locker unknown.\n", name);
	  return LOCKER_EUNKNOWN;
	}
      if (!*descs || !**descs)
	{
	  if (*descs)
	    hesiod_free_list(context->hes_context, *descs);
	  locker__error(context, "%s: Could not look up Hesiod entry: %s.\n",
			name, strerror(errno));
	  return LOCKER_EHESIOD;
	}

      *cleanup = context->hes_context;
      return LOCKER_SUCCESS;
    }
}

void locker_free_filesys(locker_context context, char **descs, void *cleanup)
{
  if (cleanup)
    hesiod_free_list(cleanup, descs);
  else
    free(descs);
}
