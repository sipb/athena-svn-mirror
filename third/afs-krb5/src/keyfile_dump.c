/*
 * $Id: keyfile_dump.c,v 1.1.1.1 2003-02-13 00:14:50 zacheiss Exp $
 *
 * keyfile_dump.c			Matt Crawford, FNAL
 *	Pluck the highest-kvno key from an AFS KeyFile and write a krb5
 *	load-dump record for service principal "afs[/cellname]@REALM" with
 *	that key.  This code steals lightly from Engert @ ANL and heavily
 *	from Hornstein @ NRL.
 */

#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <fcntl.h>
#include <krb5.h>
#include <com_err.h>
#include <k5-int.h>
#include <adm.h>

#ifndef LINT
static char rcsid[]=
	"$Id: keyfile_dump.c,v 1.1.1.1 2003-02-13 00:14:50 zacheiss Exp $";
#endif

/* following structure copied from Engert's k5105.cdiffp.980223 */
struct afs_key_entry {
    krb5_int32 kvno;
    krb5_octet key[8];
} keyent, keyent1;
/* end of theft #1 */

/*
 * theft #2 consists of interspersed blocks of code
 * ripped from Hornstein's afs2k5db.c
 */
static void db_header_output(FILE *);
static void db_entry_output(FILE *, char *, char *, char *,
			    int, krb5_deltat, krb5_key_data *, u_int);

char usage[] =
"%s [-m] [-l lifetime] [-c cellname] keyfile\n\
  Writes to stdout AFS service keys from named keyfile as Kerberos 5 keys\n\
  for \"afs/cellname@REALM\" in \"kdb5_util load_dump version 4\" format.\n\
  Cellname defaults to local realm in lower case, in which case it is\n\
  omitted from the principal name, in line with aklog's expectation.\n";

krb5_encrypt_block master_eblock;
krb5_keyblock master_kblock;
krb5_principal master_princ;
krb5_context convert_context;

char *stash_file = NULL;
char *mkey_name = NULL;

main(int argc, char **argv)
{
  int			fd, nkeys, kvno, c;
  int			errflag = 0, enctypedone = 0;
  char			*cellname = NULL, *realm;
  krb5_error_code	retval;
  krb5_realm_params	*rparams;
  krb5_boolean		prompt_master_key = FALSE;
  krb5_deltat		lifetime = 0;
  krb5_keyblock		key;
  krb5_key_data		key_data;

  retval = krb5_init_context(&convert_context);
  if (retval) {
    com_err(argv[0], retval, "in krb5_init_context");
    exit(1);
  }

  while( (c = getopt(argc, argv, "ml:c:")) != -1 )
    switch (c) {
    case 'c':
      cellname = optarg;
      break;
    case 'l':
      if ( (retval = krb5_string_to_deltat(optarg, &lifetime)) ) {
	com_err("string_to_deltat", retval, "when converting lifetime");
	exit(1);
      }
      break;
    case 'm':
      prompt_master_key = 1;
      break;
    case '?':
    default:
      errflag = 1;
    }
  if ( errflag || argc != optind+1 ) {
    fprintf(stderr, usage, argv[0]);
    exit(1);
  }

  memset(&key_data, 0, sizeof key_data);
  if ( (retval = krb5_get_default_realm(convert_context, &realm)) ) {
    com_err(argv[0], retval, "while getting default realm");
    exit(1);
  }

  /* major part of theft #2 follows */
  /*
   * Setup the realm parms, use that info to get the master key
   *
   * This is way more complicated than it needs to be!  Argh.
   */
  if ( (retval = krb5_read_realm_params(convert_context, NULL,
					NULL, NULL, &rparams)) ) {
    com_err(argv[0], retval, "While reading realm parameters");
    exit(1);
  }
  if ( rparams->realm_enctype_valid ) {
    master_kblock.enctype = rparams->realm_enctype;
    enctypedone++;
  }
  if ( !lifetime )
    if ( rparams->realm_max_life_valid )
      lifetime = rparams->realm_max_life;
    else
      lifetime = KRB5_KDB_MAX_LIFE;
  if ( rparams->realm_stash_file )
    stash_file = strdup(rparams->realm_stash_file);
  if ( rparams->realm_mkey_name )
    mkey_name = strdup(rparams->realm_mkey_name);
  krb5_free_realm_params(convert_context, rparams);

  if ( !enctypedone )
    master_kblock.enctype = DEFAULT_KDC_ENCTYPE;

  krb5_use_enctype(convert_context, &master_eblock, master_kblock.enctype);

  if ( (retval = krb5_db_setup_mkey_name(convert_context, mkey_name,
					 realm, 0, &master_princ)) ) {
    com_err(argv[0], retval, "while setting up master key");
    exit(1);
  }

  if ( (retval = krb5_db_fetch_mkey(convert_context, master_princ,
				    &master_eblock, prompt_master_key,
				    FALSE, stash_file, 0, &master_kblock)) ) {
    com_err(argv[0], retval, "while getting master key");
    exit(1);
  }

  if ( (retval = krb5_process_key(convert_context, &master_eblock,
				  &master_kblock)) ) {
    com_err(argv[0], retval, "while processing master key");
    exit(1);
  }
  /* end of this egregious theft */

  /* suppress cellname (as instance of afs princ.) if same as realm */
  if ( cellname && !strcasecmp(realm, cellname) )
    cellname = NULL;

  if ( (fd = open(argv[optind], O_RDONLY)) < 0 ) {
    perror("opening keyfile");
    exit(1);
  }
  if ( read(fd, &nkeys, sizeof nkeys) != sizeof nkeys ) {
    fputs("Short read on nkeys\n", stderr);
    exit(1);
  }
  nkeys = ntohl(nkeys);
  for ( keyent.kvno = -1; nkeys--; ) {
    if ( read(fd, &keyent1, sizeof keyent1) != sizeof keyent1 ) {
      fputs("Short read on keyent\n", stderr);
      exit(1);
    }
    keyent1.kvno = ntohl(keyent1.kvno);
    if ( keyent1.kvno > keyent.kvno )
      memcpy(&keyent, &keyent1, sizeof keyent);
  }
    
  db_header_output(stdout);
  /*
   * Encrypt this key with the master key
   */

  key.enctype = ENCTYPE_DES_CBC_CRC;
  key.length = 8;
  key.contents = keyent.key;

  if ( (retval =
	krb5_dbekd_encrypt_key_data(convert_context, &master_eblock, &key,
				    NULL, keyent.kvno, &key_data)) ) {
    com_err(argv[0], retval, "during key encryption");
    exit(1);
  }

  db_entry_output(stdout, "afs", cellname, realm, keyent.kvno, 
		  lifetime, &key_data, 0);
  exit(0);
}

/* Last big theft */
/*
 * Output the header used by the database (in this case, a Kerberos V
 * version 4 dump file)
 */

static void
db_header_output(FILE *f)
{
	fprintf(f, "kdb5_util load_dump version 4\n");
}

/*
 * Output one record in the format used by the database dump file
 */

static void
db_entry_output(FILE *f, char *user, char *instance, char *realm, int kvno,
		krb5_deltat lifetime, krb5_key_data *key_data,
		u_int expiration)
{
	krb5_principal princ;
	krb5_error_code retval;
	krb5_timestamp time;
	unsigned char moddata[4];
	char *name = NULL, *mname;
	int i, j;


/*
 * Output the stuff we need in the entry.  First, a "princ" marker
 * to identify this record as a principal, and a "base length"
 */

	fprintf(f, "princ\t%d\t", KRB5_KDB_V1_BASE_LENGTH);

/*
 * Get a string version of the principal (this is a bit wacky, but it works)
 */

	if ((retval = krb5_build_principal(convert_context, &princ,
					   strlen(realm), realm, user,
					   instance, 0))) {
		com_err("db_entry_output", retval, "while bulding principal");
		exit(1);
	}

	if ((retval = krb5_unparse_name(convert_context, princ, &name))) {
		com_err("db_entry_output", retval, "while unparsing name");
		exit(1);
	}

	fprintf(f, "%d\t", (int) strlen(name));

/*
 * These are the number of "tl_data" (one, the master principal + expire time),
 * the number of key data (only one, the AFS key), and the length of the
 * extra data (0)
 */

	fprintf(f, "1\t1\t0\t");

/*
 * The principal itself
 */

	fprintf(f, "%s\t", name);

/*
 * Attributes (none), maximum life (variable), maximum rewnewable life (7 days)
 * expiration (epoch), password expiration (never), last success (never),
 * last failure (never), failed authorization count (none)
 */

	fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", 0, (int) lifetime,
		60*60*24*7, (expiration != -1) ? expiration : 2145830400, 0, 0, 0, 0);

/*
 * Put a MOD PRINC entry (although I'm not quite sure what this is for)
 */

	if ((retval= krb5_timeofday(convert_context, &time))) {
		com_err("db_entry_output", retval, "while getting time of day");
		exit(1);
	}
	if ((retval = krb5_unparse_name(convert_context, princ, &mname))) {
		com_err("db_entry_output", retval, "while unparsing a name");
		exit(1);
	}

	krb5_kdb_encode_int32(time, moddata);

	fprintf(f, "%d\t%d\t", KRB5_TL_MOD_PRINC, (int) strlen(mname) + 5);

	for (i = 0; i < 4; i++)
		fprintf(f, "%02x", moddata[i]);
	for (i = 0; i <= strlen(mname); i++)
		fprintf(f, "%02x", (unsigned char) mname[i]);

	fprintf(f, "\t");

/*
 * Now, output the key data.  Output two entries - one for the key, one
 * for the salt (the salt is needed for AFS).  Note that we don't output
 * the salt if one isn't included (for example, if you're doing an AFS
 * service key)
 */

	fprintf(f, "%d\t%d\t", key_data->key_data_ver, kvno);

	for (i = 0; i < key_data->key_data_ver; i++) {
		fprintf(f, "%d\t%d\t", key_data->key_data_type[i],
			key_data->key_data_length[i]);
		for (j = 0; j < key_data->key_data_length[i]; j++)
			fprintf(f, "%02x", key_data->key_data_contents[i][j]);
		fprintf(f, "\t");
	}

/*
 * We're all done now
 */

	fprintf(f, "-1;\n");

	krb5_free_principal(convert_context, princ);
	krb5_xfree(name);
	krb5_xfree(mname);

}

