/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-cpyright.h>.
 *
 * Miscellaneous debug printing utilities
 */

#include <afsconfig.h>
#include <afs/param.h>

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/des/util.c,v 1.1.1.1 2002-01-31 21:49:00 zacheiss Exp $");

#include <mit-cpyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <des.h>

int des_cblock_print_file(x, fp)
    des_cblock *x;
    FILE *fp;
{
    unsigned char *y = (unsigned char *) x;
    register int i = 0;
    fprintf(fp," 0x { ");

    while (i++ < 8) {
	fprintf(fp,"%x",*y++);
	if (i < 8)
	    fprintf(fp,", ");
    }
    fprintf(fp," }");

	return(0);
}

#ifdef DEBUG
int des_debug_print(area, x, arg1, arg2)
    char *area;
    int x;
    char *arg1;
    char *arg2;
{
    ;
}
#endif
