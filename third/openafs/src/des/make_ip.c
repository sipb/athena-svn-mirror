/*
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-cpyright.h>.
 *
 * This routine generates source code implementing the initial
 * permutation of the DES.
 */

#include <afsconfig.h>
#include <afs/param.h>

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/des/make_ip.c,v 1.1.1.1 2002-01-31 21:31:50 zacheiss Exp $");

#include <mit-cpyright.h>
#include <stdio.h>
#include <stdlib.h>
#include "des_internal.h"

#define WANT_IP_TABLE
#include "tables.h"

extern afs_int32 swap_bit_pos_0();
extern afs_int32 rev_swap_bit_pos_0();
extern void test_set PROTOTYPE((FILE *, char const *, int,
                char const *, int));
extern int swap_long_bytes_bit_number(int);
extern int swap_bit_pos_0_to_ansi(int);

#define SWAP(x) swap_long_bytes_bit_number(swap_bit_pos_0_to_ansi(x))

void gen(stream)
    FILE *stream;
{
    register int i;

    /* clear the output */
    fprintf(stream,"    L2 = 0; R2 = 0;\n");

    /* first setup IP */
    fprintf(stream,"/* IP operations */\n/* first left to left */\n");

    /* first list mapping from left to left */
    for (i = 0; i <= 31; i++)
        if (IP[i] < 32)
            test_set(stream, "L1", SWAP(IP[i]), "L2", i);

    /* now mapping from right to left */
    fprintf(stream,"\n/* now from right to left */\n");
    for (i = 0; i <= 31; i++)
        if (IP[i] >= 32)
            test_set(stream, "R1", SWAP(IP[i]-32), "L2", i);

    fprintf(stream,"\n/* now from left to right */\n");
    /*  list mapping from left to right */
    for (i = 32; i <= 63; i++)
        if (IP[i] <32)
            test_set(stream, "L1", SWAP(IP[i]), "R2", i-32);

    /* now mapping from right to right */
    fprintf(stream,"\n/* last from right to right */\n");
    for (i = 32; i <= 63; i++)
        if (IP[i] >= 32)
            test_set(stream, "R1", SWAP(IP[i]-32), "R2", i-32);
    exit(0);
}