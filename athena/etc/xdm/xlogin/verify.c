/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/verify.c,v 1.19 1992-01-27 07:12:30 probe Exp $
 */

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utmp.h>
#include <netdb.h>
#include <ttyent.h>
#include <krb.h>
#include <hesiod.h>
#include <errno.h>
#include <syslog.h>

#ifdef _IBMR2
#include <userpw.h>
#include <usersec.h>
#include <sys/id.h>
#endif


#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define ROOT 0
#define LOGIN_TKT_DEFAULT_LIFETIME DEFAULT_TKT_LIFE /* from krb.h */
#define PASSWORD_LEN 14
#define TEMP_DIR_PERM 0710
#define MAXENVIRON 32

/* homedir status */
#define HD_LOCAL 0
#define HD_ATTACHED 1
#define HD_TEMP 2

#ifndef NOLOGIN
#define	NOLOGIN "/etc/nologin"
#endif
#ifndef NOCREATE
#define NOCREATE "/etc/nocreate"
#endif
#ifndef NOATTACH
#define NOATTACH "/etc/noattach"
#endif
#ifndef NOWLOCAL
#define NOWLOCAL "/etc/nowarnlocal"
#endif
#define MOTD "/etc/motd"
#define UTMP "/etc/utmp"
#define WTMP "/usr/adm/wtmp"
#define TMPDOTFILES "/usr/athena/lib/prototype_tmpuser/."

char *defaultpath = "/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin/X11:/usr/new:/usr/ucb:/bin:/usr/bin:/usr/ibm:/usr/andrew/bin:.";

#define file_exists(f) (access((f), F_OK) == 0)


extern char *crypt(), *lose(), *getenv();
extern char *krb_get_phost(); /* should be in <krb.h> */
char *get_tickets(), *attachhomedir(), *malloc(), *strsave(), *add_to_group();
int abort_verify();
extern int attach_state, attach_pid, attachhelp_state, attachhelp_pid;
extern int errno, quota_pid;

int homedir_status = HD_LOCAL;
int added_to_passwd = FALSE;


char *dologin(user, passwd, option, script, tty, session, display)
char *user;
char *passwd;
int option;
char *script;
char *tty;
char *session;
char *display;
{
    static char errbuf[5120];
    char tkt_file[128], *msg, wgfile[16];
    struct passwd *pwd;
    struct group *gr;
    struct timeval times[2];
    long salt;
    char saltc[2], c;
    char encrypt[PASSWORD_LEN+1];
    char **environment, **glist;
    char fixed_tty[16], *p;
#if defined(_AIX) && defined(_IBMR2)
    char *newargv[4];
#endif
    int i;

    /* state variables: */
    int local_passwd = FALSE;	/* user is in local passwd file */
    int local_ok = FALSE;	/* verified from local password file */
    int nocreate = FALSE;	/* not allowed to modify passwd file */
    int nologin = FALSE;	/* logins disabled */

    /* 4.2 vs 4.3 style syslog */
#ifdef ultrix
    openlog("login", LOG_NOTICE);
#else
    openlog("login", LOG_ODELAY, LOG_AUTH);
#endif
    nocreate = file_exists(NOCREATE);
    nologin = file_exists(NOLOGIN);

    /* check local password file */
    if ((pwd = getpwnam(user)) != NULL) {
	local_passwd = TRUE;
	if (strcmp(crypt(passwd, pwd->pw_passwd), pwd->pw_passwd)) {
	    if (pwd->pw_uid == ROOT)
	      return("Incorrect root password");
	} else
	  local_ok = TRUE;
    } else {
 	pwd = hes_getpwnam(user);
 	if ((pwd == NULL) || pwd->pw_dir[0] == 0) {
	    bzero(passwd, strlen(passwd));
	    cleanup(NULL);
	    if (hes_error() == HES_ER_NOTFOUND) {
		sprintf(errbuf, "Unknown user name entered (no hesiod information for \"%s\")", user);
		return(errbuf);
	    } else
	      return("Unable to find account information due to network failure.  Try another workstation or try again later.");
 	}
    }

    if (nocreate && !local_passwd) {
	sprintf(errbuf, "You are not allowed to log into this workstation.  Contact the workstation's administrator or a consultant for further information.  (User \"%s\" is not in the password file and No_Create is set.)", user);
	return(errbuf);
    }

    /* The terminal name on the Rios is likely to be something like pts/0; we */
    /* don't want any  /'s in the path name; replace them with _'s */
    strcpy(fixed_tty,tty);
    while (p = index(fixed_tty,'/'))
      *p = '_';
    sprintf(tkt_file, "/tmp/tkt_%s", fixed_tty);
    setenv("KRBTKFILE", tkt_file, 1);
    /* we set the ticket file here because a previous dest_tkt() might
       have cached the wrong ticket file. */
    krb_set_tkt_string(tkt_file);

    /* set real uid/gid for kerberos library */
#ifdef _IBMR2
    setruid_rios(pwd->pw_uid);
    setrgid_rios(pwd->pw_gid);
#else
    setruid(pwd->pw_uid);
    setrgid(pwd->pw_gid);
#endif

    if ((msg = get_tickets(user, passwd)) != NULL && pwd->pw_uid) {
	if (!local_ok) {
	    cleanup(NULL);
	    return(msg);
	} else {
	    prompt_user("Unable to get full authentication, you will have local access only during this login session (failed to get kerberos tickets).  Continue anyway?", abort_verify);
	}
    }

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

    /* don't need the password anymore */
    bzero(passwd, strlen(passwd));

    if (!local_passwd)
      pwd->pw_passwd = encrypt;

    /* if NOLOGINs and we're not root, display the contents of the
     * nologin file */
    if (nologin && pwd->pw_uid != ROOT) {
	int f, count;
	char *p;

	strcpy(errbuf, "Logins are currently disabled on this workstation.  ");
	p = &errbuf[strlen(errbuf)];
	f = open(NOLOGIN, O_RDONLY, 0);
	if (f > 0) {
	    count = read(f, p, sizeof(errbuf) - strlen(errbuf) - 1);
	    close(f);
	    p[count] = 0;
	}
	cleanup(NULL);
	return(errbuf);
    }

    /* Make sure root login is on a secure tty */
    if (pwd->pw_uid == ROOT) {
	struct ttyent *te;
	if ((te = getttynam(tty)) != NULL &&
	    !(te->ty_status & TTY_SECURE)) {
	    sprintf(errbuf, "ROOT LOGIN refused on %s", tty);
	    syslog(LOG_CRIT, errbuf);
	    cleanup(NULL);
	    return(errbuf);
	}
    }

    /* if mail-check login selected, do that now. */
    if (option == 4) {
	attach_state = -1;
	switch(attach_pid = fork()) {
	case -1:
	    fprintf(stderr, "Unable to fork to check your mail.\n");
	    break;
	case 0:
	    if (setuid(pwd->pw_uid) != 0) {
		fprintf(stderr, "Unable to set user ID to check your mail.\n");
		_exit(-1);
	    }
	    printf("Electronic mail status:\n");
	    execlp("from", "from", "-r", user, NULL);
	    fprintf(stderr, "Unable to run mailcheck program.\n");
	    _exit(-1);
	default:
	    while (attach_state == -1)
	      sigpause(0);
	    printf("\n");
	    prompt_user("A summary of your waiting email is displayed in the console window.  Continue with full login session or logout now?", abort_verify);
	}
    }

    if (msg = attachhomedir(pwd)) {
	cleanup(pwd);
	return(msg);
    }

    /* put in password file if necessary */
    if (!local_passwd && add_to_passwd(pwd)) {
	cleanup(pwd);
	return("An unexpected error occured while entering you in the local password file.");
    }

    switch(quota_pid = fork()) {
    case -1:
	fprintf(stderr, "Unable to fork to check your filesystem quota.\n");
	break;
    case 0:
	if (setuid(pwd->pw_uid) != 0) {
	    fprintf(stderr, "Unable to set user ID to check your filesystem quota.\n");
	    _exit(-1);
	}
	execlp("quota", "quota", NULL);
	fprintf(stderr, "Unable to run quota command %s\n", "quota");
	_exit(-1);
    default:
	  ;
    }

    /* show message of the day */
    sprintf(errbuf, "%s/.hushlogin", pwd->pw_dir);
    if (!file_exists(errbuf)) {
	int f, count;
	f = open(MOTD, O_RDONLY, 0);
	if (f > 0) {
	    count = read(f, errbuf, sizeof(errbuf) - 1);
	    write(1, errbuf, count);
	    close(f);
	}
    }

    if (!nocreate) {
	glist = hes_resolve(user, "grplist");
	if (glist && glist[0]) {
	    /* add_to_group() will corrupt the list, so was save a copy first */
	    strcpy(errbuf, glist[0]);
	    if (msg = add_to_group(user, glist[0])) {
		cleanup(pwd);
		return(msg);
	    }
	    strcpy(glist[0], errbuf);
	} else if (!local_passwd)
	  fprintf(stderr,
		  "Warning: could not get any groups for you from Hesiod.\n");
    }

    environment = (char **) malloc(MAXENVIRON * sizeof(char *));
    if (environment == NULL)
      return("Out of memory while trying to initialize user environment variables.");

    i = 0;
#if defined(_AIX) && defined(_IBMR2)
    environment[i++] = "USRENVIRON:";
#endif
    sprintf(errbuf, "HOME=%s", pwd->pw_dir);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf, "PATH=%s", defaultpath);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf, "USER=%s", pwd->pw_name);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf, "SHELL=%s", pwd->pw_shell);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf, "KRBTKFILE=%s", tkt_file);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf, "DISPLAY=%s", display);
    environment[i++] = strsave(errbuf);
    if (homedir_status == HD_TEMP) {
	environment[i++] = "TMPHOME=1";
    }
    strcpy(wgfile, "/tmp/wg.XXXXXX");
    mktemp(wgfile);
    sprintf(errbuf, "WGFILE=%s", wgfile);
    environment[i++] = strsave(errbuf);
#if defined(_AIX) && defined(i386)
    environment[i++] = "hosttype=ps2";
#endif
#if defined(_AIX) && defined(_IBMR2)
    environment[i++] = "hosttype=rsaix";
#endif
#if defined(ultrix) && defined(mips)
    environment[i++] = "hosttype=decmips";
#endif
    msg = getenv("TZ");
    if (msg) {                /* Pass along timezone */
	sprintf(errbuf, "TZ=%s", msg);
	environment[i++] = strsave(errbuf);
    }
#if defined(_AIX) && defined(_IBMR2)
    environment[i++] = "SYSENVIRON:";
    sprintf(errbuf,"LOGIN=%s",pwd->pw_name);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf,"LOGNAME=%s",pwd->pw_name);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf,"NAME=%s",pwd->pw_name);
    environment[i++] = strsave(errbuf);
    sprintf(errbuf,"TTY=%s",tty);
    environment[i++] = strsave(errbuf);
#endif
    environment[i++] = NULL;

    add_utmp(user, tty, display);
    if (pwd->pw_uid == ROOT)
      syslog(LOG_CRIT, "ROOT LOGIN on tty %s", tty);

    /* Set the owner and modtime on the tty */
    sprintf(errbuf, "/dev/%s", tty);
    gr = getgrnam("tty");
    chown(errbuf, pwd->pw_uid, gr ? gr->gr_gid : pwd->pw_gid);
    chmod(errbuf, 0620);
    gettimeofday(&times[0], NULL);
    times[1].tv_sec = times[0].tv_sec;
    times[1].tv_usec = times[0].tv_usec;
    utimes(errbuf, times);

#if defined(_AIX) && defined(_IBMR2)
    i = setgidx(ID_SAVED|ID_REAL|ID_EFFECTIVE, pwd->pw_gid);
#else
    i = setgid(pwd->pw_gid);
#endif
    if (i)
      return(lose("Unable to set your primary GID.\n"));

    if (initgroups(user, pwd->pw_gid) < 0)
      prompt_user("Unable to set your group access list.  You may have insufficient permission to access some files.  Continue with this login session anyway?", abort_verify);

#if defined(_AIX) && defined(_IBMR2)
    i = setuidx(ID_LOGIN|ID_SAVED|ID_REAL|ID_EFFECTIVE, pwd->pw_uid);
#else
    i = setreuid(pwd->pw_uid, pwd->pw_uid);
#endif
    if (i)
      return(lose("Unable to set your user ID.\n"));

    if (chdir(pwd->pw_dir))
      fprintf(stderr, "Unable to connect to your home directory.\n");

    sprintf(errbuf, "%d", option);
#if defined(_AIX) && defined(_IBMR2)
    newargv[0] = session;
    newargv[1] = errbuf;
    newargv[2] = script;
    newargv[3] = NULL;
    setpenv(pwd->pw_name,PENV_KLEEN|PENV_INIT|PENV_ARGV,environment,newargv);
#else
    execle(session, "sh", errbuf, script, NULL, environment);
#endif

    return(lose("Failed to start session."));
}


char *get_tickets(username, password)
char *username;
char *password;
{
    char inst[INST_SZ], realm[REALM_SZ];
    char hostname[MAXHOSTNAMELEN], phost[INST_SZ];
    char key[8], *rcmd;
    static char errbuf[1024];
    int error;
    struct hostent *hp;
    KTEXT_ST ticket;
    AUTH_DAT authdata;
    unsigned long addr;

    rcmd = "rcmd";

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
    switch (error) {
    case KSUCCESS:
	break;
    case INTK_BADPW:
	return("Incorrect password entered.");
    case KDC_PR_UNKNOWN:
	return("Unknown username entered.");
    default:
	sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s.  Try again here or on another workstation.",
		error, krb_err_txt[error]);
	return(errbuf);
    }

    if (gethostname(hostname, sizeof(hostname)) == -1) {
	fprintf(stderr, "Warning: cannot retrieve local hostname");
	return(NULL);
    }
    strncpy (phost, krb_get_phost (hostname), sizeof (phost));
    phost[sizeof(phost)-1] = '\0';

    /* without srvtab, cannot verify tickets */
    if (read_service_key(rcmd, phost, realm, 0, KEYFILE, key) == KFAILURE)
      return (NULL);

    hp = gethostbyname (hostname);
    if (!hp) {
	fprintf(stderr, "Warning: cannot get address for host %s\n", hostname);
	return(NULL);
    }
    bcopy ((char *)hp->h_addr, (char *) &addr, sizeof (addr));

    error = krb_mk_req(&ticket, rcmd, phost, realm, 0);
    if (error == KDC_PR_UNKNOWN) return(NULL);
    if (error != KSUCCESS) {
	sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s",
		error, krb_err_txt[error]);
	return(errbuf);
    }
    error = krb_rd_req(&ticket, rcmd, phost, addr, &authdata, "");
    if (error != KSUCCESS) {
	bzero(&ticket, sizeof(ticket));
	sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s",
		error, krb_err_txt[error]);
	return(errbuf);
    }
    bzero(&ticket, sizeof(ticket));
    bzero(&authdata, sizeof(authdata));
    return(NULL);
}


cleanup(pwd)
struct passwd *pwd;
{
    /* must also detach homedir, clean passwd file */
    dest_tkt();
    if (pwd && homedir_status == HD_ATTACHED) {
	attach_state = -1;
	switch (attach_pid = fork()) {
	case -1:
	    fprintf(stderr, "Unable to detach your home directory (could not fork to create attach process).");
	    break;
	case 0:
	    if (setuid(pwd->pw_uid) != 0) {
		fprintf(stderr,
			"Could not execute dettach command as user %s,\n",
			pwd->pw_name);
	    }
	    execlp("detach", "detach", "-quiet", pwd->pw_name, NULL);
	    _exit(-1);
	default:
	    while (attach_state == -1)
	      sigpause(0);
	}
    }
    if (pwd && added_to_passwd)
      remove_from_passwd(pwd);

    /* Set real uid to zero.  If this is impossible, exit.  The
       current implementation of lose() will not print a message
       so xlogin will just exit silently.  This call "can't fail",
       so this is not a serious problem. */
    if (setuid(0) == -1)
      lose ("Unable to reset real uid to root");
}


add_to_passwd(p)
struct passwd *p;
{
    int i, fd = -1;
    FILE *etc_passwd;

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
      return(errno);

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
    putuserattr(p->pw_name,"athena_temp",1,SEC_INT);
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
      return(errno);

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
	  return(errno);
	else
	  (void) unlink("/etc/ptmp");
    }

    etc_passwd = fopen("/etc/passwd", "a");
    if (etc_passwd == NULL) {
	(void) close(fd);
	(void) unlink("/etc/ptmp");
	return(-1);
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

    /* This tells the display manager to cleanup the password file for
     * us after we exit
     */
    kill(getppid(), SIGUSR2);
    added_to_passwd = TRUE;
    return(0);
}


#if defined(_AIX) && defined(_IBMR2)
remove_from_passwd(p)
struct passwd *p;
{
    char buf[1024];

    switch (quota_pid = fork()) {
    case -1:
	fprintf(stderr, "Unable to fork to edit password file.\n");
	break;
    case 0:
	execl("/bin/rmuser", "rmuser", "-p", p->pw_name, NULL);
	fprintf(stderr, "Unable to exec rmuser to edit passwd file.\n");
	_exit(-1);
    }

    /* Wait for rmuser to complete */
    while (quota_pid > 0)
	sigpause(0);

    return(0);
}
#else /* RIOS */
remove_from_passwd(p)
struct passwd *p;
{
    int fd, len, i;
    char buf[512];
    FILE *old, *new;

    for (i = 0; i < 10; i++)
      if ((fd = open("/etc/ptmp", O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1 &&
	  errno == EEXIST)
	sleep(1);
      else
	break;
    if (fd == -1) {
	if (i < 10)
	  return(errno);
    }

    old = fopen("/etc/passwd", "a");
    if (old == NULL) {
	(void) close(fd);
	(void) unlink("/etc/ptmp");
	return(-1);
    }
    new = fdopen(fd, "w");
    len = strlen(p->pw_name);

    while (fgets(buf, sizeof(buf) - 1, old)) {
	if (strncmp(p->pw_name, buf, len - 1) || buf[len] != ':')
	  fputs(buf, new);
    }

    (void) fclose(old);
    (void) fclose(new);
    (void) rename("/etc/ptmp", "/etc/passwd");
    return(0);
}
#endif /* RIOS */


abort_verify()
{
    cleanup(NULL);
    _exit(1);
}


char *attachhomedir(pwd)
struct passwd *pwd;
{
    struct stat stb;
    int i;

    /* Delete empty directory if it exists.  We just try to rmdir the 
     * directory, and if it's not empty that will fail.
     */
    rmdir(pwd->pw_dir);

    /* If a good local homedir exists, use it */
    if (file_exists(pwd->pw_dir) && !IsRemoteDir(pwd->pw_dir) &&
	homedirOK(pwd->pw_dir))
      return(NULL);

    /* Using homedir already there that may or may not be good. */
    if (file_exists(NOATTACH) && file_exists(pwd->pw_dir) &&
	homedirOK(pwd->pw_dir)) {
	prompt_user("This workstation is configured not to attach remote filesystems.  Continue with your local home directory?", abort_verify);
	return(NULL);
    }

    if (file_exists(NOATTACH))
      return("This workstation is configured not to create local home directories.  Please contact the system administrator for this machine or a consultant for further information.");

    /* attempt attach now */
    attach_state = -1;
    switch (attach_pid = fork()) {
    case -1:
	return("Unable to attach your home directory (could not fork to create attach process).  Try another workstation.");
    case 0:
 	if (setuid(pwd->pw_uid) != 0) {
 	    fprintf(stderr, "Could not execute attach command as user %s,\n",
 		    pwd->pw_name);
 	    fprintf(stderr, "Filesystem mappings may be incorrect.\n");
 	}
	/* don't do zephyr here since user doesn't have zwgc started anyway */
	execlp("attach", "attach", "-quiet", "-nozephyr", pwd->pw_name, NULL);
	_exit(-1);
    default:
	break;
    }
    while (attach_state == -1) {
	sigpause(0);
    }

    if (attach_state != 0 || !file_exists(pwd->pw_dir)) {
	prompt_user("Your home directory could not be attached.  Try again?",
		    abort_verify);
	/* attempt attach again */
	attach_state = -1;
	switch (attach_pid = fork()) {
	case -1:
	    return("Unable to attach your home directory (could not fork to create attach process).  Try another workstation.");
	case 0:
	    if (setuid(pwd->pw_uid) != 0) {
		fprintf(stderr,
			"Could not execute attach command as user %s,\n",
			pwd->pw_name);
		fprintf(stderr, "Filesystem mappings may be incorrect.\n");
	    }
	    /* don't do zephyr here since user doesn't have zwgc started */
	    execlp("attach", "attach", "-quiet", "-nozephyr",
		   pwd->pw_name, NULL);
	    _exit(-1);
	default:
	    break;
	}
	while (attach_state == -1) {
	    sigpause(0);
	}
    }

    if (attach_state != 0 || !file_exists(pwd->pw_dir)) {
	/* do tempdir here */
	char buf[BUFSIZ];
	homedir_status = HD_TEMP;

	prompt_user("Your home directory is still unavailable.  A temporary directory will be created for you.  However, it will be DELETED when you logout.  Any mail that you incorporate during this session WILL BE LOST when you logout.  Continue with this session anyway?", abort_verify);
	sprintf(buf, "/tmp/%s", pwd->pw_name);
	pwd->pw_dir = malloc(strlen(buf)+1);
	strcpy(pwd->pw_dir, buf);

	i = lstat(buf, &stb);
	if (i == 0) {
	    if ((stb.st_mode & S_IFMT) == S_IFDIR) {
		fprintf(stderr, "Warning - The temporary directory already exists.\n");
		return(NULL);
	    } else unlink(buf);
	} else if (errno != ENOENT)
	  return("Error while retrieving status of temporary homedir.");

	if (setreuid(ROOT, pwd->pw_uid) != 0)
	  return("Error while setting user ID to make temporary home directory.");

	if (mkdir(buf, TEMP_DIR_PERM))
	  return("Error while creating temporary directory.");

	attachhelp_state = -1;
	switch (attachhelp_pid = fork()) {
	case -1:
	    fprintf(stderr, "Warning - could not fork to copy user prototype files into temporary directory.\n");
	    return (NULL);
	case 0:
	    /* redirect to /dev/null to make cp quiet */
	    close(1);
	    close(2);
	    open("/dev/null", O_RDWR, 0);
	    dup(1);
	    execl("/bin/cp", "cp", "-r", TMPDOTFILES, buf, NULL);
	    fprintf(stderr, "Warning - could not copy user prototype files into temporary directory.\n");
	    _exit(-1);
	default:
	    break;
	}
	while (attachhelp_state == -1)
	  sigpause(0);

	if (chmod(buf, TEMP_DIR_PERM))
	  return("Could not change protections on temporary directory.");
	setreuid(ROOT, ROOT);
    } else
      homedir_status = HD_ATTACHED;
    return(NULL);
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
#if !defined(_AIX)
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
#else /* AIX */
    struct stat stbuf;

    if (statx(dir, &stbuf, 0, STX_NORMAL))
	return(FALSE);
    return((stbuf.st_flag & FS_REMOTE) ? TRUE : FALSE);
#endif
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


char *strsave(s)
char *s;
{
    char *ret = malloc(strlen(s) + 1);
    strcpy(ret, s);
    return(ret);
}


add_utmp(user, tty, display)
char *user;
char *tty;
char *display;
{
    struct utmp ut_entry;
    int f;
#if !(defined(_AIX) && defined(i386)) /* not PS/2 */
    int slot = ttyslot();
#endif

    bzero(&ut_entry, sizeof(ut_entry));
    strncpy(ut_entry.ut_line, tty, 8);
    strncpy(ut_entry.ut_name, user, 8);
    /* leave space for \0 */
    strncpy(ut_entry.ut_host, display, 15);
    ut_entry.ut_host[15] = 0;
    time(&(ut_entry.ut_time));
#ifdef _AIX
    strncpy(ut_entry.ut_id, "", 6);
    ut_entry.ut_pid = getppid();
    ut_entry.ut_type = USER_PROCESS;
#endif /* _AIX */

#if !(defined(_AIX) && defined(i386)) /* not PS/2 */
    if ((f = open( UTMP, O_WRONLY )) >= 0) {
	lseek(f, (long) ( slot * sizeof(ut_entry) ), L_SET);
#else /* _AIX */
    if ((f = open(UTMP, O_RDWR )) >= 0) {
	struct utmp ut_tmp;
	while (read(f, (char *) &ut_tmp, sizeof(ut_tmp)) == sizeof(ut_tmp))
	  if (ut_tmp.ut_pid == getppid())
	    break;
	if (ut_tmp.ut_pid == getppid())
	  lseek(f, -(long) sizeof(ut_tmp), 1);
	strncpy(ut_entry.ut_id, ut_tmp.ut_id, 6);
#endif /* _AIX */
	write(f, (char *) &ut_entry, sizeof(ut_entry));
	close(f);
    }

    if ( (f = open( WTMP, O_WRONLY|O_APPEND)) >= 0) {
	write(f, (char *) &ut_entry, sizeof(ut_entry));
	close(f);
    }
}


#define MAXGNAMELENGTH	32

#ifdef _IBMR2
char *add_to_group(name, glist)
char *name;
char *glist;
{
  char *cp,*p;			/* temporary */
  char *gname, *new_list;
  char *fix_gname();
  int gid;
  int len,namelen;
  int i, fd = -1, ngroups,found;
  static char data[BUFSIZ];
  char *members;
  
  for (i = 0; i < 10; i++)
    if (setuserdb(S_WRITE) == -1)
      sleep(1);
    else {
      fd = 1;
      break;
    }

  if (fd == -1) {
    sprintf(data, "Locking of user db failed: errno %d", errno);
    enduserdb();
    return(data);
  }
  
  /* count groups (there are 2 ':'s in the group list per group, except
     the first group only has one) */
  cp = glist;
  ngroups = 1;
  while (cp = index(cp, ':')) {
    ngroups++;
    cp++;
  }
  ngroups /= 2;
  if (ngroups > NGROUPS) {
    fprintf(stderr, "Warning - you are in too many groups.  Some of them will be ignored.\n");
  }

  cp = glist;
  for (i = 0; i < ngroups; i++) {
    gname = cp;
    cp = index(cp, ':');
    *cp++ = '\0';
    gid = atoi(cp);
    if (cp = index(cp, ':'))
      *cp++ = '\0';
    
    /* AIX restricts group names to 8 alphanumeric characters; the first */
    /* must be alpha.  Many existing groups do not meet this criteria, and */
    /* must be mapped into a group named "G###" where ### is the gid */
    /* Not a perfect solution- */
    gname = fix_gname(gname,gid);
    /* Make sure the group exists */
    if (getgroupattr(gname,S_USERS,(void *)&members,SEC_LIST) == -1) {
      if (errno == ENOENT) {
	/* Create group */
	if (putgroupattr(gname,(char *)NULL,((void *) 0),SEC_NEW) != 0) {
	  fprintf(stderr,"Error creating group %s (%d)\n",gname,gid);
	  continue;
	}
	putgroupattr(gname,S_ID,gid,SEC_INT);
	putgroupattr(gname,S_ADMIN,0,SEC_BOOL);
	putgroupattr(gname,"athena_temp",1,SEC_BOOL);
	putgroupattr(gname,(char *)NULL,((void *) 0),SEC_COMMIT);
      }
    }
    /* Add user to group */

    p = members;
    found = 0;
    while (1) {
      if (strcmp(p,name) == 0) {
	found = 1;
	break;
      }
      p = index(p,'\0')+1;
      if (*p == '\0')
	break;
    }
  
    if (found)
      continue;
    
    /* Add user to group */
    len = p - members + 1;
    namelen = strlen(name);
    new_list = (char *)malloc(len+namelen+1);
    bcopy(name,new_list,namelen+1);
    bcopy(members,(char *)(new_list+namelen+1),len);
    if (putgroupattr(gname,S_USERS,new_list,SEC_LIST) == -1) {
      sprintf(data,"Update of group %s failed; errno %d", gname, errno);
      return(data);
    }
    
    if (putgroupattr(gname,(char *)NULL,((void *) 0),SEC_COMMIT) == -1) {
      sprintf(data,"Commit of group %s failed; errno %d", gname, errno);
      return(data);
    }
  }
  enduserdb();
  return(NULL);
}

char *
fix_gname(gname,gid)
     char *gname;
     int gid;
{
  int valid=1;
  static char mname[8];
  char *p;

  if ((strlen(gname) > 8) || (!isalpha(gname[0])))
    valid = 0;
  else {
    for(p=gname;(*p)!='\0';p++)
      if (!isalnum(*p)) {
	valid = 0;
	break;
      }
  }

  if (valid)
    return(gname);
  else {
    sprintf(mname,"G%d",gid);
    return(mname);
  }
}

#else /* _IBMR2 */
char *add_to_group(name, glist)
char *name;
char *glist;
{
    char *cp;			/* temporary */
    char **gnames = NULL, **gids;/* array of group names, numbers */
    int i, fd = -1, ngroups;
    int namelen = strlen(name);
    FILE *etc_group, *etc_gtmp;
    char data[BUFSIZ+MAXGNAMELENGTH];	/*  space to add new username */

    for (i = 0; i < 10; i++)
      if ((fd = open("/etc/gtmp", O_RDWR | O_EXCL | O_CREAT, 0644)) == -1 &&
	  errno == EEXIST)
	sleep(1);
      else
	break;
    if (fd == -1) {
	if (i < 10) {
	    sprintf(data, "Update of group file failed: errno %d", errno);
	    return(data);
	} else
	  unlink("/etc/gtmp");
    }

    if ((etc_gtmp = fdopen(fd, "w")) == NULL ||	/* can't happen ? */
	(etc_group = fopen("/etc/group", "r")) == NULL) {
	(void) close(fd);
	(void) unlink("/etc/gtmp");
	return("Failed to open temporary group file to update your access control groups.");
    }

    /* count groups (there are 2 ':'s in the group list per group, except
       the first group only has one) */
    cp = glist;
    ngroups = 1;
    while (cp = index(cp, ':')) {
	ngroups++;
	cp++;
    }
    ngroups /= 2;
    if (ngroups > NGROUPS) {
	fprintf(stderr, "Warning - you are in too many groups.  Some of them will be ignored.\n");
    }

    if ((gnames = (char **)malloc(ngroups * sizeof(char *))) == NULL ||
	(gids = (char **)malloc(ngroups * sizeof(char *))) == NULL) {
	if (gnames)
	  free(gnames);
	(void) fclose(etc_gtmp);
	(void) fclose(etc_group);
	(void) unlink("/etc/gtmp");
	return("Ran out of memory while updating your access control groups");
    }
    cp = glist;
    for (i = 0; i < ngroups; i++) {
	gnames[i] = cp;
	cp = index(cp, ':');
	*cp++ = '\0';
	gids[i] = cp;
	if (cp = index(cp, ':'))
	  *cp++ = '\0';
    }

    while (fgets(data, sizeof(data) - MAXGNAMELENGTH, etc_group)) {
	char *gpwd, *gid, *guserlist = NULL;
	int add = -1;	/* index of group entry in user's hesiod list */

	if (data[0] == '\0')
	  continue;	/* empty line ??? */

	/* If a valid format line, check to see if the user belongs in
	   the group.  Otherwise, just write it out as-is. */
	if ((gpwd = index(data, ':')) &&
	    (gid = index(++gpwd, ':')) &&
	    (guserlist = index(++gid, ':'))) {
	    *guserlist = '\0';
	    /* step through our groups */
	    for (i = 0; i < ngroups; i++)
	      if (gids[i] && !strcmp(gid, gids[i])) {
		  /* found it, now check users */
		  for (cp = guserlist; cp; cp = index(cp, ',')) {
		      cp++;
		      if (!strncmp(name, cp, strlen(name)) &&
			  (cp[namelen] == ',' ||
			   cp[namelen] == ' ' ||
			   cp[namelen] == '\n')) {
			  gnames[i] = NULL;
			  gids[i] = NULL;
			  break;
		      }
		  }
		  if (gnames[i] != NULL)
		    add = i;
		  break;
	      }
	    *guserlist++ = ':';
	}
	if (add != -1) {
	    char *end_userlist = guserlist + strlen(guserlist);
	    *(end_userlist-1) = ',';	/* overwrite newline */
	    strcpy(end_userlist, name);
	    *(end_userlist + namelen) = '\n';
	    *(end_userlist + namelen + 1) = 0;
	    gnames[add] = NULL;
	    gids[add] = NULL;
	}
	if (fputs(data, etc_gtmp) == EOF && ferror(etc_gtmp)) {
	    (void) fclose(etc_gtmp);
	    goto fail;
	}
    }	/* end while */

    /* now append all groups remaining in gids[], gnames[] */
    for (i = 0;i < ngroups;i++)
      if (gids[i] && gnames[i]) {
	  fprintf(etc_gtmp, "%s:*:%s:%s\n", gnames[i], gids[i], name);
      }

    (void) fchmod(fd, 0644);
    if (fclose(etc_gtmp) == EOF)
      goto fail;

    (void) fclose(etc_group);
    free(gids);
    free(gnames);
    if (rename("/etc/gtmp", "/etc/group") == 0)
      return(NULL);
    else {
	sprintf(data, "Failed to install your access control groups in the group file; errno %d", errno);
	return(data);
    }

 fail:
    (void) unlink(etc_gtmp);
    (void) fclose(etc_group);
    free(gids);
    free(gnames);
    return("Failed to update your access control groups");
}
#endif /* _IBMR2 */


#if defined(_AIX) && defined(_IBMR2)

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
static int setruid_rios(uid_t ruid)
#else
static int setruid_rios(ruid)
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
static int seteuid_rios(uid_t euid)
#else
static int seteuid_rios(euid)
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
static int setreuid_rios(uid_t ruid, uid_t euid)
#else
static int setreuid_rios(ruid, euid)
  uid_t ruid;
  uid_t euid;
#endif /* __STDC__ */
{
    if (seteuid_rios(euid) == -1)
        return (-1);

    return (setruid_rios(ruid));
}

#ifdef __STDC__
static int setuid_rios(uid_t uid)
#else
static int setuid_rios(uid)
  uid_t uid;
#endif /* __STDC__ */
{
    return (setreuid_rios(uid, uid));
}

#ifdef __STDC__
static int setrgid_rios(gid_t rgid)
#else
static int setrgid_rios(rgid)
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
static int setegid_rios(gid_t egid)
#else
static int setegid_rios(egid)
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
static int setregid_rios(gid_t rgid, gid_t egid)
#else
static int setregid_rios(rgid, egid)
  gid_t rgid;
  gid_t egid;
#endif /* __STDC__ */
{
    if (setegid_rios(egid) == -1)
        return (-1);

    return (setrgid_rios(rgid));
}

#ifdef __STDC__
static int setgid_rios(gid_t gid)
#else
static int setgid_rios(gid)
  gid_t gid;
#endif /* __STDC__ */
{
    return (setregid_rios(gid, gid));
}

#endif /* RIOS */
