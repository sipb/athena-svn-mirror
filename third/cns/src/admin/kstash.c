/*
 * kstash.c
 *
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Description.
 */

#include <mit-copyright.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#include <krb.h>
#include <krb_db.h>
#include <kdc.h>

/* change this later, but krblib_dbm needs it for now */
char   *progname;

static C_Block master_key;
static Key_schedule master_key_schedule;
int     debug;
static int kfile;
static void clear_secrets();
char * progname;

usage()
{
    fprintf(stderr, "Usage: %s [-d database name] [-k master key file]\n",
	    progname);
    exit(1);
}

void
main(argc, argv)
    int     argc;
    char  **argv;
{
    long    n;
    char *stashfile = 0;
    int c;
    extern char *optarg;
    extern int optind;

    progname = argv[0];

    while ((c = getopt(argc, argv, "d:k:")) != EOF) {
	switch (c) {
	case 'd':
	    if (kerb_db_set_name(optarg) != 0) {
		fprintf(stderr, "Couldn't set alternate database name (%s)\n",
			optarg);
		exit(1);
	    }
	    break;
	case 'k':
	    stashfile = optarg;
	    break;
	default:
	    usage();
	}
    }

    if (optind != argc)
	usage();

    n = kerb_init();
    if (n) {
	fprintf(stderr, "Kerberos db and cache init failed = %ld\n", n);
	exit(1);
    }

    if (kdb_get_master_key_from (TRUE, master_key, master_key_schedule, 0, stashfile) != 0) {
      fprintf (stderr, "%s: Couldn't read master key.\n", argv[0]);
      fflush (stderr);
      clear_secrets();
      exit (-1);
    }

    if (kdb_verify_master_key (master_key, master_key_schedule, stderr) < 0) {
      clear_secrets();
      exit (-1);
    }

    kfile = open(stashfile?stashfile:MKEYFILE, O_TRUNC | O_RDWR | O_CREAT, 0600);
    if (kfile < 0) {
	clear_secrets();
	fprintf(stderr, "\n\07\07%s: Unable to open master key file\n",
		argv[0]);
	exit(1);
    }
    if (write(kfile, (char *) master_key, 8) < 0) {
	clear_secrets();
	fprintf(stderr, "\n%s: Write I/O error on master key file\n",
		argv[0]);
	exit(1);
    }
    (void) close(kfile);
    clear_secrets();
    exit (0);
}

static void 
clear_secrets()
{
    memset(master_key_schedule, 0, sizeof(master_key_schedule));
    memset(master_key, 0, sizeof(master_key));
}
