/* $Id: locker.h,v 1.5 2002-10-17 05:20:07 ghudson Exp $ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#define LOCKER_SUCCESS		0	/* Success */

/* Strictly internal errors */
#define LOCKER_EFILE		-1	/* from locker__read_line */
#define LOCKER_EOF		1	/* from locker__read_line */
#define LOCKER_ENOENT		2	/* No such file or directory. */

/* Exported errors */
#define LOCKER_EATTACHTAB	3	/* Error reading attachtab. */
#define LOCKER_EHESIOD		4	/* Unexpected Hesiod error. */
#define LOCKER_ENOMEM		5	/* Out of memory. */
#define LOCKER_EPARSE		6	/* Could not parse fs description. */
#define LOCKER_EPERM		7	/* Permission denied. */	
#define LOCKER_EUNKNOWN		8	/* Unknown locker. */

#define LOCKER_EALREADY		9	/* Locker is already attached. */
#define LOCKER_ENOTATTACHED	10	/* Locker is not attached. */

#define LOCKER_EATTACH		11	/* Could not attach locker. */
#define LOCKER_EATTACHCONF	12	/* Error reading attach.conf. */
#define LOCKER_EAUTH		13	/* Could not authenticate. */
#define LOCKER_EBADPATH		14	/* Unsafe path for mountpoint. */
#define LOCKER_EDETACH		15	/* Could not detach locker. */
#define LOCKER_EINUSE		16	/* Locker in use: not detached. */
#define LOCKER_EMOUNTPOINT	17	/* Couldn't build mountpoint. */
#define LOCKER_EMOUNTPOINTBUSY	18	/* Another locker is mounted there. */
#define LOCKER_EZEPHYR		19	/* Zephyr-related error. */

#define LOCKER_ATTACH_SUCCESS(stat) (stat == LOCKER_SUCCESS || stat == LOCKER_EALREADY)
#define LOCKER_DETACH_SUCCESS(stat) (stat == LOCKER_SUCCESS || stat == LOCKER_ENOTATTACHED)
#define LOCKER_LOOKUP_FAILURE(stat) (stat >= LOCKER_ENOENT && stat <= LOCKER_EUNKNOWN)

/* Global context */
typedef struct locker_context *locker_context;
typedef int (*locker_error_fun)(void *, char *, va_list);

struct locker_ops;

/* The attachtab directory and entries */

typedef struct locker_attachent {
  /* Data from Hesiod (or other source) */
  char *name, *mountpoint;
  struct locker_ops *fs;
  struct in_addr hostaddr;
  char *hostdir;
  int mode;

  /* Additional data kept in the attachtab file for attached lockers */
  int flags;
  int nowners;
  uid_t *owners;

  /* Is the locker attached? */
  int attached;
  /* If the mountpoint doesn't exist, where do we start building it from? */
  char *buildfrom;

  /* Filesystem state */
  FILE *mountpoint_file;
  int dirlockfd;

  /* Chaining for MUL lockers */
  struct locker_attachent *next;
} locker_attachent;

/* struct locker_ops contains the pointers to filesystem-specific code
 * and data. */
struct locker_ops {
  char *name;
  int flags;
  int (*parse)(locker_context context, char *name, char *desc,
	       char *mountpoint, locker_attachent **atp);
  int (*attach)(locker_context context, locker_attachent *at,
		char *mountoptions);
  int (*detach)(locker_context context, locker_attachent *at);
  int (*auth)(locker_context context, locker_attachent *at,
	      int mode, int op);
  int (*zsubs)(locker_context context, locker_attachent *at);

  /* Set by locker_init. */
  long id;
};

/* locker_ops flags */
#define LOCKER_FS_NEEDS_MOUNTDIR	(1 << 0)

/* Attachent flags */
#define LOCKER_FLAG_LOCKED		(1 << 0)
#define LOCKER_FLAG_KEEP		(1 << 1)
#define LOCKER_FLAG_NOSUID		(1 << 2)
#define LOCKER_FLAG_NAMEFILE		(1 << 3)

/* Attach / Detach options */
#define LOCKER_ATTACH_OPT_OVERRIDE		(1 << 0)
#define LOCKER_ATTACH_OPT_LOCK			(1 << 1)
#define LOCKER_ATTACH_OPT_ALLOW_SETUID		(1 << 2)
#define LOCKER_ATTACH_OPT_ZEPHYR		(1 << 3)
#define LOCKER_ATTACH_OPT_REAUTH		(1 << 4)
#define LOCKER_ATTACH_OPT_MASTER		(1 << 5)

#define LOCKER_ATTACH_DEFAULT_OPTIONS ( LOCKER_ATTACH_OPT_REAUTH | LOCKER_ATTACH_OPT_ZEPHYR )

#define LOCKER_DETACH_OPT_OVERRIDE		(1 << 0)
#define LOCKER_DETACH_OPT_UNLOCK		(1 << 1)
#define LOCKER_DETACH_OPT_UNZEPHYR		(1 << 2)
#define LOCKER_DETACH_OPT_UNAUTH		(1 << 3)
#define LOCKER_DETACH_OPT_OWNERCHECK		(1 << 4)
#define LOCKER_DETACH_OPT_CLEAN			(1 << 5)

#define LOCKER_DETACH_DEFAULT_OPTIONS ( LOCKER_DETACH_OPT_UNAUTH | LOCKER_DETACH_OPT_UNZEPHYR )

/* Authentication modes */
#define LOCKER_AUTH_DEFAULT 0
#define LOCKER_AUTH_NONE 'n'
#define LOCKER_AUTH_READONLY 'r'
#define LOCKER_AUTH_READWRITE 'w'
#define LOCKER_AUTH_MAYBE_READWRITE 'm'

/* Authentication ops. These numbers cannot be changed: they
 * correspond to the corresponding RPC mount call procedure numbers.
 */
enum { LOCKER_AUTH_AUTHENTICATE = 7, LOCKER_AUTH_UNAUTHENTICATE = 8,
       LOCKER_AUTH_PURGE = 9, LOCKER_AUTH_PURGEUSER = 10 };

/* Zephyr ops */
enum { LOCKER_ZEPHYR_SUBSCRIBE, LOCKER_ZEPHYR_UNSUBSCRIBE };


/* Callback function */
typedef int (*locker_callback)(locker_context, locker_attachent *, void *);


/* Context operations */
int locker_init(locker_context *context, uid_t user,
		locker_error_fun errfun, void *errdata);
void locker_end(locker_context context);

/* Attachtab operations */
int locker_read_attachent(locker_context context, char *name,
			  locker_attachent **atp);
int locker_iterate_attachtab(locker_context context,
			     locker_callback test, void *testarg,
			     locker_callback act, void *actarg);
void locker_free_attachent(locker_context context, locker_attachent *at);

int locker_check_owner(locker_context context, locker_attachent *at,
		       void *ownerp);
int locker_check_host(locker_context context, locker_attachent *at,
		       void *addrp);
int locker_convert_attachtab(locker_context context, char *oattachtab);

/* Attaching lockers */
int locker_attach(locker_context context, char *filesystem,
		  char *mountpoint, int auth, int options,
		  char *mountoptions, locker_attachent **atp);
int locker_attach_explicit(locker_context context, char *type,
			   char *desc, char *mountpoint, int auth, int options,
			   char *mountoptions, locker_attachent **atp);

int locker_attach_attachent(locker_context context, locker_attachent *at,
			    int auth, int options, char *mountoptions);

/* Detaching lockers */
int locker_detach(locker_context context, char *filesystem,
		  char *mountpoint, int options, locker_attachent **atp);
int locker_detach_explicit(locker_context context, char *type,
			   char *desc, char *mountpoint, int options,
			   locker_attachent **atp);

int locker_detach_attachent(locker_context context, locker_attachent *at,
			    int options);

/* Other locker ops */
int locker_auth(locker_context context, char *filesystem, int op);
int locker_auth_to_cell(locker_context context, char *name, char *cell,
			int op);
int locker_auth_to_host(locker_context context, char *name, char *host,
			int op);

int locker_zsubs(locker_context context, char *filesystem);

/* Lookup */
int locker_lookup_filsys(locker_context context, char *name, char ***descs,
			  void **cleanup);
void locker_free_filesys(locker_context context, char **descs, void *cleanup);

/* Zephyr */
int locker_do_zsubs(locker_context context, int op);
