/*
 * $Id: afs2k5db.c,v 1.1.1.1 2003-02-13 00:14:49 zacheiss Exp $
 *
 * Convert a AFS KA database to a Kerberos V5 (1.0) dump file
 *
 * Written by Ken Hornstein <kenh@cmf.nrl.navy.mil>
 *
 * The KAS database reading code was contributed by Derrick J Brashear
 * <shadow@dementia.org>, and most of it was originally written by
 * Dan Lovinger of Microsoft.
 *
 * The ugly code to deal with strange v4 conversions, and to deal with v4
 * keys in the kaserver can also be blamed on Derrick J Brashear.
 *
 */

#ifndef LINT
static char rcsid[]=
	"$Id: afs2k5db.c,v 1.1.1.1 2003-02-13 00:14:49 zacheiss Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <errno.h>

#include <krb5.h>
#include <com_err.h>

#include <k5-int.h>
#include <adm.h>
#include <adm_proto.h>

#include <afs/param.h>
#include <afs/cellconfig.h>
#include <afs/auth.h>
#include <afs/kauth.h>
#include <ubik.h>

#define NTOHL(x)	(x) = ntohl(x)
#define NTOHS(x)	(x) = ntohs(x)

/* Transarc licensing neccesitates this obscurity. If you have a source
 * license, consult src/kauth/kaserver.h for the real struct definitions.
 */

#define KADBVERSION     5               /* the database version */

#define HASHSIZE        8191            /* pick a prime for the length */

/* all fields are stored in network (sun or rt) byte order */
struct kaheader {
    long           version;             /* database version number */
    long           headerSize;          /* bytes in header, for skipping in bad times */
    long           freePtr;             /* first (if any) free entry in freelist */
    long           eofPtr;              /* first free byte in file */
    long           kvnoPtr;             /* first special name old keys entry */
    struct kasstats stats;              /* track interesting statistics */
    long           admin_accounts;      /* total number of users w/ admin flag set */
    long           specialKeysVersion;  /* inc if special name gets new key */
    long           hashsize;            /* allocated size of nameHash */
#if (KADBVERSION > 5)
    long           spare[10];           /* allocate some spares next time */
#endif
    long           nameHash[HASHSIZE];  /* hash table for names */
    long           checkVersion;        /* database version number, same as first field */
};

#define ENTRYSIZE               200
#define PADLEN                  (ENTRYSIZE - sizeof(kaident) - sizeof(struct ktc_encryptionKey) - 10*4)

/* all fields are stored in network byte order */
struct kaentry {
    long           flags;               /* random flags */
#define	KAFFREE		0x002		/* entry is on free list */
#define	KAFOLDKEYS	0x010		/* entry is used to store old keys */
    long           next;                /* next block same entry (or freelist) */
    Date           user_expiration;     /* user registration good till then */
    Date           modification_time;   /* time of last update */
    long           modification_id;     /* identity of user doing update */
    Date           change_password_time;/* time user last changed own password */
    long           max_ticket_lifetime; /* maximum lifetime for tickets */
    long           key_version;         /* verson number of this key */
    union { /* overload several miscellaneous fields */
        struct {
            long   nOldKeys;            /* number of outstanding old keys */
            long   oldKeys;             /* block containing old keys */
        } asServer; /* for principals that are part of the AuthServer itself */
        struct {
            long   maxAssociates;       /* associates this user can create */
            long   nInstances;          /* number of instances user's created */
        } assocRoot; /* for principals at root of associate tree */
        struct {
            long   root;                /* identity of this instance's root */
            long   spare;
        } associate; /* associate instance */
    } misc;
    /* put the strings last to simplify alignment calculations */
    struct kaident userID;              /* user and instance names */
    struct ktc_encryptionKey key;       /* the key to use */
    char           padding[PADLEN];     /* pad to 200 bytes */
};
typedef struct kaentry kaentry;

/*
 * Fix up SunOS brain damange.  Only works with gcc, though
 */

#ifdef __GNUC__
#if defined(__sun__) && !defined(__svr4__)
int getopt(int, char *[], char *);
extern int optind;
extern char *optarg;
#endif
#endif

/*
 * Function prototypes
 */

void db_header_output(FILE *);
void db_entry_output(FILE *, krb5_context, char *, char *, char *, int,
			    krb5_deltat, krb5_key_data *, Date);

int
main(int argc, char *argv[])
{
	krb5_error_code retval;
	krb5_realm_params *rparams;
	krb5_keysalt key_salt;
	krb5_keyblock key;
	krb5_key_data key_data;

	krb5_encrypt_block master_eblock;
	krb5_keyblock master_kblock;
	krb5_principal master_princ;
	krb5_context convert_context;

	char *stash_file = NULL;
	char *mkey_name = NULL;

	krb5_principal princ;
	int kvno = -1;
	int enctypedone = 0, i, errflag = 0;
	char *realm, *saltrealm = NULL;
	krb5_boolean prompt_master_key = FALSE;
	krb5_deltat lifetime = 0;
	struct ubik_hdr uheader;
	struct kaheader dbheader;
	struct kaentry dbentry;
	int kasdb, n, c, v4 = FALSE;

	retval = krb5_init_context(&convert_context);
	if (retval) {
		com_err(argv[0], retval, "in krb5_init_context");
		exit(1);
	}

	while((c = getopt(argc, argv, "k:l:r:m4")) != -1)
		switch (c) {
		case 'r':
			saltrealm = optarg;
			break;
		case '4':
			v4 = TRUE;
			break;
		case 'm':
			prompt_master_key = TRUE;
			break;
		case 'l':
			if ((retval = krb5_string_to_deltat( optarg,
							    &lifetime))) {
				com_err("string_to_deltat", retval,
					"when converting lifetime");
				exit(1);
			}
			break;
		case 'k':
			kvno = atoi(optarg);
			break;
		case '?':
		default:
			errflag++;
	}

	if (errflag++ || argc <= optind) {
		fprintf(stderr, "Usage: %s [-r saltrealm] "
			"[-m] [-l lifetime] [-4]"
			"AFS-db-filename [user ...]\n",
			argv[0]);
		exit(1);
	}

	memset(&key_data, 0, sizeof(krb5_key_data));

	if (!saltrealm) {
		krb5_get_default_realm(convert_context, &saltrealm);
	}

	/*
	 * Setup the realm parms, use that info to get the master key
	 *
	 * This is way more complicated than it needs to be!  Argh.
	 */

	if ((retval = krb5_read_realm_params(convert_context, NULL,
					      NULL, NULL, &rparams))) {
		com_err(argv[0], retval, "While reading realm parameters");
		exit(1);
	}

	if (rparams->realm_enctype_valid) {
		master_kblock.enctype = rparams->realm_enctype;
		enctypedone++;
	}

	if (rparams->realm_stash_file)
		stash_file = strdup(rparams->realm_stash_file);

	if (rparams->realm_mkey_name)
		mkey_name = strdup(rparams->realm_mkey_name);

	krb5_free_realm_params(convert_context, rparams);

	if (!enctypedone)
		master_kblock.enctype = DEFAULT_KDC_ENCTYPE;

	krb5_use_enctype(convert_context, &master_eblock,
			 master_kblock.enctype);

	if ((retval = krb5_get_default_realm(convert_context, &realm))) {
		com_err(argv[0], retval, "while getting default realm");
		exit(1);
	}

	if ((retval = krb5_db_setup_mkey_name(convert_context, mkey_name,
				     realm, 0, &master_princ))) {
		com_err(argv[0], retval, "while setting up master key");
		exit(1);
	}

	if ((retval = krb5_db_fetch_mkey(convert_context, master_princ,
					 &master_eblock, prompt_master_key,
					 FALSE, stash_file, 0,
					 &master_kblock))) {
		com_err(argv[0], retval, "while getting master key");
		exit(1);
	}

	if ((retval = krb5_process_key(convert_context, &master_eblock,
				       &master_kblock))) {
		com_err(argv[0], retval, "while processing master key");
		exit(1);
	}

	/*
	 * _Whew_, we've got the master key in a keyblock.  Now, we need to
	 * take the user information out of the AFS KA database files,
	 * encrypt the key information with the K5 master key, and output
	 * a dump record suitable for using with kdb5_util.
	 */

	if (v4 == TRUE) {
	  key_salt.type = KRB5_KDB_SALTTYPE_V4;
	  key_salt.data.data = (char *)NULL;
	  key_salt.data.length = 0;
	} else {
	  key_salt.type = KRB5_KDB_SALTTYPE_AFS3;
	  key_salt.data.data = saltrealm;
	  /*
	   * A magic hack used for AFS (probably not necessary at this stage)
	   */
	  key_salt.data.length = strlen(saltrealm);
	}

	/*
	 * Open up the KAS database and start getting records
	 */

	if ((kasdb = open(argv[optind], O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}

	n = read(kasdb, (char *) &uheader, sizeof(struct ubik_hdr));

	if (n != sizeof(struct ubik_hdr)) {
		if (n != -1) {
			fprintf(stderr, "Read of Ubik database header failed: "
				"only got %d of %d bytes\n", n,
				(int) sizeof(struct ubik_hdr));
		} else {
			perror("read");
		}

		exit(1);
	}
		
	NTOHL(uheader.magic);
	NTOHS(uheader.size);
	NTOHL(uheader.version.epoch);
	NTOHL(uheader.version.counter);

	if (lseek(kasdb, 64, SEEK_SET) == -1) {
		perror("lseek");
		exit(1);
	}

	n = read(kasdb, (char *) &dbheader, sizeof(struct kaheader));

	if (n != sizeof(struct kaheader)) {
		if (n != -1) {
			fprintf(stderr, "Read of KA database header failed: "
				"only got %d of %d bytes\n", n,
				(int) sizeof(struct kaheader));
		} else {
			perror("read");
		}

		exit(1);
	}

	NTOHL(dbheader.headerSize);
	NTOHL(dbheader.stats.minor_version);
	NTOHL(dbheader.stats.allocs);
	NTOHL(dbheader.stats.frees);
	NTOHL(dbheader.stats.cpws);
	
	/*
	 * Iterate over the database
	 */

	db_header_output(stdout);
	
	for (;;) {
		n = read(kasdb, (char *) &dbentry, sizeof(struct kaentry));
		if (! n) break;

		if (n != sizeof(struct kaentry)) {
			fprintf(stderr, "WARNING: Short read of %d bytes at "
				"the end of the database\n", n);
			break;
		}

		if (! *dbentry.userID.name ||
		    ntohl(dbentry.flags) & (KAFFREE | KAFOLDKEYS))
			continue;

		/*
		 * Okay, now we are getting records from the KA database.
		 * Now we have to decide what to do with them.  If we
		 * weren't given any users on the command line, then
		 * convert all of them over.
		 */

		if (argc > optind + 1) {
			for (i = optind + 1; i < argc; i++) {
				if (! strcmp(dbentry.userID.name, argv[i]))
					goto convert;
			}

			/*
			 * This user didn't match anyone on the command line,
			 * so don't output a record for him
			 */

			continue;
		}

		convert:

		/*
		 * Encrypt this key and salt with the master key
		 */

		key.enctype = ENCTYPE_DES_CBC_CRC;
		key.length = 8;
		key.contents = (krb5_octet *) dbentry.key.data;

		if ((retval = krb5_dbekd_encrypt_key_data(convert_context,
							  &master_eblock, &key,
							  (const krb5_keysalt *)
							  &key_salt,
							  1, &key_data))) {
			com_err(argv[0], retval, "during key encryption");
			exit(1);
		}

		if ((!strcmp(dbentry.userID.name, "krbtgt")) &&
		    (dbentry.userID.instance && 
		     (strcmp(dbentry.userID.instance, realm)))) 
		  {
		    db_entry_output(stdout, convert_context,
				    dbentry.userID.name, realm,
				    dbentry.userID.instance, kvno == -1 ? 
				    dbentry.key_version : kvno,
				    lifetime == 0 ? 
				    dbentry.max_ticket_lifetime :
				    lifetime, 
				    &key_data,dbentry.user_expiration);
		  }

		db_entry_output(stdout, convert_context, dbentry.userID.name,
				strlen(dbentry.userID.instance) == 0 ?
				NULL : dbentry.userID.instance, realm,
				kvno == -1 ? dbentry.key_version : kvno,
				lifetime == 0 ? dbentry.max_ticket_lifetime :
				lifetime, &key_data,dbentry.user_expiration);

	}

	exit(0);
}
