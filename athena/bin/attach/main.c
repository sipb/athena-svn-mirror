/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/main.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

static char *rcsid_main_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/main.c,v 1.11 1990-07-16 07:24:36 jfc Exp $";

#include "attach.h"
#include <signal.h>

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
#endif AFS
#ifdef UFS
int ufs_attach(), ufs_detach();
#endif
int err_attach(), null_detach();
char	**ufs_explicit();

struct _fstypes fstypes[] = {
    { "---", 0, -1, 0, (char *) 0, 0, null_detach, 0 },	/* The null type */
#ifdef NFS
    { "NFS", TYPE_NFS, MOUNT_NFS, FS_MNTPT | FS_REMOTE | FS_MNTPT_CANON, "rwnm",
	      nfs_attach, nfs_detach, nfs_explicit },
#endif
#ifdef RVD
    { "RVD", TYPE_RVD, MOUNT_UFS, FS_MNTPT | FS_REMOTE | FS_MNTPT_CANON, "rw",
	      rvd_attach, rvd_detach, rvd_explicit },
#endif
#ifdef UFS
    { "UFS", TYPE_UFS, MOUNT_UFS, FS_MNTPT | FS_MNTPT_CANON, "rw", 
	      ufs_attach, ufs_detach, ufs_explicit },
#endif
    { "ERR", TYPE_ERR, -1, 0, (char *) 0, err_attach, 0, 0 },
#ifdef AFS
    { "AFS", TYPE_AFS, -1, FS_MNTPT | FS_PARENTMNTPT, "nrw", afs_attach, 
	      afs_detach, afs_explicit },
#endif
    { "MUL", TYPE_MUL, -1, 0, (char *) 0, 0, 0, 0 },
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

    /* Stop overriding my explicit file modes! */
    umask(022);

    /* Install signal intercept for SIGTERM and SIGINT */
    (void) signal (SIGTERM, sig_trap);
    (void) signal (SIGINT, sig_trap);

    real_uid = owner_uid = getuid();
    effective_uid = geteuid();

    progname = argv[0];
    ptr = rindex(progname, '/');
    if (ptr)
	progname = ptr+1;

    attachtab_fn = strdup(ATTACHTAB);
    mtab_fn = strdup(MTAB);
#ifdef AFS
    afs_mount_dir = strdup(AFS_MOUNT_DIR);
#endif
    fsck_fn = strdup(FSCK_FULLNAME);

    /*
     * User can explicitly specify attach, detach, or nfsid by using the
     * -P option as the first command option.
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
				    "Must specify attach, detach or nfsid!\n");
			    exit(ERR_BADARGS);
		    } 
	    } 
    }

    if (!strcmp(progname, ATTACH_CMD))
	exit(attachcmd(argc, argv));
    if (!strcmp(progname, DETACH_CMD))
	exit(detachcmd(argc, argv));
#ifdef NFS
#ifdef KERBEROS
    if (!strcmp(progname, NFSID_CMD))
      {
	filsys_type = "NFS";
	exit(nfsidcmd(argc, argv));
      } else if(!strcmp(progname, FSID_CMD)) {
	exit(nfsidcmd(argc, argv));
      }
#endif
#endif
#ifdef ZEPHYR
    if (!strcmp(progname, ZINIT_CMD))
	exit(zinitcmd(argc, argv));
#endif

    fprintf(stderr, "Not invoked with attach, detach, nfsid, or zinit!\n");
    exit(ERR_BADARGS);
}

#ifdef NFS
#ifdef KERBEROS
/*
 * Command handler for nfsid.
 */
nfsidcmd(argc, argv)
    int argc;
    char *argv[];
{
    extern struct _attachtab	*attachtab_first;
    int gotname, i, op, filsysp;
#ifdef AFS
    int	cell_sw;
#endif
    char *ops;
    struct hostent *hent;
    char hostname[BUFSIZ];
    struct _attachtab	*atp;
    struct in_addr addr;
    static struct command_list options[] = {
	{ "-verbose", "-v" },
	{ "-quiet", "-q" },
	{ "-debug", "-d" },
	{ "-map", "-m" },
	{ "-unmap", "-u" },
	{ "-purge", "-p" },
	{ "-purgeuser", "-r" },
	{ "-all", "-a" },
	{ "-filsys", "-f" },
	{ "-spoofhost", "-s" },
	{ "-user", "-U" },
#ifdef AFS
	{ "-cell", "-c" },
#endif
	{ 0, 0 }};
	
    check_root_privs(progname);
    read_config_file(ATTACHCONFFILE);
    
    gotname = 0;
    filsysp = 0;
    verbose = 1;
#ifdef AFS
    cell_sw = 0;
#endif
    error_status = ERR_NONE;
    
    op = MOUNTPROC_KUIDMAP;
    ops = "mapped";

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
#ifdef AFS
	    case 'c':
		cell_sw = 1;
		break;
#endif
	    case 'm':
		op = MOUNTPROC_KUIDMAP;
		ops = "mapped";
		break;
	    case 'u':
		op = MOUNTPROC_KUIDUMAP;
		ops = "unmapped";
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
	    case 's':
		if (i == argc-1) {
		    fprintf(stderr, "%s: No spoof host specified\n", progname);
		    return (ERR_BADARGS);
		}
		spoofhost = argv[++i];
		break;
	    case 'r':
		op = MOUNTPROC_KUIDUPURGE;
		ops = "mappings user-purged";
		break;
	    case 'f':
		filsysp = 1;
		lock_attachtab();
		get_attachtab();
		unlock_attachtab();
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
			if (atp->fs->type == TYPE_NFS) {
				if ((nfsid(atp->host, atp->hostaddr, op, 1,
					   atp->hesiodname, 0, owner_uid)
				     == SUCCESS) && verbose)
					printf("%s: %s %s\n", progname,
					       atp->hesiodname, ops);
			} else
				printf("%s: %s ignored (not NFS)\n",
				       progname, atp->hesiodname);
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
	if (cell_sw) {
		afs_auth_to_cell(argv[i]);
	} else if (filsysp) {
	    /*
	     * Lookup the specified filsys name and perform an nfsid
	     * on the host associated with it.
	     */
	    if (atp = attachtab_lookup(argv[i])) {
		    if (atp->fs->type == TYPE_NFS) {
			    if ((nfsid(atp->host, atp->hostaddr, op, 1,
				       argv[i], 0, owner_uid) == SUCCESS) &&
				verbose)
				    printf("%s: %s %s\n", progname, argv[i], ops);
		    } else if (atp->fs->type == TYPE_AFS) {
#ifdef AFS
			    if (op == MOUNTPROC_KUIDMAP &&
				(afs_auth(atp->hesiodname, atp->hostdir) == SUCCESS)
				&& verbose)
				    printf("%s: %s %s\n", progname, argv[i], ops);
#endif
		    }
	    } else {
		error_status = ERR_NFSIDNOTATTACHED;
		fprintf(stderr, "%s: %s not attached\n", progname, argv[i]);
	    }
	} else {
	    /*
	     * Perform an nfsid on the specified host.
	     */
	    hent = gethostbyname(argv[i]);
	    if (!hent) {
		fprintf(stderr, "%s: Can't resolve host %s\n", progname, argv[i]);
		error_status = ERR_NFSIDBADHOST;
	    }
	    else {
		strcpy(hostname, hent->h_name);
		bcopy(hent->h_addr_list[0], &addr, 4);
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
#endif

char *attach_list_format = "%-22s %-22s %c%-18s%s\n";

attachcmd(argc, argv)
    int argc;
    char *argv[];
{
    int gotname, i;
    struct _attachtab *atp;
    extern struct _attachtab	*attachtab_first;
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
	{ 0, 0 }};

    read_config_file(ATTACHCONFFILE);

    /*
     * Print attachtab out if no arguments specified
     */
    if (argc == 1) {
	    char	optstr[40];

	    lock_attachtab();
	    get_attachtab();
	    unlock_attachtab();
	    atp = attachtab_first;
	    if (!atp) {
		    printf("No filesystems currently attached.\n");
		    free_attachtab();
		    return(ERR_NONE);
	    }
	    printf(attach_list_format, "filesystem", "mountpoint",
		   ' ', "user", "mode");
	    printf(attach_list_format, "----------", "----------",
		   ' ', "----", "----");
	    while (atp) {
		    optstr[0] = atp->mode;
		    optstr[1] = '\0';
		    if (atp->flags & FLAG_NOSETUID)
			    strcat(optstr, ",nosuid");
		    if (atp->flags & FLAG_LOCKED)
			    strcat(optstr, ",locked");
		    if (atp->flags & FLAG_PERMANENT)
			    strcat(optstr, ",perm");
		    if (atp->status == STATUS_ATTACHED) {
			    printf(attach_list_format, atp->hesiodname,
				   atp->mntpt,
				   atp->flags & FLAG_ANYONE ? '*' : ' ',
				   ownerlist(atp), optstr);
		    }
		    atp = atp->next;
	    }
	    free_attachtab();
	    return (ERR_NONE);
    }

    check_root_privs(progname);
    default_suid = (access(NOSUID_FILENAME, 0) == 0);
    
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
	    default:
		fprintf(stderr, "%s: Unknown switch %s\n", progname, argv[i]);
		return (ERR_BADARGS);		
	    }
	    continue;
	}
	gotname++;
	if(attach(argv[i]) == SUCCESS)
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
		if (trusted_user(real_uid)) {
			clean_detach++;
		} else {
			fprintf(stderr,
		"%s: You are not authorized to use the -clean option\n", progname);
		}
		break;
	case 'L':
		lint_attachtab();
		gotname = 1;
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
		if (p->fs->type == TYPE_AFS) 
			afs_zinit(p->hesiodname, p->hostdir);
		else if (p->fs->flags & FS_REMOTE) {
			sprintf(instbfr, "%s:%s", p->host, p->hostdir);
			zephyr_addsub(instbfr);
			zephyr_addsub(p->host);
		}
	}
	free_attachtab();
	return((zephyr_sub(1) == FAILURE) ? error_status : 0);
}
#endif /* ZEPHYR */
