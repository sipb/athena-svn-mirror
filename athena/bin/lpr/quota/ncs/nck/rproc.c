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
 * R P R O C
 */

#ifdef DSEE
#   include "$(nbase.idl).h"
#   include "$(rproc.idl).h"
#else
#   include "nbase.h"
#   include "rproc.h"
#endif

#include "std.h"

#include <pwd.h>
#include <grp.h>
#include <sys/file.h>

#ifdef apollo
#  define ANONYMOUS_USER  "user"
#  define ANONYMOUS_GROUP "none"
#else
#  define ANONYMOUS_USER  "nobody"   
#  define ANONYMOUS_GROUP "other"
#endif

#define RPROC_OK_FILE "/etc/ncs/rproc_ok"

#define MAX_ARGS 128

/*
 * R P R O C _ $ C R E A T E _ S I M P L E
 *
 * Create a process.
 */

void rproc_$create_simple(h, pname, argc, argv, proc, st)
handle_t h;
ndr_$char *pname;
int argc;
rproc_$args_t argv;
int *proc;
status_$t *st;
{
    char *largv[MAX_ARGS];
    u_short i;
    static int uid = -1;
    static int gid = -1;
    int junk;
    int pid;

    /*
     * Allow processes to be created iff the RPROC_OK_FILE file exists.
     */

    if (access(RPROC_OK_FILE, F_OK) != 0) {
        st->all = rproc_$not_allowed;
        return;
    }

    /*
     * Check to see if we can hack the specified number of arguments.  Copy them
     * over to our own vector so we can null-terminate the list.
     */

    if (argc > MAX_ARGS) {
        st->all = rproc_$too_many_args;
        return;
    }

    for (i = 0; i < argc; i++)
        largv[i] = (char *) argv[i];

    largv[argc] = NULL;

    /*
     * Get the "anonymous" user and group to run the new process as.  pwd/grp
     * are static so we only have to do the lookups once.
     */

    if (uid == -1) {
        struct passwd *pwd = getpwnam(ANONYMOUS_USER);

        if (pwd == NULL) {
            st->all = rproc_$internal_error;
            return;
        }
        uid = pwd->pw_uid;
    }

    if (gid == -1) {
        struct group *grp = getgrnam(ANONYMOUS_GROUP);

        if (grp == NULL) {
            st->all = rproc_$internal_error;
            return;
        }
        gid = grp->gr_gid;
    }

    st->all = status_$ok;

    /*
     * Create new process.  On BSD systems we use vfork(2) instead of fork(2).
     * Since the parent will block until the child execs or exits and since the
     * parent and child are sharing "stacks", the child can set the return status
     * so the remote client can have some idea whether things worked.  Systems
     * without vfork are less functional.  Also, even of vfork systems, we can't
     * really return the execv status because we must fork twice so that the
     * new process is an orphan.  On non-vfork systems, we can't return the 
     * pid of the real process (grand-child).
     */

#ifdef BSD
    pid = vfork();
#else
    *proc = 0;
    pid = fork();
#endif

    if (pid < 0) {
        perror("1st fork failed");
        st->all = rproc_$cant_create_proc;
        return;
    }

    if (pid == 0) {
        if (setgid(gid) != 0) {
            fprintf(stderr, "Can't setgid to %d (errno=%d)\n", gid, errno);
            st->all = rproc_$cant_set_id;
            exit(2);
        }

#ifdef BSD
        if (setgroups(0, &junk) != 0) {
            fprintf(stderr, "Can't setgroups to empty list (errno=%d)\n");
            st->all = rproc_$cant_set_id;
            exit(2);
        }
#endif

        if (setuid(uid) != 0) {
            fprintf(stderr, "Can't setuid to %d (errno=%d)\n", uid, errno);
            st->all = rproc_$cant_set_id;
            exit(2);
        }

        if (access(pname, X_OK) != 0) {
            st->all = rproc_$cant_run_prog;
            exit(2);
        }

        *proc = fork();

        if (*proc < 0) {
            perror("2nd fork failed");
            st->all = rproc_$cant_create_proc;
            return;
        }

        if (*proc > 0)
            exit(0);

        execv(pname, largv);
        fprintf(stderr, "execv of \"%s\" failed (errno=%d)\n", pname, errno);

        exit(2);
    }
}
