/* 
 * $Id: aklog.h,v 1.7 1994-04-14 18:45:35 probe Exp $
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology
 * For distribution and copying rights, see the file "mit-copyright.h"
 */

#ifndef __AKLOG_H__
#define __AKLOG_H__

#if !defined(lint) && !defined(SABER)
static char *rcsid_aklog_h = "$Id: aklog.h,v 1.7 1994-04-14 18:45:35 probe Exp $";
#endif /* lint || SABER */

#include <afs/param.h>

#if !defined(vax)
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#endif

#include <sys/types.h>
#include <krb.h>
#include "linked_list.h"

#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif /* __STDC__ */

typedef struct {
    int (*readlink)ARGS((char *, char *, int));
    int (*isdir)ARGS((char *, unsigned char *));
    char *(*getcwd)ARGS((char *, size_t));
    int (*get_cred)ARGS((char *, char *, char *, CREDENTIALS *));
    int (*get_user_realm)ARGS((char *));
    void (*pstderr)ARGS((char *));
    void (*pstdout)ARGS((char *));
    void (*exitprog)ARGS((char));
} aklog_params;

void aklog ARGS((int, char *[], aklog_params *));
void aklog_init_params ARGS((aklog_params *));

#endif __AKLOG_H__
