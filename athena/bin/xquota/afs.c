/*
 * Copyright (c) 1991 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * For re-distribution and warranty information, see <mit-copyright.h>
 *
 * 
 * $Id: afs.c,v 1.1 1991-06-25 11:33:38 probe Exp $
 */

#include <sys/types.h>
#include <sys/ioctl.h>

#include <afs/param.h>
#include <afs/venus.h>
#include <afs/afsint.h>

#include <ufs/quota.h>

#include "xquota.h"

#define MAXNAME 100
#define	MAXSIZE	2048

static char space[MAXSIZE];

/* ARGSUSED */
int
getafsquota(host, path, uid, dqp)
	char *host, *path;
	int uid;
	struct dqblk *dqp;
{
    int code;
    struct ViceIoctl blob;
    struct VolumeStatus *status;
    char *name;
    
    bzero((char *)dqp, sizeof(*dqp));

    blob.out_size = MAXSIZE;
    blob.in_size = 0;
    blob.out = space;
    code = pioctl(path, VIOCGETVOLSTAT, &blob, 1);
    if (code)
	return(QUOTA_ERROR);
    status = (VolumeStatus *)space;
    name = (char *)status + sizeof(*status);
    if (status->MaxQuota == 0) {
	/* no quota */
	return(QUOTA_OK);
    }
    dqp->dqb_bsoftlimit = status->MaxQuota*2;
    dqp->dqb_bhardlimit = status->MaxQuota*2;
    dqp->dqb_curblocks = status->BlocksInUse*2;
    return(QUOTA_OK);
}
