/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

/* 
 * osi_misc.c
 *
 * Implements:
 * afs_suser
 */

#include <afsconfig.h>
#include "../afs/param.h"

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/afs/FBSD/osi_misc.c,v 1.1.1.1 2002-01-31 21:48:49 zacheiss Exp $");

#include "../afs/sysincludes.h"	/* Standard vendor system headers */
#include "../afs/afsincludes.h"	/* Afs-based standard headers */

/*
 * afs_suser() returns true if the caller is superuser, false otherwise.
 *
 * Note that it must NOT set errno.
 */

afs_suser() {
    int error;

    if (suser(curproc) == 0) {
	return(1);
    }
    return(0);
}
