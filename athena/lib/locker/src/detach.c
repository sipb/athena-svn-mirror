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

/* This file is part of liblocker. It implements detaching lockers. */

static const char rcsid[] = "$Id: detach.c,v 1.3 1999-12-27 18:13:16 danw Exp $";

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hesiod.h>

#include "locker.h"
#include "locker_private.h"

static int detach_attachent(locker_context context,
			    locker_attachent *at, int options);
static void del_owner(locker_context context, locker_attachent *at,
		      uid_t user);
static void clean_owners(locker_context context, locker_attachent *at);


int locker_detach(locker_context context, char *filesystem, char *mountpoint,
		  int options, locker_attachent **atp)
{
  int status;
  locker_attachent *at;

  status = locker__lookup_attachent(context, filesystem, mountpoint, 0, &at);
  if (status)
    return status;

  status = locker_detach_attachent(context, at, options);
  if (LOCKER_DETACH_SUCCESS(status) && atp)
    *atp = at;
  else
    locker_free_attachent(context, at);
  return status;
}

int locker_detach_explicit(locker_context context, char *type,
			   char *desc, char *mountpoint, int options,
			   locker_attachent **atp)
{
  int status;
  locker_attachent *at;

  status = locker__lookup_attachent_explicit(context, type, desc,
					     mountpoint, 1, &at);
  if (status)
    return status;

  status = locker_detach_attachent(context, at, options);
  if (LOCKER_DETACH_SUCCESS(status) && atp)
    *atp = at;
  else
    locker_free_attachent(context, at);
  return status;
}

int locker_detach_attachent(locker_context context,
			    locker_attachent *at, int options)
{
  int status = LOCKER_SUCCESS;
  locker_attachent *ai;

  if (!context->trusted)
    {
      if (options & LOCKER_DETACH_OPT_UNLOCK)
	{
	  locker__error(context, "%s: You are not authorized to use the "
			"unlock option.\n", at->name);
	  return LOCKER_EPERM;
	}

      if (options & LOCKER_DETACH_OPT_OVERRIDE)
	{
	  locker__error(context, "%s: You are not authorized to use the "
			"override option.\n", at->name);
	  return LOCKER_EPERM;
	}
    }

  /* If this is a MUL locker, detach sublockers first. */
  for (ai = at->next; ai && status == LOCKER_SUCCESS; ai = ai->next)
    status = detach_attachent(context, ai, options);

  if (!LOCKER_DETACH_SUCCESS(status))
    {
      locker__error(context, "%s: MUL detach failed.\n", at->name);
      return status;
    }

  status = detach_attachent(context, at, options);

  return status;
}

/* This is the routine that does all of the work of attaching a single
 * filesystem.
 */
static int detach_attachent(locker_context context,
			    locker_attachent *at, int options)
{
  int status, dstatus;

  if (!context->trusted)
    {
      /* Don't allow a user to detach something he couldn't attach. */
      if (!locker__fs_ok(context, context->allow, at->fs, at->name))
	{
	  locker__error(context, "%s: You are not allowed to detach this "
			"locker.\n", at->name);
	  return LOCKER_EPERM;
	}
      if (strcmp(at->fs->name, "MUL") &&
	  !locker__fs_ok(context, context->mountpoint, at->fs, at->mountpoint))
	{
	  locker__error(context, "%s: You are not allowed to detach a "
			"locker from %s.\n", at->name, at->mountpoint);
	  return LOCKER_EPERM;
	}
    }

  /* Check locking. */
  if (at->flags & LOCKER_FLAG_LOCKED &&
      !(options & LOCKER_DETACH_OPT_UNLOCK))
    {
      locker__error(context, "%s: filesystem is locked.\n", at->name);
      return LOCKER_EPERM;
    }

  if (options & LOCKER_DETACH_OPT_CLEAN)
    {
      clean_owners(context, at);
      if (at->nowners)
	{
	  locker__update_attachent(context, at);
	  return LOCKER_SUCCESS;
	}
    }
  else
    del_owner(context, at, context->user);

  if (at->nowners && (context->ownercheck ||
		      (options & LOCKER_DETACH_OPT_OWNERCHECK)) &&
      !(options & LOCKER_DETACH_OPT_OVERRIDE))
    {
      locker__error(context, "%s: Locker is wanted by others. Not detached.\n",
		    at->name);
      dstatus = LOCKER_EINUSE;
    }
  else
    {
      if (!(at->flags & LOCKER_FLAG_KEEP))
	{
	  /* Detach the locker. */
	  dstatus = at->fs->detach(context, at);
	  if (!LOCKER_DETACH_SUCCESS(dstatus))
	    return dstatus;

	  /* Remove any mountpoint components we created. */
	  status = locker__remove_mountpoint(context, at);
	  if (status != LOCKER_SUCCESS && status != LOCKER_EMOUNTPOINTBUSY)
	    return status;
	}
      else
	dstatus = LOCKER_SUCCESS;

      /* Mark the locker detached. */
      at->attached = 0;
    }
  locker__update_attachent(context, at);

  /* Unauthenticate. */
  if (options & LOCKER_DETACH_OPT_UNAUTH)
    at->fs->auth(context, at, LOCKER_AUTH_DEFAULT, LOCKER_AUTH_UNAUTHENTICATE);

  /* Record zephyr unsubscriptions. */
  if (options & LOCKER_DETACH_OPT_UNZEPHYR)
    at->fs->zsubs(context, at);

  return dstatus;
}

static void del_owner(locker_context context, locker_attachent *at,
		      uid_t user)
{
  int i;

  for (i = 0; i < at->nowners; i++)
    {
      if (at->owners[i] == user)
	{
	  at->owners[i] = at->owners[--at->nowners];
	  return;
	}
    }
}

static void clean_owners(locker_context context, locker_attachent *at)
{
  int i;

  for (i = 0; i < at->nowners; )
    {
      if (!getpwuid(at->owners[i]))
	at->owners[i] = at->owners[--at->nowners];
      else
	i++;
    }
}
