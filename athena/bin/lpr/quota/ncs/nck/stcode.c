/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 * S T C O D E 
 *
 * Program to convert a hex status code to a print string.
 */

#include "nbase.h"
#include <stdio.h>

extern char *error_$c_text();

main(argc, argv)
int argc;
char *argv[];
{
    status_$t status;
    char buff[100];

    if (argc < 2) {
        fprintf(stderr, "(stcode) usage: stcode <hex status code>\n");
        exit(1); 
    }

    if (sscanf(argv[1], "%lx", &status.all) != 1) {
        fprintf(stderr, "(stcode) \"%s\" does not appear to be a valid hex status code\n", argv[1]);
        exit(1);
    }

    printf("%s\n", error_$c_text(status, buff, sizeof buff));
}
