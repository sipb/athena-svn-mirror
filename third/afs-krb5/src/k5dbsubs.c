/*
 * $Id: k5dbsubs.c,v 1.1.1.1 2003-02-13 00:14:49 zacheiss Exp $
 *
 * Convert a AFS KA database to a Kerberos V5 (1.0) dump file
 *
 * Written by Ken Hornstein <kenh@cmf.nrl.navy.mil>
 *
 * The KAS database reading code was contributed by Derrick J Brashear
 * <shadow@dementia.org>, and most of it was originally written by
 * Dan Lovinger of Microsoft.
 *
 */

#ifndef LINT
static char rcsid[]=
	"$Id: k5dbsubs.c,v 1.1.1.1 2003-02-13 00:14:49 zacheiss Exp $";
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

#include <afs/kauth.h>

/*
 * Output the header used by the database (in this case, a Kerberos V
 * version 4 dump file
 */

void
db_header_output(FILE *f)
{
	fprintf(f, "kdb5_util load_dump version 4\n");
}

/*
 * Output one record in the format used by the database dump file
 */

void
db_entry_output(FILE *f, krb5_context convert_context, char *user,
		char *instance, char *realm, int kvno,
		krb5_deltat lifetime, krb5_key_data *key_data, Date expiration)
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

	if ((retval = krb5_425_conv_principal(convert_context, 
					      user, instance,
					      realm, &princ))) 
	  {
	    com_err("db_entry_output", retval, "during principal conversion");
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
		if (key_data->key_data_length[i])
			for (j = 0; j < key_data->key_data_length[i]; j++)
				fprintf(f, "%02x",
					key_data->key_data_contents[i][j]);
		else
			fprintf(f, "%d", -1);
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
