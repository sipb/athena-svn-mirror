/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_connect.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_connect.c,v 1.1 1993-10-12 03:03:59 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_connect_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_connect.c,v 1.1 1993-10-12 03:03:59 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_connect -- establish client connection for FX *
 *              fxp->host should already be set;
 */

long
fx_connect(fxp)
     FX *fxp;
{
  /* establish RPC client connection */
  fxp->cl = clnt_create(fxp->host, FXSERVER, FXVERS, "tcp");
  if (!fxp->cl) return(_fx_rpc_errno(fxp->cl));
  return(0L);
}
