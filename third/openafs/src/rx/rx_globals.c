/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

/* RX:  Globals for internal use, basically */

#include <afsconfig.h>
#ifdef KERNEL
#include "../afs/param.h"
#else
#include <afs/param.h>
#endif

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/rx/rx_globals.c,v 1.1.1.1 2002-01-31 21:49:19 zacheiss Exp $");

/* Enable data initialization when the header file is included */
#define INIT(stuff) = stuff
#if defined(AFS_NT40_ENV) && defined(AFS_PTHREAD_ENV)
#define EXT __declspec(dllexport)
#else
#define EXT
#endif

#ifdef KERNEL
#ifndef UKERNEL
#include "../h/types.h"
#else /* !UKERNEL */
#include	"../afs/sysincludes.h"
#endif /* UKERNEL */
#endif /* KERNEL */

#include "rx_globals.h"