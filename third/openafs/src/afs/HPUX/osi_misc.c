/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

/*
 * Implements:
 * afs_suser
 */

#include <afsconfig.h>
#include "../afs/param.h"

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/afs/HPUX/osi_misc.c,v 1.1.1.1 2002-01-31 21:48:50 zacheiss Exp $");

#include "../afs/sysincludes.h"	/* Standard vendor system headers */
#include "../afs/afsincludes.h"	/* Afs-based standard headers */

/*
 * afs_suser() returns true if the caller is superuser, false otherwise.
 *
 * Note that it must NOT set errno.
 *
 * Here we have to save and restore errno since the HP-UX suser() sets errno.
 */

afs_suser() {
    int save_errno;
    int code;

    save_errno = u.u_error;
    code = suser();
    u.u_error = save_errno;

    return code;
}