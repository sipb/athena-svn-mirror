/* Copyright 1989, 1999 by the Massachusetts Institute of Technology.
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

/* This file contains the kerberos parts of the rkinit library.
 * See the comment at the top of rk_lib.c for a description of the naming
 * conventions used within the rkinit library.
 */

static const char rcsid[] = "$Id: rk_krb.c,v 1.4 2001-04-04 21:17:13 ghudson Exp $";

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

#include <krb.h>
#include <des.h>

#include <rkinit.h>
#include <rkinit_err.h>

static sigjmp_buf env;
static void sig_restore(int sig);
static void push_signals(void);
static void pop_signals(void);

/* Information to be passed around within client get_in_tkt */
typedef struct {
    KTEXT scip;			/* Server KDC packet */
    char *username;
    char *host;
} rkinit_intkt_info;

static char errbuf[BUFSIZ];

static int rki_key_proc(char *user, char *instance, char *realm, char *arg,
			des_cblock key)
{
    rkinit_intkt_info *rii = (rkinit_intkt_info *)arg;
    char password[BUFSIZ];
    int ok = 0;
    struct termios ttyb;

    SBCLEAR(ttyb);
    BCLEAR(password);

    /*
     * If the username does not match the aname in the ticket,
     * we will print that too.  Otherwise, we won't.
     */

    printf("Kerberos initialization (%s)", rii->host);
    if (strcmp(rii->username, user))
	printf(": tickets will be owned by %s", rii->username);

    printf("\nPassword for %s%s%s@%s: ", user,
	   (instance[0]) ? "." : "", instance, realm);

    fflush(stdout);

    push_signals();
    if (setjmp(env)) {
	ok = -1;
	goto lose;
    }

    (void) tcgetattr(0, &ttyb);
    ttyb.c_lflag &= ~ECHO;
    (void) tcsetattr(0, TCSAFLUSH, &ttyb);

    memset(password, 0, sizeof(password));
    if (read(0, password, sizeof(password)) == -1) {
	perror("read");
	ok = -1;
	goto lose;
    }

    if (password[strlen(password)-1] == '\n')
	password[strlen(password)-1] = 0;

     /* Generate the key from the password and destroy the password */

    des_string_to_key(password, key);

lose:
    BCLEAR(password);

    ttyb.c_lflag |= ECHO;
    (void) tcsetattr(0, TCSAFLUSH, &ttyb);

    pop_signals();
    printf("\n");

    return(ok);
}

static int rki_decrypt_tkt(char *user, char *instance, char *realm,
			   char *arg,
			   int (*key_proc)(char *, char *, char *,
					   char *, C_Block),
			   KTEXT *cipp)
{
    KTEXT cip = *cipp;
    C_Block key;		/* Key for decrypting cipher */
    Key_schedule key_s;
    KTEXT scip = 0;		/* cipher from rkinit server */

    rkinit_intkt_info *rii = (rkinit_intkt_info *)arg;

    /* generate a key */
    {
	register int rc;
	rc = (*key_proc)(user, instance, realm, arg, key);
	if (rc)
	    return(rc);
    }

    des_key_sched(key, key_s);

    /* Decrypt information from KDC */
    des_pcbc_encrypt((C_Block *)cip->dat,(C_Block *)cip->dat,
		     (long) cip->length, key_s, (C_Block *)key, 0);

    /* DescrYPT rkinit server's information from KDC */
    scip = rii->scip;
    des_pcbc_encrypt((C_Block *)scip->dat,(C_Block *)scip->dat,
		     (long) scip->length, key_s, (C_Block *)key, 0);

    /* Get rid of all traces of key */
    memset((char *)key, 0, sizeof(key));
    memset((char *)key_s, 0, sizeof(key_s));

    return(0);
}

int rki_get_tickets(int version, char *host, char *r_krealm, rkinit_info *info)
{
    int status;
    KTEXT_ST auth;
    char phost[MAXHOSTNAMELEN];
    KTEXT_ST scip;		/* server's KDC packet */
    des_cblock key;
    des_key_schedule sched;
    struct sockaddr_in caddr;
    struct sockaddr_in saddr;
    CREDENTIALS cred;
    MSG_DAT msg_data;
    u_char enc_data[MAX_KTXT_LEN];
    struct timeval tv;

    rkinit_intkt_info rii;

    SBCLEAR(auth);
    BCLEAR(phost);
    SBCLEAR(rii);
    SBCLEAR(scip);
    SBCLEAR(caddr);
    SBCLEAR(saddr);
    SBCLEAR(cred);
    SBCLEAR(msg_data);
    BCLEAR(enc_data);

    status = rki_send_rkinit_info(version, info);
    if (status != RKINIT_SUCCESS)
	return(status);

    status = rki_rpc_get_skdc(&scip);
    if (status != RKINIT_SUCCESS)
	return(status);

    rii.scip = &scip;
    rii.host = host;
    rii.username = info->username;
    status = krb_get_in_tkt(info->aname, info->inst, info->realm,
			    "krbtgt", info->realm, 1, rki_key_proc,
			    rki_decrypt_tkt, (char *)&rii);
    if (status) {
	strcpy(errbuf, krb_err_txt[status]);
	rkinit_errmsg(errbuf);
	return(RKINIT_KERBEROS);
    }

    /* Create an authenticator */
    strcpy(phost, krb_get_phost(host));
    status = krb_mk_req(&auth, KEY, phost, r_krealm, 0);
    if (status) {
	sprintf(errbuf, "krb_mk_req: %s", krb_err_txt[status]);
	rkinit_errmsg(errbuf);
	return(RKINIT_KERBEROS);
    }

    /* Re-encrypt server KDC packet in session key */
    /* Get credentials from ticket file */
    status = krb_get_cred(KEY, phost, r_krealm, &cred);
    if (status) {
	sprintf(errbuf, "krb_get_cred: %s", krb_err_txt[status]);
	rkinit_errmsg(errbuf);
	return(RKINIT_KERBEROS);
    }

    /* Exctract the session key and make the schedule */
    memcpy(key, cred.session, sizeof(key));
    status = des_key_sched(key, sched);
    if (status) {
	sprintf(errbuf, "des_key_sched: %s", krb_err_txt[status]);
	rkinit_errmsg(errbuf);
	return(RKINIT_DES);
    }

    /* Get client and server addresses */
    status = rki_get_csaddr(&caddr, &saddr);
    if (status != RKINIT_SUCCESS)
	return(status);

    /*
     * scip was passed to krb_get_in_tkt, where it was decrypted.
     * Now re-encrypt in the session key.
     */

    msg_data.app_data = enc_data;
    msg_data.app_length = krb_mk_priv(scip.dat, msg_data.app_data,
				      scip.length, sched, key, &caddr,
				      &saddr);
    if (msg_data.app_length == -1) {
	sprintf(errbuf, "krb_mk_priv failed.");
	rkinit_errmsg(errbuf);
	return(RKINIT_KERBEROS);
    }

    /* Destroy tickets, which we no longer need */
    dest_tkt();

    status = rki_rpc_send_ckdc(&msg_data);
    if (status != RKINIT_SUCCESS)
	return(status);

    if (version < 4) {
	/* Version 3 servers and below can't handle getting two packets
	 * close together.  Delay here.  Use select() to avoid disturbing
	 * the itimer. */
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	select(0, NULL, NULL, NULL, &tv);
    }

    status = rki_rpc_sendauth(&auth);
    if (status != RKINIT_SUCCESS)
	return(status);

    status = rki_rpc_get_status();
    if (status)
	return(status);

    return(RKINIT_SUCCESS);
}


static struct sigaction oact[NSIG];

static void push_signals(void)
{
    int i;
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sig_restore;
    for (i = 0; i < NSIG; i++)
        (void) sigaction (i, &act, &oact[i]);
}

static void pop_signals(void)
{
    int i;
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    for (i = 0; i < NSIG; i++)
        (void) sigaction (i, &oact[i], NULL);
}

static void sig_restore(int sig)
{
    siglongjmp(env,1);
}
