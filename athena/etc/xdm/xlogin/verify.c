/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/verify.c,v 1.2 1990-11-01 18:45:34 mar Exp $
 */

#include <stdio.h>
#include <pwd.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <netdb.h>
#include <ttyent.h>
#include <krb.h>
#include <hesiod.h>
#include <errno.h>
#include <syslog.h>


#define FALSE 0
#define TRUE (!FALSE)

#define ROOT 0
#define LOGIN_TKT_DEFAULT_LIFETIME 96 /* 8 hours */
#define PASSWORD_LEN 14

#define	NOLOGIN "/etc/nologin"
#define NOCREATE "/etc/nocreate"
#define NOATTACH "/etc/noattach"
#define NOWLOCAL "/etc/nowarnlocal"
#define ATTACH "/bin/athena/attach"

#define file_exists(f) (access((f), F_OK) == 0)


char *get_tickets(), *attachhomedir();
extern int attach_state, attach_pid;


char *dologin(user, passwd, option, script, tty)
char *user;
char *passwd;
int option;
char *script;
char *tty;
{
    static char errbuf[5120];
    char tkt_file[128], *msg;
    struct passwd *pwd;
    long salt;
    char saltc[2], c;
    char encrypt[PASSWORD_LEN+1];
    int i;

    /* state variables: */
    int local_passwd = FALSE;	/* user is in local passwd file */
    int local_ok = FALSE;	/* verified from local password file */
    int nocreate = FALSE;	/* not allowed to modify passwd file */
    int noattach = FALSE;	/* not allowed to attach homedir */
    int nologin = FALSE;	/* logins disabled */

    nocreate = file_exists(NOCREATE);
    noattach = file_exists(NOATTACH);
    nologin = file_exists(NOLOGIN);

    /* check local password file */
    if ((pwd = getpwnam(user)) != NULL) {
	local_passwd = TRUE;
	if (strcmp(crypt(passwd, pwd->pw_passwd), pwd->pw_passwd)) {
	    if (pwd->pw_uid == ROOT)
	      return("Incorrect root password");
	} else
	  local_ok = TRUE;

    }

    if (nocreate && !local_passwd) {
	sprintf(errbuf, "You are not allowed to log into this workstation.  Contact the workstation's administrator or a consultant for further information.  (User \"%s\" is not in the password file and No_Create is set.)", user);
	return(errbuf);
    }

    sprintf(tkt_file, "/tmp/tkt_%s", tty);
    setenv("KRBTKFILE", tkt_file, 1);

    if ((msg = get_tickets(user, passwd)) != NULL) {
	if (!local_ok) {
	    cleanup();
	    return(msg);
	} else {
	    fprintf(stderr, "Unable to get full authentication, you will have local access only\nduring this login session.\n");
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

    if (!local_passwd) {
	pwd = hes_getpwnam(user);
	if ((pwd == NULL) || pwd->pw_dir[0] == 0) {
	    cleanup();
	    if (hes_error() == HES_ER_NOTFOUND) {
		sprintf(errbuf, "User \"%s\" does not have an active account (no hesiod information)", user);
		return(errbuf);
	    } else
	      return("Unable to find account information due to network failure.  Try another workstation or try again later.");
	}
	pwd->pw_passwd = encrypt;
    }

    if ((chown(tkt_file, pwd->pw_uid, pwd->pw_gid) != 0) &&
	errno != ENOENT) {
	cleanup();
	sprintf(errbuf, "Could not change ownership of Kerberos ticket file \"%s\".", tkt_file);
	return(errbuf);
    }

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
	cleanup();
	return(errbuf);
    }

    /* Make sure root login is on a secure tty */
    if (pwd->pw_uid == ROOT) {
	struct ttyent *te;
	if ((te = getttynam(tty)) != NULL &&
	    !(te->ty_status & TTY_SECURE)) {
	    sprintf(errbuf, "ROOT login refused on %s", tty);
	    syslog(LOG_CRIT, errbuf);
	    cleanup();
	    return(errbuf);
	}
    }

    if (msg = attachhomedir(pwd)) {
	cleanup();
	return(msg);
    }

    /* put in password file if necessary */
    if (!local_passwd && add_to_passwd(pwd)) {
	cleanup();
	return("An unexpected error occured while entering you in the local password file.");
    }

    /* XXXXXX */

/*    run_quota(); */

    /* show message of the day */
    sprintf(errbuf, "%s/.hushlogin", pwd->pw_dir);
    if (!file_exists(errbuf)) {
	int f, count;
	f = open("/etc/motd", O_RDONLY, 0);
	if (f > 0) {
	    count = read(f, errbuf, sizeof(errbuf) - 1);
	    write(1, errbuf, count);
	    close(f);
	}
    }

/*     do_groups(); */

/*    setup_environment(); */

    if (pwd->pw_uid == ROOT)
      syslog(LOG_CRIT, "ROOT login on tty %s", tty);

/*    setreuid(pwd->pw_uid, pwd->pw_uid);
    setgid(pwd->pw_gid);

    run_xsession(); */

    return("Sorry, but the login procedure is not yet completed.");
}


char *get_tickets(username, password)
char *username;
char *password;
{
    char pname[ANAME_SZ], inst[INST_SZ], realm[REALM_SZ], passwd[REALM_SZ];
    char hostname[MAXHOSTNAMELEN], phost[INST_SZ];
    char key[8], *rcmd;
    static char errbuf[1024];
    int error;
    struct hostent *hp;
    KTEXT_ST ticket;
    AUTH_DAT authdata;
    unsigned long addr;

    rcmd = "rcmd";
    strcpy(pname, username);
    strcpy(passwd, password);

    /* inst has to be a buffer instead of the constant "" because
     * krb_get_pw_in_tkt() will write a zero at inst[INST_SZ] to
     * truncate it.
     */
    inst[0] = 0;
    dest_tkt();

    if (krb_get_lrealm(realm, 1) != KSUCCESS)
      strcpy(realm, KRB_REALM);

    error = krb_get_pw_in_tkt(pname, inst, realm, "krbtgt", realm,
			      LOGIN_TKT_DEFAULT_LIFETIME, passwd);
    switch (error) {
    case KSUCCESS:
	break;
    case INTK_BADPW:
	return("Incorrect password entered.");
    case KDC_PR_UNKNOWN:
	return("Unknown username entered.");
    default:
	sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s.  If this problem persists, please report it to Athena hotline.",
		error, krb_err_txt[error]);
	return(errbuf);
    }

    if (gethostname(hostname, sizeof(hostname)) == -1)
      return("Authentication error: cannot retrieve local hostname");
    strncpy (phost, krb_get_phost (hostname), sizeof (phost));
    phost[sizeof(phost)-1] = 0;

    /* without srvtab, cannot verify tickets */
    if (read_service_key(rcmd, phost, realm, 0, KEYFILE, key) == 0)
      return (NULL);

    hp = gethostbyname (hostname);
    if (!hp)
      return("Authentication error: cannot retrieve local host address");
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


cleanup()
{
    /* XXXXXXXXX */
    /* must also detach homedir, clean passwd file */
/*     dest_tkt(); */
}


add_to_passwd(p)
struct passwd *p;
{
    int i, fd = -1;
    FILE *etc_passwd;

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
    return(0);
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
	printf("This workstation is configured not to attach remote filesystems.\n");
	printf("Continuing with your local home directory.\n");
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
	execl(ATTACH, ATTACH, "-quiet", "-nozephyr", pwd->pw_name, NULL);
	exit(-1);
    default:
	break;
    }
    while (attach_state == -1) {
	sigpause(0);
    }

    if (attach_state != 0 || !file_exists(pwd->pw_dir)) {
	/* do tempdir here */
	char buf[BUFSIZ];

	sprintf(buf, "/tmp/%s", pwd->pw_name);
	pwd->pw_dir = malloc(strlen(buf)+1);
	strcpy(pwd->pw_dir, buf);

	i = stat(buf, &stb);
	if (i == 0) {
	    if (stb.st_mode & S_IFDIR) {
		fprintf(stderr, "Warning - The temporary directory already exists.\n");
		return(NULL);
	    } else unlink(buf);
	} else if (i != ENOENT)
	  return("Error while retrieving status of temporary homedir.");

	if (setreuid(ROOT, pwd->pw_uid) != 0)
	  return("Error while setting owner on temporary home directory.");

	if (system("cp -pr /usr/prototype_user /tmp > /dev/null 2> /dev/null")) {
	    fprintf(stderr, "Warning - could not copy user prototype files into temporary directory.\n");
	    mkdir(buf, TEMP_DIR_PERM);
	    return (NULL);
	}

	if (rename("/tmp/prototype_user", buf))
	  return("Unable to put temporary directory into place.  Try again.");

	if (chmod(buf, TEMP_DIR_PERM))
	  return("Could not change protections on temporary directory.");
	setreuid(ROOT, ROOT);
    }
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
  int f;
  char c;
  
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
    struct direct *temp;
    int count;

    if ((dp = opendir(dir)) == NULL)
      return(FALSE);

    /* Make sure that there is something here besides . and .. */
    for (count = 0; count < 3 ; count++)
      temp = readdir(dp);

    closedir(dp);
    return(temp != NULL);
}
