#include <stdio.h>
#include <sys/param.h>
#include <rpc/rpc.h>
#include <ufs/quota.h>
#include <rpcsvc/rquota2.h>

bool_t
xdr_getquota2_rslt(xdrs, gq_rsltp)
	XDR *xdrs;
	struct getquota2_rslt *gq_rsltp;
{
  int i;
  if (xdr_enum(xdrs, &gq_rsltp->gqr_status) &&
      xdr_bool(xdrs, &gq_rsltp->rq_group) &&
      xdr_int(xdrs, &gq_rsltp->rq_ngrps) &&
      xdr_int(xdrs, &gq_rsltp->rq_bsize) &&
      xdr_rquota2(xdrs, &gq_rsltp->gqr_zm))
    {
      for(i=0;i<NGROUPS+1;i++)
	if (!xdr_rquota2(xdrs, &gq_rsltp->gqr_rquota[i]))
	  break;
      if (i!=NGROUPS+1) return(FALSE);
      return(TRUE);
    }
  else return(FALSE);
}

bool_t
xdr_rquota2(xdrs, rqp)
	XDR *xdrs;
	struct rquota2 *rqp;
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
