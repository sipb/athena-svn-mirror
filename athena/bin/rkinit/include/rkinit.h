/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit.h,v 1.2 1989-11-13 14:45:27 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit.h,v $
 * $Author: qjb $
 *
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid_rkinit_h = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit.h,v 1.2 1989-11-13 14:45:27 qjb Exp $";
#endif lint || SABER

#include <krb.h>
#include <sys/param.h>

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

/* Declaration of routines */
extern int rkinit();
extern char *rkinit_errmsg();
