/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

#include <afsconfig.h>
#include <afs/param.h>

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/util/test/dtest.c,v 1.1.1.1 2002-01-31 21:49:32 zacheiss Exp $");

#include "ktime.h"

main(argc, argv)
int argc;
char **argv; {
    long code, temp;

    if (argc <= 1) {
	printf("dtest: usage is 'dtest <time to interpret>'\n");
	exit(0);
    }

    code = ktime_DateToLong(argv[1], &temp);
    if (code) {
	printf("date parse failed with code %d.\n", code);
    }
    else {
	printf("returned %d, which, run through ctime, yields %s", temp, ctime(&temp));
    }
    exit(0);
}