/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 4.12 1994-04-07 12:57:30 miki Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 4.11  93/04/29  16:12:58  vrt
 * solaris
 * 
 * Revision 4.10  91/07/18  12:51:31  epeisach
 * Under ultrix we try umounting "/" as it has been determined that the
 * filesystem will not cleanup itself of inodes which are in the text cache
 * 
 * Revision 4.9  91/06/24  15:18:13  epeisach
 * POSIX dirent handling
 * 
 * Revision 4.8  91/03/14  13:28:36  epeisach
 * Under AIX don't perform the umount.
 * 
 * Revision 4.7  90/07/12  14:45:15  epeisach
 * Don't umount / under ultrix.
 * 
 * Revision 1.2  90/06/22  15:17:28  dschmidt
 * don't unmount on ultrix
 * 
 * Revision 1.1  90/06/22  15:16:07  dschmidt
 * Initial revision
 * 
 * Revision 4.6  88/09/19  20:26:34  don
 * bellcore copyright.
 * 
 * Revision 4.5  88/06/21  19:38:41  don
 * finished changes for link-first updating, and undid writestat's
 * memory-saving entry-wise sorting.
 * added amusing hack suggested by jis: at end of each readstat()
 * call, we flush the kernel's text-table by calling 'unmount("/")'.
 * this fails, but not before causing the file-system to scavenge
 * whatever vnodes have been freed recently. probably only works
 * when the root is being updated, but that's when we're about to
 * reboot, anyway, and that's  what tends to ovefill.
 * 
 * Revision 4.4  88/06/20  18:53:42  don
 * changed updating traversal to invoke readstat() twice:
 * first pass updates only the dir's & symlinks in the statfile,
 * second pas updates everything else, including the dir's.
 * this causes the update to make space for itself, BEFORE the space
 * is needed. previously, track didn't do  well on crowded file-sys'.
 * this version needs stamp.c version 4.6 .
 * 
 * Revision 4.3  88/06/10  15:56:55  don
 * fixed two bugs: now, /tmp/sys_rvd.started is ug+w, thus deletable.
 * also, -F/ & -T/ don't make paths that begin // anymore.
 * 
 * Revision 4.2  88/06/10  12:27:18  don
 * added -I option, and changed -I to -d, -d to -W.
 * changed the way default subscriptiolist & statfile names get made.
 * slightly improved write_stat's memory-use.
 * fixed a glitch in justshow();
 * added sync() call at end of readstat().
 * 
 * Revision 4.1  88/05/04  18:11:38  shanzer
 * made sort_entries() run before justshow(), so that the augmented
 * entrylist gets dumped.
 * -don
 * 
 * Revision 4.0  88/04/14  16:43:19  don
 * this version is not compatible with prior versions.
 * it offers, chiefly, link-exporting, i.e., "->" systax in exception-lists.
 * it also offers sped-up exception-checking, via hash-tables.
 * a bug remains in -nopullflag support: if the entry's to-name top-level
 * dir doesn't exist, update_file doesn't get over it.
 * the fix should be put into the updated() routine, or possibly dec_entry().
 * 
 * Revision 3.0  88/03/09  13:18:05  don
 * this version is incompatible with prior versions. it offers:
 * 1) checksum-handling for regular files, to detect filesystem corruption.
 * 2) more concise & readable "updating" messages & error messages.
 * 3) better update-simulation when nopullflag is set.
 * 4) more support for non-default comparison-files.
 * finally, the "currentness" data-structure has replaced the statbufs
 * used before, so that the notion of currency is more readily extensible.
 * note: the statfile format has been changed.
 * 
 * Revision 2.9  88/02/19  19:07:16  don
 * bug from punctuation error, causing unbounded growth of source pathname.
 * 
 * Revision 2.8  88/01/29  18:24:10  don
 * bug fixes. also, now track can update the root.
 * 
 * Revision 2.6  87/12/07  18:25:49  don
 * removed SIGCHLD trap: signal( SIGCHLD, wait) can't work,
 * because wait() requires a pointer or NULL as an argument.
 * this signal call would pass the integer SIGCHLD to wait();
 * this is not good; further, it's unnecessary, as only
 * do_cmds() spawns children, and its pclose() call will call
 * wait() for those children.
 * 
 * Revision 2.5  87/12/07  17:16:18  shanzer
 * commented out do_cmds call; parser was leaving white-space in
 * some entries' command-fields, which do_cmds handled poorly (bus error).
 * 
 * Revision 2.4  87/12/03  20:41:52  don
 * replaced twdir & fwdir crap. these were fromroot/toroot-qualified
 * pathnames to the parent dir of slists/ & stats/. they don't both
 * need to be present, because one or the other is used, mutually
 * exclusively, according to whether -w option (writeflag) is present.
 * now, there's a single working-dir, which defaults appropriately
 * to either twdir's or fwdir's old default value, but if it is
 * specified with the -d option, it is NOT qualified with eiher
 * fromroot or toroot. got that?
 * 
 * Revision 2.3  87/12/03  17:30:35  don
 * fixed lint messages.
 * 
 * Revision 2.2  87/12/02  18:46:29  don
 * hc warnings.
 * 
 * Revision 2.1  87/12/01  16:45:00  don
 * fixed bugs in readstat's traversal of entries] and statfile:
 * cur_ent is no longer global, but is now part of get_next_match's
 * state. also, last_match() was causing entries[]'s last element to be
 * skipped.
 * 
 * Revision 2.0  87/11/30  15:14:38  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 *
 * Revision 1.6  87/10/29  		don
 * Rewrote just about everything, but especially read_names &
 * write_stat. threw out stamp data type, in favor of stat structure.
 * got rid of do_name(), rewrote walk_tree. moved update() code into
 * update_file(). renamed assorted routines, replaced lstat's where possible
 * with stat field references or access() calls.
 *
 * Revision 1.5  87/09/08  15:55:42  shanzer
 * Fixed a rename to do a copy then an unlink so it well work across
 * devices..
 * 
 * Revision 1.4  87/08/28  15:11:40  shanzer
 * Catches SIGINT, and removes lockfiles... 
 * 
 * Revision 1.3  87/08/28  13:47:30  shanzer
 * Put temp file in /tmp where the belong.. 
 * 
 * Revision 1.2  87/03/05  19:20:08  rfrench
 * Fixed lossage with extra /'s in filenames.
 * 
 * Revision 1.1  87/02/12  21:15:48  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 4.12 1994-04-07 12:57:30 miki Exp $";
#endif lint

#include "bellcore-copyright.h"
#include "mit-copyright.h"

#include "track.h"
#ifdef ultrix
#include <sys/mount.h>
#endif

char admin[WORDLEN] = DEF_ADM;		/* track administrator */
char workdir[LINELEN];			/* working directory under src/dest
					 * root where slists, statfiles, etc.
					 * can be found */
char binarydir[LINELEN] = DEF_BINDIR;	/* directory in working dir to
					 * find executables */
char fromroot[LINELEN] = DEF_FROMROOT;	/* Root directory for source */
char toroot[LINELEN] = DEF_TOROOT;	/* Root directory for destination */

char lockpath[LINELEN];			/* starting lock filename */
char logfilepath[LINELEN] = DEF_LOG;	/* default log file */
FILE *logfile = NULL;			/* the logfile */
char subfilename[LINELEN] = DEF_SUB;	/* default subscription file */
char subfilepath[LINELEN] = "";		/* alternate subscription file */
char statfilepath[LINELEN];		/* pathname to statfile */
FILE *statfile;				/* the statfile! */
Statline *statfilebufs;			/* array of line buffers for sort() */
int cur_line;				/* index into statfilebufs	*/
unsigned maxlines = 0;			/* max # lines in statfile buf array.
					 * write_statline maintains maxlines */
unsigned stackmax = STACKMAX;		/* max depth of pathname-stack vars */

char prgname[LINELEN];

extern int errno;		/* global error number location */
char errmsg[LINELEN];

int writeflag = 0;	/* if set, translate subscription list -> statfile,
			 * rather than pulling files */
int parseflag = 0;	/* if set, just parse the subscription list */
int forceflag = 0;	/* if set, will over-ride lock files */
int verboseflag = 0;	/* if set, files listed on stdout as they're updated */
int cksumflag = 0;	/* if set, compare file checksums when updating */
int nopullflag = 0;	/* if set, find out the differences,
			 *	   but don't pull anything */
int quietflag = 0;	/* if set, don't print non-fatal error messages */
int interactive = 1;	/* if set, don't send errors via mail, print them */
int uflag = NO_CLOBBER;	/* if set, copy a older file on top of a newer one */
int debug = 0;		/* if set, print debugging information */
int ignore_prots = 0;	/* if set, don't use or set uid/gid/mode-bits */
int incl_devs = 0;	/* if set, include devices in update */

/* initialize the global entry-counters with bad values,
 * so that printmsg() can detect them.
 */
Entry entries[ ENTRYMAX];	/* Subscription list entries */
int entrycnt = -1;		/* Number of entries */
int entnum = -1;		/* Current entry number */

int (*statf)();			/* dec_entry() sets value to stat() or lstat(),
				 * according to entries[].followlinks */
char *statn = "";		/* name of statf's current value. */

main(argc,argv)
int argc;
char **argv;
{
	char	scratch[LINELEN];
	int	cleanup();
	int	i;
#ifdef POSIX
	struct sigaction act;
#endif
	strcpy(prgname,argv[0]);
	strcpy(errmsg,"");

	umask(022);	/* set default umask for daemons */

#ifdef POSIX
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler= (void (*)()) cleanup;
	(void) sigaction(SIGINT, &act, NULL);
	(void) sigaction(SIGHUP, &act, NULL);
	(void) sigaction(SIGPIPE, &act, NULL);
#else
	signal( SIGINT, cleanup);
	signal( SIGHUP, cleanup);
	signal( SIGPIPE, cleanup);
#endif
	for(i=1;i<argc;i++) {
		if (argv[i][0] != '-') {
			strcpy( subfilepath, argv[i]);
			continue;
		}
		switch (argv[i][1]) {
		/* -F dirname
		 *    Specify source "root" directory.
		 */
		case 'F':
			get_arg(scratch,argv,&i);
			if (*scratch != '/') {
#ifdef POSIX
			        getcwd(fromroot, sizeof(fromroot));
#else
				getwd(  fromroot);
#endif
				strcat( strcat( fromroot, "/"), scratch);
			}
			else if (! scratch[1]) *fromroot = '\0';
			else strcpy( fromroot, scratch);
			break;

		/* -I
		 *    Ignore protections (uid,gid,mode) when tracking,
		 *    except when creating a file: then, use remote prots.
		 */
		case 'I':
			ignore_prots = 1;
			break;

		/* -S stackmax
		 *    Specify deeper path stacks
		 */ 
		case 'S':
			get_arg(scratch,argv,&i);
			sscanf( scratch, "%d", &stackmax);
			break;

		/* -T dirname
		 *    Specify destination "root" directory.
		 */ 
		case 'T':
			get_arg(scratch,argv,&i);
			if (*scratch != '/') {
#ifdef POSIX
			        getcwd(toroot, sizeof(toroot));
#else
				getwd(  toroot);
#endif
				strcat( strcat( toroot, "/"), scratch);
			}
			else if (! scratch[1]) *toroot = '\0';
			else strcpy( toroot, scratch);
			break;

		/* -W dirname
		 *    Specify the working directory for
		 *    accessing the subscription-list and statfile.
		 */
		case 'W':
			get_arg(workdir,argv,&i);
			break;
		/* -c
		 *    compare checksums of regular files, when updating.
		 *    the checksums are used to detect file-system
		 *    corruption.
		 */
		case 'c':
			cksumflag = 1;
			break;
		/* -d
		 *    Include devices in an update.
		 */
		case 'd':
			incl_devs = 1;
			break;
		/* -f
		 *    Force updating regardless of locks.
		 */
		case 'f':
			forceflag = 1;
			break;

		/* -m {user}
		 *    Send mail to root/user instead of
		 * displaying messages on the terminal.
		 */
		case 'm':
			interactive = 0;
			get_arg(admin,argv,&i);
			break;
		/* -n
		 *    Produce a list of files that need updating,
		 * but don't actually do anything about them.
		 */
		case 'n':
			nopullflag = 1;
			verboseflag = 1;
			fprintf( stderr, "-n: what we WOULD do:\n");
			break;
		/* -p
		 *    Parse only.  Display a detailed list of the fields in the
		 * subscription file.
		 */
		case 'p':
			parseflag = 1;
			break;
		/* -q
		 *    Be quiet about warning messages.
		 */
		case 'q':
			quietflag = 1;
			break;
		/* -s {pathname}
		 *   use specified file as statfile,
		 * or use stdio if pathname is "-".
		 */
		case 's':
			get_arg( statfilepath, argv, &i);
			break;
		/* -u
		 *    Copy over files regardless of which is newer.
		 */
		case 'u':
			uflag = DO_CLOBBER;
			break;
		/* -v
		 *    Explain what is going on verbosely.
		 */
		case 'v':
			verboseflag = 1;
			break;
		/* -w
		 *    Create a statfile.
		 */
		case 'w':
			writeflag = 1;
			break;
		/* -x
		 *    Display debugging information.
		 */
		case 'x':
			debug = 1;
			break;
		/*
		 * Something isn't right if we got this far...
		 */
		default:
			fprintf(stderr,"track error: bad option %s\n",argv[i]);
			break;
		}
	}

	/*
	 * Set up nullmail interface if not an interactive session.
	 */
	if (!interactive)
		setuperr();

	/* check for existence of root directories:
	 * we shouldn't create them, as they are likely to be remote,
	 * so that the user may have forgotten to attach them.
	 */
	if ( *fromroot && access( fromroot, 0)) {
		sprintf(errmsg,"can't access source root-directory %s\n",
			fromroot);
		do_panic();
	}
	if ( !writeflag && *toroot && access( toroot, 0)) {
		sprintf(errmsg,"can't access target root-directory %s\n",
			toroot);
		do_panic();
	}
	build_path( fromroot, workdir, DEF_SLISTDIR, subfilepath);
	build_path( fromroot, workdir, DEF_STATDIR, statfilepath);

	fprintf( stderr, "using %s as subscription-list\n", subfilepath);
	fprintf( stderr, "using %s as statfile\n",         statfilepath);

	/*
	**	redirect yacc/lex i/o
	*/
	parseinit( opensubfile( subfilepath));
	if (yyparse()) {
		strcpy(errmsg,"parse aborted.\n");
		do_panic();
	}
	if (debug)
		printf("parse worked\n");

	sort_entries();

	if (parseflag) {  /* -p: Just show the fields */
		justshow();
		cleanup();
	}

	setlock();

	openstat( statfilepath, writeflag);

	if ( writeflag)		/* -w: Write the exporting statfile */
		writestat();
	else {
		/* update in two passes: links & their parent-dirs first,
		 * which frees up file-system space when links replace files,
		 * then everything else. re-update dirs in second pass,
		 * to facilitate statfile-traversal.
		 */
		readstat( "ld");
		rewind( statfile);
		readstat( "fdbc");
	}
	closestat();

	clearlocks();
	exit( 0);
}			/* end of main() */

#define pathtail( p) p[1+*(int*)p[CNT]]

readstat( types) char *types; {
	struct currentness rem_currency, *cmp_currency;
	char statline[ LINELEN], *remname;
	char **from, **to, **cmp;
	char *tail = NULL;

	from = initpath( fromroot);
	to   = initpath(   toroot);
	cmp  = initpath(   toroot);

	/* prime the path-stacks for dec_entry() to
	 * pop the "old" entry-names off.
	 */
	pushpath( from, ""); pushpath( to, ""); pushpath( cmp, "");

	init_next_match();

	while ( NULL != fgets( statline, sizeof statline, statfile)) {

		/* XXX : needs data-hiding work, but will do:
		 *	 only update what main tells us to in this pass.
		 */
		if ( ! index( types, *statline)) continue;

		/* extract the currency data from the statline:
		 */
		remname = dec_statfile( statline, &rem_currency);

		/* find the subscription entry corresponding to the
		 * current pathname.
		 * if we reach an entry which is lexicographically greater than
		 * the current pathname, read statfile for the next pathname.
		 * both entries[] & statfile must be sorted by sortkey!
		 */
		if ( 0 >= ( entnum = get_next_match( remname))) continue;

		/* do a breadth-first search of the tree of entries,
		 * to find the entry corresponding to remname:
		 * for example, if /usr & /usr/bin are both entries,
		 * they appear in that order in the entries[] array.
		 * if remname is /usr/bin/foo, we want gettail() to
		 * use /usr/bin's exception-list, not /usr's exception-list.
		 * thus, /usr/bin is the "last match" for /usr/bin/foo.
		 */
		entnum = last_match( remname, entnum);

		tail = remname;
		switch ( gettail( &tail, TYPE( rem_currency.sbuf), entnum)) {
		case NORMALCASE: break;
		case DONT_TRACK: continue;
		case FORCE_LINK: fake_link( fromroot, remname, &rem_currency);
				 break;
		default:	 sprintf(errmsg,"bad value from gettail\n");
				 do_panic();
		}

		/* loosely, tail == remname - fromfile, as
		 * long as tail isn't in the exception-list.
		 * the string remname begins with the string from[ PATH]:
		 * for example: remname =            /usr/bin/foo.
		 *          from[ PATH] =            /usr/bin.
		 *          from[ ROOT] = /mountpoint/usr/bin.
		 * in this example, we get tail == "foo".
		 */

		cmp_currency = dec_entry( entnum, from, to, cmp, tail);

		pushpath( to,  tail);
		pushpath( from, tail);

		if ( ! update_file( cmp_currency, to,
				   &rem_currency, from))
			/* REWRITE:
			do_cmds( entries[entnum].cmdbuf, to[ ROOT])
			 */
			;
		/* remove tail from each path:
		 */
		poppath( to);
		poppath( from);
	}
	/* track is often used just before a reboot;
	 * flush the kernel's text-table,
	 * to ensure that the vnodes we've freed get scavenged,
	 */
#if !defined(ultrix) && !defined(_AIX) && !defined(SOLARIS)
	unmount("/");		/* XXX */
#endif
#ifdef ultrix
        {
		dev_t dev;
		struct fs_data fsd;
		if(statfs("/",&fsd) == 1) umount(fsd.fd_dev);
	}
#endif
	/* then make sure that the file-systems' superblocks are up-to-date.
	 */
	sync();
	sleep(2);
}

/*
 * Set lock for subscriptionlist file.
 */

setlock()
{
	sprintf( lockpath,"%s/%s.started", DEF_LOCKDIR, subfilename);

	if ( access( lockpath, 0));
	else if ( too_old( lockpath, LOCK_TIME) || forceflag) clearlocks();
	else {
		sprintf( errmsg, "lock set on %s--quitting\n", lockpath);
		do_gripe();
		exit(0);
	}
	if ( close( creat( lockpath,220))) {
		sprintf( errmsg, "can't create lockfile %s\n", lockpath);
		do_panic();
	}
	return(0);
}

/*
 * Erase those locks...
 */

clearlocks()
{
	if ( !*lockpath) return;
	if ( unlink( lockpath)) {
		fprintf( stderr, "can't remove lockfile %s", lockpath);
		perror( "system error is: ");
	}
	else if ( verboseflag)
		fprintf( stderr, "cleared lock %s\n",lockpath);
}

/* the array from[] is a set of pointers into a single pathname.
 * it allows us to pass a parsed pathname along in walk_trees()'
 * recursive descent of a directory.
 * the ROOT component is the entire absolute pathname,
 * including the mount-point, for use in file-system calls.
 * the NAME component is the portable pathname, without the mount-point,
 * as the file is described in the statfile.
 * the TAIL component is everything that's added during the recursive descent,
 * for comparison with the exception-list. TAIL lacks both mount-point &
 * the fromfile name.
 */

/*
 * Act like a librarian and write out the statfile.
 */

writestat()
{
	char **from, **cmp, **dummy = (char **) NULL;
	struct currentness *entry_currency;

	from = initpath( fromroot);
	cmp  = initpath( fromroot);

	/* prime the path-stacks for dec_entry() to
	 * pop the "old" entry-names off.
	 */
	pushpath( from, ""); pushpath( cmp, "");

	for( entnum = 1; entnum < entrycnt; entnum++) {

		/* dec_entry pushes pathname-qualification
		 * onto the paths 'from' & 'cmp',
		 * and pops when appropriate.
		 */
		entry_currency = dec_entry( entnum, from, dummy, cmp, NULL);

		/* write_statline returns fromfile's true type,
		 * regardless of cmpfile's type:
		 */
		if      ( S_IFDIR != write_statline( from, entry_currency));
		else if ( S_IFDIR != TYPE( entry_currency->sbuf))

			walk_trees( from, dummy, entry_currency);
		else    walk_trees( from, cmp,   entry_currency);

		/* WARNING: walk_trees alters ALL of its arguments */
		/* sort the statfile, and write it out
		 * to the correct directory:
		 */
	}
	sort_stat();
}

/* if the current entry's fromfile is a directory, but its cmpfile isn't,
 * put the same cmpstat in all of fromfile's contents' statlines.
 * if cmpfile is a directory too, its subtree must parallel fromfile's subtree,
 * and each statline reflects the one-to-one (not onto) mapping from
 * fromfile's subtree to cmpfile's subtree:
 * we take fromfile's subnode's pathname ( not including the prefix fromroot),
 * and we take the stat from cmpfile's corresponding subnode.
 */
walk_trees( f, c, currency)
char *f[], *c[];
struct currentness *currency;
{
	DIR *dirp;
#ifdef POSIX
	struct dirent *dp;
#else
	struct direct *dp;
#endif
	char *tail;

	dirp = opendir( f[ ROOT]);
	if (!dirp) {
		sprintf(errmsg,"can't open directory %s\n", f[ ROOT]);
		do_gripe();
		return;
	}
	dp = readdir( dirp);	/* skip . */
	dp = readdir( dirp);	/* skip .. */

	while( dp = readdir( dirp)) {
		if (! dp->d_ino) continue;    /* empty dir-block */

		pushpath( f, dp->d_name);
		pushpath( c, dp->d_name);

		tail = f[ NAME];
		switch ( gettail( &tail, 0, entnum)) {
		case NORMALCASE: break;
		case FORCE_LINK: fake_link( "", f[ NAME], currency);
				 write_statline( f, currency);
				 /* fall through to poppath() calls */
		case DONT_TRACK: poppath( f);
				 poppath( c);
				 continue;
		default:	 sprintf(errmsg,"bad value from gettail\n");
				 do_panic();
		}
		/* normal case: tail isn't an exception or a forced link.
		 */
		if ( c && get_currentness( c, currency)) {
			sprintf(errmsg,"can't %s comparison-file %s.\n",
				statn, c[ ROOT]);
			do_panic();
		}
		/* write_statline returns fromfile's type:
		 */
		else if ( S_IFDIR == write_statline( f, currency))
			walk_trees( f, c, currency);

		poppath( f);
		poppath( c);
	}
	closedir(dirp);
}

/*
 * Get a command line argument
 */

get_arg(to,list,ptr)
char *to,**list;
int *ptr;
{
	int offset = 2;

	if (strlen(list[*ptr]) == 2) {
		(*ptr)++;
		offset = 0;
	}
	strcpy(to,list[*ptr]+offset);
}

/*
 * Log a message to the logfile.
   UNUSED

log(ptr)
char *ptr;
{
	extern long time();
	extern char *ctime();
	static FILE *logfile = NULL;
	char namebuf[LINELEN];
	char timestring[LINELEN];
	long timebuf;

	if ( NULL == logfile) {
		sprintf( namebuf,"%s/%s",workdir,DEF_LOG);
		if( access( namebuf, 0))
			return;
		if (NULL == (logfile = fopen( namebuf,"a"))) {
			sprintf(errmsg,"can't open log file %s",namebuf);
			do_panic();
		}
	}
	timebuf = time(0);
	strcpy(timestring,ctime(&timebuf));
	timestring[ strlen(timestring)-1] = '\0';
	fprintf(logfile,"%s %s %s\n",timestring,subfilename,ptr);
}
 */

#undef ROOT

/*
 * Execute shell commands
 */

do_cmds(cmds,local)
char *cmds,*local;
{
	char *ptr,*nptr;
	FILE *shell;

	if ( ! *cmds) return;
	ptr = cmds;

	shell = popen(DEF_SHELL,"w");
	if (!shell) {
		sprintf(errmsg,"can't open shell %s\n",DEF_SHELL);
		do_gripe();
		return;
	}

	fprintf(shell,"chdir %s\n",toroot);
	fprintf(shell,"%sFILE=%s\n",DEF_SETCMD,local);
	fprintf(shell,"%sROOT=%s\n",DEF_SETCMD,toroot);

	for (;;) {
		nptr = index(ptr,'\n');
		if (nptr)
			*nptr = '\0';
		fprintf(shell,"%s\n",ptr);
		if (!nptr) {
			pclose(shell);
			return;
		}
		ptr = nptr+1;
	}
}
/*
 * Show parsing.
 */

justshow()
{
	int i,j, size;
	Entry *e;
	List_element *p;

	fprintf( stderr, "subscription-list as parsed:\n\n");

	for (i = 0; i <= entrycnt; i++) {
		e = &entries[ i];
		if ( ! e->fromfile) break;
		fprintf(stderr,
			"entry %d:%s\n\tfromfile-- %s\n",
			i,
			e->followlink ? " ( follow links)" : "",
			e->fromfile);
		fprintf(stderr,
			"\tcmpfile-- %s\n\ttofile-- %s\n\tpatterns--\n",
			e->cmpfile,
			e->tofile);
		for( p = e->patterns; p ; p = NEXT( p))
		    fprintf(stderr,"\t\t%s%s\n",
			    FLAG( p) == FORCE_LINK ? "-> " : "",
			    TEXT( p));
		fprintf( stderr, "\texceptions--\n");
		switch( SIGN( e->names.shift)) {
		case -1:
		    for( p = ( List_element *) e->names.table; p ; p = NEXT( p))
			fprintf(stderr,"\t\t%s%s\n",
				FLAG( p) == FORCE_LINK ? "-> " : "",
			 	TEXT( p));
		    fprintf(stderr,
			"track didn't fully parse the exception-list.\n");
		    fprintf(stderr,
			"the most-recently parsed exception was:\n%s%s\n",
			FLAG( e->names.table) == FORCE_LINK ? "-> " : "",
			TEXT( e->names.table));
		    continue;
		case 0: break;
		case 1:
		    size = (unsigned) 0x80000000 >> e->names.shift - 1;
		    for( j = 0; j < size; j++)
		    {
			if ( ! e->names.table[j]) continue;
			fprintf( stderr,"\t\t");
			for ( p = e->names.table[j]; p; p = NEXT( p))
			    fprintf(stderr,"%s%s, ",
				    FLAG( p) == FORCE_LINK ? "-> " : "",
				    TEXT( p));
			fprintf( stderr,"\n");
		    }
		    break;
		}
		fprintf(stderr,"\tcommand-- %s\n",e->cmdbuf);
	}
}

/*
 *	redirect standard error output to mail to the administrator
 *	use nullmail so that null messages won't get sent.
 */

/* REWORK */

setuperr()
{
	char msg[LINELEN];
	FILE *tmp;

	sprintf(msg,"%s/nullmail %s",binarydir,admin);
	/*
	**	start a process that will send mail to the adminstrator
	*/
	if ((tmp = popen(msg,"w")) == NULL) {
/*		sprintf( msg, "echo HELP track --%s %s", gargv[0],
			"can't execute nullmail cmd  > /dev/console");
		system(msg); */
		exit(1);
	}
	/*
	**	now connect stderr to the pipe
	*/
	if (dup2(fileno(tmp),2) ==  -1) {
/*		sprintf( msg, "echo HELP track --%s %s", gargv[0],
			"can't dup stderr  > /dev/console");
		system(msg); */
		exit(1);
	}
}

build_path( f, w, d, p) char *f, *w, *d, *p; {
	static char buf[ LINELEN];

	if ( ! strcmp( p, "-")) return;
	/*
	 * Get the proper working directory,
	 * where the subscription-list & statfile are.
	 */
	if ( *w) f = "";  /* don't add root-qualification to user's workdir */
	else if ( *p) f = "."; /* don't use default workdir with user's filen */
	else w = DEF_WORKDIR; /* default workdir, default filename */

	if ( *p) d = "";
	else strcpy( p, subfilename);

	sprintf( buf, "%s%s%s%s/%s", f, w, *d ? "/" : "", d, p);
	strcpy( p, buf);
}

FILE *
opensubfile( path) char *path; {
	FILE *subfile;
        if ( ! ( subfile = fopen( path, "r"))) {
                sprintf( errmsg, "Can't open subscriptionlist %s\n", path);
                do_panic();
        }
	return( subfile);
}

openstat( path, write) char *path; int write;
{
	char *mode = write? "w"   : "r";
	FILE *std =  write? stdout : stdin;

	if ( ! strcmp( path, "-"))
		statfile = std;
	else if ( ! ( statfile = fopen( path, mode))) {
		sprintf( errmsg, "can't open statfile %s\n", path);
		do_panic();
	}
}

closestat() {
	if ( EOF == fclose( statfile)) {
		sprintf( errmsg, "can't close %s\n", statfilepath);
		do_panic();
	}
}

cleanup()
{
	clearlocks();
	exit(0);
}
