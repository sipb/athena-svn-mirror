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

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/kauth/test/background.c,v 1.1.1.1 2002-01-31 21:49:04 zacheiss Exp $");

main (argc, argv)
  int   argc;
  char *argv[];
{   int pid;

    if (argc == 1) exit (-1);
    if (pid = fork ()) {		/* parent */
	printf ("%d", pid);
	exit (0);
    }
    execve (argv[1], argv+1, 0);
    perror ("execve returned");
}
