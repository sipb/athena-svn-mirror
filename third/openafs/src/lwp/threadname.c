/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 *
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */ 
/* ********************************************************************** */
/*                                                                        */
/*  trheadname.c                                                          */
/*                                                                        */
/*  Author: Hartmut Reuter                                                */
/*  reuter@rzg.mpg.de                                                     */
/*  Date: 01/12/00                                                        */
/*                                                                        */
/*  Function    - These routiens implement thread names for the           */
/*                logging from the servers                                */
/*                                                                        */
/* ********************************************************************** */

#include <afsconfig.h>
#include <afs/param.h>

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/lwp/threadname.c,v 1.1.1.1 2002-01-31 21:49:46 zacheiss Exp $");

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#if defined(AFS_PTHREAD_ENV)
#include <pthread.h>
#else /* defined(AFS_PTHREAD_ENV) */
#include "lwp.h"
#endif /* defined(AFS_PTHREAD_ENV) */

#define MAX_THREADS 128
#define MAXTHREADNAMELENGTH 64
int nThreads = 0;
#if defined(AFS_PTHREAD_ENV)
pthread_t ThreadId[MAX_THREADS];
#else /* defined(AFS_PTHREAD_ENV) */
PROCESS   ThreadId[MAX_THREADS];
#endif /* defined(AFS_PTHREAD_ENV) */
char      ThreadName[MAX_THREADS][MAXTHREADNAMELENGTH];

char * threadname ()
{
    int i;
    static char MainThread[] = "main";
    char *ptr;
    char *p;
#ifdef AFS_PTHREAD_ENV
    pthread_t me;
#else /* AFS_PTHREAD_ENV */
    PROCESS me;
#endif /* AFS_PTHREAD_ENV */

#ifdef AFS_PTHREAD_ENV
    me =  pthread_self();
#else /* AFS_PTHREAD_ENV */
    me =  (PROCESS) LWP_ThreadId();
#endif /* AFS_PTHREAD_ENV */
    ptr = (char *) &MainThread; 
    for (i = 0; i < nThreads; i++) {
       if (ThreadId[i] == me) {
           ptr = (char *)&ThreadName[i];
           break;
       }
 }
    p = ptr;
    return p;
}

int registerthread(id, name)
#ifdef AFS_PTHREAD_ENV
    pthread_t id;
#else /* AFS_PTHREAD_ENV */
    PROCESS id;
#endif /* AFS_PTHREAD_ENV */
    char *name;
{
    int i;

    for (i = 0; i < nThreads; i++) {
       if (ThreadId[i] == id) {
           strncpy(&ThreadName[i][0], name, MAXTHREADNAMELENGTH);
           return 0;
       }
    }
    if (nThreads == MAX_THREADS) return 0;
    ThreadId[nThreads] = id;
    strncpy(&ThreadName[nThreads][0], name, MAXTHREADNAMELENGTH);
    ThreadName[nThreads][MAXTHREADNAMELENGTH -1] =0;
    nThreads++;

    return 0;
}

int swapthreadname(id, new, old)
#ifdef AFS_PTHREAD_ENV
    pthread_t id;
#else /* AFS_PTHREAD_ENV */
    PROCESS id;
#endif /* AFS_PTHREAD_ENV */
    char *new;
    char *old;
{
    int i;

    for (i = 0; i < nThreads; i++) {
        if (ThreadId[i] == id) {
           if (old)
                strncpy(old, &ThreadName[i][0], MAXTHREADNAMELENGTH);
            strncpy(&ThreadName[i][0], new, MAXTHREADNAMELENGTH);
            return 0;
	}
    }
    return 1;
}