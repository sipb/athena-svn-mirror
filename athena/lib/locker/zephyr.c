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

/* This file is part of liblocker. It deals with zephyr subscriptions
 * pertaining to lockers.
 */

static const char rcsid[] = "$Id: zephyr.c,v 1.1 1999-02-26 19:04:54 danw Exp $";

#include <stdlib.h>
#include <unistd.h>

#include <com_err.h>
#include <zephyr/zephyr.h>

#include "locker.h"
#include "locker_private.h"

/* XXX The old version of attach set timeouts around the Zephyr
 * subscription code. We're not quite sure why. This code doesn't,
 * since it's more annoying to do that in a library context. But
 * if something breaks, this might be why.
 */

/* Magic number taken from zctl sources. */
#define ZEPHYR_MAXONEPACKET 7

int locker__zsubs(locker_context context, locker_attachent *at,
		  int op, char **subs)
{
  ZSubscription_t zsubs[ZEPHYR_MAXONEPACKET];
  int i = 0, j, status, retval;
  uid_t uid = geteuid();

  if (context->zephyr_wgport == -1)
    return LOCKER_EZEPHYR;

  for (j = 0; j < ZEPHYR_MAXONEPACKET; j++)
    {
      zsubs[j].zsub_recipient = "*";
      zsubs[j].zsub_class = LOCKER_ZEPHYR_CLASS;
    }

  /* seteuid since Zephyr ops may touch ticket file. */
  if (uid != context->user)
    seteuid(context->user);

  retval = LOCKER_SUCCESS;
  while (subs[i])
    {
      for (j = 0; j < ZEPHYR_MAXONEPACKET && subs[i + j]; j++)
	zsubs[j].zsub_classinst = subs[i + j];

      if (op == LOCKER_ZEPHYR_SUBSCRIBE)
	status = ZSubscribeTo(zsubs, j, context->zephyr_wgport);
      else
	status = ZUnsubscribeTo(zsubs, j, context->zephyr_wgport);
      if (status)
	{
	  locker__error(context, "%s: Error while %ssubscribing:\n%s.\n",
			at->name, op == LOCKER_ZEPHYR_SUBSCRIBE ? "" : "un",
			error_message(status));
	  retval = LOCKER_EZEPHYR;
	  break;
	}

      i += j;
    }

  if (uid != context->user)
    seteuid(uid);

  return retval;
}
