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

/* This file is part of liblocker. It implements AFS lockers. */

static const char rcsid[] = "$Id: afs.c,v 1.2 1999-03-11 04:08:42 danw Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <afs/stds.h>
#include <afs/param.h>
#include <afs/auth.h>
#include <afs/cellconfig.h>
#include <afs/ptserver.h>
#include <afs/venus.h>
#include <rx/rxkad.h>

/* This is defined in <afs/volume.h>, but it doesn't seem possible to
 * include that without dragging in most of the rest of the afs
 * includes as dependencies.
 */
#define VNAMESIZE 32

#include <com_err.h>
#include <krb.h>

#include "locker.h"
#include "locker_private.h"

static int afs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **at);
static int afs_attach(locker_context context, locker_attachent *at,
		      char *mountoptions);
static int afs_detach(locker_context context, locker_attachent *at);
static int afs_auth(locker_context context, locker_attachent *at,
		    int mode, int op);
static int afs_zsubs(locker_context context, locker_attachent *at,
		     int op);

struct locker_ops locker__afs_ops = {
  "AFS",
  0,
  afs_parse,
  afs_attach,
  afs_detach,
  afs_auth,
  afs_zsubs
};

static int afs_get_cred(char *name, char *inst, char *realm,
			CREDENTIALS *cred);

static int afs_parse(locker_context context, char *name, char *desc,
		     char *mountpoint, locker_attachent **atp)
{
  locker_attachent *at;
  char *p, *dup = NULL, *lasts = NULL;
  int status;

  at = locker__new_attachent(context, &locker__afs_ops);
  if (!at)
    return LOCKER_ENOMEM;

  if (!name)
    {
      /* This is an explicit description. */

      if (strncmp(desc, "/afs/", 5))
	{
	  locker__error(context, "%s: Path is not in AFS.\n", desc);
	  status = LOCKER_EPARSE;
	  goto cleanup;
	}

      at->name = strdup(desc);
      at->hostdir = strdup(desc);
      if (mountpoint)
	at->mountpoint = strdup(mountpoint);
      else
	{
	  p = strrchr(desc, '/') + 1;
	  at->mountpoint = malloc(strlen(context->afs_mount_dir) +
				  strlen(p) + 2);
	  if (at->mountpoint)
	    sprintf(at->mountpoint, "%s/%s", context->afs_mount_dir, p);
	}
      if (!at->name || !at->hostdir || !at->mountpoint)
	goto mem_error;

      at->mode = LOCKER_AUTH_READWRITE;
    }
  else
    {
      /* A Hesiod AFS description looks like:
       * AFS /afs/dev.mit.edu/source/src-current w /mit/source
       */

      at->name = strdup(name);
      if (!at->name)
	goto mem_error;

      dup = strdup(desc);
      if (!dup)
	goto mem_error;

      /* Skip "AFS". */
      if (!strtok_r(dup, " ", &lasts))
	goto parse_error;

      /* Hostdir */
      at->hostdir = strtok_r(NULL, " ", &lasts);
      if (!at->hostdir)
	goto parse_error;
      if (strncmp(at->hostdir, "/afs/", 5))
	{
	  locker__error(context, "%s: Path \"%s\" is not in AFS.\n", name,
			at->hostdir);
	  status = LOCKER_EPARSE;
	  goto cleanup;
	}
      at->hostdir = strdup(at->hostdir);
      if (!at->hostdir)
	goto mem_error;

      /* Auth mode */
      p = strtok_r(NULL, " ", &lasts);
      if (!p || *(p + 1))
	goto parse_error;

      switch (*p)
	{
	case 'r':
	  at->mode = LOCKER_AUTH_READONLY;
	  break;
	case 'w':
	  at->mode = LOCKER_AUTH_READWRITE;
	  break;
	case 'n':
	  at->mode = LOCKER_AUTH_NONE;
	  break;
	default:
	  locker__error(context, "%s: Unrecognized auth mode '%c' in "
			"description:\n%s\n", name, *p, desc);
	  status = LOCKER_EPARSE;
	  goto cleanup;
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

      free(dup);
      dup = NULL;
    }

  status = locker__canonicalize_path(context, 1, &(at->mountpoint),
				     &(at->buildfrom));
  if (status != LOCKER_SUCCESS)
    goto cleanup;

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
}

static int afs_attach(locker_context context, locker_attachent *at,
		      char *mountoptions)
{
  struct stat st1, st2;
  struct ViceIoctl vio;
  int32 hosts[8]; /* AFS docs say VIOCWHEREIS won't return more than 8. */
  uid_t uid = geteuid();
  int status;

  /* Make sure user can read the destination, and it's a directory. */
  if (uid != context->user)
    seteuid(context->user);
  status = lstat(at->hostdir, &st1);
  if (uid != context->user)
    seteuid(uid);

  if (status == -1)
    {
     if (errno == ETIMEDOUT)
	{
	  locker__error(context, "%s: Connection timed out while trying to "
			"attach locker.\nThis probably indicates a temporary "
			"problem with the file server containing\n"
			"this locker. Try again later.\n", at->name);
	}
      else
	{
	  locker__error(context, "%s: Could not attach locker:\n%s for %s\n",
			at->name, strerror(errno), at->hostdir);
	}
      return LOCKER_EATTACH;
    }
  if (!S_ISDIR(st1.st_mode) && !S_ISLNK(st1.st_mode))
    {
      locker__error(context, "%s: Could not attach locker:\n"
		    "%s is not a directory.\n", at->name, at->hostdir);
      return LOCKER_EATTACH;
    }

  /* Make sure nothing is already mounted on our mountpoint. */
  status = stat(at->mountpoint, &st2);
  if (!status && st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino)
    {
      locker__error(context, "%s: Locker is already attached.\n",
		    at->name);
      return LOCKER_EALREADY;
    }
  else if (!status || errno != ENOENT)
    {
      locker__error(context, "%s: Could not attach locker:\n"
		    "Mountpoint %s is busy.\n", at->name, at->mountpoint);
      return LOCKER_EMOUNTPOINTBUSY;
    }

  status = symlink(at->hostdir, at->mountpoint);
  if (status < 0)
    {
      locker__error(context, "%s: Could not attach locker:\n%s while "
		    "symlinking %s to %s\n", at->name, strerror(errno),
		    at->hostdir, at->mountpoint);
      return LOCKER_EATTACH;
    }

  /* Find host that the locker is on, and update the attachent. */
  memset(hosts, 0, sizeof(hosts));
  vio.in_size = 0;
  vio.out = (caddr_t)hosts;
  vio.out_size = sizeof(hosts);
  if (pioctl(at->hostdir, VIOCWHEREIS, &vio, 1) == 0)
    {
      /* Only record the hostaddr if the locker is on a single host.
       * (We assume that if it's on multiple hosts, it can't fail,
       * so we don't need to know what those hosts are.)
       */
      if (!hosts[1])
	at->hostaddr.s_addr = hosts[0];
    }

  return LOCKER_SUCCESS;
}

static int afs_detach(locker_context context, locker_attachent *at)
{
  int status;

  status = unlink(at->mountpoint);
  if (status < 0)
    {
      if (errno == ENOENT)
	{
	  locker__error(context, "%s: Locker is not attached.\n", at->name);
	  return LOCKER_ENOTATTACHED;
	}
      else
	{
	  locker__error(context, "%s: Could not detach locker:\n%s while "
			"trying to unlink %s.\n", at->name, strerror(errno),
			at->mountpoint);
	  return LOCKER_EDETACH;
	}
    }
  return LOCKER_SUCCESS;
}

static int afs_auth(locker_context context, locker_attachent *at,
		    int mode, int op)
{
  char *cell, *p;
  int status, cellnum;
  struct ktc_principal princ;

  if (op != LOCKER_AUTH_AUTHENTICATE)
    return LOCKER_SUCCESS;

  /* We know (from afs_parse) that at->hostdir starts with "/afs/". */
  cell = at->hostdir + 5;
  /* Skip initial "." in the cell name (if this is a path to a rw volume). */
  if (*cell == '.')
    cell++;

  p = strchr(cell, '/');
  if (p)
    *p = '\0';

  /* Check if we already have tokens for this cell. */
  cellnum = 0;
  while (!ktc_ListTokens(cellnum, &cellnum, &princ))
    {
      if (!strcmp(princ.cell, cell))
	{
	  if (p)
	    *p = '/';
	  return LOCKER_SUCCESS;
	}
    }

  status = locker_auth_to_cell(context, at->name, cell, op);
  if (p)
    *p = '/';
  return status;
}

int locker_auth_to_cell(locker_context context, char *name, char *cell,
			int op)
{
  char *crealm, urealm[REALM_SZ], *user;
  CREDENTIALS cred;
  int status;
  struct afsconf_dir *configdir;
  struct afsconf_cell cellconfig;
  struct ktc_principal server, client;
  struct ktc_token token;
  long vice_id;
  uid_t uid = geteuid(), ruid = getuid();

  if (op != LOCKER_AUTH_AUTHENTICATE)
    return LOCKER_SUCCESS;

  /* Find the cell's db servers. */
  configdir = afsconf_Open(AFSCONF_CLIENTNAME);
  if (!configdir)
    {
      locker__error(context, "%s: Could not authenticate to AFS: "
		    "error opening CellServDB file.\n", name);
      return LOCKER_EAUTH;
    }
  status = afsconf_GetCellInfo(configdir, cell, NULL, &cellconfig);
  afsconf_Close(configdir);
  if (status)
    {
      initialize_acfg_error_table();
      locker__error(context, "%s: Could not authenticate to AFS:\n%s while "
		    "reading CellServDB file.\n", name, error_message(status));
      return LOCKER_EAUTH;
    }

  /* Canonicalize the cell name. */
  cell = cellconfig.name;

  /* Get tickets for the realm containing the cell's servers. (Set uid
   * to the user before touching the ticket file.) Try
   * afs.cellname@realm first, and afs@realm if that doesn't exist.
   */
  crealm = (char *)krb_realmofhost(cellconfig.hostName[0]);
  if (uid != ruid)
    seteuid(ruid);
  krb_get_tf_realm(TKT_FILE, urealm);
  status = afs_get_cred("afs", cell, crealm, &cred);
  if (status)
    status = afs_get_cred("afs", "", crealm, &cred);
  if (uid != ruid)
    seteuid(uid);
  if (status)
    {
      if (status == NO_TKT_FIL)
	{
	  locker__error(context, "%s: Could not authenticate to AFS: %s.\n",
			name, krb_get_err_text(status));
	}
      else
	{
	  locker__error(context, "%s: Could not authenticate to AFS cell "
			"%s:\n%s while getting tickets for %s.\n",
			name, cell, krb_get_err_text(status), crealm);
	}
      return LOCKER_EAUTH;
    }

  /* Create a token from the ticket. (Code stolen from aklog.) */
  token.kvno = cred.kvno;
  token.startTime = cred.issue_date;
  /* ticket lifetime is in five-minutes blocks. */
  token.endTime = cred.issue_date + ((unsigned char)cred.lifetime * 5 * 60);
  memcpy(&token.sessionKey, cred.session, sizeof(token.sessionKey));
  token.ticketLen = cred.ticket_st.length;
  memcpy(token.ticket, cred.ticket_st.dat, token.ticketLen);

  user = malloc(strlen(cred.pname) + strlen(cred.pinst) + strlen(urealm) + 3);
  if (!user)
    {
      locker__error(context, "Out of memory authenticating to cell.\n");
      return LOCKER_ENOMEM;
    }
  strcpy(user, cred.pname);
  if (*cred.pinst)
    sprintf(user + strlen(user), ".%s", cred.pinst);
  if (strcasecmp(crealm, urealm))
    sprintf(user + strlen(user), "@%s", urealm);

  /* Look up principal's PTS id. */
  initialize_pt_error_table();
  status = pr_Initialize(0, AFSCONF_CLIENTNAME, cell);
  if (status)
    {
      locker__error(context, "%s: Could not initialize AFS protection "
		    "library while authenticating to cell \"%s\":\n%s.\n",
		    name, cell, error_message(status));
      free(user);
      return LOCKER_EAUTH;
    }
  status = pr_SNameToId(user, &vice_id);
  if (status)
    {
      locker__error(context, "%s: Could not find AFS PTS id for user \"%s\""
		    "in cell \"%s\":\n%s.\n", name, user, cell,
		    error_message(status));
      free(user);
      return LOCKER_EAUTH;
    }

  /* Select appropriate dead chicken to wave. */
  if (vice_id == ANONYMOUSID)
    strncpy(client.name, user, MAXKTCNAMELEN - 1);
  else
    sprintf(client.name, "AFS ID %d", vice_id);
  free(user);
  strcpy(client.instance, "");
  strncpy(client.cell, crealm, MAXKTCREALMLEN - 1);
  client.cell[MAXKTCREALMLEN - 1] = '\0';

  strcpy(server.name, "afs");
  strcpy(server.instance, "");
  strncpy(server.cell, cell, MAXKTCREALMLEN - 1);
  server.cell[MAXKTCREALMLEN - 1] = '\0';

  /* Store the token. */
  status = ktc_SetToken(&server, &token, &client, 0);
  if (status)
    {
      locker__error(context, "%s: Could not obtain %s tokens for cell "
		    "%s:\n%s.\n", name, user, cell, error_message(status));
      return LOCKER_EAUTH;
    }

  return LOCKER_SUCCESS;
}

static int afs_get_cred(char *name, char *inst, char *realm, CREDENTIALS *cred)
{
  int status;

  status = krb_get_cred(name, inst, realm, cred);
  if (status != KSUCCESS)
    {
      status = get_ad_tkt(name, inst, realm, 255);
      if (status == KSUCCESS)
	status = krb_get_cred(name, inst, realm, cred);
    }

  return status;
}

static int afs_zsubs(locker_context context, locker_attachent *at,
		     int op)
{
  struct ViceIoctl vio;
  char *path, *last_component, *p, *subs[4];
  char cell[MAXCELLCHARS + 1], vol[VNAMESIZE + 1];
  char cellvol[MAXCELLCHARS + VNAMESIZE + 2];
  int32 hosts[8];
  int status, pstatus;
  struct hostent *h;

  subs[0] = cell;
  subs[1] = cellvol;
  subs[3] = NULL;

  path = strdup(at->hostdir);
  if (!path)
    {
      locker__error(context, "Out of memory getting zephyr subscriptions.\n");
      return LOCKER_ENOMEM;
    }

  /* Walk down the path. At each level, add subscriptions for the cell,
   * host, and volume where that path component lives.
   */
  p = path;
  do
    {
      /* Move trailing NUL over one pathname component. */
      *p = '/';
      p = strchr(p + 1, '/');
      if (p)
	*p = '\0';

      /* Get cell */
      vio.in_size = 0;
      vio.out = cell;
      vio.out_size = sizeof(cell);
      if (pioctl(path, VIOC_FILE_CELL_NAME, &vio, 1) != 0)
	continue;

      /* Get mountpoint name and generate cell:mountpoint. */
      last_component = strrchr(path, '/');
      if (last_component)
	{
	  *last_component++ = '\0';
	  vio.in = last_component;
	}
      else
	vio.in = "/";
      vio.in_size = strlen(vio.in) + 1;
      vio.out = vol;
      vio.out_size = sizeof(vol);
      pstatus = pioctl(path, VIOC_AFS_STAT_MT_PT, &vio, 1);
      if (last_component)
	*(last_component - 1) = '/';

      if (pstatus != 0)
	continue;

      /* Get cell:volumname into cellvol, ignoring initial '#' or '%'. */
      if (strchr(vol, ':'))
	strcpy(cellvol, vol + 1);
      else
	sprintf(cellvol, "%s:%s", cell, vol + 1);

      /* If there's only one site for this volume, add the hostname
       * of the server to the subs list.
       */
      memset(hosts, 0, 2 * sizeof(*hosts));
      vio.out = (caddr_t)hosts;
      vio.out_size = sizeof(hosts);
      if (pioctl(path, VIOCWHEREIS, &vio, 1) != 0)
	continue;
      if (!hosts[1])
	{
	  h = gethostbyaddr((char *)&hosts[0], 4, AF_INET);
	  if (!h)
	    continue;
	  subs[2] = h->h_name;
	}
      else
	subs[2] = NULL;

      status = locker__zsubs(context, at, op, subs);
    }
  while (status == LOCKER_SUCCESS && strlen(path) != strlen(at->hostdir));

  free(path);
  return status;
}

/* librxkad depends on this symbol in Transarc's des library, which we
 * can't link with because of conflicts with our krb4 library. It never
 * gets called though.
 */
void des_pcbc_init(void)
{
  abort();
}
