/* Copyright 1999 by the Massachusetts Institute of Technology.
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

/* This contains the NFS quota-checking routines. */

static const char rcsid[] = "$Id: nfs.c,v 1.3 1999-06-03 14:54:24 danw Exp $";

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <rpc/rpc.h>
#ifdef HAVE_RPC_CLNT_SOC_H
#include <rpc/clnt_soc.h>
#endif
#include <rpc/pmap_prot.h>
#include <rpcsvc/rquota.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "quota.h"

#ifndef GQR_RQUOTA_USES_GQR_RQUOTA
#define gqr_rquota getquota_rslt_u.gqr_rquota
#endif

#ifndef GQR_RQUOTA_USES_GQR_STATUS
#define gqr_status status
#endif

static int callaurpc(char *host, int prognum, int versnum, int procnum,
		     xdrproc_t inproc, struct getquota_args *in,
		     xdrproc_t outproc, struct getquota_rslt *out);

int get_nfs_quota(struct quota_fs *fs, uid_t uid, int verbose)
{
  struct getquota_args gq_args;
  struct getquota_rslt gq_result;
  struct rquota quota;
  int status;
  char *host, *path;
  double bscale;

  /* If user is root, punt. (I don't know why. This is just what the
   * old code did. Probably assuming root will be mapped to "nobody"?)
   */
  if (getuid() == 0)
    return -1;

  host = strdup(fs->device);
  path = strchr(host, ':');
  if (!path)
    return -1;
  *path++ = '\0';

  gq_args.gqa_pathp = path;
  gq_args.gqa_uid = uid;

  if (callaurpc(host, RQUOTAPROG, RQUOTAVERS,
		(verbose ? RQUOTAPROC_GETQUOTA : RQUOTAPROC_GETACTIVEQUOTA),
		xdr_getquota_args, &gq_args,
		xdr_getquota_rslt, &gq_result) != 0)
    {
      free(host);
      return -1;
    }

  quota = gq_result.gqr_rquota;
  switch (gq_result.gqr_status)
    {
    case Q_OK:
      bscale = quota.rq_bsize / 512.0;
      fs->have_quota = fs->have_blocks = fs->have_files = 1;
      fs->dqb.dqb_curblocks = quota.rq_curblocks * bscale;
      fs->dqb.dqb_bsoftlimit = quota.rq_bsoftlimit * bscale;
      fs->dqb.dqb_bhardlimit = quota.rq_bhardlimit * bscale;
      fs->dqb.dqb_btimelimit = quota.rq_btimeleft;
      fs->dqb.dqb_curfiles = quota.rq_curfiles;
      fs->dqb.dqb_fsoftlimit = quota.rq_fsoftlimit;
      fs->dqb.dqb_fhardlimit = quota.rq_fhardlimit;
      fs->dqb.dqb_ftimelimit = quota.rq_ftimeleft;

      if (fs->dqb.dqb_btimelimit)
	fs->warn_blocks = 1;
      if (fs->dqb.dqb_ftimelimit)
	fs->warn_files = 1;

      status = 0;

    case Q_NOQUOTA:
      fs->have_quota = 0;
      status = -1;

    case Q_EPERM:
      if (verbose)
	fprintf(stderr, "quota: Permission denied. %s\n", host);
      status = -1;
    }

  free(host);
  return status;
}

static int callaurpc(char *host, int prognum, int versnum, int procnum,
		     xdrproc_t inproc, struct getquota_args *in,
		     xdrproc_t outproc, struct getquota_rslt *out)
{
  struct sockaddr_in server_addr;
  struct hostent *hp;
  struct timeval timeout;
  CLIENT *client;
  int socket, status;

  socket = RPC_ANYSOCK;
  if ((hp = gethostbyname(host)) == NULL)
    return RPC_UNKNOWNHOST;
  timeout.tv_usec = 0;
  timeout.tv_sec = 6;
  memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);
  server_addr.sin_family = AF_INET;

  /* ping the remote end via tcp to see if it is up */
  server_addr.sin_port = htons(PMAPPORT);
  if ((client = clnttcp_create(&server_addr, PMAPPROG,
			       PMAPVERS, &socket, 0, 0)) == NULL)
    return rpc_createerr.cf_stat;
  else
    {
      /* the fact we succeeded means the machine is up */
      close(socket);
      socket = RPC_ANYSOCK;
      clnt_destroy(client);
      client = NULL;
    }

  /* now really create a udp client handle */
  server_addr.sin_port =  0;
  if ((client = clntudp_create(&server_addr, prognum,
			       versnum, timeout, &socket)) == NULL)
    return rpc_createerr.cf_stat;
  auth_destroy(client->cl_auth);
  client->cl_auth = authunix_create_default();

  timeout.tv_usec = 0;
  timeout.tv_sec = 25;
  status = clnt_call(client, procnum, inproc, (caddr_t) in, outproc,
		     (caddr_t) out, timeout);

  auth_destroy(client->cl_auth);
  clnt_destroy(client);
  return status;
}
