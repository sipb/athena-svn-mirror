/*
 * xdm - display manager daemon
 *
 * $XConsortium: verify.c,v 1.24 91/07/18 22:22:45 rws Exp $
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xdm/verify.c,v 1.3 1996-06-27 19:58:55 miki Exp $
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

#include        <unistd.h>
#include        <limits.h>
# include	"dm.h"
# include	<pwd.h>
# include	<utmp.h>
#ifdef SOLARIS
#include <utmpx.h>
#endif
#include        <sys/wait.h>

# ifdef NGROUPS_MAX
# include	<grp.h>
# endif
#ifdef USESHADOW
# include	<shadow.h>
#endif
#ifdef X_NOT_STDC_ENV
char *getenv();
#endif
#include <signal.h>


struct passwd joeblow = {
	"Nobody", "***************"
};

#ifdef USESHADOW
struct spwd spjoeblow = {
	"Nobody", "**************"
};
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
#ifdef POSIX
static struct sigaction sigact, osigact;
#endif
int attach_pid, attach_state, attachhelp_pid, attachhelp_state, quota_pid;
static int	console = 0;
static char	consoletty[10];
extern char	*defaultpath;

Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
#ifdef USESHADOW
	struct spwd	*sp;
#endif
	char		*crypt ();
	char		**userEnv (), **systemEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;
	char		*msg, *dologin(), c;
	SIGVAL		(*oldsig)(), CatchChild();
	int		i;

	Debug ("Verify %s ...\n", greet->name);

	/* Get a console running displaying stdout & stderr
	 * through a pty.
	 */
	if (console == 0) {
#ifndef _AIX
	    /* choose a pty */
	    strcpy(consoletty, "/dev/ptyp0");
	    for (c = 'p'; c <= 's'; c++) {
		consoletty[8] = c;
		for (i = 0; i < 16; i++) {
		    consoletty[9] = "0123456789abcdef"[i];
		    console = open(consoletty, O_RDONLY, 0);
		    if (console >= 0) break;
		}
		if (console >= 0) break;
	    }
	    consoletty[5] = 't';
#else /* _AIX */
	    console = open("/dev/ptc", O_RDONLY, 0);
	    strcpy(consoletty, ttyname(console));
#endif /* _AIX */
	    dup2(console, 0);
	    if (fork() == 0) {
		execlp("/etc/athena/console", "console", "-f",
		       "/etc/athena/login/Console", "-display",
		       d->name, NULL);
		exit(0);
	    }
	    console = open(consoletty, O_RDWR, 0);
	    Debug ("got console %d\n", console);
	    dup2(console, 1);
	    dup2(console, 2);
	}
	Debug ("Console started\n");
#ifdef POSIX
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = CatchChild;
	sigaction(SIGCHLD, &sigact, &osigact);
#else
	oldsig = signal(SIGCHLD, CatchChild);
#endif


	if (!greet->string || (i = atoi(greet->string)) == 0)
	  i = 1;

	setenv("PATH", defaultpath, 1);
	msg = dologin(greet->name, greet->password, i,
		      "/etc/athena/login/Xsession", &consoletty[5],
		      "/etc/athena/login/Xsession", d->name, verify);
#ifdef POSIX
 	sigaction(SIGCHLD, &osigact, NULL);
#else
	signal(SIGCHLD, oldsig);
#endif
	if (msg) {
	    printf("%s\n", msg);
	    Debug ("dologin returned %s\n", msg);
	    return(0);
	} else {
	    static char home[256];
	    sprintf(home, "/mit/%s", greet->name);
	    Debug ("dologin was successful\n");
	    verify->systemEnviron = systemEnv (d, greet->name, home);
	    Debug ("user environment:\n");
	    printEnv (verify->userEnviron);
	    Debug ("system environment:\n");
	    printEnv (verify->systemEnviron);
	    Debug ("end of environments\n");
	    return(1);
	}
	/* NOTREACHED */
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0)
		p = &joeblow;
#ifdef USESHADOW
	sp = getspnam(greet->name);
	if (sp == NULL) {
		sp = &spjoeblow;
		Debug ("getspnam() failed.  Are you root?\n");
	}
	endspent();

	if (strcmp (crypt (greet->password, sp->sp_pwdp), sp->sp_pwdp))
#else
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd))
#endif
	{
		Debug ("verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
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


UnVerify(d, verify)
struct display		*d;
struct verify_info	*verify;
{
    int found, file;
    char login[9];
#ifdef SOLARIS
    struct utmpx utmpx;    
    struct utmpx *utx_tmp;
    char * ttyn;
    char new_id[20];
    register int f;
    char *p;
#endif
    struct utmp utmp;

    Debug ("Cleaning up on logout\n");
#ifndef SOLARIS
    found = 0;
    if ((file = open("/etc/utmp", O_RDWR, 0)) > 0) {
	while (read(file, (char *)&utmp, sizeof(utmp)) > 0) {
	    if (!strncmp(utmp.ut_line, &consoletty[5], sizeof(utmp.ut_line))
#ifdef _AIX
		&& (utmp.ut_type == USER_PROCESS)
#endif
		) {
		strncpy(login, utmp.ut_name, 8);
		login[8] = 0;
		if (utmp.ut_name[0] != '\0') {
		    strncpy(utmp.ut_name, "", sizeof(utmp.ut_name));
#ifdef _AIX
		    utmp.ut_type = EMPTY;
#endif
		    lseek(file, (long) -sizeof(utmp), L_INCR);
		    write(file, (char *) &utmp, sizeof(utmp));
		    found = 1;
		}
		break;
	    }
	}
	close(file);
    }
    if (found) {
	if ((file = open("/usr/adm/wtmp", O_WRONLY|O_APPEND, 0644)) >= 0) {
	    strncpy(utmp.ut_line, &consoletty[5], sizeof(utmp.ut_line));
	    strncpy(utmp.ut_name, "", sizeof(utmp.ut_name));
	    strncpy(utmp.ut_host, "", sizeof(utmp.ut_host));
	    time(&utmp.ut_time);
	    write(file, (char *) &utmp, sizeof(utmp));
	    close(file);
	}
    }
#ifdef DEBUG
    if (found)
      syslog(3, "Unverify called and entry removed from utmp, tty %s",
	     &consoletty[5]);
    else
      syslog(3, "Unverify called and entry NOT removed from utmp, tty %s",
	     &consoletty[5]);
#endif
#else /* SOLARIS */
    gettimeofday(&utmpx.ut_tv);
    utmpx.ut_type = 8   ;
    strncpy(utmpx.ut_line, &consoletty[5], sizeof(utmpx.ut_line));    
    setutxent();
    utx_tmp = getutxline(&utmpx);
    if ( utx_tmp != NULL ) {
      strcpy(utmpx.ut_line, utx_tmp->ut_line);
      strcpy(utmpx.ut_user,utx_tmp->ut_name);
      utmpx.ut_pid = getpid();
      if (utx_tmp)
              strcpy(new_id, utx_tmp->ut_id);
      p = index(new_id, '/');
      if (p)
              strcpy(p, "\0");
      strcpy(utmpx.ut_id , new_id);
      pututxline(&utmpx);
      getutmp(&utmpx, &utmp);
      setutent();
      pututline(&utmp);
      if ((f = open("/usr/adm/wtmp",O_WRONLY|O_APPEND)) >= 0) {
              write(f, (char *)&utmp, sizeof(utmp));
              close(f);
      }
       if ((f = open("/usr/adm/wtmpx",O_WRONLY|O_APPEND)) >= 0) {
               write(f, (char *)&utmpx, sizeof(utmpx));
              close(f);
      }
    }      
#endif
    cleanup(NULL);

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
    char 	*ow;    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    if (getenv("OPENWINHOME") == NULL)
	env = setEnv(env, "OPENWINHOME", "/usr/openwin");
    env = setEnv (env, "OPENWINHOME", ow);
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

SIGVAL CatchChild()
{
    int pid;
    waitType status; 
    char *number();

#ifdef POSIX
    pid = waitpid((pid_t)-1 , &status, WNOHANG);
#else
    pid = wait3(&status, WNOHANG, 0);
#endif
    if (pid == attach_pid) {
		attach_state = waitCode(status); 
    } else if (pid == attachhelp_pid) {
		attachhelp_state = waitCode(status); 
    } else if (pid == quota_pid) {
	/* don't need to do anything here */
    } else
      fprintf(stderr, "XLogin: child %d exited with status %d\n",
	      pid, waitCode(status));
}

char *lose(msg)
char *msg;
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

prompt_user(msg, noproc)
char *msg;
void (*noproc)();
{
    printf("%s (assuming YES)\n", msg);
    /* (*noproc)(); */
    return;
}
