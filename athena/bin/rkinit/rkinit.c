/* Copyright 1989,1999 by the Massachusetts Institute of Technology.
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

/* rkinit: a remote kinit client */

static const char rcsid[] = "$Id: rkinit.c,v 1.2 1999-12-09 22:23:58 danw Exp $";

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <com_err.h>
#include <krb.h>
#include <des.h>

#include <rkinit.h>
#include <rkinit_err.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static void usage(void)
{
    fprintf(stderr,"Usage: rkinit [host] options\n");
    fprintf(stderr,
      "Options: [-l username] [-k krb_realm] [-p principal] [-f tktfile]\n");
    fprintf(stderr, "         [-t lifetime] [-h host] [-notimeout]\n");
    fprintf(stderr, "A host must be specified either with the -h option ");
    fprintf(stderr, "or as the first argument.\n");

    exit(1);
}

int main(int argc, char *argv[])
{
    char *whoami;		/* Name of this program */

    char principal[MAX_K_NAME_SZ]; /* Principal for which to get tickets */
    char *host = NULL;		/* Remote host */
    char *username = 0;	/* Username of owner of ticket */
    char r_krealm[REALM_SZ];	/* Kerberos realm of remote host */
    char aname[ANAME_SZ];	/* Aname of remote ticket file */
    char inst[INST_SZ];		/* Instance of remote ticket file */
    char realm[REALM_SZ];	/* Realm of remote ticket file */
    char *tktfilename = NULL;	/* Name of ticket file on remote host */
    u_long lifetime = DEFAULT_TKT_LIFE;	/* Lifetime of remote tickets */
    int timeout = TRUE;		/* Should we time out? */
    rkinit_info info;		/* Information needed by rkinit */

    struct passwd *localid;	/* To determine local id */

    int status = 0;		/* general error number */

    int i;

    memset(principal, 0, sizeof(principal));
    memset(aname, 0, sizeof(aname));
    memset(inst, 0, sizeof(inst));
    memset(realm, 0, sizeof(realm));
    memset(r_krealm, 0, sizeof(r_krealm));
    /* Parse commandline arguements. */
    whoami = strrchr(argv[0], '/');
    if (whoami)
	whoami++;
    else
	whoami = argv[0];

    if (argc < 2) usage();

    if (argv[1][0] != '-') {
	host = argv[1];
	i = 2;
    }
    else
	i = 1;

    for (/* i initialized above */; i < argc; i++) {
	if (strcmp(argv[i], "-h") == NULL) {
	    if (++i >= argc)
		usage();
	    else
		host = argv[i];
	}
	else if (strcmp(argv[i], "-l") == NULL) {
	    if (++i >= argc)
		usage();
	    else
		username = argv[i];
	}
	else if (strcmp(argv[i], "-k") == NULL) {
	    if (++i >= argc)
		usage();
	    else
		strncpy(r_krealm, argv[i], sizeof(r_krealm) - 1);
	}
	else if (strcmp(argv[i], "-p") == NULL) {
	    if (++i >= argc)
		usage();
	    else
		strncpy(principal, argv[i], sizeof(principal) - 1);
	}
	else if (strcmp(argv[i], "-f") == NULL) {
	    if (++i >= argc)
		usage();
	    else
		tktfilename = argv[i];
	}
	else if (strcmp(argv[i], "-t") == NULL) {
	    if (++i >= argc)
		usage();
	    else {
		lifetime = atoi(argv[i])/5;
		if (lifetime == 0)
		    lifetime = 1;
		else if (lifetime > 255)
		    lifetime = 255;
	    }
	}
	else if (strcmp(argv[i], "-notimeout") == NULL)
	    timeout = FALSE;
	else
	    usage();
    }

    if (host == NULL)
	usage();

    /* Initialize the realm of the remote host if necessary */
    if (r_krealm[0] == 0) {
	/*
	 * Try to figure out the realm of the remote host.  If the
	 * remote host is unknown, don't worry about it; the library
	 * will handle the error better and print a good error message.
	 */
	struct hostent *hp;
	hp = gethostbyname(host);
	if (hp)
	    strcpy(r_krealm, krb_realmofhost(hp->h_name));
    }

    /* If no username was specified, use local id on client host */
    if (username == 0) {
	localid = getpwuid(getuid());
	if (localid == 0) {
	    fprintf(stderr, "You can not be found in the password file.\n");
	    exit(1);
	}
	username = localid->pw_name;
    }

    /* Find out who will go in the ticket file */
    if (! principal[0]) {
	status = krb_get_tf_fullname(TKT_FILE, aname, inst, realm);
	if (status != KSUCCESS) {
	    /*
	     * If user has no ticket file and principal was not specified,
	     * we will try to get tickets for username@remote_realm
	     */
	    strcpy(aname, username);
	    strcpy(realm, r_krealm);
	}
    }
    else {
	status = kname_parse(aname, inst, realm, principal);
	if (status != KSUCCESS) {
	    fprintf(stderr, "%s\n", krb_err_txt[status]);
	    exit(1);
	}
	if (strlen(realm) == 0) {
	    if (krb_get_lrealm(realm, 1) != KSUCCESS)
		strcpy(realm, KRB_REALM);
	}
    }

    memset(&info, 0, sizeof(info));

    strcpy(info.aname, aname);
    strcpy(info.inst, inst);
    strcpy(info.realm, realm);
    strcpy(info.sname, "krbtgt");
    strcpy(info.sinst, realm);
    strncpy(info.username, username, sizeof(info.username) - 1);
    if (tktfilename)
	strncpy(info.tktfilename, tktfilename, sizeof(info.tktfilename) - 1);
    info.lifetime = lifetime;

    status = rkinit(host, r_krealm, &info, timeout);
    if (status) {
	com_err(whoami, status, "while obtaining remote tickets:");
	fprintf(stderr, "%s\n", rkinit_errmsg(0));
	exit(1);
    }

    exit(0);
}
