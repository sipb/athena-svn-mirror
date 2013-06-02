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

/* This file is part of liblocker. It implements NFS lockers. */

static const char rcsid[] = "$Id: nfs.c,v 1.5 2006-07-25 23:29:09 ghudson Exp $";

#ifdef ENABLE_NFS

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rpc/rpc.h>
#ifdef NEED_SYS_FS_NFS_H
#include <sys/fs/nfs.h>
#endif
#include <rpcsvc/mount.h>

#include <krb.h>

#endif /* ENABLE_NFS */

#include "locker.h"
#include "locker_private.h"

#ifdef ENABLE_NFS
static bool_t xdr_krbtkt(XDR *xdrs, KTEXT authp);
#endif /* ENABLE_NFS */

static int nfs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **at);
static int nfs_auth(locker_context context, locker_attachent *at,
		    int mode, int op);
static int nfs_zsubs(locker_context context, locker_attachent *at);

struct locker_ops locker__nfs_ops = {
  "NFS",
  LOCKER_FS_NEEDS_MOUNTDIR,
  nfs_parse,
  locker__mount,
  locker__unmount,
  nfs_auth,
  nfs_zsubs
};

static int nfs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp)
{
#ifdef ENABLE_NFS
  locker_attachent *at;
  struct hostent *h;
  char *dup;
  char *p, *host, *dir, *lasts = NULL;
  int status;

  at = locker__new_attachent(context, &locker__nfs_ops);
  if (!at)
    return LOCKER_ENOMEM;

  dup = strdup(desc);
  if (!dup)
    goto mem_error;

  if (!name)
    {
      /* It's an explicit description: host:path. */

      at->name = strdup(desc);
      if (!at->name)
	goto mem_error;

      if (!(p = strchr(dup, ':')))
	{
	  locker__error(context, "%s: No \":\" in description.\n", desc);
	  status = LOCKER_EPARSE;
	  goto cleanup;
	}

      *p = '\0';
      host = dup;
      dir = p + 1;
      at->mode = LOCKER_AUTH_READWRITE;
    }
  else
    {
      /* A hesiod NFS description looks like:
       * NFS /u1/bitbucket JASON.MIT.EDU w /mit/bitbucket
       */

      at->name = strdup(name);
      if (!at->name)
	goto mem_error;

      /* Skip "NFS". */
      if (!strtok_r(dup, " ", &lasts))
	goto parse_error;

      /* Directory on remote host */
      dir = strtok_r(NULL, " ", &lasts);
      if (!dir)
	goto parse_error;

      /* Remote host */
      host = strtok_r(NULL, " ", &lasts);
      if (!host)
	goto parse_error;

      /* Mount mode */
      p = strtok_r(NULL, " ", &lasts);
      if (!p || *(p + 1))
	goto parse_error;
      switch (*p)
	{
	case 'r':
	  at->mode = LOCKER_AUTH_READONLY;
	  break;
	case 'm':
	  at->mode = LOCKER_AUTH_MAYBE_READWRITE;
	  break;
	case 'w':
	  at->mode = LOCKER_AUTH_READWRITE;
	  break;
	case 'n':
	  at->mode = LOCKER_AUTH_NONE;
	  break;
	default:
	  locker__error(context, "%s: Unrecognized mount option '%c' in\n"
			"\"%s\".\n", name, *p, desc);
	  return LOCKER_EPARSE;
	}

      /* Mountpoint */
      p = strtok_r(NULL, " ", &lasts);
      if (!p)
	goto parse_error;
      if (mountpoint)
	at->mountpoint = strdup(mountpoint);
      else
	at->mountpoint = strdup(p);
      if (!at->mountpoint)
	goto mem_error;
    }

  /* Canonicalize hostname and build hostdir. */
  h = gethostbyname(host);
  if (!h)
    {
      locker__error(context, "%s: Could not resolve hostname %s.\n",
		    name ? name : desc, host);
      status = LOCKER_EPARSE;
      goto cleanup;
    }
  memcpy(&at->hostaddr, h->h_addr, sizeof(at->hostaddr));

  at->hostdir = malloc(strlen(h->h_name) + strlen(dir) + 2);
  if (!at->hostdir)
    goto mem_error;
  sprintf(at->hostdir, "%s:%s", h->h_name, dir);

  if (!at->mountpoint)
    {
      if (mountpoint)
	at->mountpoint = strdup(mountpoint);
      else
	{
	  /* Generate a default mountpoint. This defaults to
	   * /hostname/hostdir where "hostname" is the canonical
	   * hostname, in lowercase, up to the first dot, and hostdir
	   * is the path to the NFS locker on that host. BUT, if
	   * nfs_mount_dir is set, its value is prepended, and if
	   * nfs_root_hack is set, "root" is appended if hostdir is
	   * "/".
	   */
	  char *mount_dir = context->nfs_mount_dir ?
	    context->nfs_mount_dir : "";
	  int root_hack = context->nfs_root_hack && !strcmp(dir, "/");
	  int hostlen = strcspn(host, ".");

	  at->mountpoint = malloc(strlen(mount_dir) + 1 + hostlen +
			      strlen(dir) + (root_hack ? 5 : 1));
	  if (at->mountpoint)
	    {
	      sprintf(at->mountpoint, "%s/%.*s%s%s", mount_dir, hostlen,
		      host, dir, root_hack ? "root" : "");
	    }

	  for (p = at->mountpoint + strlen(mount_dir) + 1; *p != '/'; p++)
	    *p = tolower(*p);
	}
      if (!at->mountpoint)
	goto mem_error;
    }

  status = locker__canonicalize_path(context, LOCKER_CANON_CHECK_ALL,
				     &(at->mountpoint), &(at->buildfrom));
  if (status != LOCKER_SUCCESS)
    goto cleanup;

  free(dup);
  *atp = at;
  return LOCKER_SUCCESS;

mem_error:
  locker__error(context, "Out of memory parsing locker description.\n");
  status = LOCKER_ENOMEM;
  goto cleanup;

parse_error:
  locker__error(context, "Could not parse locker description "
		"\"%s\".\n", desc);
  status = LOCKER_EPARSE;

cleanup:
  free(dup);
  locker_free_attachent(context, at);
  return status;
#else /* ENABLE_NFS */
  return LOCKER_EPARSE;
#endif /* ENABLE_NFS */
}

static int nfs_auth(locker_context context, locker_attachent *at,
		    int mode, int op)
{
#ifdef ENABLE_NFS
  int status, len;
  char *host;

  len = strcspn(at->hostdir, ":");
  host = malloc(len + 1);
  if (!host)
    {
      locker__error(context, "Out of memory authenticating to host.\n");
      return LOCKER_ENOMEM;
    }
  memcpy(host, at->hostdir, len);
  host[len] = '\0';

  status = locker_auth_to_host(context, at->name, host, op);
  free(host);
  return status;
#else /* ENABLE_NFS */
  return LOCKER_EAUTH;
#endif /* ENABLE_NFS */
}

int locker_auth_to_host(locker_context context, char *name, char *host,
			int op)
{
#ifdef ENABLE_NFS
  int status, len;
  struct timeval timeout;
  CLIENT *cl;
  struct utsname uts;
  struct passwd *pw;
  gid_t gids[NGRPS];
  KTEXT_ST authent;
  enum clnt_stat rpc_stat;

  if (op == LOCKER_AUTH_PURGE && !context->trusted)
    {
      locker__error(context, "%s: You are not allowed to use the "
		    "'purge all host mappings' option.\n", name);
      return LOCKER_EPERM;
    }

  timeout.tv_usec = 0;
  timeout.tv_sec = 20;

  /* Get an RPC handle. */
  cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
  if (!cl)
    {
      locker__error(context, "%s: server %s not responding.\n", name, host);
      return LOCKER_EATTACH;
    }
  auth_destroy(cl->cl_auth);

  /* Create authunix authentication. */
  uname(&uts);
  pw = getpwuid(context->user);
  len = getgroups(NGRPS, gids);
  cl->cl_auth = authunix_create(uts.nodename, context->user,
				pw ? pw->pw_gid : LOCKER_DEFAULT_GID,
				len, gids);

  /* Mapping and user purging are the only Kerberos-authenticated
   * functions.
   */
  if (op == LOCKER_AUTH_AUTHENTICATE || op == LOCKER_AUTH_PURGEUSER)
    {
      char *realm, *instance, *src, *dst;

      realm = (char *) krb_realmofhost(host);
      instance = malloc(strcspn(host, ".") + 1);

      for (src = host, dst = instance; *src && *src != '.'; )
	*dst++ = tolower(*src++);
      *dst = '\0';

      status = krb_mk_req(&authent, LOCKER_NFS_KSERVICE, instance, realm, 0);
      free(instance);
      if (status != KSUCCESS)
	{
	  auth_destroy(cl->cl_auth);
	  clnt_destroy(cl);
	  if (status == KDC_PR_UNKNOWN)
	    {
	      locker__error(context, "%s: (warning) Host %s isn't registered "
			    "with Kerberos.%s\n", name, host,
			    op == LOCKER_AUTH_AUTHENTICATE ?
			    " Mapping failed." : "");
	      return LOCKER_SUCCESS;
	    }
	  else
	    {
	      locker__error(context, "%s: Could not get Kerberos ticket for "
			    "NFS authentication:\n%s.\n", name,
			    krb_err_txt[status]);
	      return LOCKER_EAUTH;
	    }
	}

      rpc_stat = clnt_call(cl, op, (xdrproc_t)xdr_krbtkt, (caddr_t)&authent,
			   (xdrproc_t)xdr_void, 0, timeout);
    }
  else
    rpc_stat = clnt_call(cl, op, (xdrproc_t)xdr_void, 0, (xdrproc_t)xdr_void,
			 0, timeout);

  auth_destroy(cl->cl_auth);
  clnt_destroy(cl);

  if (rpc_stat != RPC_SUCCESS)
    {
      switch (rpc_stat)
	{
	case RPC_TIMEDOUT:
	  locker__error(context, "%s: Timeout while contacting mount "
			"daemon on %s.\n", name, host);
	  return LOCKER_EATTACH;

	case RPC_PMAPFAILURE:
	case RPC_PROGUNAVAIL:
	case RPC_PROGNOTREGISTERED:
	  locker__error(context, "%s: No mount daemon on %s.\n", name, host);
	  return LOCKER_EATTACH;

	case RPC_AUTHERROR:
	  locker__error(context, "%s: Authentication failed to host %s.\n",
		  name, host);
	  return LOCKER_EAUTH;

	case RPC_PROCUNAVAIL:
	  locker__error(context, "%s: (warning) Mount daemon on %s doesn't "
			"understand UID maps.\n", name, host);
	  return LOCKER_SUCCESS;

	default:
	  locker__error(context, "%s: System error contacting server %s.\n",
			name, host);
	  return LOCKER_EAUTH;
	}
    }

  return LOCKER_SUCCESS;
#else /* ENABLE_NFS */
  return LOCKER_EAUTH;
#endif /* ENABLE_NFS */
}

/* XDR for sending a Kerberos ticket - sends the whole KTEXT block,
 * but this is very old lossage, and nothing that can really be fixed
 * now.
 */

#ifdef ENABLE_NFS

static bool_t xdr_krbtkt(XDR *xdrs, KTEXT authp)
{
  KTEXT_ST auth;

  auth = *authp;
  auth.length = htonl(authp->length);
  return xdr_opaque(xdrs, (caddr_t)&auth, sizeof(KTEXT_ST));
}

#endif /* ENABLE_NFS */

static int nfs_zsubs(locker_context context, locker_attachent *at)
{
#ifdef ENABLE_NFS
  int len, status;
  char *subs[2];

  subs[0] = at->hostdir;

  len = strchr(at->hostdir, ':') - at->hostdir;
  subs[1] = malloc(len + 1);
  if (!subs[1])
    {
      free(subs[0]);
      free(subs);
      locker__error(context, "Out of memory getting Zephyr subscriptions.\n");
      return LOCKER_ENOMEM;
    }
  memcpy(subs[1], at->hostdir, len);
  subs[1][len] = '\0';

  status = locker__add_zsubs(context, subs, 2);
  free(subs[1]);
  return status;
#else /* ENABLE_NFS */
  return LOCKER_EPARSE;
#endif
}
