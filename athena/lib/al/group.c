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

static const char rcsid[] = "$Id: group.c,v 1.4 1997-11-13 23:26:04 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

static int retrieve_hesgroups(const char *username, struct hesgroup **groups,
			      int *ngroups, gid_t *primary_gid);
static void free_hesgroups(struct hesgroup *hesgroups, int ngroups);
static gid_t *retrieve_local_gids(int *nlocal);
static int in_local_gids(gid_t *local, int nlocal, gid_t gid);
static int parse_to_gid(char *s, char **p, gid_t *gid);
static FILE *lock_group(int *fd);
static int update_group(FILE *fp, int fd);
static void discard_group_lockfile(FILE *fp, int fd);

int al__add_to_group(const char *username, struct al_record *record)
{
  FILE *in, *out;
  char *line, *p;
  int len = strlen(username), linesize = 0, nentries, i, nhesgroups;
  int lockfd, status, ngroups;
  gid_t gid, primary_gid, *groups;
  struct hesgroup *hesgroups;

  /* Retrieve the hesiod groups. */
  if (retrieve_hesgroups(username, &hesgroups, &nhesgroups, &primary_gid) != 0)
    return AL_WGROUP;

  /* Open the input and output files. */
  out = lock_group(&lockfd);
  if (!out)
    {
      free_hesgroups(hesgroups, nhesgroups);
      return AL_WGROUP;
    }
  in = fopen(PATH_GROUP, "r");
  if (!in)
    {
      free_hesgroups(hesgroups, nhesgroups);
      discard_group_lockfile(out, lockfd);
      return AL_WGROUP;
    }

  /* Set up the groups array in the session record. */
  groups = malloc(nhesgroups * sizeof(gid_t));
  if (!groups)
    {
      free_hesgroups(hesgroups, nhesgroups);
      discard_group_lockfile(out, lockfd);
      fclose(in);
      return AL_ENOMEM;
    }
  ngroups = 0;

  /* Count the number of groups the user already belongs to. */
  nentries = 0;
  while ((status = al__read_line(in, &line, &linesize)) == 0)
    {
      /* Skip the group name, group password, and gid; record the gid. */
      if (parse_to_gid(line, &p, &gid) != 0)
	continue;

      /* Don't include the primary gid in the count. */
      if (gid == primary_gid)
	continue;

      /* Look for username in the group's user list. */
      while (p)
	{
	  p++;
	  if (strncmp(p, username, len) == 0
	      && (*(p + len) == ',' || *(p + len) == 0))
	    {
	      nentries++;
	      break;
	    }
	  p = strchr(p, ',');
	}
    }
  if (status == -1)
    {
      free(groups);
      free_hesgroups(hesgroups, nhesgroups);
      discard_group_lockfile(out, lockfd);
      fclose(in);
      return AL_ENOMEM;
    }

  /* Copy in to out, adding the user to groups as we go.  We choose to skip
   * malformed group lines because it's a little easier; you could justify
   * either skipping or preserving them.  Hopefully we won't find any. */
  rewind(in);
  while ((status = al__read_line(in, &line, &linesize)) == 0)
    {
      /* Skip the group name, group password, and gid; record the gid. */
      if (parse_to_gid(line, &p, &gid) != 0)
	continue;

      /* Write out the output line, without a newline for now. */
      fputs(line, out);

      /* Check if Hesiod has the user in this group. */
      for (i = 0; i < nhesgroups; i++)
	{
	  if (hesgroups[i].gid == gid)
	    break;
	}
      if (i < nhesgroups)
	{
	  /* Just for safety, if we've seen this group entry before, don't
	   * do anything with it.  Otherwise we might overflow
	   * groups on a bad group file. */
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
	      if (strncmp(p, username, len) == 0
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
	  if (nentries < MAX_GROUPS || gid == primary_gid)
	    {
	      if (line[strlen(line) - 1] != ':')
		putc(',', out);
	      fputs(username, out);
	      if (gid != primary_gid)
		nentries++;
	      groups[ngroups++] = gid;
	    }
	}

      /* finish up the output line. */
      putc('\n', out);
    }
  if (status == -1)
    {
      free(groups);
      free_hesgroups(hesgroups, nhesgroups);
      discard_group_lockfile(out, lockfd);
      fclose(in);
      return AL_ENOMEM;
    }

  /* Write out group lines which had no listings before. */
  for (i = 0; i < nhesgroups; i++)
    {
      gid = hesgroups[i].gid;
      if (hesgroups[i].present)
	continue;
      if (nentries < MAX_GROUPS || gid == primary_gid)
	{
	  fprintf(out, "%s:*:%lu:%s\n", hesgroups[i].name,
		  (unsigned long) gid, username);
	  if (gid != primary_gid)
	    nentries++;
	  groups[ngroups++] = gid;
	}
    }

  /* Clean up allocated memory. */
  fclose(in);
  if (linesize)
    free(line);
  free_hesgroups(hesgroups, nhesgroups);

  /* Update the group file from what we wrote out. */
  if (update_group(out, lockfd) < 0)
    {
      free(groups);
      return AL_WGROUP;
    }

  record->groups = groups;
  record->ngroups = ngroups;
  return AL_SUCCESS;
}

int al__remove_from_group(const char *username, struct al_record *record)
{
  FILE *in, *out;
  char *line, *p;
  int i, lockfd, linesize = 0, nlocal, status, len = strlen(username);
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
  while ((status = al__read_line(in, &line, &linesize)) == 0)
    {
      /* Skip the group name, group password, and gid; record the gid. */
      if (parse_to_gid(line, &p, &gid) != 0)
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
	      if (strncmp(p, username, len) == 0
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

      /* If the edited line has a non-empty user list or is in the local
       * gid list, write it out. */
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

  if (status == -1)
    {
      discard_group_lockfile(out, lockfd);
      return AL_ENOMEM;
    }

  if (update_group(out, lockfd) < 0)
    return AL_EPERM;
  return AL_SUCCESS;
}

/* Retrieve the user's hesiod groups and stuff them into *groups, with a
 * count in *ngroups.  Also put the user's primary gid into *primary_gid.
 * Return 0 on success and -1 on failure.
 */
static int retrieve_hesgroups(const char *username, struct hesgroup **groups,
			      int *ngroups, gid_t *primary_gid)
{
  char **grplistvec, **primarygidvec, *primary_name, buf[64], *p, *q;
  int n, len;
  struct hesgroup *hesgroups;
  struct passwd *pwd;
  void *hescontext;

  /* Look up the user's primary group in hesiod to retrieve the primary
   * group name.  Start by finding the gid. */
  pwd = al__getpwnam(username);
  if (!pwd)
    {
      hesiod_end(hescontext);
      return -1;
    }
  *primary_gid = pwd->pw_gid;
  al__free_passwd(pwd);

  /* Initialize the hesiod context. */
  if (hesiod_init(&hescontext) != 0)
    return -1;

  /* Now do the hesiod resolve.  If it fails with ENOENT, assume the user
   * has a local account and return no groups. */
  sprintf(buf, "%lu", (unsigned long) *primary_gid);
  primarygidvec = hesiod_resolve(hescontext, buf, "gid");
  if (!primarygidvec && errno == ENOENT)
    {
      *groups = NULL;
      *ngroups = 0;
      hesiod_end(hescontext);
      return 0;
    }
  if (!primarygidvec || !*primarygidvec || **primarygidvec == ':')
    {
      if (primarygidvec)
	hesiod_free_list(hescontext, primarygidvec);
      hesiod_end(hescontext);
      return -1;
    }

  /* Copy the name part into primary_name. */
  p = strchr(*primarygidvec, ':');
  len = (p) ? p - *primarygidvec : strlen(*primarygidvec);
  primary_name = malloc(len + 1);
  if (!primary_name)
    {
      hesiod_free_list(hescontext, primarygidvec);
      hesiod_end(hescontext);
      return -1;
    }
  memcpy(primary_name, *primarygidvec, len);
  primary_name[len] = 0;
  hesiod_free_list(hescontext, primarygidvec);

  /* Look up the Hesiod group list.  It's okay if there isn't one. */
  grplistvec = hesiod_resolve(hescontext, username, "grplist");
  if ((!grplistvec && errno != ENOENT) || (grplistvec && !*grplistvec))
    {
      if (grplistvec)
	hesiod_free_list(hescontext, grplistvec);
      hesiod_end(hescontext);
      free(primary_name);
      return -1;
    }

  /* Get a close upper bound on the number of group entries we'll need. */
  if (grplistvec)
    {
      n = 0;
      for (p = *grplistvec; *p; p++)
	{
	  if (*p == ':')
	    n++;
	}
      n = (n + 1) / 2 + 1;
    }
  else
    n = 1;

  /* Allocate memory. */
  hesgroups = malloc(n * sizeof(struct hesgroup));
  if (!hesgroups)
    {
      if (grplistvec)
	hesiod_free_list(hescontext, grplistvec);
      hesiod_end(hescontext);
      free(primary_name);
      return -1;
    }

  /* Start off the list with the primary gid. */
  hesgroups[0].name = primary_name;
  hesgroups[0].gid = *primary_gid;
  n = 1;
  
  /* Now get the entries from grplistvec, if we got one. */
  p = (grplistvec) ? *grplistvec : NULL;
  while (p)
    {
      /* Find the end of the group name.  Stop if we hit the end, if we
       * have a zero-length group name, or if we have a non-numeric gid. */
      q = strchr(p, ':');
      if (!q || q == p || !isdigit(*(q + 1)))
	break;

      hesgroups[n].name = malloc(q - p + 1);
      if (!hesgroups[n].name)
	{
	  free_hesgroups(hesgroups, n);
	  hesiod_free_list(hescontext, grplistvec);
	  hesiod_end(hescontext);
	  return -1;
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
  if (grplistvec)
    hesiod_free_list(hescontext, grplistvec);
  hesiod_end(hescontext);
  *ngroups = n;
  *groups = hesgroups;
  return 0;
}

static void free_hesgroups(struct hesgroup *hesgroups, int ngroups)
{
  int i;

  for (i = 0; i < ngroups; i++)
    free(hesgroups[i].name);
  if (ngroups)
    free(hesgroups);
}

static gid_t *retrieve_local_gids(int *nlocal)
{
  FILE *fp;
  char *line, *p;
  int linesize = 0, lines, n, status;
  gid_t *gids, gid;

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
      if (linesize)
	free(line);
      return NULL;
    }

  /* Make another pass over the file reading gids. */
  rewind(fp);
  n = 0;
  while ((status = al__read_line(fp, &line, &linesize)) == 0)
    {
      /* Avoid overrunning the bounds of the array. */
      if (n >= lines)
	break;

      /* Retrieve and store the gid. */
      if (parse_to_gid(line, &p, &gid) == 0)
	gids[n++] = gid;
    }

  fclose(fp);
  if (linesize)
    free(line);

  if (status != 1)
    {
      free(gids);
      return NULL;
    }
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

/* Given a group line in s, record the gid in *gid and put a pointer to
 * the colon after the gid field into *p.  Return 0 on success, -1 on
 * failure. */
static int parse_to_gid(char *s, char **p, gid_t *gid)
{
  /* The format of the group line is:
   *   groupname:grouppassword:groupgid:username,username,...
   * Skip to the gid field. */
  s = strchr(s, ':');
  if (!s)
    return -1;
  s = strchr(s + 1, ':');
  if (!s)
    return -1;

  /* Advance one character and store the numeric value into *gid. */
  s++;
  *gid = atoi(s);

  /* Now advance to the next colon, makig sure that everything is a digit. */
  while (isdigit(*s))
    s++;
  if (*s != ':')
    return -1;
  *p = s;
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

  /* Close fp, checking for errors.  If everything is okay, move the temp
   * file into place. */
  status = ferror(fp);
  if (fclose(fp) == 0 && !status)
    status = rename(PATH_GROUP_TMP, PATH_GROUP);
  else
    status = -1;

  /* Unlock and close the group lock file. */
  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fcntl(fd, F_SETLKW, &fl);
  close(fd);
  return (status == -1) ? AL_WGROUP : AL_SUCCESS;
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
