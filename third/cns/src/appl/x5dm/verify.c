/*
 * xdm - display manager daemon
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	<pwd.h>
# ifdef NGROUPS_MAX
# include	<grp.h>
# endif
#ifdef __SCO__
#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include "/usr/include/prot.h"
#endif

#ifndef KERBEROS
#define KERBEROS
#endif

#ifdef KERBEROS
char *get_tickets();
#define MAXHOSTNAMELEN 255
#define LOGIN_TKT_DEFAULT_LIFETIME DEFAULT_TKT_LIFE /* from krb.h */
#endif
#ifdef USESHADOW
# include	<shadow.h>
#endif
#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

static char *envvars[] = {
#if defined(sony) && !defined(SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
    "TZ",
#endif
    NULL
};

Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
#ifdef __SCO__
	struct pr_passwd *sp;
#endif	
#ifdef USESHADOW
	struct spwd	*sp;
#endif
	char		*crypt ();
	char		**userEnv (), **systemEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0) {
		Debug ("no passwd file entry\n");
		bzero (greet->password, strlen (greet->password));
		return 0;
	}
#if 0
/* SCO code */
#define PW_DID_AUTH
	sp = getprpwnam(greet->name);
	if (sp == NULL) {
		Debug ("getprpwnam() failed.  Are you root?\n");
		return 0;
	}
	endprpwent();
	if (strcmp (crypt (greet->password, sp->ufld.fd_encrypt),
		    sp->ufld.fd_encrypt))
	{
		Debug ("verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#endif	
#ifdef USESHADOW
#define PW_DID_AUTH
	sp = getspnam(greet->name);
	if (sp == NULL) {
		Debug ("getspnam() failed.  Are you root?\n");
		return 0;
	}
	endspent();

	if (strcmp (crypt (greet->password, sp->sp_pwdp), sp->sp_pwdp))
	{
		Debug ("verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#endif
#ifdef KERBEROS
#define PW_DID_AUTH
	{
#define PASSWORD_LEN 14
	  char saltc[2], c;
	  char encrypt[PASSWORD_LEN+1];
	  char *msg;
	  long salt;
	  int i;

	  /* set real uid/gid for kerberos library */
#ifdef _IBMR2
	  setruid_rios(p->pw_uid);
	  setrgid_rios(p->pw_gid);
#else
#ifdef __SCO__
	  setreuid(p->pw_uid, -1);
	  setregid(p->pw_gid, -1);
#else
#ifdef SVR4
	  setuid(p->pw_uid);
	  setgid(p->pw_gid);
#else
	  setruid(p->pw_uid);
	  setrgid(p->pw_gid);
#endif
#endif
#endif

	  if ((msg = get_tickets(greet->name, greet->password)) != NULL 
	      && p->pw_uid) 
	    {
	      Debug(msg);
	      bzero(greet->password, strlen(greet->password));
	      return 0;
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
	  strcpy(encrypt,crypt(greet->password, saltc));	

	  /* don't need the password anymore */
	  bzero(greet->password, strlen(greet->password));
	  
	  p->pw_passwd = encrypt;

	  /* put in password file if necessary */
	  if (add_to_passwd(p)) {
	    Debug("An unexpected error occured while entering you in the local password file.");
	    return 0;
	  }
	}
#endif
#ifndef PW_DID_AUTH
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd))
	{
		Debug ("raw verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#endif
	Debug ("verify succeeded\n");
/*	bzero(greet->password, strlen(greet->password)); */
	verify->uid = p->pw_uid;
#ifdef NGROUPS_MAX
	getGroups (greet->name, verify, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_uid == 0,
				       greet->name, home, shell);
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	return 1;
}

extern char **setEnv ();

char **
defaultEnv ()
{
    char    **env, **exp, *value;

    env = 0;
    for (exp = exportList; exp && *exp; ++exp)
    {
	value = getenv (*exp);
	if (value)
	    env = setEnv (env, *exp, value);
    }
    return env;
}

char **
userEnv (d, useSystemPath, user, home, shell)
struct display	*d;
int	useSystemPath;
char	*user, *home, *shell;
{
    char	**env;
    char	**envvar;
    char	*str;
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "USER", user);
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
    for (envvar = envvars; *envvar; envvar++)
    {
	if (str = getenv(*envvar))
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

char **
systemEnv (d, user, home)
struct display	*d;
char	*user, *home;
{
    char	**env;
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    if (home)
	env = setEnv (env, "HOME", home);
    if (user)
	env = setEnv (env, "USER", user);
    env = setEnv (env, "PATH", d->systemPath);
    env = setEnv (env, "SHELL", d->systemShell);
    if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
    return env;
}

#ifdef NGROUPS_MAX
groupMember (name, members)
char	*name;
char	**members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

getGroups (name, verify, gid)
char			*name;
struct verify_info	*verify;
int			gid;
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	/* SUPPRESS 560 */
	while (g = getgrent()) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS_MAX)
				LogError ("%s belongs to more than %d groups, %s ignored\n",
					name, NGROUPS_MAX, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif

#ifdef KERBEROS
#include <krb.h>
#include  <netdb.h>

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
    /* bcopy ((char *)hp->h_addr, (char *) &addr, sizeof (addr)); */
    memcpy((char *) &addr, (char *)hp->h_addr, sizeof (addr));

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

add_to_passwd(p)
struct passwd *p;
{
    int i, fd = -1;
    FILE *etc_passwd;

#ifdef __SCO__
    struct pr_passwd *sp;

    sp = getprpwnam(p->pw_name);
    if (sp == NULL) {
      Debug ("getprpwnam() failed.  Are you root?\n");
      return 1;
    }
    endprpwent();
    strcpy(sp->ufld.fd_encrypt, p->pw_passwd);
    
    if(!putprpwnam(p->pw_name, sp)) {
      Debug ("putprpwnam() failed.\n");
      return 1;
    }
#endif
    
    return(0);
}

#endif
