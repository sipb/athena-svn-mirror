/*
 * Copyright 1987 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file
 * "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/finger/hespwnam.c,v $
 *	$Author: ambar $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/finger/hespwnam.c,v 1.1 1987-08-21 18:20:40 ambar Exp $
 *	$Log: not supported by cvs2svn $
 * 
 */

#ifndef lint
static char *rcsid_hespwnam_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/finger/hespwnam.c,v 1.1 1987-08-21 18:20:40 ambar Exp $";
#endif lint

#include <stdio.h>
#include <pwd.h>
#include "mit-copyright.h"

static struct passwd passwd;
static char buf[256];

char *strcpy();

struct passwd *
hesgetpwnam(nam)
    char *nam;
{
    struct passwd *pwcommon();
    char **pp, **hes_resolve();

    pp = hes_resolve(nam, "passwd");
    return pwcommon(pp);
}

static
struct passwd *
pwcommon(pp)
    char **pp;
{
    register char *p;
    char *pwskip();

    if (pp == NULL)
	return (NULL);
    /* choose only the first response (only 1 expected) */
    (void) strcpy(buf, pp[0]);
    while (*pp)
	free(*pp++);		/* necessary to avoid leaks */
    p = buf;
    passwd.pw_name = p;
    p = pwskip(p);
    passwd.pw_passwd = p;
    p = pwskip(p);
    passwd.pw_uid = atoi(p);
    p = pwskip(p);
    passwd.pw_gid = atoi(p);
    passwd.pw_quota = 0;
    passwd.pw_comment = "";
    p = pwskip(p);
    passwd.pw_gecos = p;
    p = pwskip(p);
    passwd.pw_dir = p;
    p = pwskip(p);
    passwd.pw_shell = p;
    while (*p && *p != '\n')
	p++;
    *p = '\0';
    return (&passwd);
}

static char *
pwskip(p)
    register char *p;
{
    while (*p && *p != ':' && *p != '\n')
	++p;
    if (*p)
	*p++ = 0;
    return (p);
}
