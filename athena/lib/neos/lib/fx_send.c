/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send.c,v 1.1 1993-10-12 03:03:50 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_send_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_send.c,v 1.1 1993-10-12 03:03:50 probe Exp $";
#endif /* lint */

#include "fxcl.h"

/*
 * fx_send -- send a stream to the exchange
 */

long
fx_send(fxp, p, fp)
     FX *fxp;
     Paper *p;
     FILE *fp;
{
  long *ret, code = 0L;
  int dummy;
  burst_data data;
  Paper to_send;
#ifdef KERBEROS
  char new_owner[FX_UNAMSZ], new_author[FX_UNAMSZ];
#else
  char new_owner[9], new_author[9];
#endif

  /* take care of null pointers */
  if (p) paper_copy(p, &to_send);
  else paper_clear(&to_send);

  if (!to_send.location.host)
    to_send.location.host = fxp->host;
  if (!to_send.author) to_send.author = fxp->owner;
  if (!to_send.owner) to_send.owner = fxp->owner;
  if (!to_send.filename) to_send.filename = FX_DEF_FILENAME;
  if (!to_send.desc) to_send.desc = FX_DEF_DESC;
  if (!to_send.assignment) to_send.assignment = FX_DEF_ASSIGNMENT;
  if (!to_send.type) to_send.type = FX_DEF_TYPE;

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  to_send.owner = _fx_lengthen(fxp, to_send.owner, new_owner);
  to_send.author = _fx_lengthen(fxp, to_send.author, new_author);
#endif

  if ((ret = send_file_1(&to_send, fxp->cl)) == NULL)
    goto FX_SEND_CLEANUP;

  if (*ret) goto FX_SEND_CLEANUP;

  /* send the bursts */
  do {
    if (ret) xdr_free(xdr_long, (char *) ret);
    data.size = fread(data.data, 1, MAX_BURST_SIZE, fp);
    if ((ret = send_burst_1(&data, fxp->cl)) == NULL)
      goto FX_SEND_CLEANUP;
    if (*ret) goto FX_SEND_CLEANUP;
  } while (data.size == MAX_BURST_SIZE);

  ret = end_send_1(&dummy, fxp->cl);

 FX_SEND_CLEANUP:
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
