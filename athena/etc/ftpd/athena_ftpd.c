#ifdef ATHENA
#include <stdio.h>
#include <pwd.h>
#include <sys/file.h>
#include <krb.h>
#include <hesiod.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/wait.h>
#include <utmp.h>
#ifdef _IBMR2
#include <userpw.h>
#include <usersec.h>
#endif
#include "athena_ftpd.h"

#define LOGIN_TKT_DEFAULT_LIFETIME DEFAULT_TKT_LIFE /* from krb.h */
#define file_exists(f) (access((f), F_OK) == 0)

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define PASSWORD_LEN 14

#define UTMPFILE "/etc/utmp"

#ifndef NOLOGIN
#define NOLOGIN "/etc/nologin"
#endif
#ifndef NOCREATE
#define NOCREATE "/etc/nocreate"
#endif
#ifndef NOATTACH
#define NOATTACH "/etc/noattach"
#endif

#define ATTACH "/bin/athena/attach"
#define DETACH "/bin/athena/detach"
#define UNLOG "/bin/athena/unlog"

extern FILE *ftpd_popen();

int athena_login = LOGIN_NONE;
static int local_passwd;
#ifdef _IBMR2
static struct passwd athena_pwd = { "", "", 0, 0, "", "", "" };
#else
#ifdef ultrix
static struct passwd athena_pwd = { "", "", 0, 0, 0, 0, 0, "", "", "", "" };
#else
static struct passwd athena_pwd = { "", "", 0, 0, 0, "", "", "", "" };
#endif
#endif
static char att_errbuf[200];

struct passwd *athena_getpwnam(user)
     char *user;
{
  struct passwd *pwd;

  if (!( /* If we already have looked this user up, don't do it again */
	athena_pwd.pw_name && !strcmp(athena_pwd.pw_name, user) ))
    {
      if ((pwd = getpwnam(user)) != NULL)
	local_passwd = TRUE;
      else
	{
	  local_passwd = FALSE;
	  pwd = hes_getpwnam(user);
	  if (pwd == NULL || pwd->pw_dir[0] == 0)
	    pwd = NULL;
	}

      if (pwd)
	bcopy(pwd, &athena_pwd, sizeof(struct passwd));
      else
	return NULL;
    }

  return &athena_pwd;
}

int athena_localpasswd(user)
     char *user;
{
  struct passwd *pwd;

  pwd = athena_getpwnam(user);
  return local_passwd;
}

int athena_notallowed(user)
     char *user;
{
  if ((!athena_localpasswd(user) && file_exists(NOCREATE)) ||
      file_exists(NOLOGIN))
    return 1;
  return 0;
}

int get_tickets(username, password)
char *username;
char *password;
{
    char inst[INST_SZ], realm[REALM_SZ];
    int error;

    /* inst has to be a buffer instead of the constant "" because
     * krb_get_pw_in_tkt() will write a zero at inst[INST_SZ] to
     * truncate it.
     */
    inst[0] = 0;
    dest_tkt();

    if (krb_get_lrealm(realm, 1) != KSUCCESS)
      strcpy(realm, KRB_REALM);

    error = krb_get_pw_in_tkt(username, inst, realm, "krbtgt", realm,
			      LOGIN_TKT_DEFAULT_LIFETIME, password);

    return error;
}

#ifndef WIFEXITED
#define WIFEXITED(x)    (((union wait *)&(x))->w_stopval != WSTOPPED && \
                         ((union wait *)&(x))->w_termsig == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(x)  (((union wait *)&(x))->w_retcode)
#endif
#define PEXITSTATUS(x)  ((x>>8) & 255)

char *add_to_passwd(p)
struct passwd *p;
{
    int i, fd = -1;
    FILE *etc_passwd;
    static char passerr[100];

#ifdef _IBMR2
    struct userpw pw_stuff;
    int id;

    /* Do real locking of the user database */
    for (i = 0; i < 10; i++)
      if (setuserdb(S_WRITE) == 0) {
	fd = 1;
	break;
      }
      else
	sleep(1);

    if (fd != 1)
      {
	sprintf(passerr, "Warning: Couldn't put you in the password file; error %d from setuserdb");
	return(passerr);
      }

/* Need to have these to create empty stanzas, in the */
/* /etc/security/{environ, limits, user} files so that they pick up */
/* the default values */
    putuserattr(p->pw_name,(char *)NULL,((void *) 0),SEC_NEW);
    putuserattr(p->pw_name,S_ID,p->pw_uid,SEC_INT);
    putuserattr(p->pw_name,S_PWD,"!",SEC_CHAR);
    putuserattr(p->pw_name,S_PGRP,"mit",SEC_CHAR);
    putuserattr(p->pw_name,S_HOME,p->pw_dir,SEC_CHAR);
    putuserattr(p->pw_name,S_SHELL,p->pw_shell,SEC_CHAR);
    putuserattr(p->pw_name,S_GECOS,p->pw_gecos,SEC_CHAR);
    putuserattr(p->pw_name,S_LOGINCHK,1,SEC_BOOL);
    putuserattr(p->pw_name,S_SUCHK,1,SEC_BOOL);
    putuserattr(p->pw_name,S_RLOGINCHK,1,SEC_BOOL);
    putuserattr(p->pw_name,S_ADMIN,0,SEC_BOOL);
    putuserattr(p->pw_name,(char *)NULL,((void *) 0),SEC_COMMIT);
    enduserdb();

/* Now, lock the shadow password file */
    fd = -1;
    for (i = 0; i < 10; i++)
      if (setpwdb(S_WRITE) == 0) {
	fd = 1;
	break;
      }
      else
	sleep(1);

    if (fd != 1)
      {
	sprintf(passerr, "Warning: Couldn't put you in the password file; error %d from setpwdb");
	return(passerr);
      }

    strncpy(pw_stuff.upw_name,p->pw_name,PW_NAMELEN);
    pw_stuff.upw_passwd = p->pw_passwd;
    pw_stuff.upw_flags = 0;
    pw_stuff.upw_lastupdate = 0;
    putuserpw(&pw_stuff);
    endpwdb();
#else	/* RIOS */
    for (i = 0; i < 10; i++)
      if ((fd = open("/etc/ptmp", O_RDWR | O_CREAT | O_EXCL, 0644)) == -1 &&
	  errno == EEXIST)
	sleep(1);
      else
	break;
    if (fd == -1) {
	if (i < 10)
	  sprintf(passerr, "Warning: Couldn't put you in the password file; error %d locking", errno);
	else
	  sprintf(passerr, "Warning: Couldn't put you in the password file; couldn't lock", errno);
	return(passerr);
    }

    etc_passwd = fopen("/etc/passwd", "a");
    if (etc_passwd == NULL) {
        sprintf(passerr, "Warning: Couldn't put you in the password file; error %d in open", ferror(etc_passwd));
	(void) close(fd);
	(void) unlink("/etc/ptmp");
	return(passerr);
    }
    fprintf(etc_passwd, "%s:%s:%d:%d:%s:%s:%s\n",
	    p->pw_name,
	    p->pw_passwd,
	    p->pw_uid,
	    p->pw_gid,
	    p->pw_gecos,
	    p->pw_dir,
	    p->pw_shell);
    (void) fclose(etc_passwd);
    (void) close(fd);
    (void) unlink("/etc/ptmp");
#endif	/* RIOS */

    return(NULL);
}

char *athena_authenticate(user, passwd)
     char *user, *passwd;
{
  int local_ok = 0;
  struct passwd *pwd;
  char tkt_file[128];
  long salt;
  char saltc[2], c;
  char encrypt[PASSWORD_LEN+1];
  int child, error, i;
  char *errstring = NULL;
  static char errbuf[1024];
#ifndef _AUX_SOURCE
  union wait status;
#else
  int status;
#endif

  athena_login = LOGIN_NONE;

  pwd = athena_getpwnam(user);

  if (!local_passwd)
    {
      /* save encrypted password to put in local password file */
      salt = 9 * getpid();
      saltc[0] = salt & 077;
      saltc[1] = (salt>>6) & 077;
      for (i=0;i<2;i++) {
	c = saltc[i] + '.';
	if (c > '9')
	  c += 7;
	if (c > 'Z')
	  c += 6;
	saltc[i] = c;
      }
      strcpy(encrypt,crypt(passwd, saltc));	

      pwd->pw_passwd = encrypt;
    }

  if (local_passwd &&
      !strcmp(crypt(passwd, pwd->pw_passwd), pwd->pw_passwd))
    local_ok = 1;

  sprintf(tkt_file, "/tmp/tkt_%s.%d", pwd->pw_name, getpid());
  setenv("KRBTKFILE", tkt_file, 1);
  /* we set the ticket file here because a previous dest_tkt() might
     have cached the wrong ticket file. */
  krb_set_tkt_string(tkt_file);

  if (!(child = fork()))
    {
      /* set real uid/gid for kerberos library */
#ifdef _IBMR2
      setruid_rios(pwd->pw_uid);
      setrgid_rios(pwd->pw_gid);
#else
      setruid(pwd->pw_uid);
      setrgid(pwd->pw_gid);
#endif

      error = get_tickets(user, passwd);
      bzero(passwd, strlen(passwd));
      _exit(error);
    }

  if (child == -1)
    {
      sprintf(errbuf, "Fork to get tickets failed with error %d", errno);
      if (local_ok)
	athena_login = LOGIN_LOCAL;
      bzero(passwd, strlen(passwd));
      return errbuf;
    }

#ifdef _IBMR2
  signal(SIGCHLD, SIG_DFL);
  (void)waitpid(child, &status, 0);
  signal(SIGCHLD, SIG_IGN);
#else
  (void)wait(&status);
#endif
  bzero(passwd, strlen(passwd));

#ifdef POSIX
  if (!WIFEXITED(status))
    errstring = "ticket child failed";
  else
#endif /* POSIX */
    { /* this is probably screwed up XXX */
      error = WEXITSTATUS(status);      

      switch (error) {
      case KSUCCESS:
	break;
      case INTK_BADPW:
	errstring =  "Incorrect Kerberos password entered.";
	break;
      case KDC_PR_UNKNOWN:
	errstring = "Unknown Kerberos username entered.";
	break;
      default:
	sprintf(errbuf, "Authentication failed: %d: %s.",
		error, krb_err_txt[error]);
	errstring = errbuf;
	break;
      }
    }

  if (errstring != NULL && !local_ok)
    return errstring;

  if (errstring != NULL && local_ok)
    {
      athena_login = LOGIN_LOCAL;
      return errstring;
    }

  athena_login = LOGIN_KERBEROS;
  if (!local_passwd)
    return(add_to_passwd(pwd));
  else
    return(NULL);
}

char *athena_attach(pw, dir, auth)
     struct passwd *pw;
     char *dir;
     int auth;
{
  int attach_error, child;
  int v, out, err;
#ifndef _AUX_SOURCE
  union wait status;
#else
  int status;
#endif

  if (!(child = fork()))
    {
#ifdef _AIX
      setuid(pw->pw_uid);
      setgid(pw->pw_gid);
#else
      setruid(pw->pw_uid);
      setrgid(pw->pw_gid);
#endif

      out = fileno(stdout);
      err = fileno(stderr);

      v = open("/dev/null", O_WRONLY);
      dup2(v, out);
      dup2(v, err);
      close(v);

      execl(ATTACH, "attach", "-nozephyr", auth ? "-y" : "-n", dir, 0);
      _exit(255); /* Attach isn't supposed to return 255 */
    }

  if (child == -1)
    {
      sprintf(att_errbuf, "Fork for attach failed with error %d", errno);
      return att_errbuf;
    }

#ifdef _IBMR2
  signal(SIGCHLD, SIG_DFL);
  (void)waitpid(child, &status, 0);
  signal(SIGCHLD, SIG_IGN);
#else
  (void)wait(&status);
#endif

#ifdef POSIX
  if (!WIFEXITED(status))
    {
      sprintf(att_errbuf, "Bad exit on attach child");
      return att_errbuf;
    }
  else
#endif /* POSIX */
    { /* this is probably screwed up XXX */
      attach_error = WEXITSTATUS(status);      

      if (attach_error)
	{
	  switch(attach_error)
	    {
	    case 255:
	      sprintf(att_errbuf, "Couldn't exec attach");
	      break;
	    case 11:
	      sprintf(att_errbuf, "Server not responding");
	      break;
	    case 12:
	      sprintf(att_errbuf, "Authentication failure");
	      break;
	    case 20:
	      sprintf(att_errbuf, "Filesystem does not exist");
	      break;
	    case 24:
	      sprintf(att_errbuf, "You are not allowed to attach this filesystem.");
	      break;
	    case 25:
	      sprintf(att_errbuf, "You are not allowed to attach a filesystem here.");
	      break;
	    default:
	      sprintf(att_errbuf, "Could not attach directory: attach returned error %d", attach_error);
	      break;
	    }
	  return att_errbuf;
	}
    }
  return NULL;
}

char *athena_attachhomedir(pw, auth)
     struct passwd *pw;
     int auth;
{
  /* Delete empty directory if it exists.  We just try to rmdir the 
   * directory, and if it's not empty that will fail.
   */
  rmdir(pw->pw_dir);

  /* If a good local homedir exists, use it */
  if (file_exists(pw->pw_dir) && !IsRemoteDir(pw->pw_dir) &&
      homedirOK(pw->pw_dir))
    return(NULL);

  /* Using homedir already there that may or may not be good. */
  if (file_exists(NOATTACH) && file_exists(pw->pw_dir) &&
      homedirOK(pw->pw_dir)) {
    return(NULL);
  }

  if (file_exists(NOATTACH))
    {
      sprintf(att_errbuf, "This workstation is configured not to create local home directories.");
      return att_errbuf;
    }

  return athena_attach(pw, pw->pw_name, auth);
}

/* Function Name: IsRemoteDir
 * Description: Stolen form athena's version of /bin/login
 *              returns true of this is an NFS directory.
 * Arguments: dname - name of the directory.
 * Returns: true or false to the question (is remote dir).
 *
 * The following lines rely on the behavior of Sun's NFS (present in
 * 3.0 and 3.2) which causes a read on an NFS directory (actually any
 * non-reg file) to return -1, and AFS which also returns a -1 on
 * read (although with a different errno).  This is a fast, cheap
 * way to discover whether a user's homedir is a remote filesystem.
 * Naturally, if the NFS and/or AFS semantics change, this must also change.
 */

IsRemoteDir(dir)
char *dir;
{
  int f;
  char c;
  struct stat stbuf;
  
  if (lstat(dir, &stbuf))
    return(FALSE);
  if (!(stbuf.st_mode & S_IFDIR))
    return(TRUE);

  if ((f = open(dir, O_RDONLY, 0)) < 0)
    return(FALSE);

  if (read(f, &c, 1) < 0) {
      close(f);
      return(TRUE);
  }

  close(f);
  return(FALSE);
}

/* Function Name: homedirOK
 * Description: checks to see if our homedir is okay, i.e. exists and 
 *	contains at least 1 file
 * Arguments: dir - the directory to check.
 * Returns: TRUE if the homedir is okay.
 */

int homedirOK(dir)
char *dir;
{
    DIR *dp;
#ifdef POSIX
    struct dirent *temp;
#else
    struct direct *temp;
#endif
    int count;

    if ((dp = opendir(dir)) == NULL)
      return(FALSE);

    /* Make sure that there is something here besides . and .. */
    for (count = 0; count < 3 ; count++)
      temp = readdir(dp);

    closedir(dp);
    return(temp != NULL);
}

fork_dest_tkt(pw)
     struct passwd *pw;
{
  int child, status;

  if (!(child = fork()))
    {
      /* set real uid/gid for kerberos library */
#ifdef _IBMR2
      setruid_rios(pw->pw_uid);
      setrgid_rios(pw->pw_gid);
#else
      setruid(pw->pw_uid);
      setrgid(pw->pw_gid);
#endif

      dest_tkt();
      _exit(0);
    }

  if (child == -1)
    {
/*      sprintf(errbuf, "Fork to get tickets failed with error %d", errno);
      if (local_ok)
	athena_login = LOGIN_LOCAL;
      bzero(passwd, strlen(passwd));
      return errbuf;*/
      return;
    }

#ifdef _IBMR2
  signal(SIGCHLD, SIG_DFL);
  (void)waitpid(child, &status, 0);
  signal(SIGCHLD, SIG_IGN);
#else
  (void)wait(&status);
#endif
}

/*
 * Should be called when root
 */
athena_logout(pw)
     struct passwd *pw;
{
  FILE *att, *unlog;
  int detach_error, unlog_error, pid;
  char str_uid[10];

  if (athena_login != LOGIN_NONE)
    {
      /* destroy kerberos tickets */
      if (athena_login == LOGIN_KERBEROS)
#ifdef _IBMR2
	fork_dest_tkt(pw);
#else
        dest_tkt();
#endif

      athena_login = LOGIN_NONE;

      if (!user_logged_in(pw->pw_name))
	{
	  chdir("/");
	  sprintf(str_uid, "#%d", pw->pw_uid);

	  if ((pid = fork()) == 0) /* we just don't care about failure... */
	    {
	      execl(DETACH, DETACH, "-quiet", "-user", str_uid, "-a", 0);
	      exit(1);
	    }

	  if (pid != -1)
	    if ((pid = fork()) == 0)
	      {
		execl(UNLOG, UNLOG, 0);
		exit(1);
	      }
	}
    }
}

#if defined(_AIX) && defined(_IBMR2)
#include <sys/id.h>

/*
 * AIX 3.1 has bizzarre ideas about changing uids and gids around.  They are
 * such that the sete{u,g}id and setr{u,g}id calls here fail.  For this reason
 * we are replacing the sete{u,g}id and setr{u,g}id calls.
 * 
 * The bizzarre ideas are as follows:
 *
 * The effective ID may be changed only to the current real or
 * saved IDs.
 *
 * The saved uid may be set only if the real and effective
 * uids are being set to the same value.
 *
 * The real uid may be set only if the effective
 * uid is being set to the same value.
 *
 * Yes, POSIX rears its head..
 */

#ifdef __STDC__
int setruid_rios(uid_t ruid)
#else
int setruid_rios(ruid)
  uid_t ruid;
#endif /* __STDC__ */
{
    uid_t euid;

    if (ruid == -1)
        return (0);

    euid = geteuid();

    if (setuidx(ID_REAL | ID_EFFECTIVE, ruid) == -1)
        return (-1);
    
    return (setuidx(ID_EFFECTIVE, euid));
}


#ifdef __STDC__
int seteuid_rios(uid_t euid)
#else
int seteuid_rios(euid)
  uid_t euid;
#endif /* __STDC__ */
{
    uid_t ruid;

    if (euid == -1)
        return (0);

    ruid = getuid();

    if (setuidx(ID_SAVED | ID_REAL | ID_EFFECTIVE, euid) == -1)
        return (-1);
    
    return (setruid_rios(ruid));
}


#ifdef __STDC__
int setreuid_rios(uid_t ruid, uid_t euid)
#else
int setreuid_rios(ruid, euid)
  uid_t ruid;
  uid_t euid;
#endif /* __STDC__ */
{
    if (seteuid_rios(euid) == -1)
        return (-1);

    return (setruid_rios(ruid));
}

#ifdef __STDC__
int setuid_rios(uid_t uid)
#else
int setuid_rios(uid)
  uid_t uid;
#endif /* __STDC__ */
{
    return (setreuid_rios(uid, uid));
}

#ifdef __STDC__
int setrgid_rios(gid_t rgid)
#else
int setrgid_rios(rgid)
  gid_t rgid;
#endif /* __STDC__ */
{
    gid_t egid;

    if (rgid == -1)
        return (0);

    egid = getegid();

    if (setgidx(ID_REAL | ID_EFFECTIVE, rgid) == -1)
        return (-1);
    
    return (setgidx(ID_EFFECTIVE, egid));
}


#ifdef __STDC__
int setegid_rios(gid_t egid)
#else
int setegid_rios(egid)
  gid_t egid;
#endif /* __STDC__ */
{
    gid_t rgid;

    if (egid == -1)
        return (0);

    rgid = getgid();

    if (setgidx(ID_SAVED | ID_REAL | ID_EFFECTIVE, egid) == -1)
        return (-1);
    
    return (setrgid_rios(rgid));
}


#ifdef __STDC__
int setregid_rios(gid_t rgid, gid_t egid)
#else
int setregid_rios(rgid, egid)
  gid_t rgid;
  gid_t egid;
#endif /* __STDC__ */
{
    if (setegid_rios(egid) == -1)
        return (-1);

    return (setrgid_rios(rgid));
}

#ifdef __STDC__
int setgid_rios(gid_t gid)
#else
int setgid_rios(gid)
  gid_t gid;
#endif /* __STDC__ */
{
    return (setregid_rios(gid, gid));
}

#endif /* RIOS */
#endif
