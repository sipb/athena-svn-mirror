/* 
 * $Id: aklog.h,v 1.2 1990-06-22 18:43:18 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/aklog/aklog.h,v $
 * $Author: qjb $
 *
 */

#ifndef __AKLOG_H__
#define __AKLOG_H__

#if !defined(lint) && !defined(SABER)
static char *rcsid_aklog_h = "$Id: aklog.h,v 1.2 1990-06-22 18:43:18 qjb Exp $";
#endif /* lint || SABER */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <krb.h>
#include "linked_list.h"

#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x)
#endif /* __STDC__ */

typedef struct {
    int (*readlink)ARGS((char *, char *, int));
    int (*lstat)ARGS((char *, struct stat *));
    int (*get_cred)ARGS((char *, char *, char *, CREDENTIALS *));
    void (*pstderr)ARGS((char *));
    void (*pstdout)ARGS((char *));
    void (*exitprog)ARGS((char));
} aklog_params;

void aklog ARGS((int, char *[], aklog_params *));
void aklog_init_params ARGS((aklog_params *));

#endif __AKLOG_H__
