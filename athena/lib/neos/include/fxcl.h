/***********************************************************************
 * to be included by all fx library clients
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/fxcl.h,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/fxcl.h,v 1.2 1993-02-03 12:50:04 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef _fxcl_h_
#define _fxcl_h_

#ifndef lint
static char rcsid_fxcl_h[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/fxcl.h,v 1.2 1993-02-03 12:50:04 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <rpc/rpc.h>
#include <com_err.h>
#include <fx_prot.h>
#include <fx-internal.h>  /* includes krb.h, fxserver_err.h */
#include <rpc_err.h>
#include <fxcl_err.h>
#include <krb_err.h>
#include <sys/param.h>
#include <time.h>

#define paper_clear(p) bzero((char *) p, sizeof(Paper))
#define paper_copy(a, b)  bcopy((char *) a, (char *) b, sizeof(Paper))
#define ENV_FXPATH "FXPATH"   /* environment variable of FX hosts */
#ifdef KERBEROS
#define FX_UNAMSZ (ANAME_SZ + REALM_SZ + 2)
#else /* KERBEROS */
#define FX_UNAMSZ (9)
#endif /* KERBEROS */
#define FX_DEF_SERVICE "turnin"
#define FX_DEF_FILENAME "x"
#define FX_DEF_DESC ""
#define FX_DEF_ASSIGNMENT 1
#define FX_DEF_TYPE EXCHANGE

typedef struct {
  char name[COURSE_NAME_LEN];
  char host[MAXHOSTNAMELEN];
  char owner[FX_UNAMSZ];
  char *extension;
  CLIENT *cl;
} FX;

#ifdef __STDC__
/* ANSI C -- use prototypes */

FX *fx_open(char *, long *);
stringlist fx_host_list();
void fx_host_list_destroy(stringlist);
extern void (*fx_open_error_hook)(FX *, long);
void fx_open_perror(FX *, long);
void fx_close(FX *);
long fx_acl_list(FX *, char *, stringlist_res **);
void fx_acl_list_destroy(stringlist_res **);
long fx_acl_add(FX *, char *, char *);
long fx_acl_del(FX *, char *, char *);
long fx_list(FX *, Paper *, Paperlist_res **);
void fx_list_destroy(Paperlist_res **);
long fx_send_file(FX *, Paper *, char *);
long fx_send(FX *, Paper *, FILE *);
long fx_retrieve_file(FX *, Paper *, char *);
long fx_retrieve(FX *, Paper *, FILE *);
long fx_move(FX *, Paper *, Paper *);
long fx_copy(FX *, Paper *, Paper *);
long fx_delete(FX *, Paper *);
long fx_init(FX *, init_res **);

/* Clients are discouraged from using the following routines */
long _fx_get_auth(FX *, KTEXT_ST *);
void _fx_shorten(FX *, char *);
void _fx_unshorten(char *);
char * _fx_lengthen(FX *, char *, char *);
long _fx_rpc_errno(CLIENT *);

#else /* __STDC__ */

FX *fx_open();
stringlist fx_host_list();
void fx_host_list_destroy();
extern void (*fx_open_error_hook)();
void fx_open_perror(), fx_close();
long fx_acl_list(), fx_acl_add(), fx_acl_del();
long fx_list(), fx_send(), fx_retrieve(), fx_move(), fx_copy(), fx_delete();
stringlist _fx_host_list();
void fx_host_list_destroy();
void fx_list_destroy(), fx_acl_list_destroy();
long fx_init();

/* Clients are discouraged from using the following routines */
long _fx_get_auth(), _fx_rpc_errno();
void _fx_shorten(), _fx_unshorten();
char *_fx_lengthen();

#endif /* __STDC__ */
#endif /* _fxcl_h_ */
