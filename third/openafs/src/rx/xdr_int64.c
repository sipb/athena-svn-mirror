
/*
 *  XDR routine for int64 (long long or struct)
 */

#include <afsconfig.h>
#include <afs/param.h>

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/rx/xdr_int64.c,v 1.1.1.1 2002-01-31 21:50:18 zacheiss Exp $");

#if defined(KERNEL) && !defined(UKERNEL)
#ifdef AFS_LINUX20_ENV
#include "../h/string.h"
#if 0
#define bzero(A,C) memset((A), 0, (C))
#endif
#else
#include <sys/param.h>
#include <sys/systm.h>
#endif
#else
#include <stdio.h>
#endif
#include "xdr.h"
#if defined(KERNEL) && !defined(UKERNEL)
#ifdef        AFS_DEC_ENV
#include <afs/longc_procs.h>
#endif
#endif

#ifdef AFS_64BIT_ENV
/*
 * XDR afs_int64 integers
 */
bool_t
xdr_int64(xdrs, ulp)
        register XDR *xdrs;
        afs_int64 *ulp;
{
        static afs_int32 high;
        static afs_uint32 low;

        if (xdrs->x_op == XDR_DECODE) {
                if (!XDR_GETINT32(xdrs, (afs_int32 *) &high)) return (FALSE);
                if (!XDR_GETINT32(xdrs, (afs_int32 *) &low)) return (FALSE);
                *ulp = high;
                *ulp <<= 32;
                *ulp += low;
                return (TRUE);
        }
        if (xdrs->x_op == XDR_ENCODE) {
                high = (*ulp >> 32);
                low = *ulp & 0xFFFFFFFFL;
                if (!XDR_PUTINT32(xdrs, (afs_int32 *) &high)) return (FALSE);
                return (XDR_PUTINT32(xdrs, (afs_int32 *) &low));
        }
        if (xdrs->x_op == XDR_FREE)
                return (TRUE);
        return (FALSE);
}

/*
 * XDR afs_int64 integers
 */
bool_t
xdr_uint64(xdrs, ulp)
        register XDR *xdrs;
        afs_uint64 *ulp;
{
        static afs_uint32 high;
        static afs_uint32 low;

        if (xdrs->x_op == XDR_DECODE) {
                if (!XDR_GETINT32(xdrs, (afs_uint32 *) &high)) return (FALSE);
                if (!XDR_GETINT32(xdrs, (afs_uint32 *) &low)) return (FALSE);
                *ulp = high;
                *ulp <<= 32;
                *ulp += low;
                return (TRUE);
        }
        if (xdrs->x_op == XDR_ENCODE) {
                high = (*ulp >> 32);
                low = *ulp & 0xFFFFFFFFL;
                if (!XDR_PUTINT32(xdrs, (afs_uint32 *) &high)) return (FALSE);
                return (XDR_PUTINT32(xdrs, (afs_uint32 *) &low));
        }
        if (xdrs->x_op == XDR_FREE)
                return (TRUE);
        return (FALSE);
}
#else /* AFS_64BIT_ENV */
/*
 * XDR afs_int64 integers
 */
bool_t
xdr_int64(xdrs, ulp)
        register XDR *xdrs;
        afs_int64 *ulp;
{
        if (xdrs->x_op == XDR_DECODE) {
                if (!XDR_GETINT32(xdrs, (afs_int32 *) &ulp->high)) return (FALSE);
                return (XDR_GETINT32(xdrs, (afs_int32 *) &ulp->low));
        }
        if (xdrs->x_op == XDR_ENCODE) {
                if (!XDR_PUTINT32(xdrs, (afs_int32 *) &ulp->high)) return (FALSE);
                return (XDR_PUTINT32(xdrs, (afs_int32 *) &ulp->low));
        }
        if (xdrs->x_op == XDR_FREE)
                return (TRUE);
        return (FALSE);
}

/*
 * XDR afs_uint64 integers
 */
bool_t
xdr_uint64(xdrs, ulp)
        register XDR *xdrs;
        afs_uint64 *ulp;
{
        if (xdrs->x_op == XDR_DECODE) {
                if (!XDR_GETINT32(xdrs, (afs_uint32 *) &ulp->high)) return (FALSE);
                return (XDR_GETINT32(xdrs, (afs_uint32 *) &ulp->low));
        }
        if (xdrs->x_op == XDR_ENCODE) {
                if (!XDR_PUTINT32(xdrs, (afs_uint32 *) &ulp->high)) return (FALSE);
                return (XDR_PUTINT32(xdrs, (afs_uint32 *) &ulp->low));
        }
        if (xdrs->x_op == XDR_FREE)
                return (TRUE);
        return (FALSE);
}
#endif /* AFS_64BIT_ENV */