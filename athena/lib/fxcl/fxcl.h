/***********************************************************************
 * to be included by all fx library clients
 *
 * $Id: fxcl.h,v 1.1 1999-09-28 22:07:22 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef _fxcl_h_
#define _fxcl_h_

/* Needed to get correct prototypes from fx_prot.h under Irix. */
#define _RPCGEN_CLNT

#include <stdio.h>
#include <rpc/rpc.h>
#include <com_err.h>
#include <fx/fx_prot.h>
#include <fx/fx-internal.h>
#include <fx/rpc_err.h>
#include <fx/fxcl_err.h>
#include <fx/krb_err.h>
#include <sys/param.h>
#include <time.h>
#include <netdb.h>
#include <string.h>

#define paper_clear(p) memset((char *) p, 0, sizeof(Paper))
#define paper_copy(a, b)  memcpy(b, a, sizeof(Paper))
#define ENV_FXPATH "FXPATH"   /* environment variable of FX hosts */
#define FX_UNAMSZ 100	/* must be ANAME_SZ + REALM_SZ + 2 or larger */
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
char *full_name(char *);

/* Clients are discouraged from using the following routines */
long _fx_get_auth(FX *, KTEXT_ST *);
void _fx_shorten(FX *, char *);
void _fx_unshorten(char *);
char * _fx_lengthen(FX *, char *, char *);
long _fx_rpc_errno(CLIENT *);

#endif /* _fxcl_h_ */
