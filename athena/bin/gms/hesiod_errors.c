/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/hesiod_errors.c,v $
 * $Author: eichin $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_hes_errors_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/hesiod_errors.c,v 1.1 1988-09-20 21:48:18 eichin Exp $";
#endif lint

#include "hesiod_err.h"
#include <hesiod.h>

Code_t hesiod_error()
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
