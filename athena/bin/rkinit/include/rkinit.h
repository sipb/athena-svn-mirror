/* 
 * $Id: rkinit.h,v 1.4.4.1 2000-02-04 22:50:10 ghudson Exp $
 *
 * Main header file for rkinit library users
 */

#ifndef __RKINIT_H__
#define __RKINIT_H__

#if !defined(lint) && !defined(SABER) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_rkinit_h = "$Id: rkinit.h,v 1.4.4.1 2000-02-04 22:50:10 ghudson Exp $";
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
    char tktfilename[1025];
    long lifetime;
} rkinit_info;

#define RKINIT_SUCCESS 0

/* Function declarations */
extern int rkinit RK_PROTO((char *, char *, rkinit_info *, int));
extern char *rkinit_errmsg RK_PROTO((char *));

#endif /* __RKINIT_H__ */
