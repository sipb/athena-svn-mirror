#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#if defined(_AIX)
#include <sys/select.h>
#endif
#if !defined(ultrix) && !defined(_AIX) && !defined(SOLARIS)
#include <ufs/quota.h>
#endif
#include <rpc/rpc.h>
#include <rpcsvc/rcquota.h>

bool_t
xdr_getcquota_args(xdrs, gq_argsp)
	XDR *xdrs;
	struct getcquota_args *gq_argsp;
{
	extern bool_t xdr_path();

	return (xdr_path(xdrs, &gq_argsp->gqa_pathp) &&
	    xdr_int(xdrs, &gq_argsp->gqa_uid));
}


bool_t
xdr_getcquota_rslt(xdrs, gq_rsltp)
	XDR *xdrs;
	struct getcquota_rslt *gq_rsltp;
{
  int i;
  if (xdr_enum(xdrs, &gq_rsltp->gqr_status) &&
      xdr_bool(xdrs, &gq_rsltp->rq_group) &&
      xdr_int(xdrs, &gq_rsltp->rq_ngrps) &&
      xdr_int(xdrs, &gq_rsltp->rq_bsize) &&
      xdr_rcquota(xdrs, &gq_rsltp->gqr_zm))
    {
      for(i=0;i<NGROUPS+1;i++)
	if (!xdr_rcquota(xdrs, &gq_rsltp->gqr_rcquota[i]))
	  break;
      if (i!=NGROUPS+1) return(FALSE);
      return(TRUE);
    }
  else return(FALSE);
}

bool_t
xdr_rcquota(xdrs, rqp)
	XDR *xdrs;
	struct rcquota *rqp;
{

  return (xdr_int(xdrs, &rqp->rq_id) &&
	  xdr_u_long(xdrs, &rqp->rq_bhardlimit) &&
	  xdr_u_long(xdrs, &rqp->rq_bsoftlimit) &&
	  xdr_u_long(xdrs, &rqp->rq_curblocks) &&
	  xdr_u_long(xdrs, &rqp->rq_fhardlimit) &&
	  xdr_u_long(xdrs, &rqp->rq_fsoftlimit) &&
	  xdr_u_long(xdrs, &rqp->rq_curfiles) &&
	  xdr_u_long(xdrs, &rqp->rq_btimeleft) &&
	  xdr_u_long(xdrs, &rqp->rq_ftimeleft) );
}
