/**********************************************************************
 * File Exchange client library
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_open.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_open.c,v 1.2 1996-09-20 04:36:13 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_open_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_open.c,v 1.2 1996-09-20 04:36:13 ghudson Exp $";
#endif /* lint */

#include <stdio.h>
#include <string.h>
#include "fxcl.h"
#include "memory.h"

/*** Global variables ***/

#ifdef __STDC__
void (*fx_open_error_hook)(FX *, long) = fx_open_perror;
#else
void (*fx_open_error_hook)() = fx_open_perror;
#endif

char *fx_sync_host = NULL;  /* host known as sync site */
extern int errno;

/*
 * fx_open(s, codep) -- connect to file exchange named s.
 *
 * Sets *codep to error number if codep != NULL;
 *   (Some access may still be allowed despite error)
 * Returns NULL upon total failure
 */

FX *
fx_open(s, codep)
     char *s;
     long *codep;
{
  init_res *res = NULL;
  FX *ret;
  stringlist hosts, node;
  long code = 0L;

  /* Initialization needed for com_err routines */
  initialize_fxcl_error_table();
  initialize_rpc_error_table();
  initialize_fxsv_error_table();
  initialize_krb_error_table();

  /* set up new FX */
  if ((ret = New(FX)) == NULL) {
    code = (long) errno;
    goto FX_OPEN_CLEANUP;
  }
  (void) strcpy(ret->name, s);
  ret->cl = NULL;

  /* get list of hosts to try */
  code = ERR_FXCL_HOSTS;
  hosts = fx_host_list(FX_DEF_SERVICE);

  /* try to initialize course at each host */
  node = hosts;
  while (code && node) {
    (void) strcpy(ret->host, node->s);
    if (res) xdr_free(xdr_init_res, (char *) res);
    code = fx_init(ret, &res);
    if (node->next) fx_open_error_hook(ret, code);
    node = node->next;
  }
  fx_host_list_destroy(hosts);
  if (code) goto FX_OPEN_CLEANUP;

  /* if this host is not the sync host, need to start over */
  if (res->errno == ERR_NOT_SYNC) {
    clnt_destroy(ret->cl);
    (void) strcpy(ret->host, res->init_res_u.sync);
    code = fx_init(ret, &res);
  }
  if (res->errno) code = res->errno;

 FX_OPEN_CLEANUP:
  if (res) xdr_free(xdr_init_res, (char *) res);
  if (codep) *codep = code;
  if (ret)
    if (!ret->cl) {
      free((char *) ret);
      ret = NULL;
    }
  return(ret);
}

void
fx_open_perror(fxp, code)
     FX *fxp;
     long code;
{
  if (code)
    com_err(fxp->host, code, "(%s)", fxp->name);
}

void
fx_close(fxp)
     FX *fxp;
{
  if (fxp) {
    if (fxp->cl) clnt_destroy(fxp->cl);
    free((char *)fxp);
  }
}
