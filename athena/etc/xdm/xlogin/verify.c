/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/verify.c,v 1.1 1990-11-01 11:37:47 mar Exp $
 */

#include <stdio.h>
#include <pwd.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/param.h>
#include <netdb.h>
#include <ttyent.h>
#include <krb.h>
#include <hesiod.h>
#include <errno.h>
#include <syslog.h>


#define FALSE 0
#define TRUE (!FALSE)

#define ROOT 0
#define LOGIN_TKT_DEFAULT_LIFETIME 108 /* 9 hours */
#define PASSWORD_LEN 14
#define	NOLOGIN "/etc/nologin"
#define NOCREATE "/etc/nocreate"
#define NOATTACH "/etc/noattach"

#define file_exists(f) (access((f), F_OK) == 0)


char *get_tickets(), *attachhomedir();

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
    setenv("KRBTKFILE", tkt_file);
    if (!local_ok && ((msg = get_tickets(user, passwd)) != NULL)) {
	dest_tkt();
	return(msg);
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

    /* put in password file if necessary */
    if (!local_passwd && add_to_passwd(pwd)) {
	cleanup();
	return("An unexpected error occured while entering you in the local password file.");
    }

/*    if (msg = attachhomedir(pwd)) {
	cleanup();
	return(msg);
    } */

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
    default:
	sprintf(errbuf, "Unable to authenticate you, kerberos failure %d: %s",
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
    dest_tkt();
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
