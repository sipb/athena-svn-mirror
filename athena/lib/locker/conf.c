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

/* This file is part of liblocker. It implements reading attach.conf. */

static const char rcsid[] = "$Id: conf.c,v 1.10 2006-08-08 21:50:09 ghudson Exp $";

#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com_err.h>
#include <hesiod.h>

#include "locker.h"
#include "locker_private.h"

extern struct locker_ops locker__err_ops, locker__mul_ops, locker__afs_ops;
extern struct locker_ops locker__nfs_ops, locker__ufs_ops, locker__loc_ops;

#define LOCKER_ALL_FSTYPES (-1L)

static void free_regexp_list(struct locker__regexp_list lst, int strings);

static int parse_bool(locker_context context, char *line, void *val);
static int parse_string(locker_context context, char *line, void *val);
static int parse_trusted_list(locker_context context, char *line, void *val);
static int parse_regex_list_yes(locker_context context, char *line, void *val);
static int parse_regex_list_no(locker_context context, char *line, void *val);
static int parse_regex_list(locker_context context, char *line,
			    struct locker__regexp_list *val, int flag);
static int parse_fs_list(locker_context context, char *line, void *val);
static int add_regexp(locker_context context, struct locker__regexp_list *val,
		      char *regexp);
static int parse_obsolete(locker_context context, char *line, void *val);

static struct opt_def {
  char *name;
  int (*parse)(locker_context, char *, void *);
  size_t offset;
} conf_options[] = {
  /* Boolean options */
  { "explicit", parse_bool, offsetof(struct locker_context, exp_desc) },
  { "explicit-mntpt", parse_bool, offsetof(struct locker_context, exp_mountpoint) },
  { "keep-mount", parse_bool, offsetof(struct locker_context, keep_mount) },
  { "nfs-root-hack", parse_bool, offsetof(struct locker_context, nfs_root_hack) },
  { "ownercheck", parse_bool, offsetof(struct locker_context, ownercheck) },
  { "use-krb4", parse_bool, offsetof(struct locker_context, use_krb4) },

  /* String options */
  { "afs-mount-dir", parse_string, offsetof(struct locker_context, afs_mount_dir) },
  { "attachtab", parse_string, offsetof(struct locker_context, attachtab) },
  { "local-dir", parse_string, offsetof(struct locker_context, local_dir) },
  { "nfs-mount-dir", parse_string, offsetof(struct locker_context, nfs_mount_dir) },

  /* Trusted user list */
  { "trusted", parse_trusted_list, offsetof(struct locker_context, trusted) },

  /* Regexp list options */
  { "allow", parse_regex_list_yes, offsetof(struct locker_context, allow) },
  { "noallow", parse_regex_list_no, offsetof(struct locker_context, allow) },
  { "mountpoint", parse_regex_list_yes, offsetof(struct locker_context, mountpoint) },
  { "nomountpoint", parse_regex_list_no, offsetof(struct locker_context, mountpoint) },
  { "setuid", parse_regex_list_yes, offsetof(struct locker_context, setuid) },
  { "suid", parse_regex_list_yes, offsetof(struct locker_context, setuid) },
  { "nosetuid", parse_regex_list_no, offsetof(struct locker_context, setuid) },
  { "nosuid", parse_regex_list_no, offsetof(struct locker_context, setuid) },

  /* Filesystem regexp and argument options */
  { "allowoptions", parse_fs_list, offsetof(struct locker_context, allowopts) },
  { "defoptions", parse_fs_list, offsetof(struct locker_context, defopts) },
  { "filesystem", parse_fs_list, offsetof(struct locker_context, filesystem) },
  { "options", parse_fs_list, offsetof(struct locker_context, reqopts) },

  /* Obsolete options */
  { "aklog", parse_obsolete, 0 },
  { "debug", parse_obsolete, 0 },
  { "fsck", parse_obsolete, 0 },
  { "mtab", parse_obsolete, 0 },
  { "ownerlist", parse_obsolete, 0 },
  { "verbose", parse_obsolete, 0 },
};

static int noptions = sizeof(conf_options) / sizeof(struct opt_def);

int locker_init(locker_context *contextp, uid_t user,
		locker_error_fun errfun, void *errdata)
{
  locker_context context;
  FILE *fp;
  char *buf = NULL, *p, *q;
  int i, size, status;

  context = *contextp = malloc(sizeof(struct locker_context));
  if (!context)
    {
      locker__error(context, "Out of memory reading attach.conf.\n");
      return LOCKER_ENOMEM;
    }

  /* Set defaults. */
  memset(context, 0, sizeof(struct locker_context));
  if (errfun)
    {
      context->errfun = errfun;
      context->errdata = errdata;
    }
  else
    context->errfun = NULL;
  context->nfs_root_hack = context->exp_desc = context->exp_mountpoint = 1;
  context->afs_mount_dir = strdup(LOCKER_AFS_MOUNT_DIR);
  context->attachtab = strdup(LOCKER_PATH_ATTACHTAB);
  context->local_dir = strdup(LOCKER_PATH_LOCAL);
  if (!context->afs_mount_dir || !context->attachtab)
    {
      locker__error(context, "Out of memory reading attach.conf.\n");
      locker_end(context);
      return LOCKER_ENOMEM;
    }
  context->nfs_mount_dir = NULL;
  context->user = user;
  if (context->user == 0)
    context->trusted = 1;
  context->zsubs = NULL;

  context->setuid.tab = NULL;
  context->setuid.defflag = 1;
  context->allow.tab = NULL;
  context->allow.defflag = 1;
  context->mountpoint.tab = NULL;
  context->mountpoint.defflag = 1;
  context->filesystem.tab = NULL;
  context->reqopts.tab = NULL;
  context->defopts.tab = NULL;
  context->allowopts.tab = NULL;

  /* Filesystem defaults. */
  context->nfstypes = 6;
  context->fstype = malloc(context->nfstypes * sizeof(struct locker_ops *));
  if (!context->fstype)
    {
      locker__error(context, "Out of memory reading attach.conf.\n");
      locker_end(context);
      return LOCKER_ENOMEM;
    }
  context->fstype[0] = &locker__err_ops;
  context->fstype[1] = &locker__mul_ops;
  context->fstype[2] = &locker__afs_ops;
  context->fstype[3] = &locker__nfs_ops;
  context->fstype[4] = &locker__ufs_ops;
  context->fstype[5] = &locker__loc_ops;
  for (i = 0; i < context->nfstypes; i++)
    context->fstype[i]->id = 1 << i;

  fp = fopen(LOCKER_PATH_ATTACH_CONF, "r");
  if (!fp)
    {
      locker__error(context, "%s while trying to open attach.conf.\n",
		    strerror(errno));
      locker_end(context);
      return LOCKER_EATTACHCONF;
    }

  while ((status = locker__read_line(fp, &buf, &size)) == 0)
    {
      if (!buf[0] || buf[0] == '#')
	continue;

      p = buf;
      while (isspace((unsigned char)*p))
	p++;
      q = p;
      while (*q && !isspace((unsigned char)*q))
	q++;

      for (i = 0; i < noptions; i++)
	{
	  if (!strncmp(conf_options[i].name, p, q - p) &&
	      !conf_options[i].name[q - p])
	    break;
	}
      if (i == noptions)
	{
	  locker__error(context, "Unrecognized attach.conf line:\n%s\n", buf);
	  locker_end(context);
	  free(buf);
	  fclose(fp);
	  return LOCKER_EATTACHCONF;
	}
      else
	{
	  while (*q && isspace((unsigned char)*q))
	    q++;
	  status = conf_options[i].parse(context, q,
					 ((char *)context +
					  conf_options[i].offset));
	}

      if (status)
	break;
    }

  free(buf);
  fclose(fp);

  if (status && status != LOCKER_EOF)
    {
      if (status == LOCKER_ENOMEM)
	locker__error(context, "Out of memory reading attach.conf.\n");
      else if (status == LOCKER_EFILE)
	{
	  locker__error(context, "Error reading attach.conf:\n%s\n",
			strerror(errno));
	  status = LOCKER_EATTACHCONF;
	}
      else
	locker__error(context, "Bad attach.conf line: \"%s\"\n", buf);

      locker_end(context);
      return status;
    }

  status = locker__canonicalize_path(context, LOCKER_CANON_CHECK_NONE,
				     &context->attachtab, NULL);
  if (status)
    {
      locker_end(context);
      return status;
    }

  /* Initialize Hesiod. */
  if (hesiod_init(&context->hes_context) != 0)
    {
      locker__error(context, "Could not create locker context:\n"
		    "%s while initializing Hesiod.\n", strerror(errno));
      locker_end(context);
      return LOCKER_EHESIOD;
    }

  return LOCKER_SUCCESS;
}

void locker_end(locker_context context)
{
  free(context->afs_mount_dir);
  free(context->attachtab);
  free(context->nfs_mount_dir);
  free(context->local_dir);
  free(context->fstype);
  free_regexp_list(context->setuid, 0);
  free_regexp_list(context->allow, 0);
  free_regexp_list(context->mountpoint, 0);
  free_regexp_list(context->filesystem, 1);
  free_regexp_list(context->reqopts, 1);
  free_regexp_list(context->defopts, 1);
  free_regexp_list(context->allowopts, 1);

  if (context->hes_context)
    hesiod_end(context->hes_context);
  locker__free_zsubs(context);

  free(context);
}

static void free_regexp_list(struct locker__regexp_list lst, int strings)
{
  int i;

  for (i = 0; i < lst.num; i++)
    {
      regfree(&(lst.tab[i].pattern));
      if (strings)
	free(lst.tab[i].data.string);
    }
  free(lst.tab);
}

static int parse_bool(locker_context context, char *line, void *val)
{
  int *boolval = val;

  /* Nothing = yes */
  if (!*line)
    {
      *boolval = 1;
      return LOCKER_SUCCESS;
    }

  if (!strcmp(line, "on"))
    *boolval = 1;
  else if (!strcmp(line, "off"))
    *boolval = 0;
  else
    {
      locker__error(context, "Unrecognized boolean value \"%s\".\n", line);
      return LOCKER_EATTACHCONF;
    }
  return LOCKER_SUCCESS;
}

static int parse_string(locker_context context, char *line, void *val)
{
  char **strval = val;

  free(*strval);
  *strval = strdup(line);

  if (!*strval)
    return LOCKER_ENOMEM;
  return LOCKER_SUCCESS;
}

static int parse_trusted_list(locker_context context, char *line, void *val)
{
  int *trusted = val;
  char *lasts = NULL;

  for (line = strtok_r(line, " \t", &lasts); line;
       line = strtok_r(NULL, " \t", &lasts))
    {
      if (isdigit((unsigned char)*line) && context->user == atoi(line))
	*trusted = 1;
      else
	{
	  struct passwd *pw;

	  pw = getpwnam(line);
	  if (!pw)
	    continue;
	  if (pw->pw_uid == context->user)
	    *trusted = 1;
	}
    }
  return LOCKER_SUCCESS;
}

static int parse_regex_list_yes(locker_context context, char *line, void *val)
{
  return parse_regex_list(context, line, val, 1);
}

static int parse_regex_list_no(locker_context context, char *line, void *val)
{
  return parse_regex_list(context, line, val, 0);
}

static int parse_regex_list(locker_context context, char *line,
			    struct locker__regexp_list *val, int flag)
{
  int status = LOCKER_SUCCESS;
  char *data, *dup = strdup(line), *lasts = NULL;

  if (!dup)
    return LOCKER_ENOMEM;
  for (data = strtok_r(dup, " \t", &lasts); data && !status;
       data = strtok_r(NULL, " \t", &lasts))
    {
      status = add_regexp(context, val, data);
      if (status == LOCKER_SUCCESS)
	val->tab[val->num++].data.flag = flag;
    }
  free(dup);
  return status;
}

static int parse_fs_list(locker_context context, char *line, void *val)
{
  int status;
  char *data = strdup(line), *lasts = NULL;
  struct locker__regexp_list *lst = val;

  if (!data)
    return LOCKER_ENOMEM;

  line = strtok_r(data, " \t", &lasts);
  if (!line || !lasts)
    return LOCKER_EATTACHCONF;
  for (line = lasts; isspace((unsigned char)*line); line++)
    ;

  status = add_regexp(context, lst, data);
  if (status == LOCKER_SUCCESS)
    {
      lst->tab[lst->num].data.string = strdup(line);
      if (!lst->tab[lst->num++].data.string)
	status = LOCKER_ENOMEM;
    }
  free(data);
  return status;
}

static int add_regexp(locker_context context, struct locker__regexp_list *lst,
		      char *regexp)
{
  int i, len, status;

  /* Extend the array if we ran out of slots. */
  if (lst->num >= lst->size - 1)
    {
      struct locker__regexp *new;
      int nsize;

      if (lst->size)
	nsize = 2 * lst->size;
      else
	nsize = 5;
      new = realloc(lst->tab, nsize * sizeof(struct locker__regexp));
      if (!new)
	return LOCKER_ENOMEM;
      /* Zero out the newly-allocated parts, but not the old parts. */
      memset(new + lst->size, 0,
	     (nsize - lst->size) * sizeof(struct locker__regexp));
      lst->size = nsize;
      lst->tab = new;
    }

  /* Check for {[-+]?^?fstype,fstype,...}: prefix. */
  if (*regexp == '{')
    {
      int invert = 0;

      if (*++regexp == '+')
	{
	  regexp++;
	  lst->tab[lst->num].explicit = LOCKER_EXPLICIT;
	}
      else if (*regexp == '-')
	{
	  regexp++;
	  lst->tab[lst->num].explicit = LOCKER_NOEXPLICIT;
	}
      else if (*regexp == '^')
	{
	  regexp++;
	  invert = 1;
	}

      while (*regexp != '}')
	{
	  for (i = 0; i < context->nfstypes; i++)
	    {
	      len = strlen(context->fstype[i]->name);
	      if (!strncasecmp(regexp, context->fstype[i]->name, len) &&
		  (regexp[len] == ',' || regexp[len] == '}'))
		break;
	    }
	  if (i == context->nfstypes)
	    {
	      len = strcspn(regexp, " ,}");
	      locker__error(context, "Unrecognized filesystem type "
			    "\"%.*s\".\n", len, regexp);
	      return LOCKER_EATTACHCONF;
	    }

	  lst->tab[lst->num].fstypes |= context->fstype[i]->id;
	  regexp += len;
	}
      if (invert)
	lst->tab[lst->num].fstypes ^= LOCKER_ALL_FSTYPES;

      /* Skip "}" or "}:". */
      regexp++;
      if (*regexp == ':')
	regexp++;
    }
  else
    lst->tab[lst->num].fstypes = LOCKER_ALL_FSTYPES;

  /* Now compile the normal regexp part. */
  status = regcomp(&(lst->tab[lst->num].pattern), regexp, REG_NOSUB);
  if (status)
    {
      char *errbuf;
      int size = regerror(status, &(lst->tab[lst->num].pattern), NULL, 0);
      errbuf = malloc(size);
      if (!errbuf)
	return LOCKER_ENOMEM;
      else
	{
	  regerror(status, &(lst->tab[lst->num].pattern), errbuf, size);
	  locker__error(context, "Could not parse regular expression "
			"\"%s\":%s.\n", regexp, errbuf);
	  return LOCKER_EATTACHCONF;
	}
    }

  return LOCKER_SUCCESS;
}

static int parse_obsolete(locker_context context, char *line, void *val)
{
  locker__error(context, "Ignoring obsolete attach.conf option: %s\n",
		line);
  return LOCKER_SUCCESS;
}

void locker__error(locker_context context, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  if (context && context->errfun)
    context->errfun(context->errdata, fmt, ap);
  else
    vfprintf(stderr, fmt, ap);
  va_end(ap);
}

int locker__fs_ok(locker_context context, struct locker__regexp_list list,
		  struct locker_ops *fs, char *filesystem)
{
  int i;

  for (i = 0; i < list.num; i++)
    {
      if (fs && !(list.tab[i].fstypes & fs->id))
	continue;

      if (regexec(&(list.tab[i].pattern), filesystem, 0, NULL, 0) == 0)
	return list.tab[i].data.flag;
    }

  return list.defflag;
}

char *locker__fs_data(locker_context context, struct locker__regexp_list list,
		      struct locker_ops *fs, char *filesystem)
{
  int i;

  for (i = 0; i < list.num; i++)
    {
      if (fs && !(list.tab[i].fstypes & fs->id))
	continue;

      if (regexec(&(list.tab[i].pattern), filesystem, 0, NULL, 0) == 0)
	return list.tab[i].data.string;
    }

  return NULL;
}

struct locker_ops *locker__get_fstype(locker_context context, char *type)
{
  int i, len;

  for (i = 0; i < context->nfstypes; i++)
    {
      len = strlen(context->fstype[i]->name);
      if (!strncasecmp(type, context->fstype[i]->name, len) &&
	  (!type[len] || isspace((unsigned char)type[len])))
	return context->fstype[i];
    }
  return NULL;
}
