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

static const char rcsid[] = "$Id: hesiod_errors.c,v 1.1 1999-12-08 22:06:45 danw Exp $";

#include "globalmessage.h"
#include "hesiod_err.h"
#include <hesiod.h>

Code_t hesiod_error(void)
{
  init_hes_err_tbl();
  
  switch(hes_error()) {
  case HES_ER_UNINIT:
    return(HESIOD_ER_UNINIT);
  case HES_ER_NOTFOUND:
    return(HESIOD_ER_NOTFOUND);
  case HES_ER_CONFIG:
    return(HESIOD_ER_CONFIG);
  case HES_ER_NET:
    return(HESIOD_ER_NET);
  case HES_ER_OK:
    return(HESIOD_ER_OK);
  default:
    return(HESIOD_ER_INVALID);
  }
}
