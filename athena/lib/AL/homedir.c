/**********************************************************************
 *  homedir.c -- attach or create home directory
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

/* get POSIX-compliant WEXITSTATUS() macro, not old BSD-style */
#if defined(_AIX) || defined(hpux)
#undef _BSD
#endif /* defined(_AIX) || defined(hpux) */
#include <AL/AL.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#ifdef ultrix
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/fs_types.h>
#endif
#ifdef SOLARIS
#include <sys/mkdev.h>
#endif
#ifdef hpux
#define seteuid(x) setresuid(-1, (x), -1)
#endif /* hpux */
#ifdef linux
#include <sys/vfs.h>
#include <linux/nfs_fs.h>
#include <sys/sysmacros.h>
#endif
#ifdef __osf__
#include <sys/mount.h>
#endif
#ifdef sgi
#include <sys/statfs.h>
#include <sys/sysmacros.h>
#endif

/* Function Name: ALisRemoteDir
 * Arguments: dir - name of the directory.
 * Returns: true or false to the question (is remote dir).
 *    false may also indicate that no directory exists.
 *
 * If we cannot stat the directory, we will assume the directory is
 * remote.  Getting information about a directory may not be possible
 * if the pre-requisite authentication has not yet been performed.
 *
 * Under AIX, we use stat and check the FS_REMOTE flag.
 * Under Ultrix, we use statfs to determine the filesystem type.
 * Under BSD, we check the device [0,1=AFS; 255,0=NFS].
 *
 * NOTE: This routine must be CHANGED whenever a new architecture
 * is introduced or if any filesystem semantics change.
 */

int
ALisRemoteDir(dir)
char *dir;
{
#ifdef _AIX
#define REMOTEDONE
    struct stat stbuf;

    if (statx(dir, &stbuf, 0, STX_NORMAL))
	return(1);
    return((stbuf.st_flag & FS_REMOTE) ? 1 : 0);
#endif

#if defined(__osf__)
#define REMOTEDONE
    struct statfs sbuf;

    if (statfs(dir, &sbuf, sizeof(sbuf)) < 0)
	  return(1);

    switch (sbuf.f_type) {
/*    case MOUNT_NONE: --- afs uses this */
    case MOUNT_UFS:
    case MOUNT_MFS:
    case MOUNT_PC:
    case MOUNT_S5FS:
    case MOUNT_CDFS:
    case MOUNT_PROCFS:
    case MOUNT_MSFS:
	return(0);
    }

    return(1);
#endif /* osf */

#ifdef ultrix
#define REMOTEDONE
    struct fs_data sbuf;

    if (statfs(dir, &sbuf) < 0)
	return(1);

    switch(sbuf.fd_req.fstype) {
    case GT_ULTRIX:
    case GT_CDFS:
	return(0);
    }
    return(1);
#endif

#if (defined(hpux) || defined(sun) || defined(sgi) || defined(__NetBSD__) || defined(linux)) && !defined(REMOTEDONE)
#define REMOTEDONE
#if defined(hpux)
#undef major
#define major(x) ((int)(((unsigned)(x)>>24)&0377))
#define NFS_MAJOR 0xff
#endif
#if defined(sun) || defined(sgi)
#define NFS_MAJOR 130
#endif
#ifdef linux
#define NFS_MAJOR 0
#define AFS_MAJOR 0x01ff
#endif
#ifndef AFS_MAJOR
#define AFS_MAJOR 0x0001
#endif

    struct stat stbuf;
  
    if (stat(dir, &stbuf))
	return(1);

    if (major(stbuf.st_dev) == NFS_MAJOR)
	return(1);
    if (stbuf.st_dev == AFS_MAJOR)			/* AFS */
	return(1);

    return(0);
#endif

#ifndef REMOTEDONE
    ERROR --- ROUTINE NOT IMPLEMENTED ON THIS PLATFORM;
#endif
}


/* Function Name: ALhomedirOK
 * Description: checks to see if our homedir is okay, i.e. exists and 
 *	contains at least 1 file
 * Arguments: dir - the directory to check.
 * Returns: 1 if the homedir is okay.
 */

int
ALhomedirOK(dir)
char *dir;
{
    DIR *dp;
    struct dirent *temp;
    int count;

    if ((dp = opendir(dir)) == NULL)
      return(0);

    /* Make sure that there is something here besides . and .. */
    for (count = 0; count < 3 ; count++)
      temp = readdir(dp);

    closedir(dp);
    return(temp != NULL);
}

long
ALgetHomedir(ALsession session)
{
  long warning = ALwarnTempDir;
  struct stat stb;
  int i, attach_state= -1;

  /* Delete empty directory if it exists.  We just try to rmdir the
   * directory, and if it's not empty that will fail.
   */
  rmdir(ALpw_dir(session));

  /* If a good local homedir exists, use it. (Such a homedir need not
   * necessarily be owned by the user logging in, as it may be some
   * sort of a captive account or who knows what.) */
  if (ALfileExists(ALpw_dir(session)) && !ALisRemoteDir(ALpw_dir(session)) &&
      ALhomedirOK(ALpw_dir(session)))
    return 0L;

  /* Using homedir already there that may or may not be good. */
  if (ALisTrue(session, ALhaveNOATTACH) &&
      ALfileExists(ALpw_dir(session)) &&
      ALhomedirOK(ALpw_dir(session)))
    return ALwarnNOATTACH;

  if (ALisTrue(session, ALhaveNOATTACH))
    return ALerrNOATTACH;

  /* attempt attach now */
  switch (ALattach_pid(session) = fork()) {
  case -1:
    ALreturnError(session, (long) errno, "while forking");
  case 0:
    if (setuid(ALpw_uid(session)) != 0)
      {
	/* can't return warning 'cause we're the child */
	fprintf(stderr, "Could not execute attach command as user %s,\n",
		ALpw_name(session));
	fprintf(stderr, "Filesystem mappings may be incorrect.\n");
      }
    /* don't do zephyr here since user doesn't have zwgc started anyway */
    if (ALisTrue(session, ALhaveAuthentication))
      execl("/bin/athena/attach",
	    "attach", "-quiet", "-nozephyr", ALpw_name(session), NULL);
    else
      execl("/bin/athena/attach",
	    "attach", "-quiet", "-nomap", "-nozephyr",
	    ALpw_name(session), NULL);

    _exit(-1);
  default:
    break;
  }

  /* wait for attach to finish */
  waitpid(ALattach_pid(session), &attach_state, 0);

  /* If we didn't get Permission Denied because of no authentication yet,
     and if we got nonzero status from attach or no homedir, then... */
  if ((ALisTrue(session, ALhaveAuthentication) ||
       WEXITSTATUS(attach_state) != 26) &&
      (attach_state != 0 || !ALfileExists(ALpw_dir(session))))
    {
      /* do tempdir here */
      char *buf;
      pid_t pid;
      int state;

      /* make name of temporary directory */
      buf = malloc(strlen(ALpw_name(session)) + sizeof("HOME=/tmp/") + 1);
      if (buf == NULL)
	return ALerrNoMem;
      sprintf(buf, "HOME=/tmp/%s", ALpw_name(session));
      strcpy(ALpw_dir(session), buf+5);

      /* make HOME point to the right homedir */
      putenv(buf);

      /* If all programs were using this library, we wouldn't need
	 to modify the passwd file.  As of 12/94, telnetd uses libAL
	 but login.krb doesn't, so login.krb wouldn't get the right
	 homedir and would log the user in with home="/" */
      ALmodifyLinesOfFile(session, "/etc/passwd", "/etc/ptmp",
			  ALmodifyRemoveUser,
			  ALmodifyAppendPasswd);

      i = lstat(ALpw_dir(session), &stb);
      if (i == 0)
	{
	  /* Existing temporary home directory is only acceptable if
	     it's actually owned by the user and it's a directory.
	     Otherwise, blow it away. */
	  if ((stb.st_mode & S_IFMT) == S_IFDIR)
	    {
	      if (stb.st_uid == ALpw_uid(session))
		ALreturnError(session, ALwarnTempDirExists, ALpw_dir(session));

	      switch (pid = fork()) {
	      case -1:
		ALreturnError(session, (long) errno, "while forking");
	      case 0:
		execl("/bin/rm", "rm", "-rf", ALpw_dir(session), NULL);
		fprintf(stderr, "Warning - could not remove old temporary directory.\n");
		_exit(-1);
	      default:
		break;
	      }

	      /* wait for rm to finish */
	      waitpid(pid, &state, 0);
	      /* Failure can be caught at mkdir. */
	    }
	  else
	    unlink(ALpw_dir(session));
	}
      else
	if (errno != ENOENT)
	  ALreturnError(session, (long) errno, ALpw_dir(session));

      if (seteuid(ALpw_uid(session)) != 0)
	warning = ALwarnUidCopy;

      /* We want to die even if the error is that the directory
	 already exists - because it shouldn't. */
      if (mkdir(ALpw_dir(session), ALtempDirPerm))
	ALreturnError(session, (long) errno, ALpw_dir(session));

      switch (pid = fork()) {
      case -1:
	ALreturnError(session, (long) errno, "while forking");
      case 0:
	execl("/bin/cp", "cp", "-r", ALtempDotfiles, ALpw_dir(session), NULL);
	fprintf(stderr, "Warning - could not copy user prototype files into temporary directory.\n");
	_exit(-1);
      default:
	break;
      }

      /* wait for copy to finish */
      waitpid(pid, &state, 0);

      /* set effective uid back to root */
      seteuid(0);

      /* remember we created a temporary homedir */
      ALflagSet(session, ALdidCreateHomedir);

      ALreturnError(session, warning, "");
  } else
    ALflagSet(session, ALdidAttachHomedir);

  return 0L;
}
