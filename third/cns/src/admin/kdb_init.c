/*
 * kdb_init.c
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * program to initialize the database,  reports error if database file
 * already exists. 
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <des.h>
#include <krb.h>
#include <krb_db.h>
#include <string.h>

#define	TRUE	1

enum ap_op {
    NULL_KEY,			/* setup null keys */
    MASTER_KEY,                 /* use master key as new key */
    RANDOM_KEY			/* choose a random key */
};

int     debug = 0;
char   *progname;

C_Block master_key;
Key_schedule master_key_schedule;

main(argc, argv)
    char   *argv[];
{
    char    realm[REALM_SZ];
    char   *cp;
    int code;
#ifndef HAVE_SYS_ERRLIST_DECL
    extern char *sys_errlist[];
#endif
    char *database;
    
    progname = (cp = strrchr(*argv, '/')) ? cp + 1 : *argv;

    if (argc > 3) {
	fprintf(stderr, "Usage: %s [realm-name] [database-name]\n", argv[0]);
	exit(1);
    }
    if (argc == 3) {
	database = argv[2];
	--argc;
    } else
	database = DBM_FILE;

    /* Do this first, it'll fail if the database exists */
    if ((code = kerb_db_create(database)) != 0) {
	fprintf(stderr, "Couldn't create database: %s\n",
		sys_errlist[code]);
	exit(1);
    }
    kerb_db_set_name(database);

    if (argc == 2)
	strncpy(realm, argv[1], REALM_SZ);
    else {
	fprintf(stderr, "Realm name [default  %s ]: ", KRB_REALM);
	if (fgets(realm, sizeof(realm), stdin) == NULL) {
	    fprintf(stderr, "\nEOF reading realm\n");
	    exit(1);
	}
	if (cp = strchr(realm, '\n'))
	    *cp = '\0';
	if (!*realm)			/* no realm given */
	    strcpy(realm, KRB_REALM);
    }
    if (!k_isrealm(realm)) {
	fprintf(stderr, "%s: Bad kerberos realm name \"%s\"\n",
		progname, realm);
	exit(1);
    }
    printf("You will be prompted for the database Master Password.\n");
    printf("It is important that you NOT FORGET this password.\n");
    fflush(stdout);

    if (kdb_get_master_key (TRUE, master_key, master_key_schedule, 1) != 0) {
      fprintf (stderr, "Couldn't read master key.\n");
      exit (-1);
    }
    des_init_random_number_generator(master_key);

    if (
	add_principal(KERB_M_NAME, KERB_M_INST, MASTER_KEY) ||
	add_principal(KERB_DEFAULT_NAME, KERB_DEFAULT_INST, NULL_KEY) ||
	add_principal("krbtgt", realm, RANDOM_KEY) ||
	add_principal("changepw", KRB_MASTER, RANDOM_KEY) 
	) {
	fprintf(stderr, "\n%s: couldn't initialize database.\n",
		progname);
	exit(1);
    }

    /* play it safe */
    memset (master_key, 0, sizeof (C_Block));
    memset (master_key_schedule, 0, sizeof (Key_schedule));
    exit(0);
}

/* use a return code to indicate success or failure.  check the return */
/* values of the routines called by this routine. */

add_principal(name, instance, aap_op)
    char   *name, *instance;
    enum ap_op aap_op;
{
    Principal principal;
    char    datestring[50];
    char    pw_str[255];
    time_t  mod_date;
    struct tm *tm, *localtime();
    C_Block new_key;

    memset(&principal, 0, sizeof(principal));
    strncpy(principal.name, name, ANAME_SZ);
    strncpy(principal.instance, instance, INST_SZ);
    switch (aap_op) {
    case NULL_KEY:
	principal.key_low = 0;
	principal.key_high = 0;
	break;
    case RANDOM_KEY:
#ifdef NOENCRYPTION
	memset(new_key, 0, sizeof(C_Block));
	new_key[0] = 127;
#else
	des_new_random_key(new_key);
#endif
	kdb_encrypt_key (new_key, new_key, master_key, master_key_schedule,
			 ENCRYPT);
	memcpy(&principal.key_low, new_key, sizeof(KRB_INT32));
	memcpy(&principal.key_high, ((KRB_INT32 *) new_key) + 1, sizeof(KRB_INT32));
	break;
    case MASTER_KEY:
	memcpy (new_key, master_key, sizeof (C_Block));
	kdb_encrypt_key (new_key, new_key, master_key, master_key_schedule,
			 ENCRYPT);
	memcpy(&principal.key_low, new_key, sizeof(KRB_INT32));
	memcpy(&principal.key_high, ((KRB_INT32 *) new_key) + 1, sizeof(KRB_INT32));
	break;
    }
    principal.exp_date = 946702799+((365*10+3)*24*60*60);
    strncpy(principal.exp_date_txt, "12/31/2009", DATE_SZ);
    principal.mod_date = mod_date = time(0);

    tm = localtime(&mod_date);
    principal.attributes = 0;
    principal.max_life = 255;

    principal.kdc_key_ver = 1;
    principal.key_version = 1;

    strncpy(principal.mod_name, "db_creation", ANAME_SZ);
    strncpy(principal.mod_instance, "", INST_SZ);
    principal.old = 0;

    kerb_db_put_principal(&principal, 1);
    
    /* let's play it safe */
    memset (new_key, 0, sizeof (C_Block));
    memset (&principal.key_low, 0, sizeof(KRB_INT32));
    memset (&principal.key_high, 0, sizeof(KRB_INT32));
    return 0;
}
