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

/* This file is part of liblocker. It implements some of the code for
 * MUL lockers. (Most of the code for MUL lockers is special-cased
 * into attachtab.c, attach.c, and detach.c.)
 */

static const char rcsid[] = "$Id: mul.c,v 1.3 1999-09-22 22:25:08 danw Exp $";

#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "locker.h"
#include "locker_private.h"

static int mul_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **at);
static int mul_attach(locker_context context, locker_attachent *at,
		      char *mountoptions);
static int mul_detach(locker_context context, locker_attachent *at);
static int mul_auth(locker_context context, locker_attachent *at,
		    int mode, int op);
static int mul_zsubs(locker_context context, locker_attachent *at);

struct locker_ops locker__mul_ops = {
  "MUL",
  0,
  mul_parse,
  mul_attach,
  mul_detach,
  mul_auth,
  mul_zsubs
};

static int mul_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp)
{
  int status;
  char *p, *dup, *lasts = NULL;
  locker_attachent *at, *attmp;

  if (!name)
    {
      locker__error(context, "Cannot explicitly specify MUL locker.\n");
      return LOCKER_EPARSE;
    }

  if (mountpoint)
    {
      locker__error(context, "%s: Explicit mountpoint is meaningless for "
		    "MUL locker.\n", name);
      return LOCKER_EPARSE;
    }

  at = locker__new_attachent(context, &locker__mul_ops);
  if (!at)
    return LOCKER_ENOMEM;

  /* Skip "MUL". (But if this is called from read_attachent, the "MUL"
   * won't be there.)
   */
  if (!strncmp(desc, "MUL ", 4))
    {
      p = desc + 4;
      while (isspace((unsigned char)*p))
	p++;
    }
  else
    p = desc;

  at->name = strdup(name);
  at->mountpoint = strdup(p);
  at->hostdir = strdup("");
  at->mode = '-';
  dup = strdup(p);
  if (!at->name || !at->mountpoint || !at->hostdir || !dup)
    {
      locker_free_attachent(context, at);
      locker__error(context, "Out of memory parsing locker description.\n");
      return LOCKER_ENOMEM;
    }

  /* Read list of locker names. */
  for (p = strtok_r(dup, " ", &lasts); p; p = strtok_r(NULL, " ", &lasts))
    {
      status = locker__lookup_attachent(context, p, NULL, 1, &attmp);
      if (status)
	break;

      if (attmp->fs == at->fs)
	{
	  locker__error(context, "%s: Cannot have MUL locker \"%s\" as a "
			"component of a MUL locker.\n", at->name,
			attmp->name);
	  locker_free_attachent(context, attmp);
	  status = LOCKER_EPARSE;
	  break;
	}

      attmp->next = at->next;
      at->next = attmp;
    }

  free(dup);

  if (status)
    {
      for (; at; at = attmp)
	{
	  attmp = at->next;
	  locker_free_attachent(context, at);
	}
    }
  else
    *atp = at;

  return status;
}

static int mul_attach(locker_context context, locker_attachent *at,
		      char *mountoptions)
{
  return LOCKER_SUCCESS;
}

static int mul_detach(locker_context context, locker_attachent *at)
{
  return LOCKER_SUCCESS;
}

static int mul_auth(locker_context context, locker_attachent *at,
		    int mode, int op)
{
  return LOCKER_SUCCESS;
}

static int mul_zsubs(locker_context context, locker_attachent *at)
{
  return LOCKER_SUCCESS;
}
