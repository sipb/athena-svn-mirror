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

static const char rcsid[] = "$Id: gethost_errors.c,v 1.1 1999-12-08 22:06:45 danw Exp $";

#include "globalmessage.h"
#include "gethost_err.h"
#include <netdb.h>

extern int h_errno;

Code_t gethost_error(void)
{
  init_ghs_err_tbl();

  switch(h_errno) {
  case HOST_NOT_FOUND:
    return(GETHOST_HOST_NOT_FOUND);
  case TRY_AGAIN:
    return(GETHOST_TRY_AGAIN);
  case NO_RECOVERY:
    return(GETHOST_NO_RECOVERY);
  case NO_ADDRESS:
    return(GETHOST_NO_ADDRESS);
  default:
    return(GETHOST_INVALID);
  }
}
    
