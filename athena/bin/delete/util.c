/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.c,v $
 * $Author: cfields $
 *
 * This program is a replacement for rm.  Instead of actually deleting
 * files, it marks them for deletion by prefixing them with a ".#"
 * prefix.
 *
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_util_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.c,v 1.29 1995-07-31 23:26:54 cfields Exp $";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#ifdef SYSV
#include <string.h>
#define index strchr
#define rindex strrchr
#else
#include <strings.h>
#endif /* SYSV */
#include <pwd.h>
#include <errno.h>
#ifdef AFS_MOUNTPOINTS
#include <sys/ioctl.h>
#include <afs/param.h>
#include <afs/vice.h>
#include <netinet/in.h>
#include <afs/venus.h>
#endif
#ifdef SOLARIS
#include <sys/ioccom.h>
#include <sys/stat.h>
#endif
#include "delete_errs.h"
#include "util.h"
#include "directories.h"
#include "mit-copying.h"
#include "errors.h"

extern char *getenv();
extern int errno;

#ifdef UTEK
extern int getuid();
#else /* ! UTEK */
extern uid_t getuid();
#endif /* UTEK */

char *convert_to_user_name(real_name, user_name)
char real_name[];
char user_name[];  /* RETURN */
{
     char *ptr, *q;
     
     (void) strcpy(user_name, real_name);
     while (ptr = strrindex(user_name, ".#")) {
	  for (q = ptr; *(q + 2); q++)
	       *q = *(q + 2);
	  *q = '\0';
     }
     return (user_name);
}

     



char *strindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr = str;
     while (ptr = index(ptr, *sub_str)) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr++;
     }
     return ((char *) NULL);
}



char *strrindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr;

     if (strlen(str))
	  ptr = &str[strlen(str) - 1];
     else
	  return((char *) NULL);
     while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     while (ptr != str) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr--;
	  while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     }
     if (! strncmp(ptr, sub_str, strlen(sub_str)))
	  return(str);
     else
	  return ((char *) NULL);
}
     
     
/*
 * NOTE: Append uses a static array, so its return value must be
 * copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
{
     static char buf[MAXPATHLEN];

     (void) strcpy(buf, filepath);
     if ((! *filename) || (! *filepath)) {
	  (void) strcpy(buf, filename);
	  return(buf);
     }
     if (buf[strlen(buf) - 1] == '/')
	  buf[strlen(buf) - 1] = '\0';
     if (strlen(buf) + strlen(filename) + 2 > MAXPATHLEN) {
	  set_error(ENAMETOOLONG);
	  strncat(buf, "/", MAXPATHLEN - strlen(buf) - 1);
	  strncat(buf, filename, MAXPATHLEN - strlen(buf) - 1);
	  error(buf);
 	  *buf = '\0';
	  return buf;
     }
     (void) strcat(buf, "/");
     (void) strcat(buf, filename);
     return buf;
}




yes() {
     char buf[BUFSIZ];
     char *val;
     
     val = fgets(buf, BUFSIZ, stdin);
     if (! val) {
	  printf("\n");
	  exit(1);
     }
     if (! index(buf, '\n')) do
	  (void) fgets(buf + 1, BUFSIZ - 1, stdin);
     while (! index(buf + 1, '\n'));
     return(*buf == 'y');
}




char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}




char *firstpart(filename, rest)
char *filename;
char *rest; /* RETURN */
{
     char *part;
     static char buf[MAXPATHLEN];

     (void) strcpy(buf, filename);
     part = index(buf, '/');
     if (! part) {
	  *rest = '\0';
	  return(buf);
     }
     (void) strcpy(rest, part + 1);
     *part = '\0';
     return(buf);
}





get_home(buf)
char *buf;
{
     char *user;
     struct passwd *psw;
     
     (void) strcpy(buf, getenv("HOME"));
     
     if (*buf)
	  return(0);

     user = getenv("USER");
     psw = getpwnam(user);

     if (psw) {
	  (void) strcpy(buf, psw->pw_dir);
	  return(0);
     }
     
     psw = getpwuid((int) getuid());

     if (psw) {
	  (void) strcpy(buf, psw->pw_dir);
	  return(0);
     }

     set_error(NO_HOME_DIR);
     error("get_home");
     return error_code;
}




timed_out(file_ent, current_time, min_days)
filerec *file_ent;
time_t current_time, min_days;
{
     if ((current_time - file_ent->specs.st_chtime) / 86400 >= min_days)
	  return(1);
     else
	  return(0);
}



int directory_exists(dirname)
char *dirname;
{
     struct stat stat_buf;

     if (stat(dirname, &stat_buf))
	  return(0);
     else if ((stat_buf.st_mode & S_IFMT) == S_IFDIR)
	  return(1);
     else
	  return(0);
}



is_link(name, oldbuf)
char *name;
struct stat *oldbuf;
{
#ifdef S_IFLNK
     struct stat statbuf;

     if (oldbuf)
	  statbuf = *oldbuf;
     else if (lstat(name, &statbuf) < 0) {
	  set_error(errno);
	  error("is_link");
	  return(0);
     }

     if ((statbuf.st_mode & S_IFMT) == S_IFLNK)
	  return 1;
     else
#endif
	  return 0;
}



/*
 * This is one of the few procedures that is allowed to break the
 * rule of always returning an error value if an error occurs.  That's
 * because it's stupid to expect a boolean function to do that, and
 * because defaulting to not being a mountpoint if there is an error
 * is a reasonable thing to do.
 */
/*
 * The second parameter is optional -- if it is non-NULL, it is
 * presumed to be a stat structure for the file being passed in.
 */
int is_mountpoint(name, oldbuf)
char *name;
struct stat *oldbuf;
{
     struct stat statbuf;
     dev_t device;
     char buf[MAXPATHLEN];
#ifdef AFS_MOUNTPOINTS
     struct ViceIoctl blob;
     char retbuf[MAXPATHLEN];
     int retval;
     char *shortname;
#endif

     /* First way to check for a mount point -- if the device number */
     /* of name is different from the device number of name/..       */
     if (oldbuf)
	  statbuf = *oldbuf;
     else if (lstat(name, &statbuf) < 0) {
	  set_error(errno);
	  error(name);
	  return 0;
     }

     device = statbuf.st_dev;

     if (strlen(name) + 4 /* length of "/.." + a NULL */ > MAXPATHLEN) {
	  set_error(ENAMETOOLONG);
	  error(name);
	  return 0;
     }

     strcpy(buf, name);
     strcat(buf, "/..");
     if (lstat(buf, &statbuf) < 0) {
	  set_error(errno);
	  error(name);
	  return 0;
     }

     if (statbuf.st_dev != device)
	  return 1;

#ifdef AFS_MOUNTPOINTS
     /* Check for AFS mountpoint using the AFS pioctl call. */
     if ((shortname = lastpart(name)) == name) {
	  strcpy(buf, ".");
	  blob.in = name;
	  blob.in_size = strlen(name) + 1;
	  blob.out = retbuf;
	  blob.out_size = MAXPATHLEN;
	  memset(retbuf, 0, MAXPATHLEN);
     }
     else {
	  strncpy(buf, name, shortname - name - 1);
	  buf[shortname - name - 1] = '\0';
	  if (*buf == '\0')
	       strcpy(buf, "/");
	  blob.in = shortname;
	  blob.in_size = strlen(shortname) + 1;
	  blob.out = retbuf;
	  blob.out_size = MAXPATHLEN;
	  memset(retbuf, 0, MAXPATHLEN);
     }

     retval = pioctl(buf, VIOC_AFS_STAT_MT_PT, &blob, 0);

     if (retval == 0) {
#ifdef DEBUG
	  printf("%s is an AFS mountpoint, is_mountpoint returning true.\n",
		 name);
#endif
	  return 1;
     }
     else {
	  if (errno != EINVAL) {
	       set_error(errno);
	       error(name);
	  }
     }
#endif /* AFS_MOUNTPOINTS */

     return 0;
}

#ifdef MALLOC_DEBUG
char *Malloc(size)
unsigned size;
{
     extern char *malloc();

     static int count = 0;
     char buf[10];
     
     count++;

     fprintf(stderr, "This is call number %d to Malloc, for %u bytes.\n",
	     count, size);
     fprintf(stdout, "Shall I return NULL for this malloc? ");
     fgets(buf, 10, stdin);
     if ((*buf == 'y') || (*buf == 'Y')) {
	  errno = ENOMEM;
	  return ((char *) NULL);
     }
     else
	  return (malloc(size));
}
#endif

	  
