/* 
 * $Id: rkinit.h,v 1.3 1990-07-16 14:14:29 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit.h,v $
 * $Author: qjb $
 *
 * Main header file for rkinit library users
 */

#ifndef __RKINIT_H__
#define __RKINIT_H__

#if !defined(lint) && !defined(SABER) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_rkinit_h = "$Id: rkinit.h,v 1.3 1990-07-16 14:14:29 qjb Exp $";
#endif /* lint || SABER || LOCORE || RCS_HDRS */

#include <krb.h>
#include <sys/param.h>

#ifdef __STDC__
#define RK_PROTO(x) x
#else
#define RK_PROTO(x) ()
#endif /* __STDC__ */

typedef struct {
    char aname[ANAME_SZ + 1];
    char inst[INST_SZ + 1];
    char realm[REALM_SZ + 1];
    char sname[ANAME_SZ + 1];
    char sinst[INST_SZ + 1];
    char username[9];		/* max local name length + 1 */
    char tktfilename[MAXPATHLEN + 1];
    long lifetime;
} rkinit_info;

#define RKINIT_SUCCESS 0

/* Function declarations */
extern int rkinit RK_PROTO((char *, char *, rkinit_info *, int));
extern char *rkinit_errmsg RK_PROTO((char *));

#endif /* __RKINIT_H__ */
