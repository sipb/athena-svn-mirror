/* Copyright 1997 by the Massachusetts Institute of Technology.
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

/* This file is part of the Athena login library.  It implements
 * functions to add and remove a user from the group database.
 */

static const char rcsid[] = "$Id: group.c,v 1.2 1997-10-30 23:58:54 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hesiod.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include "al.h"
#include "al_private.h"

/* Maximum number of groups the user can be in, not counting the primary
 * group.  Use this value only to bound nentries, not for memory
 * allocation; some day on some system it may be quite large. */
#define MAX_GROUPS 13

struct hesgroup {
  char *name;
  gid_t gid;
  int present;
};

static struct hesgroup *retrieve_hesgroups(const char *username, int *ngroups,
					   int *have_primary,
					   gid_t *primary_gid);
static void free_hesgroups(struct hesgroup *hesgroups, int ngroups);
static gid_t *retrieve_local_gids(int *nlocal);
static int in_local_gids(gid_t *local, int nlocal, gid_t gid);
static FILE *lock_group(int *fd);
static int update_group(FILE *fp, int fd);
static void discard_group_lockfile(FILE *fp, int fd);

int al__add_to_group(const char *username, struct al_record *record)
{
  FILE *in, *out;
  char *line;
  int len = strlen(username);
  int linesize = 0, nentries, i, ngroups, have_primary, lockfd;
  const char *p, *q;
  gid_t gid, primary_gid;
  struct hesgroup *hesgroups;

  /* Retrieve the hesiod groups. */
  hesgroups = retrieve_hesgroups(username, &ngroups, &have_primary,
				 &primary_gid);
  if (!hesgroups)
    return AL_WGROUP;

  /* Open the input and output files. */
  out = lock_group(&lockfd);
  if (!out)
    {
      free_hesgroups(hesgroups, ngroups);
      return AL_WGROUP;
    }
  in = fopen(PATH_GROUP, "r");
  if (!in)
    {
      free_hesgroups(hesgroups, ngroups);
      discard_group_lockfile(out, lockfd);
      return AL_WGROUP;
    }

  /* Set up the groups array in the session record. */
  record->groups = malloc(ngroups * sizeof(gid_t));
  if (!record->groups)
    {
      free_hesgroups(hesgroups, ngroups);
      discard_group_lockfile(out, lockfd);
      fclose(in);
      return AL_ENOMEM;
    }
  record->ngroups = 0;

  /* Count the number of groups the user already belongs to. */
  nentries = 0;
  while (al__read_line(in, &line, &linesize) == 0)
    {
      /* Skip the group name, group password, and gid; record the gid. */
      p = strchr(line, ':');
      if (!p)
	continue;
      p = strchr(p + 1, ':');
      if (!p)
	continue;
      gid = atoi(p + 1);
      p = strchr(p + 1, ':');
      if (!p)
	continue;

      /* Don't include the primary gid in the count. */
      if (have_primary && gid == primary_gid)
	continue;

      /* Look for username in the group's user list. */
      while (p)
	{
	  p++;
	  if (memcmp(p, username, len) == 0
	      && (*(p + len) == ',' || *(p + len) == 0))
	    {
	      nentries++;
	      break;
	    }
	  p = strchr(p, ',');
	}
    }

  /* Copy in to out, adding the user to groups as we go.  We choose to skip
   * malformed group lines because it's a little easier; you could justify
   * either skipping or preserving them.  Hopefully we won't find any. */
  rewind(in);
  while (al__read_line(in, &line, &linesize) == 0)
    {
      /* Skip over the group name, group password, and gid; record the gid. */
      p = strchr(line, ':');
      if (!p)
	continue;
      p = strchr(p + 1, ':');
      if (!p)
	continue;
      gid = atoi(p + 1);
      p = strchr(p + 1, ':');
      if (!p)
	continue;

      /* Write out the output line, without a newline for now. */
      fputs(line, out);

      /* Check if Hesiod has the user in this group. */
      for (i = 0; i < ngroups; i++)
	{
	  if (hesgroups[i].gid == gid)
	    break;
	}
      if (i < ngroups)
	{
	  /* Just for safety, if we've seen this group entry before, don't
	   * do anything with it.  Otherwise we might overflow
	   * record->groups on a bad group file. */
	  if (hesgroups[i].present)
	    {
	      putc('\n', out);
	      continue;
	    }

	  /* Make a note that this Hesiod group already has a listing. */
	  hesgroups[i].present = 1;

	  /* Check if the user is already listed in this group. */
	  while (p)
	    {
	      p++;
	      if (memcmp(p, username, len) == 0
		  && (*(p + len) == ',' || *(p + len) == 0))
		break;
	      p = strchr(p, ',');
	    }
	  if (p)
	    {
	      putc('\n', out);
	      continue;
	    }

	  /* Add the user to the group if it's the primary group or if
	   * the user isn't already in MAX_GROUPS other groups. */
	  if (nentries < MAX_GROUPS || (have_primary && gid == primary_gid))
	    {
	      if (line[strlen(line) - 1] != ':')
		putc(',', out);
	      fputs(username, out);
	      if (!have_primary || gid != primary_gid)
		nentries++;
	      record->groups[record->ngroups] = gid;
	      record->ngroups++;
	    }
	}

      /* finish up the output line. */
      putc('\n', out);
    }

  /* Write out group lines which had no listings before. */
  for (i = 0; i < ngroups; i++)
    {
      gid = hesgroups[i].gid;
      if (hesgroups[i].present)
	continue;
      if (nentries < MAX_GROUPS || (have_primary && gid == primary_gid))
	{
	  fprintf(out, "%s:*:%d:%s\n", hesgroups[i].name, gid, username);
	  if (!have_primary || gid != primary_gid)
	    nentries++;
	  record->groups[record->ngroups] = gid;
	  record->ngroups++;
	}
    }

  /* Clean up allocated memory. */
  fclose(in);
  if (linesize)
    free(line);
  free_hesgroups(hesgroups, ngroups);
  if (update_group(out, lockfd) < 0)
    return AL_WGROUP;

  return AL_SUCCESS;
}

int al__remove_from_group(const char *username, struct al_record *record)
{
  FILE *in, *out;
  char *line, *p;
  int i, lockfd, linesize = 0, nlocal, len = strlen(username);
  gid_t gid, *local;

  local = retrieve_local_gids(&nlocal);

  out = lock_group(&lockfd);
  if (!out)
    {
      if (local)
	free(local);
      return AL_EPERM;
    }
  in = fopen(PATH_GROUP, "r");
  if (!in)
    {
      if (local)
	free(local);
      discard_group_lockfile(out, lockfd);
      return AL_EPERM;
    }

  /* Copy in to out, eliminating the user from groups in record->groups. */
  while (al__read_line(in, &line, &linesize) == 0)
    {
      /* Skip the group name, group password, and gid; record the gid. */
      p = strchr(line, ':');
      if (!p)
	continue;
      p = strchr(p + 1, ':');
      if (!p)
	continue;
      gid = atoi(p + 1);
      p = strchr(p + 1, ':');
      if (!p)
	continue;

      for (i = 0; i < record->ngroups; i++)
	{
	  if (record->groups[i] == gid)
	    break;
	}
      if (i < record->ngroups)
	{
	  /* Search for username in the membership list and remove it. */
	  while (p)
	    {
	      p++;
	      if (memcmp(p, username, len) == 0
		  && (*(p + len) == ',' || *(p + len) == 0))
		{
		  /* Found it; now remove it. */
		  i = (*(p + len) == ',') ? len + 1 : len;
		  memmove(p, p + i, strlen(p + i) + 1);
		  if (*p == 0 && *(p - 1) == ',')
		    *(p - 1) = 0;
		}
	      p = strchr(p, ',');
	    }
	}

      /* Write out the edited line, unless it's an empty group not in the
       * local gid list. */
      if (line[strlen(line) - 1] != ':' || in_local_gids(local, nlocal, gid))
	{
	  fputs(line, out);
	  putc('\n', out);
	}
    }

  fclose(in);
  if (linesize)
    free(line);
  if (local)
    free(local);

  if (update_group(out, lockfd) < 0)
    return AL_EPERM;
}

static struct hesgroup *retrieve_hesgroups(const char *username, int *ngroups,
					   int *have_primary,
					   gid_t *primary_gid)
{
  char **grplistvec, **primarygidvec, buf[64];
  const char *p, *q;
  int n, len;
  struct hesgroup *hesgroups;
  struct passwd *pwd;
  void *hescontext;

  if (hesiod_init(&hescontext) != 0)
    return NULL;

  /* Look up the Hesiod group list. */
  grplistvec = hesiod_resolve(hescontext, username, "grplist");
  if (!grplistvec || !*grplistvec)
    {
      hesiod_end(hescontext);
      return NULL;
    }

  /* Get a close upper bound on the number of group entries we'll need. */
  n = 0;
  for (p = *grplistvec; *p; p++)
    {
      if (*p == ':')
	n++;
    }
  n = (n + 1) / 2 + 1;

  /* Allocate memory. */
  hesgroups = malloc(n * sizeof(struct hesgroup));
  if (!hesgroups)
    {
      hesiod_free_list(hescontext, grplistvec);
      hesiod_end(hescontext);
      return NULL;
    }

  /* Start counting group entries again from 0. */
  n = 0;

  /* Try to get the primary group, but don't lose if we fail. */
  *have_primary = 0;
  pwd = al__getpwnam(username);
  if (pwd)
    {
      sprintf(buf, "%lu", (unsigned long) pwd->pw_gid);
      primarygidvec = hesiod_resolve(hescontext, buf, "gid");
      if (primarygidvec && *primarygidvec)
	{
	  p = strchr(*primarygidvec, ':');
	  len = (p) ? p - *primarygidvec : strlen(*primarygidvec);
	  hesgroups[n].name = malloc(len + 1);
	  if (hesgroups[n].name)
	    {
	      memcpy(hesgroups[n].name, *primarygidvec, len);
	      hesgroups[n].name[len] = 0;
	      hesgroups[n].gid = pwd->pw_gid;
	      hesgroups[n].present = 0;
	      n++;
	      *have_primary = 1;
	      *primary_gid = pwd->pw_gid;
	    }
	}
      if (primarygidvec)
	hesiod_free_list(hescontext, primarygidvec);
      al__free_passwd(pwd);
    }

  /* Now get the entries from grplistvec. */
  p = *grplistvec;
  while (p)
    {
      q = strchr(p, ':');
      if (!q || !*(q + 1))
	break;
      hesgroups[n].name = malloc(q - p + 1);
      if (!hesgroups[n].name)
	{
	  free_hesgroups(hesgroups, n);
	  hesiod_free_list(hescontext, grplistvec);
	  hesiod_end(hescontext);
	  return NULL;
	}
      memcpy(hesgroups[n].name, p, q - p);
      hesgroups[n].name[q - p] = 0;
      hesgroups[n].gid = atoi(q + 1);
      hesgroups[n].present = 0;
      n++;
      p = strchr(q + 1, ':');
      if (p)
	p++;
    }

  /* Clean up allocated memory and return. */
  hesiod_free_list(hescontext, grplistvec);
  hesiod_end(hescontext);
  *ngroups = n;
  return hesgroups;
}

static void free_hesgroups(struct hesgroup *hesgroups, int ngroups)
{
  int i;

  for (i = 0; i < ngroups; i++)
    free(hesgroups[i].name);
  free(hesgroups);
}

static gid_t *retrieve_local_gids(int *nlocal)
{
  FILE *fp;
  char *line;
  int linesize = 0, lines, n;
  gid_t *gids;
  const char *p;

  /* Open the local group file.  If it doesn't exist, we have no local
   * gid list. */
  fp = fopen(PATH_GROUP_LOCAL, "r");
  if (!fp)
    return NULL;

  /* Count the number of lines in the file and allocate memory. */
  lines = 0;
  while (al__read_line(fp, &line, &linesize) == 0)
    lines++;
  gids = malloc(lines * sizeof(gid_t));
  if (!gids)
    {
      fclose(fp);
      return NULL;
    }

  /* Make another pass over the file reading gids. */
  rewind(fp);
  n = 0;
  while (al__read_line(fp, &line, &linesize) == 0)
    {
      /* Paranoia: don't exceed the amount of allocated memory if
       * group.local increases in size since the first pass.  In that
       * case, return NULL ("be conservative"), since we wouldn't get
       * a complete list if we just broke out and returned. */
      if (n >= lines)
	{
	  free(line);
	  free(gids);
	  fclose(fp);
	  return NULL;
	}

      /* Find and record the gid. */
      p = strchr(line, ':');
      if (!p)
	continue;
      p = strchr(p + 1, ':');
      if (!p)
	continue;
      gids[n++] = atoi(p + 1);
    }

  fclose(fp);
  if (linesize)
    free(line);
  *nlocal = n;
  return gids;
}

static int in_local_gids(gid_t *local, int nlocal, gid_t gid)
{
  int i;

  /* If we have no local gid list, be conservative and treat every gid as a
   * local group, so we don't delete anything important. */
  if (!local)
    return 1;

  for (i = 0; i < nlocal; i++)
    {
      if (local[i] == gid)
	return 1;
    }

  return 0;
}

static FILE *lock_group(int *fd)
{
  struct flock fl;
  FILE *fp;

  /* Open and lock the group lock file. */
  *fd = open(PATH_GROUP_LOCK, O_CREAT|O_RDWR|O_TRUNC, S_IWUSR|S_IRUSR);
  if (*fd < 0)
    return NULL;
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  if (fcntl(*fd, F_SETLKW, &fl) < 0)
    {
      close(*fd);
      return NULL;
    }

  /* That taken care of, open the group temp file. */
  fp = fopen(PATH_GROUP_TMP, "w");
  if (fp)
    fchmod(fileno(fp), S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
  return fp;
}

static int update_group(FILE *fp, int fd)
{
  struct flock fl;
  int status;

  /* Move the group temp file into place. */
  fclose(fp);
  status = rename(PATH_GROUP_TMP, PATH_GROUP);

  /* Unlock and close the group lock file. */
  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fcntl(fd, F_SETLKW, &fl);
  close(fd);
  return (status < 0) ? AL_WGROUP : AL_SUCCESS;
}

static void discard_group_lockfile(FILE *fp, int fd)
{
  struct flock fl;

  /* Discard the group temp file. */
  fclose(fp);
  unlink(PATH_GROUP_TMP);

  /* Unlock and close the group lock file. */
  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fcntl(fd, F_SETLKW, &fl);
  close(fd);
}
