/* kpasswd.c --- verify kerberos passwords.
 * written by Nat Lanza (magus@cs.cmu.edu) for
 * xscreensaver, Copyright (c) 1993-1997, 1998, 2000, 2003
 *  Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef NO_LOCKING  /* whole file */

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

/* I'm not sure if this is exactly the right test...
   Might __APPLE__ be defined if this is apple hardware, but not
   an Apple OS?

   Thanks to Alexei Kosut <akosut@stanford.edu> for the MacOS X code.
 */
#ifdef __APPLE__
# define HAVE_DARWIN
#endif


#if defined(HAVE_DARWIN)
# include <Kerberos/Kerberos.h>
#elif defined(HAVE_KERBEROS5)
# include <krb5.h>
# include <kerberosIV/krb.h>
# include <kerberosIV/des.h>
# include <com_err.h>
#else /* !HAVE_KERBEROS5 (meaning Kerberos 4) */
# include <krb.h>
# include <des.h>
#endif /* !HAVE_KERBEROS5 */

#if !defined(VMS) && !defined(HAVE_ADJUNCT_PASSWD)
# include <pwd.h>
#endif


#ifdef __bsdi__
# include <sys/param.h>
# if _BSDI_VERSION >= 199608
#  define BSD_AUTH
# endif
#endif /* __bsdi__ */

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Intrinsic.h>
#include "xscreensaver.h"
#include "prefs.h"

#define TICKET_LIFETIME (10 * 60 * 60)	/* 10 hours */

/* The user information we need to store */
#ifdef HAVE_DARWIN
 static KLPrincipal princ;
#else /* !HAVE_DARWIN */
 static int get_krb4_with_password(char *name, char *inst, char *realm,
				   const char *password, int lifetime);
 static char realm[REALM_SZ];
 static char  name[ANAME_SZ];
 static char  inst[INST_SZ];
#ifdef HAVE_KERBEROS5
 static time_t get_krb5_expiration(void);
 static Bool renew(krb5_creds *creds, const char *typed_passwd,
		   const char *command);
 static Bool convert_524(krb5_creds *k5creds);
 static krb5_context k5context;
 static krb5_principal k5princ = NULL;
#else /* Use Kerberos 4 */
 static char *tk_file;
#endif /* !HAVE_KERBEROS5 */
#endif /* !HAVE_DARWIN */


/* Called at startup to grab user, instance, and realm information
   from the user's ticketfile (remember, name.inst@realm). Since we're
   using tf_get_pname(), this should work even if your kerberos username
   isn't the same as your local username. We grab the ticket at startup
   time so that even if your ticketfile dies while the screen's locked
   we'll still have the information to unlock it.

   Problems: the password dialog currently displays local username, so if
     you have some non-standard name/instance when you run xscreensaver,
     you'll need to remember what it was when unlocking, or else you lose.

   Like the original lock_init, we return false if something went wrong.
   We don't use the arguments we're given, though.
 */
Bool
kerberos_lock_init (saver_preferences *p)
{
# ifdef HAVE_DARWIN

    KLBoolean found;
    return ((klNoErr == (KLCacheHasValidTickets (NULL, kerberosVersion_Any,
                                                 &found, &princ, NULL)))
            && found);

# else /* !HAVE_DARWIN */

    /* Perhaps we should be doing it the Mac way (above) all the time?
       The following code assumes Unix-style file-based Kerberos credentials
       cache, which Mac OS X doesn't use.  But is there any real reason to
       do it this way at all, even on other Unixen?
     */

#ifdef HAVE_KERBEROS5
    krb5_ccache ccache;
    krb5_error_code k5status;

    /* Initialize a Kerberos 5 context. */
    if (krb5_init_context(&k5context) != 0) {
	return False;
    }

    /* Access the default credentials cache. */
    if (krb5_cc_default(k5context, &ccache) != 0) {
	krb5_free_context(k5context);
	return False;
    }

    /* Get the user's principal from the credentials cache. */
    k5status = krb5_cc_get_principal(k5context, ccache, &k5princ);
    krb5_cc_close(k5context, ccache);
    if (k5status != 0) {
	krb5_free_context(k5context);
	return False;
    }

    /* Translate to a Kerberos 4 principal. */
    k5status = krb5_524_conv_principal(k5context, k5princ, name, inst, realm);
    if (k5status != 0) {
	memset(name, 0, sizeof(name));
	memset(inst, 0, sizeof(inst));
	memset(realm, 0, sizeof(realm));
    }

    return True;

#else /* !HAVE_KERBEROS5 (meaning Kerberos 4) */

    int k_errno;
    
    memset(name, 0, sizeof(name));
    memset(inst, 0, sizeof(inst));
    
    /* find out where the user's keeping his tickets.
       squirrel it away for later use. */
    tk_file = tkt_string();

    /* open ticket file or die trying. */
    if ((k_errno = tf_init(tk_file, R_TKT_FIL))) {
	return False;
    }

    /* same with principal and instance names */
    if ((k_errno = tf_get_pname(name)) ||
	(k_errno = tf_get_pinst(inst))) {
	return False;
    }

    /* close the ticketfile to release the lock on it. */
    tf_close();

    /* figure out what realm we're authenticated to. this ought
       to be the local realm, but it pays to be sure. */
    if ((k_errno = krb_get_tf_realm(tk_file, realm))) {
	return False;
    }

    /* last-minute sanity check on what we got. */
    if ((strlen(name)+strlen(inst)+strlen(realm)+3) >
	(REALM_SZ + ANAME_SZ + INST_SZ + 3)) {
	return False;
    }

    /* success */
    return True;
#endif /* !HAVE_KERBEROS5 */
# endif /* !HAVE_DARWIN */
}

/* Called to see if the user's typed password is valid. We do this by asking
   the kerberos server for a ticket and checking to see if it gave us one.
   For Kerberos 5, we might renew the user's credentials, depending on the
   "renew" preference.  This option is not supported For Kerberos 4; there
   we need to move the ticketfile first, or otherwise we end up updating the
   user's tkfile with new tickets. This would break services like zephyr that
   like to stay authenticated, and it would screw with AFS authentication at
   some sites. So, we do a quick, painful hack with a tmpfile.
 */
Bool
kerberos_passwd_valid_p (const char *typed_passwd, saver_preferences *p)
{
# ifdef HAVE_DARWIN
    return (klNoErr ==
            KLAcquireNewInitialTicketsWithPassword (princ, NULL,
                                                    typed_passwd, NULL));
# else /* !HAVE_DARWIN */

    /* See comments in kerberos_lock_init -- should we do it the Mac Way
       on all systems?
     */
#ifdef HAVE_KERBEROS5
    time_t current_endtime, now;
    krb5_get_init_creds_opt options;
    krb5_creds creds;
    krb5_error_code k5status;

    /* Initialize the options structure. */
    krb5_get_init_creds_opt_init(&options);

    /* Set the ticket lifetime. */
    krb5_get_init_creds_opt_set_tkt_life(&options, TICKET_LIFETIME);

    /* Initialize the credentials structure. */
    memset(&creds, 0, sizeof(creds));

    /* Get a new TGT. */
    k5status = krb5_get_init_creds_password(k5context, &creds, k5princ,
					    (char *) typed_passwd, NULL, NULL,
					    0, NULL, &options);
    if (k5status != 0) {
	return False;
    }

    /* We got a new TGT.  See if we should renew the user's cached
       credentials.  We will renew in the following situations:
       1. The renew preference setting is "invalid", and the user's
          credentials no longer exist, or have expired.
       2. The renew setting is "older", and the user's credentials
          will expire before the new TGT expires (or do not exist).
       3. The renew setting is "always".
     */

    if (p->renew == INVALID || p->renew == OLDER) {
	/* Get the end time of the existing TGT, if any. */
	current_endtime = get_krb5_expiration();
	time(&now);
    }

    if (p->renew == ALWAYS ||
	(p->renew == INVALID && now >= current_endtime) ||
	(p->renew == OLDER && creds.times.endtime > current_endtime)) {
	/* Renew using the newly-obtained TGT. */
	renew(&creds, typed_passwd, p->renew_command);
    }

    krb5_free_cred_contents(k5context, &creds);
    return True;

#else /* !HAVE_KERBEROS5 (meaning Kerberos 4) */
    Bool success;
    char *newtkfile;

    /* temporarily switch to a new ticketfile.
       I'm not using tmpnam() because it isn't entirely portable.
       this could probably be fixed with autoconf. */
    newtkfile = malloc(80 * sizeof(char));
    memset(newtkfile, 0, sizeof(newtkfile));

    sprintf(newtkfile, "/tmp/xscrn-%i", getpid());

    krb_set_tkt_string(newtkfile);

    success = get_krb4_with_password(name, inst, realm, typed_passwd,
				     TICKET_LIFETIME);

    /* quickly block out the tempfile to prevent snooping,
       then restore the old ticketfile and clean up a bit. */
    
    dest_tkt();
    krb_set_tkt_string(tk_file);
    free(newtkfile);

    /* Did we verify successfully? */
    return success;
#endif /* !HAVE_KERBEROS5 */

# endif /* !HAVE_DARWIN */
}

#ifdef HAVE_KERBEROS5
/* Renew the user's cached credentials, using the given Kerberos 5 TGT.
   If the typed_passwd argument is non-NULL, we use it to try and get
   Kerberos 4 tickets, too; otherwise, we convert the Kerberos 5 TGT.
   Finally, we run the given command string, if any, as a shell command.
 */
static Bool renew(krb5_creds *creds, const char *typed_passwd,
		  const char *command)
{
    krb5_ccache ccache;
    krb5_error_code k5status;

    /* Get a handle on the credentials cache. */
    k5status = krb5_cc_default(k5context, &ccache);
    if (k5status != 0) {
	com_err(blurb(), k5status, "getting default cache");
	return False;
    }

    /* Initialize the cache. */
    k5status = krb5_cc_initialize(k5context, ccache, k5princ);
    if (k5status != 0) {
	com_err(blurb(), k5status, "initializing cache");
	krb5_cc_close(k5context, ccache);
	return False;
    }

    /* Store the new credentials in the cache. */
    k5status = krb5_cc_store_cred(k5context, ccache, creds);
    krb5_cc_close(k5context, ccache);
    if (k5status != 0) {
	com_err(blurb(), k5status, "storing credentials");
	return False;
    }

    /* Try to get krb4 tickets, too, via either the typed password, or
       by converting the krb5 TGT. */
    if (name[0]) {
	if (typed_passwd == NULL ||
	    !get_krb4_with_password(name, inst, realm, typed_passwd,
				    TICKET_LIFETIME)) {
	    convert_524(creds);
	}
    }

    /* Run the renew shell command, if any. */
    if (command && *command) {
	system(command);
    }

    return True;
}

/* Get the expiration time of the existing Kerberos 5 TGT, or, if no
   such credentials are found, 0.
 */
static time_t get_krb5_expiration(void)
{
    time_t expiration = 0;
    krb5_ccache ccache = NULL;
    krb5_creds match_creds, creds;
    krb5_data *realm;

    memset(&match_creds, 0, sizeof(match_creds));
    memset(&creds, 0, sizeof(creds));

    /* Access the credentials cache. */
    if (krb5_cc_default(k5context, &ccache) != 0) {
	return 0;
    }

    /* Set the matching creds' client to the principal from the cache. */
    if (krb5_cc_get_principal(k5context, ccache, &match_creds.client) != 0) {
	krb5_cc_close(k5context, ccache);
	return 0;
    }

    /* Build the matching creds' server principal (krbtgt/<realm>@<realm>). */
    realm = krb5_princ_realm(k5_context, match_creds.client);
    if (krb5_build_principal_ext(k5context, &match_creds.server, 
				 realm->length, realm->data,
				 KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME, 
				 realm->length, realm->data, NULL) != 0) {
	krb5_free_cred_contents(k5context, &match_creds);
	krb5_cc_close(k5context, ccache);
	return 0;
    }

    /* Get the TGT from the cache. */
    if (krb5_cc_retrieve_cred(k5context, ccache, 0, &match_creds, &creds)
	== 0) {
	expiration = creds.times.endtime;
	krb5_free_cred_contents(k5context, &creds);
    }
    krb5_cc_close(k5context, ccache);
    krb5_free_cred_contents(k5context, &match_creds);
    return expiration;
}
#endif /* HAVE_KERBEROS5 */

/* Get a Kerberos 4 TGT using the given password. */
static int get_krb4_with_password(char *name, char *inst, char *realm,
				  const char *password, int lifetime)
{
    int k4life;
    int k4status;

    /* Kerberos 4 lifetime is an 8-bit value, in 5-minute units. */
    k4life = lifetime / (5 * 60);
    if (k4life < 1)
	k4life = 1;
    else if (k4life > 0xff)
	k4life = 0xff;

    k4status = krb_get_pw_in_tkt(name, inst, realm,
				 KRB_TICKET_GRANTING_TICKET, realm,
				 k4life, (char *) password);
    return (k4status == 0);
}

#ifdef HAVE_KERBEROS5
/* Convert krb5 credentials to krb4, and store them in the
   krb4 ticket cache.
 */
static Bool convert_524(krb5_creds *k5creds)
{
    krb5_error_code k5status;
    int k4status;
    CREDENTIALS k4creds;

    k5status = krb524_convert_creds_kdc(k5context, k5creds, &k4creds);
    if (k5status != 0) {
	com_err(blurb(), k5status, "converting krb5 credentials to krb4");
	return False;
    }
    /* Initialize the krb4 ticket cache. */
    k4status = in_tkt(k4creds.pname, k4creds.pinst);
    if (k4status != KSUCCESS) {
	com_err(blurb(), k4status, "initializing the krb4 ticket file");
	memset(&k4creds, 0, sizeof(k4creds));
	return False;
    }

    /* Store the new ticket in the cache. */
    k4status = krb_save_credentials(k4creds.service,
				    k4creds.instance,
				    k4creds.realm,
				    k4creds.session,
				    k4creds.lifetime,
				    k4creds.kvno,
				    &k4creds.ticket_st,
				    k4creds.issue_date);
    memset(&k4creds, 0, sizeof(k4creds));
    if (k4status != KSUCCESS) {
	com_err(blurb(), k4status, "saving the krb4 ticket");
	return False;
    }

    /* Success. */
    return True;
}
#endif /* HAVE_KERBEROS5 */

#endif /* NO_LOCKING -- whole file */
