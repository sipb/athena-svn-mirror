/* Copyright 1988, 1998 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: view_message_by_tty.c,v 1.1 1999-12-08 22:06:46 danw Exp $";

#include "globalmessage.h"

void view_message_by_tty(char *message)
{
  char *ptr;

  /* skip magic headers */
  ptr = strchr(message, '\n')+1;
  /* note that if there is nothing, strlen(ptr) == 0 */
  write(1,ptr,strlen(ptr));
}
