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

static const char rcsid[] = "$Id: cleanup.c,v 2.22 1997-10-21 02:05:12 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <al.h>

#ifdef SOLARIS
#include <sys/param.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <kvm.h>
#include <fcntl.h>
#endif

#ifdef IRIX
#include <sys/procfs.h>
#include <fcntl.h>
#define PATH_PINFO "/proc/pinfo"
#endif

#ifdef HAVE_MASTER_PASSWD
#define PATH_PASSWD "/etc/master.passwd"
#else
#define PATH_PASSWD "/etc/passwd"
#endif

struct process {
  pid_t pid;
  uid_t uid;
};

static struct process *get_processes(int *nprocs);
static uid_t *get_passwd_uids(int *nuids);
static uid_t *cleanup_sessions(int *nuids);
static void kill_processes(struct process *plist, int nprocs,
			   uid_t *approved, int nuids);
static int uid_okay(uid_t uid, uid_t *approved, int nuids);
static int uidcomp(const void *p1, const void *p2);
static int read_line(FILE *fp, char **buf, int *bufsize);
static void *emalloc(size_t size);
static void *erealloc(void *old_ptr, size_t size);

int main(int argc, char **argv)
{
  int passwd = 0, nprocs, nuids;
  struct process *plist;
  uid_t *approved;

  if (argc == 2 && strcmp(argv[1], "-passwd") == 0)
    passwd = 1;
  else if (argc != 1 && (argc != 2 || strcmp(argv[1], "-loggedin") != 0))
    {
      fprintf(stderr, "Usage: %s [-passwd]\n", argv[0]);
      return 1;
    }

  /* Fetch the process list and list of approved uids.  Also cleans up
   * sessions if -passwd not specified. */
  plist = get_processes(&nprocs);
  if (passwd)
    approved = get_passwd_uids(&nuids);
  else
    approved = cleanup_sessions(&nuids);

  /* Sort the approved uid list for fast search. */
  qsort(approved, nuids, sizeof(uid_t), uidcomp);

  kill_processes(plist, nprocs, approved, nuids);
  return 0;
}

/* Retrieve a list of process pids and uids. */
static struct process *get_processes(int *nprocs)
{
  struct process *plist = emalloc(256 * sizeof(struct process));
  int psize = 256, n = 0;

#ifdef SOLARIS
  kvm_t *kv;
  struct proc *p;
  struct pid pid_buf;
  struct cred cred_buf;

  /* Open the kernel address space and prepare for process iteration. */
  kv = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
  if (kv == NULL)
    {
      fprintf(stderr, "Can't open kvm.\n");
      exit(1);
    }
  if (kvm_setproc(kv) == -1)
    {
      fprintf(stderr, "kvm_setproc failed.\n");
      exit(1);
    }

  while ((p = kvm_nextproc(kv)) != NULL)
    {
      kvm_read(kv, (unsigned long) p->p_pidp, (char *) &pid_buf,
	       sizeof(struct pid));
      kvm_read(kv, (unsigned long) p->p_cred, (char *) &cred_buf,
	       sizeof(struct cred));
      if (pid_buf.pid_id != 0)
	{
	  if (n == psize)
	    {
	      psize *= 2;
	      plist = erealloc(plist, psize * sizeof(struct process));
	    }
	  plist[n].pid = pid_buf.pid_id;
	  plist[n].uid = cred_buf.cr_uid;
	  n++;
	}
    }

  kvm_close(kv);
#endif

#ifdef IRIX
  DIR *procdir;
  struct dirent *entry;
  char *filename;
  int fd;
  prpsinfo_t pbuf;

  /* Open the process info directory. */
  procdir = opendir(PATH_PINFO);
  if (!procdir)
    {
      fprintf(stderr, "Can't open /proc/pinfo: %s\n", strerror(errno));
      exit(1);
    }

  /* Step through the directory and read off the processes. */
  while ((entry = readdir(procdir)) != NULL)
    {
      /* Skip the current and parent directory entries. */
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
	continue;

      /* Open the process file. */
      filename = emalloc(strlen(PATH_PINFO) + strlen(entry->d_name) + 2);
      sprintf(filename, "%s/%s", PATH_PINFO, entry->d_name);
      fd = open(filename, O_RDONLY, 0);
      free(filename);
      if (fd == -1)
	continue;

      /* Get the process information and record the pid and uid. */
      if (ioctl(fd, PIOCPSINFO, &pbuf) != -1 && pbuf.pr_pid != 0)
	{
	  if (n == psize)
	    {
	      psize *= 2;
	      plist = erealloc(plist, psize * sizeof(struct process));
	    }
	  plist[n].pid = pbuf.pr_pid;
	  plist[n].uid = pbuf.pr_uid;
	  n++;
	}
      close(fd);
    }
  closedir(procdir);
#endif

  *nprocs = n;
  return plist;
}

static uid_t *get_passwd_uids(int *nuids)
{
  FILE *fp;
  char *line;
  int linesize = 0, lines, n;
  const char *p;
  uid_t *uids;

  /* Open the passwd file. */
  fp = fopen(PATH_PASSWD, "r");
  if (!fp)
    {
      fprintf(stderr, "Can't open %s: %s\n", PATH_PASSWD, strerror(errno));
      exit (1);
    }

  /* Count lines in the file and allocate memory. */
  lines = 0;
  while (read_line(fp, &line, &linesize) == 0)
    lines++;
  uids = emalloc(lines * sizeof(uid_t));

  /* Reread the file, recording uids. */
  rewind(fp);
  n = 0;
  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Be paranoid and stop if the passwd file grew. */
      if (n >= lines)
	break;

      /* Find and record the uid. */
      p = strchr(line, ':');
      if (!p)
	continue;
      p = strchr(p + 1, ':');
      if (!p)
	continue;
      uids[n++] = atoi(p + 1);
    }

  if (linesize)
    free(line);
  fclose(fp);
  *nuids = n;
  return uids;
}

/* Clean up sessions and return a list of uids of users logged in. */
static uid_t *cleanup_sessions(int *nuids)
{
  uid_t *uids = emalloc(256 * sizeof(uid_t));
  int n = 0, uids_size = 256;
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  struct passwd *pwd;
  char *filename;

  dir = opendir(AL_PATH_SESSIONS);
  if (!dir)
    {
      fprintf(stderr, "Can't open directory %s: %s\n", AL_PATH_SESSIONS,
	      strerror(errno));
      exit(1);
    }

  /* Iterate over the files in the session directory. */
  while ((entry = readdir(dir)) != NULL)
    {
      /* If it's the current or parent directory, punt. */
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
	continue;

      /* Punt any defunct login processes for this user and revert the
       * account if there are any. */
      al_acct_cleanup(entry->d_name);

      /* Construct the filename for the session record and stat it. */
      filename = emalloc(strlen(AL_PATH_SESSIONS) + strlen(entry->d_name) + 2);
      sprintf(filename, "%s/%s", AL_PATH_SESSIONS, entry->d_name);
      if (stat(filename, &statbuf) == 0 && statbuf.st_size > 0)
	{
	  /* Looks like the user is logged in.  Get the uid and record it. */
	  pwd = getpwnam(entry->d_name);
	  if (pwd)
	    {
	      if (n == uids_size)
		{
		  uids_size *= 2;
		  uids = erealloc(uids, uids_size * sizeof(uid_t));
		}
	      uids[n++] = pwd->pw_uid;
	    }
	}
      free(filename);
    }

  closedir(dir);
  *nuids = n;
  return uids;
}

/* Kill unapproved processes. */
static void kill_processes(struct process *plist, int nprocs,
			   uid_t *approved, int nuids)
{
  int i, found = 0;

  /* Send a SIGHUP to any process not belonging to an approved uid. */
  for (i = 0; i < nprocs; i++)
    {
      if (!uid_okay(plist[i].uid, approved, nuids))
	{
	  if (kill(plist[i].pid, SIGHUP) == 0)
	    found = 1;
	}
    }

  /* If we didn't find any processes to kill, call it a day. */
  if (!found)
    return;

  /* Send a SIGKILL To any process not belonging to an approved uid. */
  sleep(5);
  for (i = 0; i < nprocs; i++)
    {
      if (!uid_okay(plist[i].uid, approved, nuids))
	kill(plist[i].pid, SIGKILL);
    }
}

/* Return nonzero if uid is approved according to the given list. */
static int uid_okay(uid_t uid, uid_t *approved, int nuids)
{
  /* Some guys get all the breaks. */
  if (uid == 0 || uid == 1)
    return 1;

  return (bsearch(&uid, approved, nuids, sizeof(uid_t), uidcomp) != NULL);
}

/* qsort/bsearch comparison function for pointer to uid_t. */
static int uidcomp(const void *p1, const void *p2)
{
  const uid_t *u1 = p1, *u2 = p2;

  return (*u1 > *u2) ? 1 : (*u1 == *u2) ? 0 : -1;
}

/* Read a line from a file.  Return 0 on success, -1 on error, 1 on EOF. */
static int read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*bufsize == 0)
    {
      *buf = emalloc(128);
      *bufsize = 128;
    }

  while (1)
    {
      if (!fgets(*buf + offset, *bufsize - offset, fp))
	return (offset != 0) ? 0 : (ferror(fp)) ? -1 : 1;
      len = offset + strlen(*buf + offset);
      if ((*buf)[len - 1] == '\n')
	  return 0;
      offset = len;

      /* Allocate more space. */
      newbuf = erealloc(*buf, *bufsize * 2);
      *buf = newbuf;
      *bufsize *= 2;
    }
}

static void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    {
      fprintf(stderr, "Can't malloc %lu bytes!\n", (unsigned long) size);
      exit(1);
    }
  return ptr;
}

static void *erealloc(void *old_ptr, size_t size)
{
  void *ptr;

  ptr = realloc(old_ptr, size);
  if (!ptr)
    {
      fprintf(stderr, "Can't realloc %lu bytes!\n", (unsigned long) size);
      exit(1);
    }
  return ptr;
}
