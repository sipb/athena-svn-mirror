#include <rpc/rpc.h>
#include "rstat.h"


bool_t
xdr_rstat_timeval(xdrs, objp)
	XDR *xdrs;
	rstat_timeval *objp;
{
	if (!xdr_u_int(xdrs, &objp->tv_sec)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->tv_usec)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_statstime(xdrs, objp)
	XDR *xdrs;
	statstime *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->cp_time, CPUSTATES, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dk_xfer, DK_NDRIVE, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_intr)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ipackets)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ierrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_oerrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_collisions)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_swtch)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->avenrun, 3, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_rstat_timeval(xdrs, &objp->boottime)) {
		return (FALSE);
	}
	if (!xdr_rstat_timeval(xdrs, &objp->curtime)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_opackets)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_statsswtch(xdrs, objp)
	XDR *xdrs;
	statsswtch *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->cp_time, CPUSTATES, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dk_xfer, DK_NDRIVE, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_intr)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ipackets)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ierrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_oerrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_collisions)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_swtch)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->avenrun, 3, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_rstat_timeval(xdrs, &objp->boottime)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_opackets)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_stats(xdrs, objp)
	XDR *xdrs;
	stats *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->cp_time, CPUSTATES, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dk_xfer, DK_NDRIVE, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pgpgout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpin)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_pswpout)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->v_intr)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ipackets)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_ierrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_oerrors)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_collisions)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->if_opackets)) {
		return (FALSE);
	}
	return (TRUE);
}


