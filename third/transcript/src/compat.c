/* compat.c
 *
 * Copyright (C) 1991,1992 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/compat.c,v 1.1.1.1 1996-10-07 20:25:48 ghudson Exp $
 *
 */

#ifndef XPG3
#define P_tmpdir "/usr/tmp/"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

char *tempnam(dir, pfx)
    char *dir, *pfx;
{
    char template[12];
    int cnt;
    char *envdir;
    char *temp;
    char *name;
    struct stat sbuf;

    if (strlen(pfx) > 5) return NULL;
    strncpy(template, pfx, 12);
    strncat(template, "XXXXXX", 12);
    temp = (char *)mktemp(template);
    envdir = (char *) getenv("TMPDIR");
    if (envdir) {
	/* this overrides everything */
	cnt = strlen(temp) + strlen(envdir);
	name = (char *) malloc(cnt+1);
	if (name == NULL) return NULL;
	strncpy(name, envdir, cnt);
	strncat(name, temp, cnt);
	return name;
    }
    if (dir == NULL)
	dir = P_tmpdir;
    cnt = strlen(temp) + strlen(dir);
    name = (char *) malloc(cnt+1);
    if (name == NULL) return NULL;
    strncpy(name, dir, cnt);
    strncat(name, temp, cnt);
    return name;
}
#endif /* XPG3 */
