/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Copyright 1996 by the Massachusetts Institute of Technology.
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

static char rcsid[] = "$Id: install.c,v 1.5 1996-12-16 08:29:02 ghudson Exp $";

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h> 
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAXARGS 1024

#ifndef MAXBSIZE
#define MAXBSIZE 10240
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

static uid_t uid;
static gid_t gid;

static int docopy = 0;
static int dostrip = 0;
static int domove = 0;
static int dotime = 0;
static int multiple = 0;
static mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

static char *group;
static char *owner;
static char pathbuf[MAXPATHLEN];

static void install(const char *from_name, const char *to_name, int isdir);
static void copy(int from_fd, const char *from_name, int to_fd,
		 const char *to_name);
static void strip(const char *name);
static int isnumber(const char *s);
static int atoo(const char *str);
static void bad(const char *head, const char *str);
static void usage(void);

int main(int cmdline_argc, char **cmdline_argv)
{
  extern char *optarg;
  extern int optind;
  struct stat from_sb, to_sb;
  int ch, no_target;
  char *to_name;
  struct passwd *pp;
  struct group *gp;
  int argc = 1;
  char *args[MAXARGS], **argv = args;
  char *inst_env;

  /* Copy any command-line arguments from INSTOPT into argv. */
  inst_env = getenv("INSTOPT");
  if (inst_env)
    {
      while (isspace(*inst_env))
	inst_env++;
      while (*inst_env)
	{
	  argv[argc++] = inst_env;
	  while (*inst_env && !isspace(*inst_env))
	    inst_env++;
	  if (*inst_env)
	    *inst_env++ = 0;
	  while (isspace(*inst_env))
	    inst_env++;
	}
    }

  if (argc + cmdline_argc > MAXARGS)
    {
      fprintf(stderr, "install: too many command-line arguments.\n");
      return 1;
    }

  /* Copy the original arguments into argv. */
  argv[0] = *cmdline_argv++;
  while (--cmdline_argc)
    argv[argc++] = *cmdline_argv++;

  while ((ch = getopt(argc, argv, "cdstg:m:o:")) != EOF)
    {
      switch(ch)
	{
	case 'c':
	  docopy = 1;
	  break;
	case 'd':
	  domove = 1;
	  break;
	case 'g':
	  group = optarg;
	  break;
	case 'm':
	  mode = atoo(optarg);
	  break;
	case 'o':
	  owner = optarg;
	  break;
	case 's':
	  dostrip = 1;
	  break;
	case 't':
	  dotime = 1;
	  break;
	case '?':
	default:
	  usage();
	}
    }
  argc -= optind;
  argv += optind;
  if (argc < 2)
    usage();

  /* Check for multiple specifications of copy and move. */
  if (domove && docopy)
    {
      fprintf(stderr, "install: cannot specify both -c and -d\n");
      return 1;
    }

  /* If neither copy nor move specified, do copy. */
  if (!domove)
    docopy = 1;

  /* Get group and owner ids. */
  if (owner)
    {
      to_name = strchr(owner, '.');
      if (to_name)
	{
	  *to_name++ = '\0';
	  if (!group)
	    group = to_name;
	  else
	    {
	      fputs("install: multiple specification of the group\n", stderr);
	      return 1;
	    }
	}
      if (!isnumber(owner))
	{
	  pp = getpwnam(owner);
	  if (!pp)
	    {
	      fprintf(stderr, "install: unknown user %s.\n", owner);
	      return 1;
	    }
	  else
	    uid = pp->pw_uid;
	}
      else
	uid = atoi(owner);
    }
  else 
    uid = -1;

  if (group)
    {
      if (!isnumber(group))
	{
	  gp = getgrnam(group);
	  if (!gp)
	    {
	      fprintf(stderr, "install: unknown group %s.\n", group);
	      return 1;
	    }
	  else
	    gid = gp->gr_gid;
	}
      else 
	gid = atoi(group);
    }
  else 
    gid = -1;

  to_name = argv[argc - 1];
  no_target = stat(to_name, &to_sb);
  if (!no_target && S_ISDIR(to_sb.st_mode))
    {
      for (; *argv != to_name; argv++)
	install(*argv, to_name, 1);
      return 0;
    }

  /* can't do file1 file2 directory/file */
  if (argc != 2)
    usage();

  if (!no_target)
    {
      if (stat(*argv, &from_sb))
	{
	  fprintf(stderr, "install: can't find %s.\n", *argv);
	  return 1;
	}
      if (!S_ISREG(to_sb.st_mode))
	{
	  fprintf(stderr, "install: %s isn't a regular file.\n", to_name);
	  exit(1);
	}
      if (to_sb.st_dev == from_sb.st_dev && to_sb.st_ino == from_sb.st_ino)
	{
	  fprintf(stderr, "install: %s and %s are the same file.\n", *argv,
		  to_name);
	  return 1;
	}

      /* Unlink now, avoid ETXTBSY errors later. */
      unlink(to_name);
    }
  install(*argv, to_name, 0);
  return 0;
}

/* install -- build a path name and install the file */
static void install(const char *from_name, const char *to_name, int isdir)
{
  struct stat from_sb;
  struct timeval timep[2];
  int devnull, from_fd, to_fd;
  const char *c;

  /* If trying to install "/dev/null" to a directory, fail. */
  if (isdir || strcmp(from_name, "/dev/null"))
    {
      if (stat(from_name, &from_sb))
	{
	  fprintf(stderr, "install: can't find %s.\n", from_name);
	  exit(1);
	}
      if (!S_ISREG(from_sb.st_mode))
	{
	  fprintf(stderr, "install: %s isn't a regular file.\n", from_name);
	  exit(1);
	}

      /* Build the target path. */
      if (isdir)
	{
	  c = strrchr(from_name, '/');
	  c = (c) ? c + 1 : from_name;
	  sprintf(pathbuf, "%s/%s", to_name, c);
	}
      else
	strcpy(pathbuf, to_name);
      devnull = 0;
    }
  else
    devnull = 1;

  /* Unlink now, avoid ETXTBSY errors later. */
  unlink(pathbuf);

  /* Create target. */
  to_fd = open(pathbuf, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  if (to_fd < 0)
    {
      fprintf(stderr, "install: %s: %s\n", pathbuf, strerror(errno));
      exit(1);
    }
  if (!devnull)
    {
      from_fd = open(from_name, O_RDONLY, 0);
      if (from_fd < 0)
	{
	  unlink(pathbuf);
	  bad("install: open: ", from_name);
	  exit(1);
	}
      copy(from_fd, from_name, to_fd, pathbuf);
      close(from_fd);
      close(to_fd);
      if (dostrip)
	strip(pathbuf);
      if (!docopy)
	unlink(from_name);
    }
  if (dotime)
    {
      timep[0].tv_sec = from_sb.st_atime;
      timep[1].tv_sec = from_sb.st_mtime;
      timep[0].tv_usec = timep[1].tv_usec = 0;
      if (utimes(pathbuf, timep))
	bad("install: utimes", pathbuf);
    }
  /* set owner, group, mode. and time for target */
  if (chmod(pathbuf, mode))
    bad("install: fchmod", pathbuf);
  if ((uid != -1 || gid != -1))
    {
      uid = (uid != -1) ? uid : from_sb.st_uid;
      gid = (gid != -1) ? gid : from_sb.st_gid;
      if (chown(pathbuf, uid, gid) < 0)
	bad("install: chown: ", to_name);
    }
}

/* copy -- copy from one file to another */
static void copy(int from_fd, const char *from_name, int to_fd,
		 const char *to_name)
{
  int n;
  char buf[MAXBSIZE];

  while ((n = read(from_fd, buf, sizeof(buf))) > 0)
    {
      if (write(to_fd, buf, n) != n)
	bad("install: write: ", to_name);
    }
  if (n == -1)
      bad("install: read: ", from_name);
}

static void strip(const char *name)
{
  pid_t pid;

  fflush(NULL);
  pid = fork();
  if (pid == 0)
    {
      execlp(STRIP, STRIP, name, NULL);
      fprintf(stderr, "Cannot execute %s: %s\n", STRIP, strerror(errno));
      exit(1);
    }
  else if (pid > 0)
    {
      while (waitpid(pid, NULL, 0) < 0 && errno == EINTR)
	;
    }
}

/* isnumber -- determine whether string is a number */
static int isnumber(const char *s)
{
  while(*s)
    {
      if (!isdigit(*s))
	return 0;
      else
	s++;
    }
  return 1;
}

/* atoo -- octal string to int */
static int atoo(const char *str)
{
  int val;

  for (val = 0; isdigit(*str); ++str)
    val = val * 8 + *str - '0';
  return val;
}

/* bad -- remove created target and die */
static void bad(const char *head, const char *str)
{
  fprintf(stderr, "%s%s: %s\n", head, str, strerror(errno));
  unlink(pathbuf);
  exit(1);
}

/* usage -- print a usage message and die */
static void usage()
{
  fputs("usage: install [-cds] [-g group] [-m mode] [-o owner]"
	" file1 file2;\n\tor file1 ... fileN directory\n", stderr);
  exit(1);
}
