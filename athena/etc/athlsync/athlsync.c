/* Copyright 2002 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: athlsync.c,v 1.1.2.1 2002-12-09 22:14:21 ghudson Exp $";

/* athlsync - Sync a locker's contents to local disk */

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <afs/venus.h>

static void walk(const char *srcpath, const char *destpath,
		 unsigned long *space);
static void walk_recurse(const char *srcpath, const char *destpath,
			 unsigned long *space);
static void compare(const char *srcpath, const char *destpath,
		    struct stat *srcstat, unsigned long *space);
static void sync_symlink(const char *srcpath, const char *destpath,
			 struct stat *srcstat);
static void sync_dir(const char *srcpath, const char *destpath,
		     struct stat *srcstat);
static void sync_file(const char *srcpath, const char *destpath,
		      struct stat *srcstat);
static void make_sys_symlink(const char *dir, const char *name);
static int check_existing_sys_link(const char *path, int ind);
static int find_sysname(const char *name, int namelen);
static void set_stats(const char *path, struct stat *st);
static void set_times(const char *path, struct stat *st);
static void nuke(const char *path);
static int is_backup_mountpoint(const char *parent, const char *name,
				struct stat *st);
static void get_sysnames(void);
static int same_symlinks(const char *srcpath, const char *destpath);
static int same_stats(struct stat *st1, struct stat *st2);
static mode_t adjust_mode(mode_t mode);
static char *path_concat(const char *a, const char *b);
static void e_unlink(const char *path);
static void e_rmdir(const char *path);
static void e_symlink(const char *symval, const char *path);
static void e_lstat(const char *path, struct stat *st);
static void e_stat(const char *path, struct stat *st);
static void e_mkdir(const char *path, mode_t mode);
static int e_open(const char *path, int flags, mode_t mode);
static ssize_t e_read(int fd, void *buf, size_t count);
static ssize_t e_write(int fd, const void *buf, size_t count);
static void e_chown(const char *path, uid_t owner, uid_t group);
static void e_chmod(const char *path, mode_t mode);
static void e_utime(const char *path, struct utimbuf *buf);
static DIR *e_opendir(const char *path);
static int e_readlink(const char *path, char *buf, size_t bufsize);
static void *e_malloc(size_t size);
static char *e_strdup(const char *str);
static void usage(void);
static void die(const char *fmt, ...);

static char **sysnames = NULL;

int main(int argc, char **argv)
{
  int c, space_flag = 0;
  unsigned long space = 0;
  const char *source, *dest;

  while ((c = getopt(argc, argv, "s")) != EOF)
    {
      switch (c)
	{
	case 's':
	  space_flag = 1;
	  break;
	default:
	  usage();
	  break;
	}
    }
  argc -= optind;
  argv += optind;
  if (argc != 2)
    usage();
  source = argv[0];
  dest = argv[1];

  /* Grab the AFS sysname into a global variable, for later use in
   * symlink adjustment.
   */
  get_sysnames();

  /* Perform the synchronization or space check. */
  walk(source, dest, (space_flag) ? &space : NULL);
  if (space_flag)
    printf("%lu\n", space / 2);

  return 0;
}

static void walk(const char *srcpath, const char *destpath,
		 unsigned long *space)
{
  struct stat srcstat;

  /* Allow the top level of srcpath to be a symlink. */
  e_stat(srcpath, &srcstat);

  /* Create the top level of destpath if necessary. */
  compare(srcpath, destpath, &srcstat, space);

  walk_recurse(srcpath, destpath, space);
}

static void walk_recurse(const char *srcpath, const char *destpath,
			 unsigned long *space)
{
  DIR *dir;
  struct dirent *d;
  struct stat srcstat;
  char *child_srcpath, *child_destpath;

  /* Recursively compare srcpath against destpath. */
  dir = e_opendir(srcpath);
  while ((d = readdir(dir)) != NULL)
    {
      if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
	continue;
      child_srcpath = path_concat(srcpath, d->d_name);
      child_destpath = path_concat(destpath, d->d_name);
      e_lstat(child_srcpath, &srcstat);
      if (!is_backup_mountpoint(srcpath, d->d_name, &srcstat))
	{
	  compare(child_srcpath, child_destpath, &srcstat, space);
	  if (space == NULL)
	    make_sys_symlink(destpath, d->d_name);
	  if (S_ISDIR(srcstat.st_mode))
	    walk_recurse(child_srcpath, child_destpath, space);
	}
      free(child_srcpath);
      free(child_destpath);
    }
  closedir(dir);

  /* If this is not a dry run, remove anything in destpath which isn't
   * in srcpath.
   */
  if (space != NULL)
    return;
  dir = e_opendir(destpath);
  while ((d = readdir(dir)) != NULL)
    {
      if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
	continue;
      child_srcpath = path_concat(srcpath, d->d_name);
      child_destpath = path_concat(destpath, d->d_name);
      if (lstat(child_srcpath, &srcstat) != 0)
	nuke(child_destpath);
      free(child_srcpath);
      free(child_destpath);
    }
  closedir(dir);
}

static void compare(const char *srcpath, const char *destpath,
		    struct stat *srcstat, unsigned long *space)
{
  struct stat deststat;

  /* Check if destpath looks sufficiently like srcpath, by comparing
   * symlink values or stat information as appropriate.
   */
  if (S_ISLNK(srcstat->st_mode) && same_symlinks(srcpath, destpath))
    return;
  if (S_ISREG(srcstat->st_mode) && lstat(destpath, &deststat) == 0
      && same_stats(srcstat, &deststat))
    return;
  if (S_ISDIR(srcstat->st_mode) && lstat(destpath, &deststat) == 0
      && S_ISDIR(deststat.st_mode))
    {
      /* At most we need to adjust the mode and ownership. */
      if (space == NULL
	  && (adjust_mode(srcstat->st_mode) != deststat.st_mode
	      || srcstat->st_uid != deststat.st_uid
	      || srcstat->st_gid != deststat.st_gid))
	set_stats(destpath, srcstat);
      return;
    }

  /* They're not the same; take appropriate action. */
  if (space != NULL)
    *space += srcstat->st_blocks;
  else if (S_ISLNK(srcstat->st_mode))
    sync_symlink(srcpath, destpath, srcstat);
  else if (S_ISDIR(srcstat->st_mode))
    sync_dir(srcpath, destpath, srcstat);
  else if (S_ISREG(srcstat->st_mode))
    sync_file(srcpath, destpath, srcstat);
}

static void sync_symlink(const char *srcpath, const char *destpath,
			 struct stat *srcstat)
{
  char srcval[1024];
  int len;

  nuke(destpath);
  len = e_readlink(srcpath, srcval, sizeof(srcval) - 1);
  srcval[len] = '\0';
  e_symlink(srcval, destpath);
}

static void sync_dir(const char *srcpath, const char *destpath,
		     struct stat *srcstat)
{
  nuke(destpath);
  e_mkdir(destpath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  set_stats(destpath, srcstat);
}

static void sync_file(const char *srcpath, const char *destpath,
		      struct stat *srcstat)
{
  int srcfd, destfd, count, n;
  char buf[8192];

  nuke(destpath);
  srcfd = e_open(srcpath, O_RDONLY, 0);
  destfd = e_open(destpath, O_RDWR | O_CREAT,
		  S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
  set_stats(destpath, srcstat);
  while ((count = e_read(srcfd, buf, sizeof(buf))) != 0)
    {
      n = 0;
      while (n < count)
	n += e_write(destfd, buf + n, count - n);
    }
  close(srcfd);
  close(destfd);
  set_times(destpath, srcstat);
}

/* If we see a path element ending in one of our AFS sysnames, make a
 * symlink so that @sys references to that element can still work in
 * the local copy.
 */
static void make_sys_symlink(const char *dir, const char *name)
{
  int ind, namelen, syslen;
  char *subname, *path;

  /* Look for a sysname at the end of this path component. */
  namelen = strlen(name);
  ind = find_sysname(name, namelen);
  if (ind == -1)
    return;

  /* Construct subname by substituting @sys for the sysname. */
  syslen = strlen(sysnames[ind]);
  subname = e_malloc(namelen - syslen + 4 + 1);
  memcpy(subname, name, namelen - syslen);
  strcpy(subname + namelen - syslen, "@sys");

  /* Make a symlink at subname, if nothing better is in the way. */
  path = path_concat(dir, subname);
  if (!check_existing_sys_link(path, ind))
    symlink(name, path);
  free(path);
  free(subname);
}

/* Check if there is something in the way of making an @sys symlink.
 * If there's something there and it should stay, return 1.
 * Otherwise, blow away what's there and return 0.
 */
static int check_existing_sys_link(const char *path, int ind)
{
  struct stat st;
  char linkval[1024];
  int len, xind;

  /* If nothing's there, we're all set. */
  if (lstat(path, &st) != 0)
    return 0;

  /* If there's a non-link there, leave it alone. */
  if (!S_ISLNK(st.st_mode))
    return 1;

  /* See if the link points to something ending in a sysname. */
  len = e_readlink(path, linkval, sizeof(linkval) - 1);
  linkval[len] = 0;
  xind = find_sysname(linkval, len);

  /* If it points to something equally good (earlier in the sysname
   * list is better), which still exists, leave it.
   */
  if (xind != -1 && xind <= ind && stat(path, &st) == 0)
    return 1;

  /* Otherwise, blow it away. */
  unlink(path);
  return 0;
}

static int find_sysname(const char *name, int namelen)
{
  int syslen, i;

  if (sysnames == NULL)
    return -1;
  for (i = 0; sysnames[i]; i++)
    {
      syslen = strlen(sysnames[i]);
      if (namelen >= syslen
	  && strcmp(name + namelen - syslen, sysnames[i]) == 0)
	return i;
    }
  return -1;
}

static void set_stats(const char *path, struct stat *st)
{
  e_chown(path, st->st_uid, st->st_gid);
  e_chmod(path, adjust_mode(st->st_mode & ~S_IFMT));
}

static void set_times(const char *path, struct stat *st)
{
  struct utimbuf buf;

  buf.actime = st->st_atime;
  buf.modtime = st->st_mtime;
  e_utime(path, &buf);
}

/* Remove a file or directory. */
static void nuke(const char *path)
{
  DIR *dir;
  struct dirent *d;
  struct stat st;
  char *childpath;

  /* If it doesn't exist, we're done. */
  if (lstat(path, &st) != 0)
    return;

  /* If it's not a directory, just unlink it. */
  if (!S_ISDIR(st.st_mode))
    {
      e_unlink(path);
      return;
    }

  /* It's a directory; recursively nuke all of its children. */
  dir = e_opendir(path);
  while ((d = readdir(dir)) != NULL)
    {
      if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
	continue;
      childpath = path_concat(path, d->d_name);
      nuke(childpath);
      free(childpath);
    }
  closedir(dir);

  e_rmdir(path);
}

static int is_backup_mountpoint(const char *parent, const char *name,
				struct stat *st)
{
  struct ViceIoctl blob;
  static char mountpoint[2048];
  int len;

  /* Directories with even-numbered inodes are mountpoints, assuming
   * they're in AFS at all.  Anything else is not.
   */
  if (!S_ISDIR(st->st_mode) || (st->st_ino & 1) != 0)
    return 0;

  /* Check that it really is a mountpoint and that the target volume
   * name ends with ".backup".
   */
  blob.in = (char *) name;
  blob.in_size = strlen(name) + 1;
  blob.out = mountpoint;
  blob.out_size = sizeof(mountpoint);
  memset(mountpoint, 0, sizeof(mountpoint));
  if (pioctl(parent, VIOC_AFS_STAT_MT_PT, &blob, 1) == 0)
    {
      len = strlen(mountpoint);
      if (len > 7 && strcmp(mountpoint + len - 7, ".backup") == 0)
	return 1;
    }
  return 0;
}

/* Stash the AFS sysnames in a global variable, for later use in
 * manipulating symlink values.
 */
static void get_sysnames(void)
{
  afs_int32 nitems = 0, i;
  struct ViceIoctl blob;
  char buf[2048];
  const char *p;

  /* The same pioctl is used to set and get the sysname.  A zero
   * value in nitems says not to set it.
   */
  blob.in = (void *) &nitems;
  blob.in_size = sizeof(nitems);
  blob.out = buf;
  blob.out_size = sizeof(buf);
  memset(buf, 0, sizeof(buf));

  if (pioctl(0, VIOC_AFS_SYSNAME, &blob, 1) != 0)
    return;
  
  memcpy(&nitems, buf, sizeof(nitems));
  sysnames = e_malloc((nitems + 1) * sizeof(char *));
  p = buf + sizeof(nitems);
  for (i = 0; i < nitems; i++)
    {
      sysnames[i] = e_strdup(p);
      p += strlen(p) + 1;
    }
  sysnames[nitems] = NULL;
}

/* Check if srcpath and destpath have the same symlink value,
 * modulo @sys substitution.
 */
static int same_symlinks(const char *srcpath, const char *destpath)
{
  char srcval[1024], destval[1024];
  int srclen, destlen;

  srclen = e_readlink(srcpath, srcval, sizeof(srcval));
  destlen = readlink(destpath, destval, sizeof(destval));
  return (srclen == destlen && memcmp(srcval, destval, srclen) == 0);
}

/* Check if two files seem to be the same. */
static int same_stats(struct stat *st1, struct stat *st2)
{
  return (adjust_mode(st1->st_mode) == st2->st_mode
	  && st1->st_uid == st2->st_uid
	  && st1->st_gid == st2->st_gid
	  && st1->st_size == st2->st_size
	  && st1->st_mtime == st2->st_mtime);
}

/* Return mode adjusted for a copy from AFS to local disk, by turning
 * on the read bits and possibly the executable bits for "group" and
 * "other" access.
 */
static mode_t adjust_mode(mode_t mode)
{
  mode_t rmask = S_IRGRP | S_IROTH;
  mode_t xmask = (mode & S_IXUSR) ? (S_IXGRP | S_IXOTH) : 0;

  return mode | rmask | xmask;
}

/* Concatenate two paths, making sure there is a / between them. */
static char *path_concat(const char *a, const char *b)
{
  int alen = strlen(a), blen = strlen(b);
  char *result;

  result = e_malloc(alen + blen + 2);
  strcpy(result, a);
  if ((alen == 0 || a[alen - 1] != '/') && b[0] != '/')
    result[alen++] = '/';
  strcpy(result + alen, b);
  return result;
}

/* These are error-checking wrappers around system calls and standard
 * library functions.
 */

static void e_unlink(const char *path)
{
  if (unlink(path) != 0)
    die("Can't unlink %s: %s", path, strerror(errno));
}

static void e_rmdir(const char *path)
{
  if (rmdir(path) != 0)
    die("Can't rmdir %s: %s", path, strerror(errno));
}

static void e_symlink(const char *symval, const char *path)
{
  if (symlink(symval, path) != 0)
    die("Can't make symlink to %s at %s: %s", symval, path, strerror(errno));
}

static void e_lstat(const char *path, struct stat *st)
{
  if (lstat(path, st) != 0)
    die("Can't lstat %s: %s", path, strerror(errno));
}

static void e_stat(const char *path, struct stat *st)
{
  if (stat(path, st) != 0)
    die("Can't stat %s: %s", path, strerror(errno));
}

static void e_mkdir(const char *path, mode_t mode)
{
  if (mkdir(path, mode) != 0)
    die("Can't make directory %s: %s", path, strerror(errno));
}

static int e_open(const char *path, int flags, mode_t mode)
{
  int fd;

  fd = open(path, flags, mode);
  if (fd == -1)
    {
      die("Can't open file %s for %s: %s", path,
	  (flags | O_RDWR) ? "writing" : "reading", strerror(errno));
    }
  return fd;
}

static ssize_t e_read(int fd, void *buf, size_t count)
{
  ssize_t result;

  result = read(fd, buf, count);
  if (result == -1)
    die("Read failure: %s", strerror(errno));
  return result;
}

static ssize_t e_write(int fd, const void *buf, size_t count)
{
  ssize_t result;

  result = write(fd, buf, count);
  if (result == -1)
    die("Write failure: %s", strerror(errno));
  return result;
}

static void e_chown(const char *path, uid_t owner, uid_t group)
{
  if (chown(path, owner, group) != 0)
    die("Can't chown %s: %s", path, strerror(errno));
}

static void e_chmod(const char *path, mode_t mode)
{
  if (chmod(path, mode) != 0)
    die("Can't chmod %s: %s", path, strerror(errno));
}

static void e_utime(const char *path, struct utimbuf *buf)
{
  if (utime(path, buf) != 0)
    die("Can't set times on %s: %s", path, strerror(errno));
}

static DIR *e_opendir(const char *path)
{
  DIR *dir;

  dir = opendir(path);
  if (dir == NULL)
    die("Can't open directory %s: %s", path, strerror(errno));
  return dir;
}

static int e_readlink(const char *path, char *buf, size_t bufsize)
{
  int len;

  len = readlink(path, buf, bufsize);
  if (len == -1)
    die("Can't readlink %s: %s", path, strerror(errno));
  return len;
}

static void *e_malloc(size_t size)
{
  void *p;

  p = malloc(size);
  if (p == NULL && size != 0)
    die("Can't allocate %lu bytes of memory", (unsigned long) size);
  return p;
}

static char *e_strdup(const char *str)
{
  char *result;

  result = e_malloc(strlen(str) + 1);
  strcpy(result, str);
  return result;
}

static void usage(void)
{
  die("Usage: athlsync [-s] source destination");
}

static void die(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(1);
}
