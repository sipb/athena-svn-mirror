/*
 * NFS quota routines
 * 
 *   Uses the rcquota rpc call for group and user quotas
 *
 * $Id: nfs.c,v 1.1 1992-04-10 20:22:58 probe Exp $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netdb.h>
#include <netinet/in.h>

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpcsvc/rquota.h>
#include <rpcsvc/rcquota.h>


extern int uflag, gflag, vflag, fsind;


int
getnfsquota(hostp, path, uid, qvp)
    char *hostp;
    char *path;
    int uid;
    struct getcquota_rslt *qvp;
{
    struct getcquota_args gq_args;
    extern char *index();
    int oldrpc = 0;

    gq_args.gqa_pathp = path;
    gq_args.gqa_uid = (gflag ? getuid() : uid);
    if ((enum clnt_stat)
	callrpc(hostp, RCQUOTAPROG, RCQUOTAVERS,
		(vflag? RCQUOTAPROC_GETQUOTA: RCQUOTAPROC_GETACTIVEQUOTA),
		xdr_getcquota_args, &gq_args, xdr_getcquota_rslt, qvp) ==
	RPC_PROGNOTREGISTERED){

	/* Fallback on old rpc, unless gflag is true, or caller is root */
	struct getquota_rslt oldquota_result;

	if (gflag || !getuid()) return(0);
	oldrpc = 1;
	if (callaurpc(hostp, RQUOTAPROG, RQUOTAVERS,
		      (vflag? RQUOTAPROC_GETQUOTA:
		       RQUOTAPROC_GETACTIVEQUOTA),
		      xdr_getquota_args, &gq_args,
		      xdr_getquota_rslt, &oldquota_result) != 0){
	    /* Okay, it really failed */
	    return (0);
	} else {
	    /* The getquota_rslt structure needs to be converted to
	     * a getcquota_rslt structure. */

	    switch (oldquota_result.gqr_status){
	    case Q_OK: qvp->gqr_status = QC_OK; break;
	    case Q_NOQUOTA: qvp->gqr_status = QC_NOQUOTA; break;
	    case Q_EPERM: qvp->gqr_status = QC_EPERM; break;
	    }

	    qvp->rq_group = 0;		/* only user quota on old rpc's */
	    qvp->rq_ngrps = 1;
	    qvp->rq_bsize = oldquota_result.gqr_rquota.rq_bsize;
	    bzero(&qvp->gqr_zm, sizeof(struct rcquota));

	    qvp->gqr_rcquota[0].rq_id = gq_args.gqa_uid;
	    qvp->gqr_rcquota[0].rq_bhardlimit =
		oldquota_result.gqr_rquota.rq_bhardlimit;
	    qvp->gqr_rcquota[0].rq_bsoftlimit =
		oldquota_result.gqr_rquota.rq_bsoftlimit;
	    qvp->gqr_rcquota[0].rq_curblocks =
		oldquota_result.gqr_rquota.rq_curblocks;
	    qvp->gqr_rcquota[0].rq_fhardlimit =
		oldquota_result.gqr_rquota.rq_fhardlimit;
	    qvp->gqr_rcquota[0].rq_fsoftlimit =
		oldquota_result.gqr_rquota.rq_fsoftlimit;
	    qvp->gqr_rcquota[0].rq_curfiles = 
		oldquota_result.gqr_rquota.rq_curfiles;
	    qvp->gqr_rcquota[0].rq_btimeleft = 
		oldquota_result.gqr_rquota.rq_btimeleft;
	    qvp->gqr_rcquota[0].rq_ftimeleft = 
		oldquota_result.gqr_rquota.rq_ftimeleft;
	}
    }

    switch (qvp->gqr_status) {
    case QC_OK:
	{
	    struct timeval tv;
	    int i;
	    float blockconv;

	    if (gflag){
		if (!qvp->rq_group) return(0);	/* Not group controlled */
		for(i=0;i<qvp->rq_ngrps;i++)
		    if (uid == qvp->gqr_rcquota[i].rq_id) break;
		if (i == qvp->rq_ngrps) return(0); /* group id not in list */
		bcopy(&(qvp->gqr_rcquota[i]), &(qvp->gqr_rcquota[0]),
		      sizeof(struct rcquota));
		qvp->rq_ngrps = 1;
	    }

	    if (uflag && qvp->rq_group) return(0); /* Not user-controlled */

	    gettimeofday(&tv, NULL);

	    blockconv = (float)qvp->rq_bsize / 512;
	    qvp->gqr_zm.rq_bhardlimit *= blockconv;
	    qvp->gqr_zm.rq_bsoftlimit *= blockconv;
	    qvp->gqr_zm.rq_curblocks  *= blockconv;
	    if (!qvp->rq_group) qvp->rq_ngrps = 1;
	    for(i=0;i<qvp->rq_ngrps;i++){
		qvp->gqr_rcquota[i].rq_bhardlimit *= blockconv;
		qvp->gqr_rcquota[i].rq_bsoftlimit *= blockconv;
		qvp->gqr_rcquota[i].rq_curblocks  *= blockconv;
		qvp->gqr_rcquota[i].rq_btimeleft += tv.tv_sec;
		qvp->gqr_rcquota[i].rq_ftimeleft += tv.tv_sec;
	    }
	    return (1);
	}

    case QC_NOQUOTA:
	break;

    case QC_EPERM:
	if (vflag && fsind && !oldrpc)
	    fprintf(stderr, "quota: Warning--no NFS mapping on host: %s\n", hostp);
	if (vflag && fsind && oldrpc)
	    fprintf(stderr, "quota: Permission denied. %s\n", hostp);
	break;

    default:
	fprintf(stderr, "bad rpc result, host: %s\n",  hostp);
	break;
    }
    return (0);
}

callaurpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
    char *host;
    xdrproc_t inproc, outproc;
    struct getcquota_args *in;
    struct getquota_rslt *out;
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
