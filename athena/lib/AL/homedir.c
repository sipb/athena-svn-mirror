/**********************************************************************
 *  homedir.c -- attach or create home directory
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    
#if (defined(hpux) || defined(vax) || defined(ibm032) || defined(sun)) && !defined(REMOTEDONE)
#define REMOTEDONE
#if defined(hpux)
#undef major
#define major(x) ((int)(((unsigned)(x)>>24)&0377))
#endif
#if defined(vax) || defined(ibm032) || defined(hpux)
#define NFS_MAJOR 0xff
#endif
#if defined(sun)
#define NFS_MAJOR 130
#endif
    struct stat stbuf;
  
    if (stat(dir, &stbuf))
	return(1);

    if (major(stbuf.st_dev) == NFS_MAJOR)
	return(1);
    if (stbuf.st_dev == 0x0001)			/* AFS */
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

  /* If a good local homedir exists, use it */
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

  if (attach_state != 0 || !ALfileExists(ALpw_dir(session)))
    {
      /* do tempdir here */
      char buf[BUFSIZ];
      pid_t cp_pid;
      int cp_state;

      /* make name of temporary directory */
      sprintf(buf, "/tmp/%s", ALpw_name(session));
      strcpy(ALpw_dir(session), buf);

      /* make sure /etc/passwd has the right homedir */
      ALmodifyLinesOfFile(session, "/etc/passwd", "/etc/ptmp",
			  ALmodifyRemoveUser,
			  ALmodifyAppendPasswd);
      /* printf("Tried to add %s to /etc/passwd\n", ALpw_dir(session)); */

      i = lstat(buf, &stb);
      if (i == 0)
	{
	  if ((stb.st_mode & S_IFMT) == S_IFDIR)
	    ALreturnError(session, ALwarnTempDirExists, buf)
	  else unlink(buf);
      } else if (errno != ENOENT)
	ALreturnError(session, (long) errno, buf);

      if (seteuid(ALpw_uid(session)) != 0)
	warning = ALwarnUidCopy;

      if (mkdir(buf, ALtempDirPerm))
	ALreturnError(session, (long) errno, buf);

      switch (cp_pid = fork()) {
      case -1:
	ALreturnError(session, (long) errno, "while forking");
      case 0:
	execl("/bin/cp", "cp", "-r", ALtempDotfiles, buf, NULL);
	fprintf(stderr, "Warning - could not copy user prototype files into temporary directory.\n");
	_exit(-1);
      default:
	break;
      }

      /* wait for copy to finish */
      waitpid(cp_pid, &cp_state, 0);

      /* set effective uid back to root */
      seteuid(0);

      /* remember we created a temporary homedir */
      ALflagSet(session, ALdidCreateHomedir);

      ALreturnError(session, warning, "");
  } else
    ALflagSet(session, ALdidAttachHomedir);

  return 0L;
}
