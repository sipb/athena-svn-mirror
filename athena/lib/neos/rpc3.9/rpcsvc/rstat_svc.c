#include <stdio.h>
#include <rpc/rpc.h>
#include "rstat.h"

static void rstatprog_3();
static void rstatprog_2();
static void rstatprog_1();

main()
{
	SVCXPRT *transp;

	pmap_unset(RSTATPROG, RSTATVERS_TIME);
	pmap_unset(RSTATPROG, RSTATVERS_SWTCH);
	pmap_unset(RSTATPROG, RSTATVERS_ORIG);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, RSTATPROG, RSTATVERS_TIME, rstatprog_3, IPPROTO_UDP)) {
		fprintf(stderr, "unable to register (RSTATPROG, RSTATVERS_TIME, udp).\n");
		exit(1);
	}
	if (!svc_register(transp, RSTATPROG, RSTATVERS_SWTCH, rstatprog_2, IPPROTO_UDP)) {
		fprintf(stderr, "unable to register (RSTATPROG, RSTATVERS_SWTCH, udp).\n");
		exit(1);
	}
	if (!svc_register(transp, RSTATPROG, RSTATVERS_ORIG, rstatprog_1, IPPROTO_UDP)) {
		fprintf(stderr, "unable to register (RSTATPROG, RSTATVERS_ORIG, udp).\n");
		exit(1);
	}
	svc_run();
	fprintf(stderr, "svc_run returned\n");
	exit(1);
}

static void
rstatprog_3(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		int fill;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, NULL);
		return;

	case RSTATPROC_STATS:
		xdr_argument = xdr_void;
		xdr_result = xdr_statstime;
		local = (char *(*)()) rstatproc_stats_3;
		break;

	case RSTATPROC_HAVEDISK:
		xdr_argument = xdr_void;
		xdr_result = xdr_u_int;
		local = (char *(*)()) rstatproc_havedisk_3;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}


static void
rstatprog_2(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		int fill;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, NULL);
		return;

	case RSTATPROC_STATS:
		xdr_argument = xdr_void;
		xdr_result = xdr_statsswtch;
		local = (char *(*)()) rstatproc_stats_2;
		break;

	case RSTATPROC_HAVEDISK:
		xdr_argument = xdr_void;
		xdr_result = xdr_u_int;
		local = (char *(*)()) rstatproc_havedisk_2;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}


static void
rstatprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		int fill;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, NULL);
		return;

	case RSTATPROC_STATS:
		xdr_argument = xdr_void;
		xdr_result = xdr_stats;
		local = (char *(*)()) rstatproc_stats_1;
		break;

	case RSTATPROC_HAVEDISK:
		xdr_argument = xdr_void;
		xdr_result = xdr_u_int;
		local = (char *(*)()) rstatproc_havedisk_1;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}

