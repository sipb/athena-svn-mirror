/*
 * $Id: main.c,v 1.34.2.1 1998-09-24 14:18:59 ghudson Exp $
 *
 * Copyright (c) 1988,1992 by the Massachusetts Institute of Technology.
 */

static char *rcsid_main_c = "$Id: main.c,v 1.34.2.1 1998-09-24 14:18:59 ghudson Exp $";

#include "attach.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <athdir.h>

int verbose = 1, debug_flag = 0, map_anyway = 1, do_nfsid = 1;
int print_path = 0, explicit = 0, owner_check = 0, override = 0;
int owner_list = 1, clean_detach = 0, exp_allow = 1, exp_mntpt = 1;

/* real userid of proces, effective userid of process, userid used
   for attachtab manipulation */
int real_uid, effective_uid, owner_uid;

int lock_filesystem = 0;
int nfs_root_hack = 1;		/* By default, translate for the */
				/* default mountpoint / as /root */

int keep_mount = 0;		/* By default, let mounted filesystems */
				/* that are not in attachtab get */
				/* unmounted when that filesystem gets */
				/* detached. */
int error_status = ERR_NONE;
int force = 0;
#ifdef ZEPHYR
int use_zephyr = 1;
#endif /* ZEPHYR */
char	override_mode, *mount_options, *filsys_type;
char 	*mntpt;
int 	override_suid, default_suid, skip_fsck, lookup;
char	*spoofhost, *attachtab_fn = NULL, *mtab_fn = NULL;
char	*fsck_fn = NULL, *aklog_fn = NULL;
char	*nfs_mount_dir = NULL, *afs_mount_dir = NULL;
char	*progname;

#ifdef NFS
int	nfs_attach(), nfs_detach();
char	**nfs_explicit();
#endif
#ifdef RVD
int	rvd_attach(), rvd_detach();
char	**rvd_explicit();
#endif
#ifdef AFS
int	afs_attach(), afs_detach();
char	**afs_explicit();
#endif
#ifdef UFS
int ufs_attach(), ufs_detach();
#endif
int err_attach(), null_detach();
char	**ufs_explicit();
int mul_attach(), mul_detach();

struct _fstypes fstypes[] = {
    { "---", 0, -1, 0, (char *) 0, 0, null_detach, 0 },	/* The null type */
#ifdef NFS
    { "NFS", TYPE_NFS, MOUNT_NFS,
	  AT_FS_MNTPT | AT_FS_REMOTE | AT_FS_MNTPT_CANON,
	  "rwnm", nfs_attach, nfs_detach, nfs_explicit },
#endif
#ifdef RVD
    { "RVD", TYPE_RVD, MOUNT_UFS,
	  AT_FS_MNTPT | AT_FS_REMOTE | AT_FS_MNTPT_CANON,
	  "rw", rvd_attach, rvd_detach, rvd_explicit },
#endif
#ifdef UFS
    { "UFS", TYPE_UFS, MOUNT_UFS,
	  AT_FS_MNTPT | AT_FS_MNTPT_CANON,
	  "rw", ufs_attach, ufs_detach, ufs_explicit },
#endif
#ifdef AFS
    { "AFS", TYPE_AFS, -1,
	  AT_FS_MNTPT | AT_FS_PARENTMNTPT,
	  "nrw", afs_attach, afs_detach, afs_explicit },
#endif
    { "ERR", TYPE_ERR, -1, 0, (char *) 0, err_attach, 0, 0 },
    { "MUL", TYPE_MUL, -1, 0, "-", mul_attach, mul_detach, 0 },
    { 0, -1, 0, 0, (char *) 0, 0, 0, 0 }
};

/*
 * Primary entry point -- look at argv[0] to figure out who we are and
 * dispatch to the proper handler accordingly.
 */

main(argc, argv)
    int argc;
    char *argv[];
{
    char	*ptr;
    extern sig_catch	sig_trap();
#ifdef POSIX
    struct sigaction sig;
#endif

    /* Stop overriding my explicit file modes! */
    umask(022);

    /* Install signal handlers */
#ifdef POSIX
    sig.sa_handler = sig_trap;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGHUP, &sig, NULL);
#else
    (void) signal (SIGTERM, sig_trap);
    (void) signal (SIGINT, sig_trap);
    (void) signal (SIGHUP, sig_trap);
#endif

    real_uid = owner_uid = getuid();
    effective_uid = geteuid();

    progname = argv[0];
    ptr = strrchr(progname, '/');
    if (ptr)
	progname = ptr+1;

    attachtab_fn = strdup(ATTACHTAB);
    mtab_fn = strdup(MTAB);
#ifdef AFS
    aklog_fn = strdup(AKLOG_FULLNAME);
    afs_mount_dir = strdup(AFS_MOUNT_DIR);
#endif
    fsck_fn = strdup(FSCK_FULLNAME);

    /*
     * User can explicitly specify attach, detach, nfsid, fsid, zinit,
     * add, or attachandrun by using the -P option as the first command
     * option.
     */
    
    if (argv[1] && !strncmp(argv[1], "-P", 2)) {
	    progname = argv[1]+2;
	    if (*progname) {
		    argv += 1;
		    argc -= 1;
	    } else {
		    if (progname = argv[2]) {
			    argv += 2;
			    argc -= 2;
		    } else {
			    fprintf(stderr,
				    "Must specify attach, detach, nfsid, fsid, zinit, add, or attachandrun!\n");
			    exit(ERR_BADARGS);
		    } 
	    } 
    }

    if (!strcmp(progname, ATTACH_CMD))
	exit(attachcmd(argc, argv, NULL));
    if (!strcmp(progname, DETACH_CMD))
	exit(detachcmd(argc, argv));
#ifdef KERBEROS
#ifdef NFS
    if (!strcmp(progname, NFSID_CMD))
    {
	filsys_type = "NFS";
	exit(fsidcmd(argc, argv));
    }
#endif
    if (!strcmp(progname, FSID_CMD))
	exit(fsidcmd(argc, argv));
#endif
#ifdef ZEPHYR
    if (!strcmp(progname, ZINIT_CMD))
	exit(zinitcmd(argc, argv));
#endif
    if (!strcmp(progname, ADD_CMD))
	exit(addcmd(argc, argv));
    if (!strcmp(progname, RUN_CMD))
	exit(attachandruncmd(argc, argv));

    fprintf(stderr, "Not invoked with attach, detach, nfsid, fsid, zinit, add, or attachandrun!\n");
    exit(ERR_BADARGS);
}

#ifdef KERBEROS
/*
 * Command handler for nfsid.
 */
fsidcmd(argc, argv)
    int argc;
    char *argv[];
{
    extern struct _attachtab	*attachtab_first;
    int gotname, i, op, filsysp, hostp;
#ifdef AFS
    int	cell_sw;
#endif
    char *ops;
    struct hostent *hent;
    char hostname[BUFSIZ];
    register struct _attachtab	*atp;
    struct in_addr addr;
    static struct command_list options[] = {
	{ "-verbose", "-v" },
	{ "-quiet", "-q" },
	{ "-debug", "-d" },
	{ "-map", "-m" },
	{ "-unmap", "-u" },
	{ "-purge", "-p" },
	{ "-purgeuser", "-r" },
	{ "-spoofhost", "-s" },
	{ "-filsys", "-f" },
	{ "-host", "-h" },
#ifdef AFS
	{ "-cell", "-c" },
#endif
	{ "-user", "-U" },
	{ "-all", "-a" },
	{ 0, 0 }};
	
    check_root_privs(progname);
    read_config_file(ATTACHCONFFILE);
    
    gotname = 0;
    filsysp = 1;
    hostp = 0;
    verbose = 1;
#ifdef AFS
    cell_sw = 0;
#endif
    error_status = ERR_NONE;
    
    op = MOUNTPROC_KUIDMAP;
    ops = "mapped";

#ifdef ATHENA_COMPAT73
    if (filsys_type && !strcmp(filsys_type, "NFS")) {
	hostp = 1;
	filsysp = 0;
    } else {
	hostp = filsysp = 1;
    }
#endif

    for (i=1;i<argc;i++) {
	if (*argv[i] == '-') {
	    switch (internal_getopt(argv[i], options)) {
	    case 'v':
		verbose = 1;
		break;
	    case 'q':
		verbose = 0;
		break;
	    case 'd':
		debug_flag = 1;
		break;
	    case 'm':
		op = MOUNTPROC_KUIDMAP;
		ops = "mapped";
		break;
	    case 'u':
		op = MOUNTPROC_KUIDUMAP;
		ops = "unmapped";
		break;
	    case 'r':
		op = MOUNTPROC_KUIDUPURGE;
		ops = "mappings user-purged";
		break;
	    case 'p':
		if (!trusted_user(real_uid)) {
		    fprintf(stderr,
			    "%s: nfsid purge is a privileged operation\n",
			    progname);
		    return ERR_NFSIDPERM;
		}
		op = MOUNTPROC_KUIDPURGE;
		ops = "mappings purged";
		break;
#ifdef AFS
	    case 'c':
		filsysp = 0;
		cell_sw = 1;
		break;
#endif
	    case 'f':
		filsysp = 1;
		hostp = 0;
		break;
	    case 'h':
		filsysp = 0;
		hostp = 1;
		break;
	    case 'U':
		++i;
		if (trusted_user(real_uid)) {
			if (!argv[i]) {
				fprintf(stderr, "%s: Username required with -U.\n",
					progname);
				return ERR_BADARGS;
			}
			owner_uid = parse_username(argv[i]);
		} else {
			fprintf(stderr,
		"%s: You are not authorized to use the -user option\n", progname);
		}
		break;
	    case 's':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No spoof host specified\n", progname);
		    return (ERR_BADARGS);
		}
		spoofhost = argv[++i];
		break;
	    case 'a':
		/*
		 * Read the attachtab one entry at a time, and perform the
		 * operation on each host found therein.  Note this
		 * will even include hosts that are associated with
		 * filesystems in the process of being attached or
		 * detached.  Assume that the -a option implies more
		 * than one file for the exit status computation.
		 */
		lock_attachtab();
		get_attachtab();
		unlock_attachtab();
		atp = attachtab_first;
		while (atp) {
			if (atp->fs->type == TYPE_MUL) {
				/* Do nothing
				 * Type MUL filesystems are authenticated
				 * by their individual parts.
				 */
			} else if ((op == MOUNTPROC_KUIDMAP ||
				    op == MOUNTPROC_KUIDUMAP) &&
				   !is_an_owner(atp,owner_uid)) {
				/* Do nothing
				 * Only map/unmap to filesystems that the
				 * user has attached.
				 *
				 * Purges apply to ALL attached filesystems.
				 */
			} else if (atp->fs->type == TYPE_NFS) {
				if ((op == MOUNTPROC_KUIDMAP ||
				     op == MOUNTPROC_KUIDUMAP) &&
				    atp->mode == 'n') {
					/* Do nothing */
				} else if (nfsid(atp->host, atp->hostaddr[0],
						 op, 1, atp->hesiodname, 0,
						 owner_uid) == SUCCESS &&
					   verbose)
					printf("%s: %s %s\n", progname,
					       atp->hesiodname, ops);
#ifdef AFS
			} else if (atp->fs->type == TYPE_AFS &&
				   op == MOUNTPROC_KUIDMAP) {
				/* We only support map operations on AFS */
				if (atp->mode != 'n' &&
				    (afs_auth(atp->hesiodname, atp->hostdir)
				     == SUCCESS) && verbose)
					printf("%s: %s %s\n", progname,
					       atp->hesiodname, ops);
#endif
			} else
				if (verbose)
				    printf("%s: %s ignored (operation not supported on %s)\n",
					   progname, atp->hesiodname,
					   atp->fs->name);
			atp = atp->next;
		}
		free_attachtab();
		gotname = 2;
		break;
	    default:
		fprintf(stderr, "%s: Unknown switch %s\n", progname, argv[i]);
		return (ERR_BADARGS);
	    }
	    continue;
	}
	gotname++;

#ifdef AFS
	if (cell_sw) {
	    afs_auth_to_cell(argv[i]);
	    continue;
	}
#endif

	if (filsysp) {
	    lock_attachtab();
	    get_attachtab();
	    unlock_attachtab();
	    /*
	     * Lookup the specified filsys name and perform an nfsid
	     * on the host associated with it.
	     */
	    if (atp = attachtab_lookup(argv[i])) {
		if (atp->fs->type == TYPE_MUL)
		    gotname = 2;
		fsid_filsys(atp, op, ops, argv[i], owner_uid);
		continue;
	    } else if (!hostp) {
		error_status = ERR_NFSIDNOTATTACHED;
		fprintf(stderr, "%s: %s not attached\n", progname, argv[i]);
	    }
	}

	if (hostp) {
	    /*
	     * Perform an nfsid on the specified host.
	     */
	    hent = gethostbyname(argv[i]);
	    if (!hent) {
		fprintf(stderr, "%s: Can't resolve %s\n", progname, argv[i]);
		error_status = ERR_NFSIDBADHOST;
	    }
	    else {
		strcpy(hostname, hent->h_name);
#ifdef POSIX
		memmove(&addr, hent->h_addr_list[0], 4);
#else
		bcopy(hent->h_addr_list[0], &addr, 4);
#endif
		if ((nfsid(hostname, addr,
			   op, 1, "nfsid", 0, owner_uid) == SUCCESS) && verbose)
		    printf("%s: %s %s\n", progname, hostname, ops);
	    }
	} 
    }

    if (gotname == 1)
	return (error_status);
    if (gotname > 1)
	return (error_status ? ERR_SOMETHING : ERR_NONE);

    fprintf(stderr, "Usage: nfsid [options] [host host ...] or [filsys filsys ...]\n");
    return (ERR_BADARGS);
}
#endif

fsid_filsys(atp, op, ops, filsys, uid)
struct _attachtab *atp;
int op;
char *ops;
char *filsys;
int uid;
{
	char mul_buf[BUFSIZ], *cp;
	
	if (atp->fs->type == TYPE_MUL) {
		strcpy(mul_buf, atp->hostdir);
		cp = strtok(mul_buf, ",");
		while (cp) {
			atp = attachtab_lookup(cp);
			if (atp)
				fsid_filsys(atp,op,ops,atp->hesiodname,uid);
			cp = strtok(NULL, ",");
		}
#ifdef NFS
	} else if (atp->fs->type == TYPE_NFS) {
		if ((nfsid(atp->host, atp->hostaddr[0], op, 1,
			   filsys, 0, owner_uid) == SUCCESS) &&
		    verbose)
			printf("%s: %s %s\n", progname, filsys, ops);
#endif
#ifdef AFS
	} else if (atp->fs->type == TYPE_AFS) {
		if (op == MOUNTPROC_KUIDMAP &&
		    (afs_auth(atp->hesiodname, atp->hostdir) == SUCCESS)
		    && verbose)
			printf("%s: %s %s\n", progname, filsys, ops);
#endif
	}
}

attachcmd(argc, argv, mountpoint_list)
    int argc;
    char *argv[];
    string_list **mountpoint_list;
{
    int gotname, i;
    int print_host = 0;

    static struct command_list options[] = {
	{ "-verbose", "-v" },
	{ "-quiet", "-q" },
	{ "-force", "-f" },
	{ "-printpath", "-p" },
	{ "-lookup", "-l" },
	{ "-debug", "-d" },
	{ "-map", "-y" },
	{ "-nomap", "-n" },
	{ "-remap", "-g" },
	{ "-noremap", "-a" },
#ifdef ZEPHYR
	{ "-zephyr", "-z" },
	{ "-nozephyr", "-h" },
#endif /* ZEPHYR */
	{ "-readonly", "-r" },
	{ "-write", "-w" },
	{ "-mountpoint", "-m" },
	{ "-noexplicit", "-x" },
	{ "-explicit", "-e" },
	{ "-type", "-t" },
	{ "-mountoptions", "-o" },
	{ "-spoofhost", "-s" },
	{ "-nosetuid", "-N" },
	{ "-setuid", "-S" },
	{ "-nosuid", "-N" },
	{ "-suid", "-S" },
	{ "-override", "-O" },
	{ "-skipfsck", "-F" },
	{ "-lock", "-L" },
	{ "-user", "-U" },
	{ "-host", "-H" },
	{ 0, 0 }};

    read_config_file(ATTACHCONFFILE);

    check_root_privs(progname);
    default_suid = 0;
    
    gotname = 0;

    verbose = 1;
    explicit = 0;
    force = 0;
    lookup = 0;
    mntpt = (char *)NULL;
    override_mode = '\0';
    override_suid = -1;		/* -1 means use default */
    mount_options = "";
    error_status = ERR_NONE;
    map_anyway = 1;

    if (argc == 1)
	return (attach_print(0));
    
    for (i=1;i<argc;i++) {
	if (*argv[i] == '-') {
	    switch (internal_getopt(argv[i], options)) {
	    case 'v':
		verbose = 1;
		print_path = 0;
		break;
	    case 'l':
		lookup = 1;
		break;
	    case 'q':
		verbose = 0;
		break;
	    case 'd':
		debug_flag = 1;
		break;
	    case 'y':
		do_nfsid = 1;
		break;
	    case 'n':
		do_nfsid = 0;
		map_anyway = 0;
		break;
	    case 'p':
		print_path = 1;
		verbose = 0;
		break;
	    case 'm':
		if (i == argc-1) {
			fprintf(stderr, "%s: No mount point specified\n", 
				progname);
			return (ERR_BADARGS);
		}
		if (exp_mntpt || trusted_user(real_uid)) {
			mntpt = argv[++i];
		} else {
			fprintf(stderr,
		"%s: You are not allowed to use the -mountpoint option\n",
				progname);
			i++;
		}
		break;
	    case 'r':
		override_mode = 'r';
		break;
	    case 'w':
		override_mode = 'w';
		break;
	    case 'f':
		force = 1;
		break;
	    case 'g':
		map_anyway = 1;
		break;
	    case 'a':
		map_anyway = 0;
		break;
#ifdef ZEPHYR
	    case 'z':
		use_zephyr = 1;
		break;
	    case 'h':
		use_zephyr = 0;
		break;
#endif /* ZEPHYR */
	    case 'x':
		explicit = 0;
		break;
	    case 'e':
		if (exp_allow || trusted_user(real_uid))
			explicit = 1;
		else
			fprintf(stderr,
		"%s: You are not allowed to use the -explicit option\n", progname);
		break;
	    case 't':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No filesystem type specified\n", progname);
		    return (ERR_BADARGS);
		}
		filsys_type = argv[++i];
		break;
	    case 'o':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No mount options specified\n", progname);
		    return (ERR_BADARGS);
		}
		mount_options = argv[++i];
		break;
	    case 's':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No spoof host specified\n", progname);
		    return (ERR_BADARGS);
		}
		spoofhost = argv[++i];
		break;
	    case 'N':
		override_suid = 0;
		break;
	    case 'S':
		if (trusted_user(real_uid))
			override_suid = 1;
		else {
			fprintf(stderr,

		"%s: You are not authorized to the -setuid option\n", progname);
		}
		break;
	    case 'O':
		if (trusted_user(real_uid))
			override = 1;
		else {
			fprintf(stderr, 
		"%s: You are not authorized to use -override option\n", progname);
		}
		break;
	    case 'L':
		if (trusted_user(real_uid))
			lock_filesystem = 1;
		else {
			fprintf(stderr,
		"%s: You are not authorized to use -lock option\n", progname);
		}
		break;
	    case 'F':
		skip_fsck = 1;
		break;
   	    case 'U':
		++i;
		if (trusted_user(real_uid)) {
			if (argv[i])
				owner_uid = parse_username(argv[i]);
		} else {
			fprintf(stderr,
		"You are not authorized to use the -user option\n", progname);
		}
		break;
	    case 'H':
		print_host++;
		break;
	    default:
		fprintf(stderr, "%s: Unknown switch %s\n", progname, argv[i]);
		return (ERR_BADARGS);		
	    }
	    continue;
	}
	gotname++;

	if (print_host)
		attach_print(argv[i]);
	else if (attach(argv[i], mountpoint_list) == SUCCESS)
		error_status = 0;

	override_mode = '\0';
	override_suid = -1;
	override = 0;
	lock_filesystem = 0;
	mntpt = (char *)NULL;
    }

    /* Flush Zephyr subscriptions */
#ifdef ZEPHYR
    zephyr_sub(0);
#endif /* ZEPHYR */

    if (gotname == 1)
	return (error_status);
    if (gotname > 1)
	return (error_status ? ERR_SOMETHING : ERR_NONE);

    fprintf(stderr,
	    "Usage: attach [options] filesystem [options] filesystem ...\n");
    return (ERR_BADARGS);
}

detachcmd(argc, argv)
    int argc;
    char *argv[];
{
    int gotname, i;
    int	dohost;
    static struct command_list options[] = {
	{ "-verbose", "-v" },
	{ "-quiet", "-q" },
	{ "-all", "-a" },
	{ "-debug", "-d" },
	{ "-unmap", "-y" },
	{ "-nomap", "-n" },
#ifdef ZEPHYR
	{ "-zephyr", "-z" },
	{ "-nozephyr", "-h" },
#endif /* ZEPHYR */
	{ "-type", "-t" },
	{ "-explicit", "-e" },
	{ "-noexplicit", "-x" },
	{ "-force", "-f" },
	{ "-spoofhost", "-s" },
	{ "-override", "-O" },
	{ "-host", "-H" },
	{ "-user", "-U" },
	{ "-clean", "-C" },
	{ "-lint", "-L" },
	{ 0, 0}};

    check_root_privs(progname);
    read_config_file(ATTACHCONFFILE);
    
    gotname = 0;

    verbose = 1;
    explicit = 0;
    force = 0;
    override = 0;
    dohost = 0;
    error_status = ERR_NONE;
    filsys_type = NULL;
    
    for (i=1;i<argc;i++) {
	if (*argv[i] == '-') {
	    switch (internal_getopt(argv[i], options)) {
	    case 'v':
		verbose = 1;
		break;
	    case 'q':
		verbose = 0;
		break;
	    case 'd':
		debug_flag = 1;
		break;
	    case 'y':
		do_nfsid = 1;
		break;
	    case 'n':
		do_nfsid = 0;
		break;
#ifdef ZEPHYR
	    case 'z':
		use_zephyr = 1;
		break;
	    case 'h':
		use_zephyr = 0;
		break;
#endif /* ZEPHYR */
 	    case 'H':
		dohost = 1;
		break;
	    case 'f':
		force = 1;
		break;
	    case 'x':
		explicit = 0;
		break;
	    case 'e':
		explicit = 1;
		break;
	    case 't':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No filesystem type specified\n", 
			    progname);
		    return (ERR_BADARGS);
		}
		filsys_type = argv[++i];
		break;
	    case 'a':
		detach_all();
		gotname = 2;
		break;
	    case 's':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No spoof host specified\n", progname);
		    return (ERR_BADARGS);
		}
		spoofhost = argv[++i];
		break;
	    case 'O':
		if (trusted_user(real_uid))
			override = 1;
		else {
			fprintf(stderr,
		"%s: You are not authorized to use -override option\n", progname);
		}
		break;
   	    case 'U':
		++i;
		if (trusted_user(real_uid)) {
			if (!argv[i]) {
				fprintf(stderr, "%s: Username required with -U.\n",
					progname);
				return (ERR_BADARGS);
			}
			owner_uid = parse_username(argv[i]);
		} else {
			fprintf(stderr,
		"%s: You are not authorized to use the -user option\n", progname);
		}
		break;
	case 'C':
		clean_detach = 1;
		break;
	case 'L':
		if (!trusted_user(real_uid)) {
		    fprintf(stderr,
			    "%s: You are not authorized to use the -lint option\n",
			    progname);
		    return(ERR_BADARGS);
		} else {
		    lint_attachtab();
		    gotname = 1;
		}
		break;
	default:
		fprintf(stderr, "%s: Unknown switch %s\n", progname, argv[i]);
		return (ERR_BADARGS);
	    }
	    continue;
	}
	gotname++;
	if (dohost)
		detach_host(argv[i]);
	else
		detach(argv[i]);
	dohost = 0;
    }

    /* Flush Zephyr unsubscriptions */
#ifdef ZEPHYR
    zephyr_unsub(0);
#endif /* ZEPHYR */
    
    if (gotname == 1)
	return (error_status);
    if (gotname > 1)
	return (error_status ? ERR_SOMETHING : ERR_NONE);

    fprintf(stderr,
	    "Usage: detach [options] filesystem [options] filesystem ...\n");
    return (ERR_BADARGS);
}

#ifdef ZEPHYR
zinitcmd(argc, argv)
	int	argc;
	char	**argv;
{
	extern struct _attachtab *attachtab_first;
	char instbfr[BUFSIZ];
	struct _attachtab *p;
	int	i;
#define	USER_ONLY	0
#define	ROOT_TOO	1
#define	ALL_USERS	2
	int	who = ROOT_TOO;

	static struct command_list options[] = {
		{ "-verbose", "-v" },
		{ "-quiet", "-q" },
		{ "-all", "-a"},
		{ "-me", "-m"},
		{ "-debug", "-d" },
		{ 0, 0}};

	read_config_file(ATTACHCONFFILE);
	for (i=1;i<argc;i++) {
		if (*argv[i] == '-') {
			switch (internal_getopt(argv[i], options)) {
			case 'v':
				verbose = 1;
				break;
			case 'q':
				verbose = 0;
				break;
			case 'd':
				debug_flag = 1;
				break;
			case 'm':
				who = USER_ONLY;
				break;
			case 'a':
				who = ALL_USERS;
				break;
			}
		}
	}

	lock_attachtab();
	get_attachtab();
	unlock_attachtab();

	for (p = attachtab_first; p; p = p->next ) {
		if (p->status == STATUS_ATTACHING)
			/*
			 * If it is being attached, someone else will
			 * deal with the zephyr subscriptions.
			 * (Also, all the information won't be here yet.)
			 */
			continue;
		if(who != ALL_USERS && !wants_to_subscribe(p, real_uid, who))
			continue;
#ifdef AFS
		if (p->fs->type == TYPE_AFS) 
			afs_zinit(p->hesiodname, p->hostdir);
		else
#endif
		if (p->fs->flags & AT_FS_REMOTE) {
			sprintf(instbfr, "%s:%s", p->host, p->hostdir);
			zephyr_addsub(instbfr);
			zephyr_addsub(p->host);
		}
	}
	free_attachtab();
	return((zephyr_sub(1) == FAILURE) ? error_status : 0);
}
#endif /* ZEPHYR */

attachandruncmd(argc, argv)
    int argc;
    char **argv;
{
    string_list *mountpoint_list = NULL;
    char *attach_argv[4], **mountpoint, **found, *path;
    int status;

    if (argc < 4) {
        fprintf(stderr,
	      "Usage: attachandrun locker program argv0 [argv1...]\n");
	return(ERR_BADARGS);
    }

    attach_argv[0] = argv[0];
    attach_argv[1] = "-q";
    attach_argv[2] = argv[1];
    attach_argv[3] = NULL;
    status = attachcmd(sizeof(attach_argv) / sizeof(char *) - 1,
		       attach_argv, &mountpoint_list);

    mountpoint = sl_grab_string_array(mountpoint_list);
    if (mountpoint != NULL && *mountpoint != NULL) {
	if (setuid(getuid())) {
	    fprintf(stderr, "%s: setuid call failed: %s\n", progname,
		    strerror(errno));
	    return(ERR_FATAL);
	}

        found = athdir_get_paths(*mountpoint, "bin",
				 NULL, NULL, NULL, NULL, 0);
	if (found != NULL) {
	    path = malloc(strlen(*found) + 1 + strlen(argv[2]) + 1);
	    if (path == NULL) {
	        fprintf(stderr, "%s: Can't malloc while preparing for exec\n",
			progname);
		return(ERR_FATAL);
	    }

	    strcpy(path, *found);
	    strcat(path, "/");
	    strcat(path, argv[2]);

	    execv(path, argv + 3);

	    /* Feed text files to the shell by hand. */
	    if (errno == ENOEXEC)
		try_shell_exec(path, argc - 3, argv + 3);

	    fprintf(stderr, "%s: failure to exec %s: %s\n", progname,
		    argv[2], strerror(errno));
	    free(path);
	    sl_free(&mountpoint_list);
	    athdir_free_paths(found);
	    return(ERR_FATAL);
	} else {
	    fprintf(stderr, "%s: couldn't find a binary directory in %s\n",
		    progname, *mountpoint);
	    sl_free(&mountpoint_list);
	    return(ERR_FATAL);
	}
    } else {
        /* Assume attach code must have already given an error. */
        return(status);
    }
}

/* Try to feed a text file to the shell by hand.  On error, errno will
 * be the error value from open() or read() or execv(), or ENOEXEC if
 * the file looks binary, or ENOMEM if we couldn't allocate memory for
 * an argument list. */
try_shell_exec(path, argc, argv)
    char *path;
    int argc;
    char **argv;
{
    int i, count, fd, err;
    unsigned char sample[128];
    char **arg;

    /* First we should check if the file looks binary.  Open the file. */
    fd = open(path, O_RDONLY, 0);
    if (fd == -1)
	return(-1);

    /* Read in a bit of data from the file. */
    count = read(fd, sample, sizeof(sample));
    err = errno;
    close(fd);
    if (count < 0) {
	errno = err;
	return(-1);
    }

    /* Look for binary characters in the first line. */
    for (i = 0; i < count; i++) {
	if (sample[i] == '\n')
	    break;
	if (!isspace(sample[i]) && !isprint(sample[i])) {
	    errno = ENOEXEC;
	    return(-1);
	}
    }

    /* Allocate space for a longer argument list. */
    arg = malloc((argc + 2) * sizeof(char *));
    if (!arg) {
	errno = ENOMEM;
	return(-1);
    }

    /* Set up the argument list.  Copy in the argument part of argv
     * including the terminating NULL.  argv[0] is lost,
     * unfortunately. */
    arg[0] = "/bin/sh";
    arg[1] = path;
    memcpy(arg + 2, argv + 1, argc * sizeof(char *));

    execv(arg[0], arg);
    free(arg);
    return(-1);
}
