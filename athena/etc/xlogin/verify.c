/* Copyright 1990, 1999 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: verify.c,v 1.16 2004-06-16 16:56:49 ghudson Exp $";

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <dirent.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <netdb.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#include <utime.h>
#include <utmp.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif

#include <al.h>
#include <hesiod.h>
#include <krb.h>
#ifdef HAVE_KRB5
#include <krb5.h>
#endif
#include <larv.h>

#include "environment.h"
#include "xlogin.h"

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define LOGIN_TKT_DEFAULT_LIFETIME 120 /* Ten hours */
#define PASSWORD_LEN 14
#define MAXENVIRON 32

#define MOTD "/etc/motd"

#ifdef HAVE_GETUTXENT
/* We need to know where the wtmpx file is for updwtmpx(), sadly. */
#if defined(WTMPX_FILE)
#define WTMPX WTMPX_FILE
#elif defined(_PATH_WTMPX)
#define WTMPX _PATH_WTMPX
#else
#define WTMPX "/var/adm/wtmpx"
#endif
#else /* HAVE_GETUTXENT */
/* Lacking the System V utmpx interfaces, we need to know where the
 * utmp and wtmp files are.
 */
#if defined(UTMP_FILE)
#define UTMP UTMP_FILE
#define WTMP WTMP_FILE
#elif defined(_PATH_UTMP)
#define UTMP _PATH_UTMP
#define WTMP _PATH_WTMP
#else
#define UTMP "/var/adm/utmp"
#define WTMP "/var/adm/wtmp"
#endif
#endif /* HAVE_GETUTXENT */

#ifdef SOLARIS
char *defaultpath = "/srvd/patch:/usr/athena/bin:/bin/athena:/usr/openwin/bin:/bin:/usr/ucb:/usr/sbin:/usr/andrew/bin:.";
#else
#if defined(__NetBSD__) || defined(__linux__)
char *defaultpath = "/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin:/bin:/usr/sbin:/sbin:/usr/X11R6/bin:/usr/andrew/bin:.";
#else
char *defaultpath = "/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin/X11:/usr/new:/usr/ucb:/bin:/usr/bin:/usr/ibm:/usr/andrew/bin:.";
#endif
#endif

int al_pid;

static char *get_tickets(char *username, char *password);
static void abort_verify(void *user);
static void add_utmp(char *user, char *tty, char *display);

#ifdef HAVE_KRB5
static krb5_error_code do_v5_kinit(char *name, char *instance, char *realm,
				   int lifetime, char *password,
				   char **ret_cache_name, char **etext);
static krb5_error_code do_v5_kdestroy(const char *cachename);
#endif

extern pid_t attach_pid, attachhelp_pid, quota_pid;
extern int attach_state, attachhelp_state, errno;
extern sigset_t sig_zero;

#ifdef HAVE_AFS
/* If we call setpag() when AFS is not loaded, we will get a SIGSYS,
 * at least on systems which have SIGSYS.
 */
static void try_setpag()
{
#ifdef SIGSYS
  struct sigaction sa, osa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGSYS, &sa, &osa);
  setpag();
  sigaction(SIGSYS, &osa, NULL);
#else
  setpag();
#endif
}
#endif

#ifdef HAVE_GETSPNAM
static struct passwd *get_pwnam(char *usr)
{
  struct passwd *pwd;
  struct spwd *sp;

  pwd = getpwnam(usr);
  sp = getspnam(usr);
  if ((sp != NULL) && (pwd != NULL))
    pwd->pw_passwd = sp->sp_pwdp;
  return pwd;
}
#else
#define get_pwnam(x) getpwnam(x)
#endif

char *dologin(char *user, char *passwd, int option, char *script,
	      char *tty, char *startup, char *session, char *display)
{
  static char errbuf[5120];
  char tkt_file[128], *msg, wgfile[16];
#ifdef HAVE_KRB5
  char tkt5_file[128];
#endif
  struct passwd *pwd;
  struct group *gr;
  struct utimbuf times;
  char **environment;
  char fixed_tty[16], *p;
  int i;
  /* state variables: */
  int local_ok = FALSE;		/* verified from local password file */
  int local_acct;		/* user's account is supposed to be local */
  char *altext = NULL, *alerrmem;
  int status, *warnings, *warning;
  int tmp_homedir = 0;

  /* 4.2 vs 4.3 style syslog */
#ifndef  LOG_ODELAY
  openlog("xlogin", LOG_NOTICE);
#else
  openlog("xlogin", LOG_ODELAY, LOG_AUTH);
#endif

  /* Check to make sure a username was entered. */
  if (!strcmp(user, ""))
    return "No username entered.  Please enter a username and "
	   "password to try again.";

  /* Check that the user is allowed to log in. */
  status = al_login_allowed(user, 0, &local_acct, &altext);
  if (status != AL_SUCCESS)
    {
      memset(passwd, 0, strlen(passwd));	/* zap ASAP */
      switch(status)
	{
	case AL_ENOUSER:
	  sprintf(errbuf,
		  "Unknown user name entered (no hesiod information "
		  "for \"%s\")", user);
	  break;
	case AL_ENOLOGIN:
	  strcpy(errbuf,
		 "Logins are currently disabled on this workstation.  ");
	  break;
	case AL_ENOCREATE:
	  strcpy(errbuf,
		 "You are not allowed to log into this workstation.  "
		 "Contact the workstation's administrator or a consultant "
		 "for further information.  ");
	  break;
	case AL_EBADHES:
	  strcpy(errbuf, "This account conflicts with a locally defined "
		 "account... aborting.");
	  break;
	case AL_ENOMEM:
	  strcpy(errbuf, "Out of memory.");
	  break;
	default:
	  strcpy(errbuf, al_strerror(status, &alerrmem));
	  al_free_errmem(alerrmem);
	  break;
	}

      if (altext)
	{
	  strncat(errbuf, altext, sizeof(errbuf) - strlen(errbuf) - 1);
	  free(altext);
	}

      syslog(LOG_INFO, "Unauthorized login attempt for username %s: %s", user,
	     al_strerror(status, &alerrmem));
      al_free_errmem(alerrmem);

      return errbuf;
    }

  /* Test to see if the user can be authenticated locally. If not,
   * grab their password information from Hesiod, since the uid is
   * potentially needed for mail-check login and the ticket file
   * name, before we want to call al_acct_create().
   */
  pwd = get_pwnam(user);
  if (pwd != NULL)
    {
      if (strcmp(crypt(passwd, pwd->pw_passwd), pwd->pw_passwd) == 0)
	local_ok = TRUE;
      else if (local_acct)
	return "Incorrect password";
    }
  else
    {
      pwd = hes_getpwnam(user);
      if (pwd == NULL) /* "can't" happen */
	return "Strange failure in Hesiod lookup.";
    }

  /* Only do Kerberos-related things if the account is not local. */
  if (!local_acct)
      {
	/* Terminal names may be something like pts/0; we don't want any /'s
	 * in the path name; replace them with _'s.
	 */
	if (tty != NULL)
	  {
	    strcpy(fixed_tty, tty);
	    while ((p = strchr(fixed_tty, '/')))
	      *p = '_';
	  }
	else
	  sprintf(fixed_tty, "%lu", (unsigned long)pwd->pw_uid);
	sprintf(tkt_file, "/tmp/tkt_%s", fixed_tty);
	psetenv("KRBTKFILE", tkt_file, 1);

	/* We set the ticket file here because a previous dest_tkt() might
	 * have cached the wrong ticket file.
	 */
	krb_set_tkt_string(tkt_file);

#ifdef HAVE_KRB5
	sprintf(tkt5_file, "/tmp/krb5cc_%s", fixed_tty);
	psetenv("KRB5CCNAME", tkt5_file, 1);
#endif

	msg = get_tickets(user, passwd);
	memset(passwd, 0, strlen(passwd));

	if (msg)
	  {
	    if (!local_ok)
	      return msg;
	    else
	      {
		prompt_user("Unable to get full authentication, you will "
			    "have local access only during this login "
			    "session (failed to get kerberos tickets).  "
			    "Continue anyway?", abort_verify, NULL);
	      }
	  }

	chown(tkt_file, pwd->pw_uid, pwd->pw_gid);
#ifdef HAVE_KRB5
	chown(tkt5_file, pwd->pw_uid, pwd->pw_gid);
#endif
      }

  /* Code for verifying a secure tty used to be here. */

  /* If a mail-check login has been selected, do that now. */
  if (option == 4)
    {
      attach_state = -1;
      switch(fork_and_store(&attach_pid))
	{
	case -1:
	  fprintf(stderr, "Unable to fork to check your mail.\n");
	  break;
	case 0:
	  if (setuid(pwd->pw_uid) != 0)
	    {
	      fprintf(stderr, "Unable to set user ID to check your mail.\n");
	      _exit(-1);
	    }
	  printf("Electronic mail status:\n");
	  execlp("from", "from", "-r", user, NULL);
	  fprintf(stderr, "Unable to run mailcheck program.\n");
	  _exit(-1);
	default:
	  while (attach_state == -1)
	    sigsuspend(&sig_zero);
	  printf("\n");
	  prompt_user("A summary of your waiting email is displayed in "
		      "the console window.  Continue with full login "
		      "session or logout now?", abort_verify, NULL);
	}
    }
#ifdef HAVE_AFS
  try_setpag();
#endif

  al_pid = getpid();

  if (!local_acct)
    {
      status = al_acct_create(user, al_pid, !msg, 1, &warnings);
      if (status != AL_SUCCESS)
	{
	  switch(status)
	    {
	    case AL_EPASSWD:
	      strcpy(errbuf, "An unexpected error occured while entering you "
		     "in the local password file.");
	      return errbuf;
	    case AL_WARNINGS:
	      warning = warnings;
	      while (*warning != AL_SUCCESS)
		{
		  switch(*warning)
		    {
		    case AL_WGROUP:
		      prompt_user("Unable to set your group access list.  "
				  "You may have insufficient permission to "
				  "access some files.  Continue with this "
				  "login session anyway?", abort_verify, user);
		      break;
		    case AL_WXTMPDIR:
		      tmp_homedir = 1;
		      prompt_user("You are currently logged in with a "
				  "temporary home directory, so this login "
				  "session will use that directory. Continue "
				  "with this login session anyway?",
				  abort_verify, user);
		      break;
		    case AL_WTMPDIR:
		      tmp_homedir = 1;
		      prompt_user("Your home directory is unavailable.  A "
				  "temporary directory will be created for "
				  "you.  However, it will be DELETED when you "
				  "logout.  Any mail that you incorporate "
				  "during this session WILL BE LOST when you "
				  "logout.  Continue with this login session "
				  "anyway?", abort_verify, user);
		      break;
		    case AL_WNOHOMEDIR:
		      prompt_user("No home directory is available.  Continue "
				  "with this login session anyway?",
				  abort_verify, user);
		      break;
		    case AL_WNOATTACH:
		      prompt_user("This workstation is configured not to "
				  "attach remote filesystems.  Continue with "
				  "your local home directory?", abort_verify,
				  user);
		      break;
		    case AL_WBADSESSION:
		    default:
		      break;
		    }
		  warning++;
		}
	      free(warnings);
	      break;
	    default:
	      strcpy(errbuf, al_strerror(status, &alerrmem));
	      al_free_errmem(alerrmem);
	      return errbuf;
	    }
	}
    }

  /* Get the password entry again. We need a new copy because it
   * may have been edited by al_acct_create().
   */
  pwd = get_pwnam(user);
  if (pwd == NULL) /* "can't" happen */
    return lose("Unable to get your password entry.\n");

  switch(fork_and_store(&quota_pid))
    {
    case -1:
      fprintf(stderr, "Unable to fork to check your filesystem quota.\n");
      break;
    case 0:
      if (setuid(pwd->pw_uid) != 0)
	{
	  fprintf(stderr,
		  "Unable to set user ID to check your filesystem quota.\n");
	  _exit(-1);
	}
      execlp("quota", "quota", NULL);
      fprintf(stderr, "Unable to run quota command %s\n", "quota");
      _exit(-1);
    default:
      ;
    }

  /* Show the message of the day. */
  sprintf(errbuf, "%s/.hushlogin", pwd->pw_dir);
  if (!file_exists(errbuf))
    {
      int f, count;

      f = open(MOTD, O_RDONLY, 0);
      if (f >= 0)
	{
	  count = read(f, errbuf, sizeof(errbuf) - 1);
	  write(1, errbuf, count);
	  close(f);
	}
    }

  /*
   * Set up the user's environment.
   *
   *   By default, none of xlogin's environment is passed to
   *   users who log in.
   *
   *   The PASSENV macro is defined to make it trivial to pass
   *   an element of xlogin's environment on to the user.
   *
   *   Note that the environment for pre-login options is set
   *   up in xlogin.c: it is NOT RELATED to this environment
   *   setup. If you add a new environment variable here,
   *   consider whether or not it also needs to be added there.
   *   Note that variables that need to be PASSENVed here do not
   *   need similar treatment in the pre-login area, since there
   *   all variables as passed by default.
   */
#define PASSENV(envvar)					\
  msg = getenv(envvar);					\
  if (msg)						\
    {							\
      sprintf(errbuf, "%s=%s", envvar, msg);		\
      environment[i++] = strdup(errbuf);		\
    }

  environment = (char **) malloc(MAXENVIRON * sizeof(char *));
  if (environment == NULL)
    return "Out of memory while trying to initialize user environment "
	   "variables.";

  i = 0;
  sprintf(errbuf, "HOME=%s", pwd->pw_dir);
  environment[i++] = strdup(errbuf);
  sprintf(errbuf, "PATH=%s", defaultpath);
  environment[i++] = strdup(errbuf);
  sprintf(errbuf, "USER=%s", pwd->pw_name);
  environment[i++] = strdup(errbuf);
  sprintf(errbuf, "SHELL=%s", pwd->pw_shell);
  environment[i++] = strdup(errbuf);
  sprintf(errbuf, "DISPLAY=%s", display);
  environment[i++] = strdup(errbuf);
  if (!local_acct)
    {
      sprintf(errbuf, "KRBTKFILE=%s", tkt_file);
      environment[i++] = strdup(errbuf);
#ifdef HAVE_KRB5
      sprintf(errbuf, "KRB5CCNAME=%s", tkt5_file);
      environment[i++] = strdup(errbuf);
#endif
    }
#ifdef HOSTTYPE
  sprintf(errbuf, "hosttype=%s", HOSTTYPE); /* environment.h */
  environment[i++] = strdup(errbuf);
#endif

#ifdef SOLARIS
  PASSENV("LD_LIBRARY_PATH");
  PASSENV("OPENWINHOME");
#endif

  if (tmp_homedir)
    environment[i++] = "TMPHOME=1";
  strcpy(wgfile, "/tmp/wg.XXXXXX");
  mktemp(wgfile);
  sprintf(errbuf, "WGFILE=%s", wgfile);
  environment[i++] = strdup(errbuf);
  PASSENV("TZ");

  environment[i++] = NULL;

  add_utmp(user, tty, display);
  if (pwd->pw_uid == ROOT)
    syslog(LOG_CRIT, "ROOT LOGIN on tty %s", tty ? tty : "X");
  else
    syslog(LOG_INFO, "%s LOGIN on tty %s", user, tty ? tty : "X");

  /* Set the owner and modtime on the tty. */
  sprintf(errbuf, "/dev/%s", tty);
  gr = getgrnam("tty");
  chown(errbuf, pwd->pw_uid, gr ? gr->gr_gid : pwd->pw_gid);
  chmod(errbuf, 0620);

  times.actime = times.modtime = time(NULL);
  utime(errbuf, &times);

  i = setgid(pwd->pw_gid);
  if (i) 
    return lose("Unable to set your primary GID.\n");
        
  if (initgroups(user, pwd->pw_gid) < 0)
    prompt_user("Unable to set your group access list.  You may have "
		"insufficient permission to access some files.  "
		"Continue with this login session anyway?",
		abort_verify, user);

  /* Invoke the Xstartup script.  This should ensure that the various user
   * devices (e.g. audio) are chown'ed to the user.
   */
  exec_script(startup, environment);

#ifdef HAVE_SETLOGIN
  i = setlogin(pwd->pw_name);
  if (i)
    return(lose("Unable to set your login credentials.\n"));
#endif

  i = setuid(pwd->pw_uid);
  if (i)
    return lose("Unable to set your user ID.\n");

  if (chdir(pwd->pw_dir))
    fprintf(stderr, "Unable to connect to your home directory.\n");

  /* Stuff first arg for xsession into a string. */
  sprintf(errbuf, "%d", option);

  execle(session, "sh", errbuf, script, NULL, environment);

  return lose("Failed to start session.");
}

static char *get_tickets(char *username, char *password)
{
  char inst[INST_SZ], realm[REALM_SZ];
  char hostname[MAXHOSTNAMELEN], phost[INST_SZ];
  char key[8], *rcmd;
  static char errbuf[1024];
  int error;
  KTEXT_ST ticket;
  AUTH_DAT authdata;

  rcmd = "rcmd";

  /* inst has to be a buffer instead of the constant "" because
   * krb_get_pw_in_tkt() will write a zero at inst[INST_SZ] to
   * truncate it.
   */
  inst[0] = 0;
  dest_tkt();
#ifdef HAVE_KRB5
  do_v5_kdestroy(0);
#endif

  if (krb_get_lrealm(realm, 1) != KSUCCESS)
    strcpy(realm, KRB_REALM);

  error = krb_get_pw_in_tkt(username, inst, realm, "krbtgt", realm,
			    LOGIN_TKT_DEFAULT_LIFETIME, password);
  switch(error)
    {
    case KSUCCESS:
      break;
    case INTK_BADPW:
      return "Incorrect password entered.";
    case KDC_PR_UNKNOWN:
      return "Unknown username entered.";
    default:
      sprintf(errbuf, "Unable to authenticate you, kerberos failure "
	      "%d: %s.  Try again here or on another workstation.",
	      error, krb_err_txt[error]);
      return errbuf;
    }

#ifdef HAVE_KRB5
  {
    krb5_error_code krb5_ret;
    char *etext;

    krb5_ret = do_v5_kinit(username, inst, realm,
			   LOGIN_TKT_DEFAULT_LIFETIME, password,
			   0, &etext);
    if (krb5_ret && krb5_ret != KRB5KRB_AP_ERR_BAD_INTEGRITY)
      com_err("xlogin", krb5_ret, etext);
  }
#endif

  if (gethostname(hostname, sizeof(hostname)) == -1)
    {
      fprintf(stderr, "Warning: cannot retrieve local hostname\n");
      return NULL;
    }
  strncpy(phost, krb_get_phost(hostname), sizeof(phost));
  phost[sizeof(phost) - 1] = '\0';

  /* Without a srvtab, we cannot verify tickets. */
  if (read_service_key(rcmd, phost, realm, 0, NULL, key) == KFAILURE)
    return NULL;

  error = krb_mk_req(&ticket, rcmd, phost, realm, 0);
  if (error == KDC_PR_UNKNOWN)
    return NULL;
  if (error != KSUCCESS)
    {
      sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s",
	      error, krb_err_txt[error]);
      return errbuf;
    }

  error = krb_rd_req(&ticket, rcmd, phost, 0, &authdata, "");
  if (error != KSUCCESS)
    {
      memset(&ticket, 0, sizeof(ticket));
      sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s",
	      error, krb_err_txt[error]);
      return errbuf;
    }
  memset(&ticket, 0, sizeof(ticket));
  memset(&authdata, 0, sizeof(authdata));
  return NULL;
}

/* Destroy kerberos tickets and let al_acct_revert clean up the rest. */
void cleanup(char *user)
{
  dest_tkt();
#ifdef HAVE_KRB5
  do_v5_kdestroy(0);
#endif

  if (user)
    al_acct_revert(user, al_pid);

  /* Set real uid to zero.  If this is impossible, exit.  The
   * current implementation of lose() will not print a message
   * so xlogin will just exit silently.  This call "can't fail",
   * so this is not a serious problem.
   */
  if (setuid(0) == -1)
    lose("Unable to reset real uid to root");
}

static void abort_verify(void *user)
{
  cleanup(user);
  _exit(1);
}

#if HAVE_GETUTXENT && !HAVE_LOGIN
static void add_utmp(char *user, char *tty, char *display)
{
  struct utmp ut_entry;
  struct utmp *ut_tmp;
  struct utmpx utx_entry;
  struct utmpx *utx_tmp;

  memset(&utx_entry, 0, sizeof(utx_entry));

  strncpy(utx_entry.ut_line, tty, sizeof(utx_entry.ut_line));
  strncpy(utx_entry.ut_name, user, sizeof(utx_entry.ut_name));

  /* Be sure the host string is null terminated. */
  strncpy(utx_entry.ut_host, display, sizeof(utx_entry.ut_host));
  utx_entry.ut_host[sizeof(utx_entry.ut_host) - 1] = '\0';

  gettimeofday(&utx_entry.ut_tv, NULL);
  utx_entry.ut_pid = getppid();
  utx_entry.ut_type = USER_PROCESS;
  strncpy(utx_entry.ut_id, "XLOG", sizeof(utx_entry.ut_id));

  getutmp(&utx_entry, &ut_entry);

  setutent();
  while ((ut_tmp = getutline(&ut_entry)))
    if (!strncmp(ut_tmp->ut_id, "XLOG", sizeof(ut_tmp->ut_id)))
      break;
  pututline(&ut_entry);

  setutxent();
  while ((utx_tmp = getutxline(&utx_entry)))
    if (!strncmp(utx_tmp->ut_id, "XLOG", sizeof(utx_tmp->ut_id)))
      break;
  pututxline(&utx_entry);

  updwtmpx(WTMPX, &utx_entry);
}
#else /* HAVE_GETUTXENT && !HAVE_LOGIN */
static void add_utmp(char *user, char *tty, char *display)
{
  struct utmp ut_entry;
  struct utmp ut_tmp;
  int f;

  memset(&ut_entry, 0, sizeof(ut_entry));

  strncpy(ut_entry.ut_line, tty, sizeof(ut_entry.ut_line));
  strncpy(ut_entry.ut_name, user, sizeof(ut_entry.ut_name));

  /* Be sure the host string is null terminated. */
  strncpy(ut_entry.ut_host, display, sizeof(ut_entry.ut_host));
  ut_entry.ut_host[sizeof(ut_entry.ut_host) - 1] = '\0';

  time(&(ut_entry.ut_time));
#ifdef USER_PROCESS
  ut_entry.ut_pid = getppid();
  ut_entry.ut_type = USER_PROCESS;
#endif

#ifdef HAVE_LOGIN
  login(&ut_entry);
#else
  f = open(UTMP, O_RDWR);
  if (f >= 0)
    {
      while (read(f, (char *)&ut_tmp, sizeof(ut_tmp)) == sizeof(ut_tmp))
	if (ut_tmp.ut_pid == ut_entry.ut_pid)
	  {
	    strncpy(ut_entry.ut_id, ut_tmp.ut_id, sizeof(ut_tmp.ut_id));
	    lseek(f, (long) -sizeof(ut_tmp), 1);
	    break;
	  }
      write(f, (char *)&ut_entry, sizeof(ut_entry));
      close(f);
    }

  f = open(WTMP, O_WRONLY | O_APPEND);
  if (f >= 0)
    {
      write(f, (char *)&ut_entry, sizeof(ut_entry));
      close(f);
    }
#endif
}
#endif /* GETUTXENT && !HAVE_LOGIN */

#define MAXGNAMELENGTH	32

/* Fork, storing the pid in a variable var and returning the pid.
 * Make sure that the pid is stored before any SIGCHLD can be
 * delivered.
 */
pid_t fork_and_store(pid_t *var)
{
  pid_t pid;
  sigset_t mask, omask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);
  pid = fork();
  *var = pid;
  sigprocmask(SIG_SETMASK, &omask, NULL);
  return pid;
}

/* Emulate setenv() with the more portable (these days) putenv(). */
int psetenv(const char *name, const char *value, int overwrite)
{
  char *var;

  if (!overwrite && getenv(name) != NULL)
    return 0;
  var = malloc(strlen(name) + strlen(value) + 2);
  if (!var)
    {
      errno = ENOMEM;
      return -1;
    }
  sprintf(var, "%s=%s", name, value);
  putenv(var);
  return 0;
}

/* Emulate unsetenv() by fiddling with the environment. */
int punsetenv(const char *name)
{
  extern char **environ;
  char **p, **q;
  int len = strlen(name);

  q = environ;
  for (p = environ; *p; p++)
    {
      if (strncmp(*p, name, len) != 0 || (*p)[len] != '=')
	*q++ = *p;
    }
  *q = NULL;
  return 0;
}

#ifdef HAVE_KRB5
/* This routine takes v4 kinit parameters and performs a V5 kinit.
 * 
 * name, instance, realm is the v4 principal information
 *
 * lifetime is the v4 lifetime (i.e., in units of 5 minutes)
 * 
 * password is the password
 *
 * ret_cache_name is an optional output argument in case the caller
 * wants to know the name of the actual V5 credentials cache (to put
 * into the KRB5_ENV_CCNAME environment variable)
 *
 * etext is a mandatory output variable which is filled in with
 * additional explanatory text in case of an error.
 */
static krb5_error_code do_v5_kinit(char *name, char *instance, char *realm,
				   int lifetime, char *password,
				   char **ret_cache_name, char **etext)
{
  krb5_context context;
  krb5_error_code retval;
  krb5_principal me = 0, server = 0;
  krb5_ccache ccache = NULL;
  krb5_creds my_creds;
  krb5_get_init_creds_opt options;

  const char *cache_name;

  *etext = 0;
  if (ret_cache_name)
    *ret_cache_name = 0;
  memset(&my_creds, 0, sizeof(my_creds));
  krb5_get_init_creds_opt_init(&options);

  retval = krb5_init_context(&context);
  if (retval)
    return retval;

  cache_name = krb5_cc_default_name(context);
  krb5_init_ets(context);

  retval = krb5_425_conv_principal(context, name, instance, realm, &me);
  if (retval)
    {
      *etext = "while converting V4 principal";
      goto cleanup;
    }

  retval = krb5_cc_resolve(context, cache_name, &ccache);
  if (retval)
    {
      *etext = "while resolving ccache";
      goto cleanup;
    }

  retval = krb5_cc_initialize(context, ccache, me);
  if (retval)
    {
      *etext = "while initializing cache";
      goto cleanup;
    }

  retval = krb5_build_principal_ext(context, &server,
				    krb5_princ_realm(context,
						     me)->length,
				    krb5_princ_realm(context, me)->data,
				    KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
				    krb5_princ_realm(context,
						     me)->length,
				    krb5_princ_realm(context, me)->data,
				    0);
  if (retval)
    {
      *etext = "while building server name";
      goto cleanup;
    }

  my_creds.client = me;
  my_creds.server = server;

  krb5_get_init_creds_opt_set_tkt_life(&options, lifetime * 5 * 60);
  krb5_get_init_creds_opt_set_forwardable(&options, 1);
  krb5_get_init_creds_opt_set_proxiable(&options, 1);
  retval = krb5_get_init_creds_password(context, &my_creds, me, password,
					NULL, NULL, 0, NULL, &options);
  if (retval)
    {
      *etext = "while calling krb5_get_init_creds_password";
      goto cleanup;
    }

  retval = krb5_cc_store_cred(context, ccache, &my_creds);
  if (retval)
    {
      *etext = "while calling krb5_cc_store_cred";
      goto cleanup;
    }

  if (ret_cache_name)
    {
      *ret_cache_name = malloc(strlen(cache_name) + 1);
      if (!*ret_cache_name)
	{
	  retval = ENOMEM;
	  goto cleanup;
	}
      strcpy(*ret_cache_name, cache_name);
    }

cleanup:
  if (me)
    krb5_free_principal(context, me);
  if (server)
    krb5_free_principal(context, server);
  if (ccache)
    krb5_cc_close(context, ccache);
  my_creds.client = 0;
  my_creds.server = 0;
  krb5_free_cred_contents(context, &my_creds);
  krb5_free_context(context);
  return retval;
}

static krb5_error_code do_v5_kdestroy(const char *cachename)
{
  krb5_context context;
  krb5_error_code retval;
  krb5_ccache cache;

  retval = krb5_init_context(&context);
  if (retval)
    return retval;

  if (!cachename)
    cachename = krb5_cc_default_name(context);

  krb5_init_ets(context);

  retval = krb5_cc_resolve(context, cachename, &cache);
  if (retval)
    {
      krb5_free_context(context);
      return retval;
    }

  retval = krb5_cc_destroy(context, cache);

  krb5_free_context(context);
  return retval;
}
#endif /* HAVE_KRB5 */
