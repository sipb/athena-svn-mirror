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

/* This file is part of liblocker. It implements attaching lockers. */

static const char rcsid[] = "$Id: attach.c,v 1.3 1999-05-11 21:13:50 danw Exp $";

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hesiod.h>

#include "locker.h"
#include "locker_private.h"

static int attach_attachent(locker_context context, locker_attachent *at,
			    int authmode, int options, char *mountoptions);
static int check_mountoptions(locker_context context, locker_attachent *at,
			      int authmode, int options, char **mountoptions);
static int add_owner(locker_context context, locker_attachent *at,
		     uid_t user);

int locker_attach(locker_context context, char *filesystem, char *mountpoint,
		  int authmode, int options, char *mountoptions,
		  locker_attachent **atp)
{
  int status, astatus = LOCKER_EATTACH, order;
  locker_attachent *at;

  if (!context->trusted)
    {
      if (mountpoint && !context->exp_mountpoint)
	{
	  locker__error(context, "%s: You are not allowed to specify "
			"an explicit mountpoint.\n", filesystem);
	  return LOCKER_EPERM;
	}
    }

  /* Try each Hesiod entry for the named filesystem until we run out
   * or succeed in attaching one.
   */
  for (order = 1; ; order++)
    {
      status = locker__lookup_attachent(context, filesystem,
					mountpoint, order, &at);
      if (LOCKER_LOOKUP_FAILURE(status))
	break;
      if (status)
	continue;

      astatus = status = locker_attach_attachent(context, at, authmode,
						 options, mountoptions);
      if (LOCKER_ATTACH_SUCCESS(status))
	{
	  if (atp)
	    *atp = at;
	  else
	    locker_free_attachent(context, at);
	  return status;
	}

      locker_free_attachent(context, at);
    }

  return status == LOCKER_ENOENT ? astatus : status;
}

int locker_attach_explicit(locker_context context, char *type,
			   char *desc, char *mountpoint, int authmode,
			   int options, char *mountoptions,
			   locker_attachent **atp)
{
  int status;
  locker_attachent *at;

  if (!context->trusted)
    {
      if (!context->exp_desc)
	{
	  locker__error(context, "%s: You are not allowed to specify "
			"an explicit locker description.\n", desc);
	  return LOCKER_EPERM;
	}

      if (mountpoint && !context->exp_mountpoint)
	{
	  locker__error(context, "%s: You are not allowed to specify "
			"an explicit mountpoint.\n", desc);
	  return LOCKER_EPERM;
	}
    }

  status = locker__lookup_attachent_explicit(context, type, desc,
					     mountpoint, 1, &at);
  if (status)
    return status;

  status = locker_attach_attachent(context, at, authmode,
				   options, mountoptions);

  if (LOCKER_ATTACH_SUCCESS(status) && atp)
    *atp = at;
  else
    locker_free_attachent(context, at);

  return status;
}

int locker_attach_attachent(locker_context context, locker_attachent *at,
			    int authmode, int options, char *mountoptions)
{
  int status = LOCKER_SUCCESS;
  locker_attachent *ai;

  if (!context->trusted)
    {
      if (options & LOCKER_ATTACH_OPT_OVERRIDE)
	{
	  locker__error(context, "%s: You are not authorized to use the "
			"override option.\n", at->name);
	  return LOCKER_EPERM;
	}
      if (options & LOCKER_ATTACH_OPT_LOCK)
	{
	  locker__error(context, "%s: You are not authorized to use the "
			"lock option.\n", at->name);
	  return LOCKER_EPERM;
	}
      if (options & LOCKER_ATTACH_OPT_ALLOW_SETUID)
	{
	  locker__error(context, "%s: You are not authorized to use the "
			"setuid option.\n", at->name);
	  return LOCKER_EPERM;
	}
    }

  /* If this is a MUL locker, attach sublockers first. */
  for (ai = at->next; ai && status == LOCKER_SUCCESS; ai = ai->next)
    status = attach_attachent(context, ai, authmode, options, mountoptions);

  if (!LOCKER_ATTACH_SUCCESS(status))
    {
      locker__error(context, "%s: MUL attach failed.\n", at->name);
      return status;
    }

  return attach_attachent(context, at, authmode, options, mountoptions);
}

/* This is the routine that does all of the work of attaching a single
 * filesystem.
 */
static int attach_attachent(locker_context context, locker_attachent *at,
			    int authmode, int options, char *mountoptions)
{
  int status;

  if (!(options & LOCKER_ATTACH_OPT_OVERRIDE))
    {
      /* Make sure this locker is allowed. */
      if (!locker__fs_ok(context, context->allow, at->fs, at->name))
	{
	  locker__error(context, "%s: You are not allowed to attach this "
			"locker.\n", at->name);
	  return LOCKER_EPERM;
	}

      /* Make sure this mountpoint is allowed. */
      if (strcmp(at->fs->name, "MUL") &&
	  !locker__fs_ok(context, context->mountpoint, at->fs, at->mountpoint))
	{
	  locker__error(context, "%s: You are not allowed to attach a "
			"locker on %s\n", at->name, at->mountpoint);
	  return LOCKER_EPERM;
	}
    }

  /* Authenticate. */
  if (authmode == LOCKER_AUTH_DEFAULT)
    authmode = at->mode;
  if ((authmode != LOCKER_AUTH_NONE) &&
      (!at->attached || (options & LOCKER_ATTACH_OPT_REAUTH)))
    {
      status = at->fs->auth(context, at, authmode, LOCKER_AUTH_AUTHENTICATE);
      if (status != LOCKER_SUCCESS && authmode != LOCKER_AUTH_MAYBE_READWRITE)
	return status;
    }

  if (!at->attached)
    {
      /* Check and update the mountoptions. */
      if (locker__fs_ok(context, context->setuid, at->fs, at->name))
	{
	  options |= LOCKER_ATTACH_OPT_ALLOW_SETUID;
	  at->flags &= ~LOCKER_FLAG_NOSUID;
	}
      status = check_mountoptions(context, at, authmode, options,
				  &mountoptions);
      if (status != LOCKER_SUCCESS)
	return status;

      /* Build the mountpoint if all of the directories don't exist. */
      status = locker__build_mountpoint(context, at);
      if (status != LOCKER_SUCCESS)
	{
	  free(mountoptions);
	  return status;
	}

      /* Attach the locker. */
      status = at->fs->attach(context, at, mountoptions);
      free(mountoptions);
      if (status == LOCKER_EALREADY)
	{
	  if (context->keep_mount)
	    at->flags |= LOCKER_FLAG_KEEP;
	}
      else if (status != LOCKER_SUCCESS)
	return status;
    }
  else
    status = LOCKER_SUCCESS;

  /* Update attachent on disk. */
  at->attached = 1;
  add_owner(context, at, context->user);
  if (options & LOCKER_ATTACH_OPT_LOCK)
    at->flags |= LOCKER_FLAG_LOCKED;
  if (!(options & LOCKER_ATTACH_OPT_ALLOW_SETUID))
    at->flags |= LOCKER_FLAG_NOSUID;
  locker__update_attachent(context, at);

  /* Record zephyr subscriptions. */
  if (options & LOCKER_ATTACH_OPT_ZEPHYR)
    at->fs->zsubs(context, at);

  return status;
}

/* This function constructs a complete string of mount options for the
 * filesystem based on the authmode, options, mountoptions, and
 * attach.conf.
 */
static int check_mountoptions(locker_context context, locker_attachent *at,
			      int authmode, int options, char **mountoptions)
{
  char *mo, *req, *def, *allow, *p, *q;
  int len;

  req = locker__fs_data(context, context->reqopts, at->fs, at->name);
  def = locker__fs_data(context, context->defopts, at->fs, at->name);
  allow = locker__fs_data(context, context->allowopts, at->fs, at->name);

  /* Malloc a string large enough to hold all necessary mount options. */
  len = 1;
  if (*mountoptions)
    len += strlen(*mountoptions) + 1;
  if (req)
    len += strlen(req) + 1;
  if (def)
    len += strlen(def) + 1;
  if (!(options & LOCKER_ATTACH_OPT_ALLOW_SETUID))
    len += 7;
  if (authmode != LOCKER_AUTH_NONE)
    len += 3;
  mo = malloc(len);
  if (!mo)
    {
      locker__error(context, "Out of memory checking mountoptions.\n");
      return LOCKER_ENOMEM;
    }
  *mo = '\0';

  /* Later mountoptions will override earlier ones. So we copy them in
   * in the order: defaults, authmode, mountoptions, setuid (from
   * options), and finally the mountoptions specified by attach.conf.
   * This means that an explicitly specified "ro" or "rw" will
   * override the default locker mode, an explicitly specified "suid"
   * will NOT override an enforced "nosuid", and nothing will override
   * the attach.conf-mandated options. In addition to this, we have to
   * verify that each option specified in mountoptions is allowed at
   * all.
   */

  if (def)
    sprintf(mo, "%s,", def);
  if (authmode == LOCKER_AUTH_READONLY)
    strcat(mo, "ro,");
  else if (authmode != LOCKER_AUTH_NONE)
    strcat(mo, "rw,");
  p = mo + strlen(mo);
  if (*mountoptions)
    strcpy(p, *mountoptions);

  /* Interlude: check user-specified mountoptions */
  while (p && *p)
    {
      /* Get the length of the option pointed to by p. */
      len = strcspn(p, "=,");

      /* See if this option appears in the allowed options list. */
      q = allow;
      while (q)
	{
	  if (!strncmp(p, q, len))
	    break;

	  q = strchr(q, ',');
	  if (q)
	    q++;
	}

      if (!q)
	{
	  /* Option not allowed. Silently discard it. */
	  q = strchr(p, ',');
	  if (q)
	    memmove(p, q + 1, strlen(q + 1) + 1);
	  else
	    *p = '\0';
	}
      else
	{
	  p = strchr(p, ',');
	  if (p)
	    p++;
	}
    }

  /* Now continue appending options. */
  if (!(options & LOCKER_ATTACH_OPT_ALLOW_SETUID))
    strcat(mo, "nosuid,");
  if (req)
    strcat(mo, req);

  /* If mo ends with a comma, remove it. */
  len = strlen(mo);
  if (mo[len - 1] == ',')
    mo[len - 1] = '\0';

  /* Finally, go back through and remove duplicates and contradictions. */
  p = mo - 1;
  while (p)
    {
      p++;
      len = strcspn(p, "=,");
      q = strchr(p, ',');

      if (q)
	{
	  /* Read through options, looking for a match for p. If p
	   * starts with "no", also look for a match without the "no".
	   * If p doesn't start with "no", look for a match with "no".
	   */
	  while (q)
	    {
	      q++;
	      if (!strncmp(p, q, len))
		break;
	      if (!strncmp(p, "no", 2) && !strncmp(p + 2, q, len - 2))
		break;
	      else if (!strncmp(q, "no", 2) && !strncmp(p, q + 2, len))
		break;
	      q = strchr(q, ',');
	    }

	  /* If we found a match, delete the first occurrence. */
	  if (q)
	    {
	      q = strchr(p, ','); /* We know this won't return NULL here. */
	      memmove(p, q + 1, strlen(q + 1) + 1);
	    }
	  else
	    p = strchr(p, ',');
	}
      else
	break;
    }

  *mountoptions = mo;
  return LOCKER_SUCCESS;
}

static int add_owner(locker_context context, locker_attachent *at, uid_t user)
{
  int i;
  uid_t *new;

  for (i = 0; i < at->nowners; i++)
    {
      if (at->owners[i] == user)
	return LOCKER_SUCCESS;
    }

  new = realloc(at->owners, (at->nowners + 1) * sizeof(uid_t));
  if (!new)
    {
      locker__error(context, "Out of memory adding new owner to "
		    "attachtab entry.\n");
      return LOCKER_ENOMEM;
    }
  at->owners = new;
  at->owners[at->nowners++] = user;
  return LOCKER_SUCCESS;
}
