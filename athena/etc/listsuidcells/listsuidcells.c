/* $Id: listsuidcells.c,v 1.1 1996-03-25 18:12:24 ghudson Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/etc/listsuidcells/listsuidcells.c,v $
 * Created by Greg Hudson.
 * Copyright (c) 1996 by the Massachusetts Institute of Technology.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/filio.h>
#include <netinet/in.h>
#include <afs/param.h>
#include <afs/afs.h>
#include <afs/venus.h>

static void try_cell(char *name);

int main(int argc, char **argv)
{
    int32 i;
    char out[2048];
    struct ViceIoctl ioc;
    
    for (i = 0; i < 1000; i++) {
	ioc.in = (caddr_t) &i;
	ioc.in_size = sizeof(i);
	ioc.out = (caddr_t) out;
	ioc.out_size = sizeof(out);
	if (pioctl(0, VIOCGETCELL, &ioc, 1) < 0)
	    exit((errno == EDOM) ? 0 : 1);
	try_cell(space + OMAXHOSTS * sizeof(int32));
    }
    return 0;
}

static void try_cell(char *name)
{
    register int32 code;
    struct ViceIoctl ioc;
    int32 out[2];
    
    ioc.in = (caddr_t) name;
    ioc.in_size = strlen(name) + 1;
    ioc.out = (caddr_t) out;
    ioc.out_size = sizeof(out);
    pioctl(0, VIOC_GETCELLSTATUS, &ioc, 1);
    if (!(out[0] & 2))
	printf("%s\n", name);
}

