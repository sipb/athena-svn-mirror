/* $XConsortium: verify.c /main/36 1996/04/18 14:56:15 gildea $ */
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
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	<pwd.h>
#ifdef USESHADOW
# include	<shadow.h>
# include	<errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif
#endif

#ifdef KRB5LOGIN
#include <sys/param.h>
#include <krb5.h>
#include <com_err.h>
#endif

# include	"greet.h"

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

static char *envvars[] = {
    "TZ",			/* SYSV and SVR4, but never hurts */
#if defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
#endif
#if (defined(SVR4) || defined(SYSV)) && defined(i386) && !defined(sun)
    "XLOCAL",
#endif
    NULL
};

static char **
userEnv (d, useSystemPath, user, home, shell)
struct display	*d;
int	useSystemPath;
char	*user, *home, *shell;
{
    char	**env;
    char	**envvar;
    char	*str;
    extern char **defaultEnv (), **setEnv ();
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "LOGNAME", user); /* POSIX, System V */
    env = setEnv (env, "USER", user);    /* BSD */
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
    for (envvar = envvars; *envvar; envvar++)
    {
	str = getenv(*envvar);
	if (str)
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

int
Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
#ifdef USESHADOW
	struct spwd	*sp;
#endif
	char		*user_pass;
#if !defined(SVR4) || !defined(GREET_LIB) /* shared lib decls handle this */
	char		*crypt ();
	char		**systemEnv (), **parseArgs ();
	extern char	**setEnv();
#endif
	char		*shell, *home;
#ifdef KRB5LOGIN
	char ccfile[MAXPATHLEN+6];
	int krb5_auth = 0;
#endif
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0) {
		Debug ("getpwnam() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	} else {
	    user_pass = p->pw_passwd;
	}

#ifdef KRB5LOGIN
#define DEFAULT_LIFETIME "10h 0m 0s"
	/*
	 * If we use Kerberos, try checking the Kerberos database for this
	 * person.  Don't check root.
	 */

	if (p->pw_uid != 0) {
		krb5_context context;
		krb5_auth_context auth_context = NULL;
		krb5_error_code code;
		krb5_ccache cc;
		krb5_principal me, tgt, host;
		krb5_deltat lifetime;
		krb5_timestamp now;
		krb5_creds creds;
		krb5_keyblock *key = NULL;
		krb5_data packet;
		int options = 0, forward = 0;
		int have_host_key;
		char *strlifetime;
		char tmpstr[BUFSIZ], *p;
		char phost[BUFSIZ];

		if ((code = krb5_init_context(&context))) {
			Debug("krb5_init_context failed: %s\n",
			      error_message(code));
			goto bad_krb5auth;
		}

		/*
		 * Setup the KRB5CCNAME environment variable first,
		 * e.g. krb5cc_xdm_hostname_0 or krb5cc_xdm_132.241.3.10_1
		 */

		strcpy(tmpstr, d->name);
		if ((p = strchr(tmpstr, '.')) && !isdigit(tmpstr[0]))
			*p = '\0';
		if (p = strchr(tmpstr, ':'))
			*p = '\0';
		sprintf(ccfile, 
		    "FILE:/tmp/krb5cc_xdm_%s%s%s", 
		    tmpstr, 
		    strlen(tmpstr) > 0 ? "_" : "",
		    (p = strrchr(d->name, ':')) ? p + 1 : "0");

		if ((code = krb5_cc_resolve(context, ccfile, &cc))) {
			Debug("krb5_cc_resolve failed: %s\n",
			      error_message(code));
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * Build a principal with my name in it
		 */

		if ((code = krb5_parse_name(context, greet->name, &me))) {
			Debug("krb5_parse_name failed: %s\n",
			      error_message(code));
			krb5_cc_close(context, cc);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * Initialize the credential cache
		 */

		if ((code = krb5_cc_initialize(context, cc, me))) {
			Debug("krb5_cc_initialize failed: %s\n",
			      error_message(code));
			krb5_cc_close(context, cc);
			krb5_free_principal(context, me);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * Build the TGT principal
		 */

		code = krb5_build_principal_ext(context, &tgt,
					krb5_princ_realm(context, me)->length,
					krb5_princ_realm(context, me)->data,
					KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
					krb5_princ_realm(context, me)->length,
					krb5_princ_realm(context, me)->data,
					0);
		
		if (code) {
			Debug("krb5_build_principal_ext failed: %s\n",
			      error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_principal(context, me);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * Get the default lifetime from the config file
		 */

		krb5_appdefault_string(context, "xdm",
				       krb5_princ_realm(context, me),
				       "default_lifetime", DEFAULT_LIFETIME,
				       &strlifetime);
		
		if ((code = krb5_string_to_deltat(strlifetime, &lifetime))) {
			lifetime = 10*60*60;	/* 10 hours */
		}

		free(strlifetime);

		/*
		 * Build the credential structure
		 */

		if ((code = krb5_timeofday(context, &now))) {
			Debug("krb5_timeofday failed: %s\n",
			      error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_principal(context, me);
			krb5_free_principal(context, tgt);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		memset((char *) &creds, 0, sizeof(creds));
		creds.client = me;
		creds.server = tgt;
		creds.times.starttime = 0;
		creds.times.endtime = now + lifetime;
		creds.times.renew_till = 0;

		/*
		 * Get a forwardable ticket, if we requested it in the
		 * config file
		 */

		krb5_appdefault_boolean(context, "xdm",
				       krb5_princ_realm(context, me),
				       "forwardable", forward, &forward);

		if (forward) {
			options |= KDC_OPT_FORWARDABLE;
		}

		/*
		 * Actually try getting a TGT.
		 */

		if ((code = krb5_get_in_tkt_with_password(context, options,
							  0, NULL, 0,
							  greet->password,
							  cc, &creds, NULL))) {
			Debug("krb5_get_in_tkt_with_password failed: %s\n",
			      error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_cred_contents(context, &creds);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		krb5_free_cred_contents(context, &creds);

		/*
		 * You would _think_ that we're done.... but we're not.
		 *
		 * We need to actually verify that this was from the KDC,
		 * and not spoofed.  So we try to get a service ticket
		 * for this host.
		 */

		/*
		 * Get a service principal for this host (it defaults to
		 * "host" and the local hostname).
		 */

		if ((code = krb5_sname_to_principal(context, NULL, NULL,
						    KRB5_NT_SRV_HST, &host))) {
			Debug("krb5_sname_to_principal failed: %s\n",
			      error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * Save the hostname, since we need it later
		 */

		strncpy(phost, krb5_princ_component(context, host, 1)->data,
			BUFSIZ);

		/*
		 * See if we have a "host" key
		 */

		code = krb5_kt_read_service_key(context, NULL, host, 0, 0,
						&key);
		
		if (key)
			krb5_free_keyblock(context, key);

		have_host_key = code ? 0 : 1;

		/*
		 * Get a service ticket for "host"
		 */

		packet.data = NULL;

		code = krb5_mk_req(context, &auth_context, 0, "host", phost,
				   0, cc, &packet);
		
		/*
		 * Clear out the auth context (otherwise we'll use the wrong
		 * key when we try to decrypt
		 */

		if (auth_context) {
			krb5_auth_con_free(context, auth_context);
			auth_context = NULL;
		}

		if (code == KRB5KDC_ERR_S_PRINCIPAL_UNKNOWN) {
			if (have_host_key) {
				/*
				 * The KDC doesn't know about this key,
				 * but we have one.  Might have been spoofed.
				 */
				Debug("We have a key, but the KDC didn't!"
				      " Time to bail\n");
				if (packet.data)
					free(packet.data);
				krb5_free_principal(context, host);
				krb5_cc_destroy(context, cc);
				krb5_free_context(context);
				goto bad_krb5auth;
			} else {
				/*
				 * Nobody has a key, so I guess things are
				 * okay.
				 */

				Debug("No key found, so I guess we're okay\n");
				if (packet.data)
					free(packet.data);
				krb5_free_principal(context, host);
				krb5_cc_close(context, cc);
				krb5_free_context(context);
				krb5_auth = 1;
				goto verify_okay;
			}
		} else {
			if (code) {
				Debug("krb5_mk_req failed: %s\n",
				       error_message(code));
				if (packet.data)
					free(packet.data);
				krb5_free_principal(context, host);
				krb5_cc_destroy(context, cc);
				krb5_free_context(context);
				goto bad_krb5auth;
			}
		}

		/*
		 * Now that we have a ticket, let's try to verify it
		 */
		
		code = krb5_rd_req(context, &auth_context, &packet, host,
				   NULL, NULL, NULL);

		krb5_free_principal(context, host);

		if (auth_context)
			krb5_auth_con_free(context, auth_context);
		
		if (packet.data)
			free(packet.data);

		if (code) {
			Debug("krb5_rd_req failed: %s\n", error_message(code));
			krb5_cc_destroy(context, cc);
			krb5_free_context(context);
			goto bad_krb5auth;
		}

		/*
		 * _Whew!_ Everything is okay!
		 */

		krb5_cc_close(context, cc);
		krb5_free_context(context);

		krb5_auth = 1;
		goto verify_okay;
	}

bad_krb5auth:
#endif /* KRB5LOGIN */

#ifdef USESHADOW
	errno = 0;
	sp = getspnam(greet->name);
	if (sp == NULL) {
	    Debug ("getspnam() failed, errno=%d.  Are you root?\n", errno);
	} else {
	    user_pass = sp->sp_pwdp;
	}
	endspent();
#endif
#if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user(p, greet->password, NULL) < 0)
#else
	if (strcmp (crypt (greet->password, user_pass), user_pass))
#endif
	{
		Debug ("password verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
verify_okay:
	Debug ("verify succeeded\n");
	bzero(user_pass, strlen(user_pass)); /* in case shadow password */
	/* The password is passed to StartClient() for use by user-based
	   authorization schemes.  It is zeroed there. */
	verify->uid = p->pw_uid;
	verify->gid = p->pw_gid;
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
	verify->systemEnviron = systemEnv (d, greet->name, home);
#ifdef KRB5LOGIN
	if (krb5_auth) {
		verify->userEnviron = setEnv(verify->userEnviron, "KRB5CCNAME",
					     ccfile);
		verify->systemEnviron = setEnv(verify->systemEnviron,
					       "KRB5CCNAME", ccfile);
	}
#endif /* KRB5LOGIN */
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	return 1;
}
