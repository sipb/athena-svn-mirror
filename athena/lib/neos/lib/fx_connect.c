/**********************************************************************
 * File Exchange client library
 *
 * $Id: fx_connect.c,v 1.2 1999-01-22 23:17:54 ghudson Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_connect_c[] = "$Id: fx_connect.c,v 1.2 1999-01-22 23:17:54 ghudson Exp $";
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
