/*
 * Disk quota reporting program.
 */
#include <stdio.h>
#ifdef SOLARIS
#include <sys/mntent.h> 
#include <rpcsvc/rquota.h>
#else
#include <mntent.h> 
#endif
#include <strings.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#ifdef SOLARIS
#include <sys/fs/ufs_quota.h>
#else
#include <ufs/quota.h>
#endif

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/rquota.h>
#include <sys/time.h>

#include "xquota.h"

static int callaurpc(); 

/************************************************************
 * 
 * stolen from ucb's quota. (Modified somewhat)
 *
 ************************************************************/

int
getnfsquota(host, path, uid, dqp)
	char *host, *path;
	int uid;
	struct dqblk *dqp;
{
	struct getquota_args gq_args;
	struct getquota_rslt gq_rslt;
	extern char *index();

	gq_args.gqa_pathp = path;
	gq_args.gqa_uid = uid;
	if (callaurpc(host, RQUOTAPROG, RQUOTAVERS, RQUOTAPROC_GETQUOTA,
		      xdr_getquota_args, (char *) &gq_args, xdr_getquota_rslt, 
		      (char *) &gq_rslt) != 0) {
		return (QUOTA_ERROR);
	}
#ifdef SOLARIS
	switch (gq_rslt.status) {
#else
	switch (gq_rslt.gqr_status) {
#endif
	case Q_OK:
		{
		struct timeval tv;

		gettimeofday(&tv, NULL);
#ifdef SOLARIS
		dqp->dqb_bhardlimit =
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_bhardlimit *
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_bsoftlimit =
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_bsoftlimit *
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_curblocks =
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_curblocks *
		    gq_rslt.getquota_rslt_u.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_fhardlimit = gq_rslt.getquota_rslt_u.gqr_rquota.rq_fhardlimit;
		dqp->dqb_fsoftlimit = gq_rslt.getquota_rslt_u.gqr_rquota.rq_fsoftlimit;
		dqp->dqb_curfiles = gq_rslt.getquota_rslt_u.gqr_rquota.rq_curfiles;
		dqp->dqb_btimelimit =
		    tv.tv_sec + gq_rslt.getquota_rslt_u.gqr_rquota.rq_btimeleft;
		dqp->dqb_ftimelimit =
		    tv.tv_sec + gq_rslt.getquota_rslt_u.gqr_rquota.rq_ftimeleft;
#else
		dqp->dqb_bhardlimit =
		    gq_rslt.gqr_rquota.rq_bhardlimit *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_bsoftlimit =
		    gq_rslt.gqr_rquota.rq_bsoftlimit *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_curblocks =
		    gq_rslt.gqr_rquota.rq_curblocks *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_fhardlimit = gq_rslt.gqr_rquota.rq_fhardlimit;
		dqp->dqb_fsoftlimit = gq_rslt.gqr_rquota.rq_fsoftlimit;
		dqp->dqb_curfiles = gq_rslt.gqr_rquota.rq_curfiles;
		dqp->dqb_btimelimit =
		    tv.tv_sec + gq_rslt.gqr_rquota.rq_btimeleft;
		dqp->dqb_ftimelimit =
		    tv.tv_sec + gq_rslt.gqr_rquota.rq_ftimeleft;
#endif
		return (QUOTA_OK);
		}

	case Q_NOQUOTA:
		return(QUOTA_NONE);
		break;

	case Q_EPERM:
		fprintf(stderr, "quota permission error, host: %s\n", host);
		return(QUOTA_PERMISSION);
		break;

	default:
		fprintf(stderr, "bad rpc result, host: %s\n",  host);
		break;
	}
	return (QUOTA_ERROR);
}

static int
callaurpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	static CLIENT *client = NULL;
	static int socket = RPC_ANYSOCK;
	static int valid = 0;
	static int oldprognum, oldversnum;
	static char oldhost[256];

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& strcmp(oldhost, host) == 0) {
		/* reuse old client */		
	}
	else {
		valid = 0;
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 6;
		bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		/* ping the remote end via tcp to see if it is up */
		server_addr.sin_port =  htons(PMAPPORT);
		if ((client = clnttcp_create(&server_addr, PMAPPROG,
		    PMAPVERS, &socket, 0, 0)) == NULL) {
			return ((int) rpc_createerr.cf_stat);
		} else {
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
			return ((int) rpc_createerr.cf_stat);
		client->cl_auth = authunix_create_default();
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		strcpy(oldhost, host);
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		valid = 0;
	return ((int) clnt_stat);
}
