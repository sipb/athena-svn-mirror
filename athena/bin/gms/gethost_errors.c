/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/gethost_errors.c,v $
 * $Author: eichin $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_gethost_errors_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/gethost_errors.c,v 1.1 1988-09-26 15:37:33 eichin Exp $";
#endif lint

#include "gethost_err.h"
#include <netdb.h>

typedef int Code_t;

Code_t gethost_error()
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
    
