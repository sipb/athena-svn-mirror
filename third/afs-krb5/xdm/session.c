/* $XConsortium: session.c /main/77 1996/11/24 17:32:33 rws $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * session.c
 */

#include "dm.h"
#include "greet.h"
#include <X11/Xlib.h>
#include <signal.h>
#include <X11/Xatom.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#ifdef AIXV3
# include <usersec.h>
#endif
#ifdef SECURE_RPC
# include <rpc/rpc.h>
# include <rpc/key_prot.h>
#endif
#ifdef K5AUTH
# include <krb5/krb5.h>
#endif

#if defined(IRIX_CAPABILITY) && defined(sgi)
#include <sys/capability.h>
#endif

#ifdef KRB5LOGIN
#include <krb5.h>
#include <com_err.h>
#include <setjmp.h>

/*
 * Setjmp/signal support for doing PAGs
 */

#ifdef SIGNALRETURNSINT
#define krb5_sigtype int
#else
#define krb5_sigtype void
#endif

typedef krb5_sigtype sigtype;

#ifndef POSIX_SETJMP
#undef sigjmp_buf
#undef sigsetjmp
#undef siglongjmp
#define sigjmp_buf	jmp_buf
#define sigsetjmp(j,s)	setjmp(j)
#define siglongjmp	longjmp
#endif

#if !defined(SIGSYS) && defined(__linux__)
/* Linux doesn't seem to have SIGSYS */
#define SIGSYS  SIGUNUSED
#endif

#ifdef POSIX_SIGNALS
typedef struct sigaction handler;
#define handler_init(H,F)		(sigemptyset(&(H).sa_mask), \
					 (H).sa_flags=0, \
					 (H).sa_handler=(F))
#define handler_swap(S,NEW,OLD)		sigaction(S, &NEW, &OLD)
#define handler_set(S,OLD)		sigaction(S, &OLD, NULL)
#else
typedef sigtype (*handler)();
#define handler_init(H,F)		((H) = (F))
#define handler_swap(S,NEW,OLD)		((OLD) = signal ((S), (NEW)))
#define handler_set(S,OLD)		(signal ((S), (OLD)))
#endif

#ifdef AFSPAG
extern setpag(), ktc_ForgetAllTokens();

static int pagflag = 0;

static sigjmp_buf setpag_buf;

static sigtype sigsys()
{
	siglongjmp(setpag_buf, 1);
}

static int try_afscall(scall)
	int (*scall)();
{
	handler sa, osa;
	volatile int retval = 0;

	(void) &retval;
	handler_init(sa, sigsys);
	handler_swap(SIGSYS, sa, osa);
	if (sigsetjmp(setpag_buf, 1) == 0) {
	    (*scall)();
		retval = 1;
	}
	handler_set(SIGSYS, osa);
	return retval;
}

#define try_setpag()	try_afscall(setpag)
#define try_unlog()	try_afscall(ktc_ForgetAllTokens)
#endif /* AFSPAG */
#endif /* KRB5LOGIN */

#ifndef GREET_USER_STATIC
#include <dlfcn.h>
#ifndef RTLD_NOW
#define RTLD_NOW 1
#endif
#endif

#ifdef CSRG_BASED
#include <sys/param.h>
#endif

extern	int	PingServer();
extern	int	SessionPingFailed();
extern	int	Debug();
extern	int	RegisterCloseOnFork();
extern	int	SecureDisplay();
extern	int	UnsecureDisplay();
extern	int	ClearCloseOnFork();
extern	int	SetupDisplay();
extern	int	LogError();
extern	int	SessionExit();
extern	int	DeleteXloginResources();
extern	int	source();
extern	char	**defaultEnv();
extern	char	**setEnv();
extern	char	**parseArgs();
extern	int	printEnv();
extern	char	**systemEnv();
extern	int	LogOutOfMem();
extern	void	setgrent();
extern	struct group	*getgrent();
extern	void	endgrent();
#ifdef USESHADOW
extern	struct spwd	*getspnam();
extern	void	endspent();
#endif
extern	struct passwd	*getpwnam();
extern	char	*crypt();

static	struct dlfuncs	dlfuncs = {
	PingServer,
	SessionPingFailed,
	Debug,
	RegisterCloseOnFork,
	SecureDisplay,
	UnsecureDisplay,
	ClearCloseOnFork,
	SetupDisplay,
	LogError,
	SessionExit,
	DeleteXloginResources,
	source,
	defaultEnv,
	setEnv,
	parseArgs,
	printEnv,
	systemEnv,
	LogOutOfMem,
	setgrent,
	getgrent,
	endgrent,
#ifdef USESHADOW
	getspnam,
	endspent,
#endif
	getpwnam,
	crypt,
	};

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

static Bool StartClient();

static int			clientPid;
static struct greet_info	greet;
static struct verify_info	verify;

static Jmp_buf	abortSession;

/* ARGSUSED */
static SIGVAL
catchTerm (n)
    int n;
{
    Longjmp (abortSession, 1);
}

static Jmp_buf	pingTime;

/* ARGSUSED */
static SIGVAL
catchAlrm (n)
    int n;
{
    Longjmp (pingTime, 1);
}

static Jmp_buf	tenaciousClient;

/* ARGSUSED */
static SIGVAL
waitAbort (n)
    int n;
{
	Longjmp (tenaciousClient, 1);
}

#if defined(_POSIX_SOURCE) || defined(SYSV) || defined(SVR4)
#define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif

static void
AbortClient (pid)
    int pid;
{
    int	sig = SIGTERM;
#ifdef __STDC__
    volatile int	i;
#else
    int	i;
#endif
    int	retId;
    for (i = 0; i < 4; i++) {
	if (killpg (pid, sig) == -1) {
	    switch (errno) {
	    case EPERM:
		LogError ("xdm can't kill client\n");
	    case EINVAL:
	    case ESRCH:
		return;
	    }
	}
	if (!Setjmp (tenaciousClient)) {
	    (void) Signal (SIGALRM, waitAbort);
	    (void) alarm ((unsigned) 10);
	    retId = wait ((waitType *) 0);
	    (void) alarm ((unsigned) 0);
	    (void) Signal (SIGALRM, SIG_DFL);
	    if (retId == pid)
		break;
	} else
	    (void) Signal (SIGALRM, SIG_DFL);
	sig = SIGKILL;
    }
}

SessionPingFailed (d)
    struct display  *d;
{
    if (clientPid > 1)
    {
    	AbortClient (clientPid);
	source (verify.systemEnviron, d->reset);
    }
    SessionExit (d, RESERVER_DISPLAY, TRUE);
}

/*
 * We need our own error handlers because we can't be sure what exit code Xlib
 * will use, and our Xlib does exit(1) which matches REMANAGE_DISPLAY, which
 * can cause a race condition leaving the display wedged.  We need to use
 * RESERVER_DISPLAY for IO errors, to ensure that the manager waits for the
 * server to terminate.  For other X errors, we should give up.
 */

/*ARGSUSED*/
static
IOErrorHandler (dpy)
    Display *dpy;
{
    LogError("fatal IO error %d (%s)\n", errno, _SysErrorMsg(errno));
    exit(RESERVER_DISPLAY);
}

static int
ErrorHandler(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
    LogError("X error\n");
    if (XmuPrintDefaultErrorMessage (dpy, event, stderr) == 0) return 0;
    exit(UNMANAGE_DISPLAY);
    /*NOTREACHED*/
}

ManageSession (d)
struct display	*d;
{
    int			pid, code;
    Display		*dpy;
    greet_user_rtn	greet_stat; 
    static GreetUserProc greet_user_proc = NULL;
    void		*greet_lib_handle;
#ifdef sgi
    Window		root_win;
    Atom		prop;
    XEvent		event;
#endif

    Debug ("ManageSession %s\n", d->name);
    (void)XSetIOErrorHandler(IOErrorHandler);
    (void)XSetErrorHandler(ErrorHandler);
    SetTitle(d->name, (char *) 0);
    /*
     * Load system default Resources
     */
    LoadXloginResources (d);

#ifdef GREET_USER_STATIC
    greet_user_proc = GreetUser;
#else
    Debug("ManageSession: loading greeter library %s\n", greeterLib);
    greet_lib_handle = dlopen(greeterLib, RTLD_NOW);
    if (greet_lib_handle != NULL)
	greet_user_proc = (GreetUserProc)dlsym(greet_lib_handle, "GreetUser");
    if (greet_user_proc == NULL)
	{
	LogError("%s while loading %s\n", dlerror(), greeterLib);
	exit(UNMANAGE_DISPLAY);
	}
#endif

    /* tell the possibly dynamically loaded greeter function
     * what data structure formats to expect.
     * These version numbers are registered with the X Consortium. */
    verify.version = 1;
    greet.version = 1;
    greet_stat = (*greet_user_proc)(d, &dpy, &verify, &greet, &dlfuncs);

    if (greet_stat == Greet_Success)
    {
	clientPid = 0;
	if (!Setjmp (abortSession)) {
	    (void) Signal (SIGTERM, catchTerm);
	    /*
	     * Start the clients, changing uid/groups
	     *	   setting up environment and running the session
	     */
	    if (StartClient (&verify, d, &clientPid, greet.name, greet.password)) {
		Debug ("Client Started\n");
		/*
		 * Wait for session to end,
		 */
		for (;;) {
		    if (d->pingInterval)
		    {
			if (!Setjmp (pingTime))
			{
			    (void) Signal (SIGALRM, catchAlrm);
			    (void) alarm (d->pingInterval * 60);
			    pid = wait ((waitType *) 0);
			    (void) alarm (0);
			}
			else
			{
			    (void) alarm (0);
			    if (!PingServer (d, (Display *) NULL))
				SessionPingFailed (d);
			}
		    }
		    else
		    {
			pid = wait ((waitType *) 0);
		    }
		    if (pid == clientPid)
			break;

		}
#ifdef sgi
	        Debug("Checking for _SGI_SESSION_PROPERTY ...\n");
		dpy = XOpenDisplay(d->name);
		root_win = RootWindow(dpy, DefaultScreen(dpy));

		if (root_win != -1)
		{
		    Debug("Was able to get the root window\n");
		    prop = XInternAtom(dpy, "_SGI_SESSION_PROPERTY", True);
		    if (prop != None)
		    {
			Debug("Found _SGI_SESSION_PROPERTY, waiting ...\n");
			XSelectInput(dpy, root_win, PropertyChangeMask);
			for (;;) {
			    XNextEvent(dpy, &event);
			    if (prop == event.xproperty.atom) {
				Debug("Got an event for it\n");
				break;
			    }
			}
		    } else
			Debug("Didn't find property, ending session ...\n");
		}
		else
		    LogError ("unable to access root window of display\n");
#endif
	    } else {
		LogError ("session start failed\n");
	    }
	} else {
	    /*
	     * when terminating the session, nuke
	     * the child and then run the reset script
	     */
	    AbortClient (clientPid);
	}
    }
    /*
     * run system-wide reset file
     */
    Debug ("Source reset program %s\n", d->reset);
    source (verify.systemEnviron, d->reset);
    SessionExit (d, OBEYSESS_DISPLAY, TRUE);
}

LoadXloginResources (d)
struct display	*d;
{
    char	**args, **parseArgs();
    char	**env = 0, **setEnv(), **systemEnv();

    if (d->resources[0] && access (d->resources, 4) == 0) {
	env = systemEnv (d, (char *) 0, (char *) 0);
	args = parseArgs ((char **) 0, d->xrdb);
	args = parseArgs (args, d->resources);
	Debug ("Loading resource file: %s\n", d->resources);
	(void) runAndWait (args, env);
	freeArgs (args);
	freeEnv (env);
    }
}

SetupDisplay (d)
struct display	*d;
{
    char	**env = 0, **setEnv(), **systemEnv();

    if (d->setup && d->setup[0])
    {
    	env = systemEnv (d, (char *) 0, (char *) 0);
    	(void) source (env, d->setup);
    	freeEnv (env);
    }
}

/*ARGSUSED*/
DeleteXloginResources (d, dpy)
struct display	*d;
Display		*dpy;
{
    int i;
    Atom prop = XInternAtom(dpy, "SCREEN_RESOURCES", True);

    XDeleteProperty(dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER);
    if (prop) {
	for (i = ScreenCount(dpy); --i >= 0; )
	    XDeleteProperty(dpy, RootWindow (dpy, i), prop);
    }
}

static Jmp_buf syncJump;

/* ARGSUSED */
static SIGVAL
syncTimeout (n)
    int n;
{
    Longjmp (syncJump, 1);
}

SecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
    Debug ("SecureDisplay %s\n", d->name);
    (void) Signal (SIGALRM, syncTimeout);
    if (Setjmp (syncJump)) {
	LogError ("WARNING: display %s could not be secured\n",
		   d->name);
	SessionExit (d, RESERVER_DISPLAY, FALSE);
    }
    (void) alarm ((unsigned) d->grabTimeout);
    Debug ("Before XGrabServer %s\n", d->name);
    XGrabServer (dpy);
    if (XGrabKeyboard (dpy, DefaultRootWindow (dpy), True, GrabModeAsync,
		       GrabModeAsync, CurrentTime) != GrabSuccess)
    {
	(void) alarm (0);
	(void) Signal (SIGALRM, SIG_DFL);
	LogError ("WARNING: keyboard on display %s could not be secured\n",
		  d->name);
	SessionExit (d, RESERVER_DISPLAY, FALSE);
    }
    Debug ("XGrabKeyboard succeeded %s\n", d->name);
    (void) alarm (0);
    (void) Signal (SIGALRM, SIG_DFL);
    pseudoReset (dpy);
    if (!d->grabServer)
    {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
    Debug ("done secure %s\n", d->name);
}

UnsecureDisplay (d, dpy)
struct display	*d;
Display		*dpy;
{
    Debug ("Unsecure display %s\n", d->name);
    if (d->grabServer)
    {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
}

SessionExit (d, status, removeAuth)
    struct display  *d;
{
#ifdef KRB5LOGIN
    char *ccfile;
    char *getEnv();
#endif /* KRB5LOGIN */
    /* make sure the server gets reset after the session is over */
    if (d->serverPid >= 2 && d->resetSignal)
	kill (d->serverPid, d->resetSignal);
    else
	ResetServer (d);
    if (removeAuth)
    {
	setgid (verify.gid);
	setuid (verify.uid);
	RemoveUserAuthorization (d, &verify);
#ifdef K5AUTH
	/* do like "kdestroy" program */
        {
	    krb5_error_code code;
	    krb5_ccache ccache;

	    code = Krb5DisplayCCache(d->name, &ccache);
	    if (code)
		LogError("%s while getting Krb5 ccache to destroy\n",
			 error_message(code));
	    else {
		code = krb5_cc_destroy(ccache);
		if (code) {
		    if (code == KRB5_FCC_NOFILE) {
			Debug ("No Kerberos ccache file found to destroy\n");
		    } else
			LogError("%s while destroying Krb5 credentials cache\n",
				 error_message(code));
		} else
		    Debug ("Kerberos ccache destroyed\n");
		krb5_cc_close(ccache);
	    }
	}
#endif /* K5AUTH */
#ifdef KRB5LOGIN
	if ((ccfile = getEnv(verify.userEnviron, "KRB5CCNAME"))) {
		krb5_context context;
		krb5_error_code code;
		krb5_principal me;
		krb5_ccache cc;
		int retain_ccache = 0;
#ifdef AFSPAG
		int afs_retain_token = 0;
#endif /* AFSPAG */

		if ((code = krb5_init_context(&context))) {
			Debug("krb5_init_context failed: %s\n",
			      error_message(code));
			goto nodestroy;
		}

		if ((code = krb5_cc_resolve(context, ccfile, &cc))) {
			Debug("krb5_cc_resolve failed: %s\n",
			      error_message(code));
			krb5_free_context(context);
			goto nodestroy;
		}

		if ((code = krb5_cc_get_principal(context, cc, &me))) {
			Debug("krb5_cc_get_pricnipal failed: %s\n",
			      error_message(code));
			krb5_cc_close(context, cc);
			krb5_free_context(context);
			goto nodestroy;
		}

		krb5_appdefault_boolean(context, "xdm",
					krb5_princ_realm(context, me),
					"retain_ccache", retain_ccache,
					&retain_ccache);

#ifdef AFSPAG
		krb5_appdefault_boolean(context, "xdm",
					krb5_princ_realm(context, me),
					"afs_retain_token", afs_retain_token,
					&afs_retain_token);
#endif /* AFSPAG */

		krb5_free_principal(context, me);

		if (! retain_ccache) {
			Debug("Destroying credential cache\n");
			krb5_cc_destroy(context, cc);
		} else
			krb5_cc_close(context, cc);
		
		krb5_free_context(context);

#ifdef AFSPAG
		if (! afs_retain_token)
			if (pagflag) {
				Debug("Destroying afs token\n");
				try_unlog();
			}
#endif /* AFSPAG */

	}
#endif /* KRB5LOGIN */
    }
#ifdef KRB5LOGIN
nodestroy:
#endif /* KRB5LOGIN */
    Debug ("Display %s exiting with status %d\n", d->name, status);
    exit (status);
}

static Bool
StartClient (verify, d, pidp, name, passwd)
    struct verify_info	*verify;
    struct display	*d;
    int			*pidp;
    char		*name;
    char		*passwd;
{
    char	**f, *home, *getEnv ();
    char	*failsafeArgv[2];
    int	pid;
#ifdef KRB5LOGIN
    int rewrite_ccache = 0;
    krb5_creds saved_creds, mcreds;
    krb5_error_code code;
    krb5_context context;
    krb5_ccache cc;
    char *ccfile;
#endif /* KRB5LOGIN */

    if (verify->argv) {
	Debug ("StartSession %s: ", verify->argv[0]);
	for (f = verify->argv; *f; f++)
		Debug ("%s ", *f);
	Debug ("; ");
    }
    if (verify->userEnviron) {
	for (f = verify->userEnviron; *f; f++)
		Debug ("%s ", *f);
	Debug ("\n");
    }
#ifdef AFSPAG
	/*
	 * Allocate a PAG for us, before we fork.
	 */

	pagflag = try_setpag();

#endif /* AFSPAG */

    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();

	/* Do system-dependent login setup here */

#ifdef KRB5LOGIN
	if ((ccfile = getEnv(verify->userEnviron, "KRB5CCNAME"))) {
		memset(&saved_creds, 0, sizeof (saved_creds));
		memset(&mcreds, 0, sizeof (mcreds));

		/*
		 * Setup stuff to read in our credential cache and save
		 * the creds.
		 */

		if ((code = krb5_init_context(&context))) {
			Debug("krb5_init_context failed: %s\n",
			      error_message(code));
			goto noccache;
		}

		/*
		 * Get our name
		 */

		if ((code = krb5_parse_name(context, name, &mcreds.client))) {
			Debug("krb5_parse_name failed: %s\n",
			      error_message(code));
			krb5_free_context(context);
			goto noccache;
		}

		/*
		 * Build the TGT principal
		 */

		code = krb5_build_principal_ext(context, &mcreds.server,
			krb5_princ_realm(context, mcreds.client)->length,
			krb5_princ_realm(context, mcreds.client)->data,
			KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
			krb5_princ_realm(context, mcreds.client)->length,
			krb5_princ_realm(context, mcreds.client)->data,
			0);

		if (code) {
			Debug("krb5_build_principal_ext failed: %s\n",
			      error_message(code));
			krb5_free_cred_contents(context, &mcreds);
			krb5_free_context(context);
			goto noccache;
		}

		/*
		 * Open up our credential cache
		 */

		if ((code = krb5_cc_resolve(context, ccfile, &cc))) {
			Debug("krb5_cc_resolve failed: %s\n",
			      error_message(code));
			krb5_free_cred_contents(context, &mcreds);
			krb5_free_context(context);
			goto noccache;
		}

		/*
		 * Get our TGT out of the cache
		 */

		if ((code = krb5_cc_retrieve_cred(context, cc, 0, &mcreds,
						  &saved_creds))) {
			Debug("krb5_cc_retrieve_cred failed: %s\n",
			      error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_cred_contents(context, &mcreds);
			krb5_free_context(context);
			goto noccache;
		}

		/*
		 * Everything is cool!  Destroy the old cache.
		 */

		krb5_free_cred_contents(context, &mcreds);

		krb5_cc_destroy(context, cc);

		Debug("Got our TGT out of the ccache!\n");

		rewrite_ccache = 1;
	}

noccache:
#endif /* KRB5LOGIN */
#if defined(IRIX_CAPABILITY) && defined(sgi)
	/*
	 * Initialize process capabilities
	 *
	 * Note that this is currently a hack. We really should read
	 * /etc/capability and set based on it's contents. For now
	 * though we just want to clear the capabilites if we're not
	 * root so users don't run into problems.
	 */
	
	if (verify->uid != 0) {
	    cap_set_t cap;

	    cap.cap_effective = CAP_ALL_OFF;
	    cap.cap_permitted = CAP_ALL_OFF;
	    cap.cap_inheritable = CAP_ALL_OFF;

	    if (cap_set_proc(&cap) == -1) {

		switch(errno) {
		case ENOSYS:
		    /* Function not implemented. Fail silently. */
		    break;

		default:
	    	    LogError("cap_set_proc() for \"%s\" failed, errno=%d\n",
			     name, errno);
		}
	    }
	}
#endif /* IRIX_CAPABILITY && sgi */

#ifdef AIXV3
	/*
	 * Set the user's credentials: uid, gid, groups,
	 * audit classes, user limits, and umask.
	 */
	if (setpcred(name, NULL) == -1)
	{
	    LogError("setpcred for \"%s\" failed, errno=%d\n", name, errno);
	    return (0);
	}
#else /* AIXV3 */
	if (setgid(verify->gid) < 0)
	{
	    LogError("setgid %d (user \"%s\") failed, errno=%d\n",
		     verify->gid, name, errno);
	    return (0);
	}
#if (BSD >= 199103)
	if (setlogin(name) < 0)
	{
	    LogError("setlogin for \"%s\" failed, errno=%d", name, errno);
	    return(0);
	}
#endif
	if (initgroups(name, verify->gid) < 0)
	{
	    LogError("initgroups for \"%s\" failed, errno=%d\n", name, errno);
	    return (0);
	}
	if (setuid(verify->uid) < 0)
	{
	    LogError("setuid %d (user \"%s\") failed, errno=%d\n",
		     verify->uid, name, errno);
	    return (0);
	}
#endif /* AIXV3 */
	/*
	 * for user-based authorization schemes,
	 * use the password to get the user's credentials.
	 */
#ifdef SECURE_RPC
	/* do like "keylogin" program */
	{
	    char    netname[MAXNETNAMELEN+1], secretkey[HEXKEYBYTES+1];
	    int	    nameret, keyret;
	    int	    len;
	    int     key_set_ok = 0;

	    nameret = getnetname (netname);
	    Debug ("User netname: %s\n", netname);
	    len = strlen (passwd);
	    if (len > 8)
		bzero (passwd + 8, len - 8);
	    keyret = getsecretkey(netname,secretkey,passwd);
	    Debug ("getsecretkey returns %d, key length %d\n",
		    keyret, strlen (secretkey));
	    /* is there a key, and do we have the right password? */
	    if (keyret == 1)
	    {
		if (*secretkey)
		{
		    keyret = key_setsecret(secretkey);
		    Debug ("key_setsecret returns %d\n", keyret);
		    if (keyret == -1)
			LogError ("failed to set NIS secret key\n");
		    else
			key_set_ok = 1;
		}
		else
		{
		    /* found a key, but couldn't interpret it */
		    LogError ("password incorrect for NIS principal \"%s\"\n",
			      nameret ? netname : name);
		}
	    }
	    if (!key_set_ok)
	    {
		/* remove SUN-DES-1 from authorizations list */
		int i, j;
		for (i = 0; i < d->authNum; i++)
		{
		    if (d->authorizations[i]->name_length == 9 &&
			memcmp(d->authorizations[i]->name, "SUN-DES-1", 9) == 0)
		    {
			for (j = i+1; j < d->authNum; j++)
			    d->authorizations[j-1] = d->authorizations[j];
			d->authNum--;
			break;
		    }
		}
	    }
	    bzero(secretkey, strlen(secretkey));
	}
#endif
#ifdef K5AUTH
	/* do like "kinit" program */
	{
	    int i, j;
	    int result;
	    extern char *Krb5CCacheName();

	    result = Krb5Init(name, passwd, d);
	    if (result == 0) {
		/* point session clients at the Kerberos credentials cache */
		verify->userEnviron =
		    setEnv(verify->userEnviron,
			   "KRB5CCNAME", Krb5CCacheName(d->name));
	    } else {
		for (i = 0; i < d->authNum; i++)
		{
		    if (d->authorizations[i]->name_length == 14 &&
			memcmp(d->authorizations[i]->name, "MIT-KERBEROS-5", 14) == 0)
		    {
			/* remove Kerberos from authorizations list */
			for (j = i+1; j < d->authNum; j++)
			    d->authorizations[j-1] = d->authorizations[j];
			d->authNum--;
			break;
		    }
		}
	    }
	}
#endif /* K5AUTH */
#ifdef KRB5LOGIN
#ifndef AKLOG_PATH
#define AKLOG_PATH "/usr/krb5/bin/aklog"
#endif /* AKLOG_PATH */
	/*
	 * Put a new credential cache in, if we found something from the
	 * old one.
	 */
	if (rewrite_ccache) {
		char *aklog_path;
		int run_aklog = 0;

		Debug("Putting our TGT in a new ccache\n");

		if ((code = krb5_cc_resolve(context, ccfile, &cc))) {
			Debug("krb5_cc_resolve failed: %s\n",
			      error_message(code));
			krb5_free_cred_contents(context, &saved_creds);
			krb5_free_context(context);
			goto failedkerb5;
		}

		if ((code = krb5_cc_initialize(context, cc,
					       saved_creds.client))) {
			Debug("krb5_cc_initialize failed: %s\n",
			      error_message(code));
			krb5_free_cred_contents(context, &saved_creds);
			krb5_cc_destroy(context, cc);
			krb5_free_context(context);
			goto failedkerb5;
		}

		if ((code = krb5_cc_store_cred(context, cc, &saved_creds))) {
			Debug("krb5_cc_store_cred failed: %s\n",
			      error_message(code));
			krb5_free_cred_contents(context, &saved_creds);
			krb5_cc_destroy(context, cc);
			krb5_free_context(context);
			goto failedkerb5;
		}

		krb5_cc_close(context, cc);

		Debug("TGT in ccache successfully\n");

		/*
		 * _If_ we're compiled in with PAG support, then run
		 * aklog.
		 */
#ifdef AFSPAG
		krb5_appdefault_boolean(context, "xdm",
				krb5_princ_realm(context, saved_creds.client),
				"krb5_run_aklog", run_aklog,
				&run_aklog);
		
		if (run_aklog) {
			char *args[2];
			krb5_appdefault_string(context, "xdm",
				krb5_princ_realm(context, saved_creds.client),
				"krb5_aklog_path", AKLOG_PATH,
				&aklog_path);
			
			Debug("We're running aklog (%s)\n", aklog_path);
			args[0] = aklog_path;
			args[1] = NULL;
			runAndWait(args, verify->userEnviron);

			free(aklog_path);
		}
#endif /* AFSPAG */

		krb5_free_cred_contents(context, &saved_creds);
		krb5_free_context(context);
	}
failedkerb5:
#endif /* KRB5LOGIN */
	bzero(passwd, strlen(passwd));
	SetUserAuthorization (d, verify);
	home = getEnv (verify->userEnviron, "HOME");
	if (home)
	    if (chdir (home) == -1) {
		LogError ("user \"%s\": cannot chdir to home \"%s\" (err %d), using \"/\"\n",
			  getEnv (verify->userEnviron, "USER"), home, errno);
		chdir ("/");
		verify->userEnviron = setEnv(verify->userEnviron, "HOME", "/");
	    }
	if (verify->argv) {
		Debug ("executing session %s\n", verify->argv[0]);
		execute (verify->argv, verify->userEnviron);
		LogError ("Session \"%s\" execution failed (err %d)\n", verify->argv[0], errno);
	} else {
		LogError ("Session has no command/arguments\n");
	}
	failsafeArgv[0] = d->failsafeClient;
	failsafeArgv[1] = 0;
	execute (failsafeArgv, verify->userEnviron);
	exit (1);
    case -1:
	bzero(passwd, strlen(passwd));
	Debug ("StartSession, fork failed\n");
	LogError ("can't start session on \"%s\", fork failed, errno=%d\n",
		  d->name, errno);
	return 0;
    default:
	bzero(passwd, strlen(passwd));
	Debug ("StartSession, fork succeeded %d\n", pid);
	*pidp = pid;
	return 1;
    }
}

int
source (environ, file)
char			**environ;
char			*file;
{
    char	**args, *args_safe[2];
    extern char	**parseArgs ();
    int		ret;

    if (file && file[0]) {
	Debug ("source %s\n", file);
	args = parseArgs ((char **) 0, file);
	if (!args)
	{
	    args = args_safe;
	    args[0] = file;
	    args[1] = NULL;
	}
	ret = runAndWait (args, environ);
	freeArgs (args);
	return ret;
    }
    return 0;
}

int
runAndWait (args, environ)
    char	**args;
    char	**environ;
{
    int	pid;
    waitType	result;

    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
	execute (args, environ);
	LogError ("can't execute \"%s\" (err %d)\n", args[0], errno);
	exit (1);
    case -1:
	Debug ("fork failed\n");
	LogError ("can't fork to execute \"%s\" (err %d)\n", args[0], errno);
	return 1;
    default:
	while (wait (&result) != pid)
		/* SUPPRESS 530 */
		;
	break;
    }
    return waitVal (result);
}

void
execute (argv, environ)
    char **argv;
    char **environ;
{
    /* give /dev/null as stdin */
    (void) close (0);
    open ("/dev/null", 0);
    /* make stdout follow stderr to the log file */
    dup2 (2,1);
    execve (argv[0], argv, environ);
    /*
     * In case this is a shell script which hasn't been
     * made executable (or this is a SYSV box), do
     * a reasonable thing
     */
    if (errno != ENOENT) {
	char	program[1024], *e, *p, *optarg;
	FILE	*f;
	char	**newargv, **av;
	int	argc;

	/*
	 * emulate BSD kernel behaviour -- read
	 * the first line; check if it starts
	 * with "#!", in which case it uses
	 * the rest of the line as the name of
	 * program to run.  Else use "/bin/sh".
	 */
	f = fopen (argv[0], "r");
	if (!f)
	    return;
	if (fgets (program, sizeof (program) - 1, f) == NULL)
 	{
	    fclose (f);
	    return;
	}
	fclose (f);
	e = program + strlen (program) - 1;
	if (*e == '\n')
	    *e = '\0';
	if (!strncmp (program, "#!", 2)) {
	    p = program + 2;
	    while (*p && isspace (*p))
		++p;
	    optarg = p;
	    while (*optarg && !isspace (*optarg))
		++optarg;
	    if (*optarg) {
		*optarg = '\0';
		do
		    ++optarg;
		while (*optarg && isspace (*optarg));
	    } else
		optarg = 0;
	} else {
	    p = "/bin/sh";
	    optarg = 0;
	}
	Debug ("Shell script execution: %s (optarg %s)\n",
		p, optarg ? optarg : "(null)");
	for (av = argv, argc = 0; *av; av++, argc++)
	    /* SUPPRESS 530 */
	    ;
	newargv = (char **) malloc ((argc + (optarg ? 3 : 2)) * sizeof (char *));
	if (!newargv)
	    return;
	av = newargv;
	*av++ = p;
	if (optarg)
	    *av++ = optarg;
	/* SUPPRESS 560 */
	while (*av++ = *argv++)
	    /* SUPPRESS 530 */
	    ;
	execve (newargv[0], newargv, environ);
    }
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
    {
	env = setEnv (env, "USER", user);
	env = setEnv (env, "LOGNAME", user);
    }
    env = setEnv (env, "PATH", d->systemPath);
    env = setEnv (env, "SHELL", d->systemShell);
    if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
    return env;
}
