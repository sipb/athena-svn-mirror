/*
 *	$Id: track.c,v 4.21 1999-01-22 23:16:03 ghudson Exp $
 */

#ifndef lint
static char *rcsid_header_h = "$Id: track.c,v 4.21 1999-01-22 23:16:03 ghudson Exp $";
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

main(argc,argv)
int argc;
char **argv;
{
	char	scratch[LINELEN];
	int	cleanup();
	int	i;

	strcpy(prgname,argv[0]);
	strcpy(errmsg,"");

	umask(022);	/* set default umask for daemons */

#ifdef SYSV
	sigset( SIGINT, (void *)cleanup);
	sigset( SIGHUP, (void *)cleanup);
	sigset( SIGPIPE, (void *)cleanup);
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
				getcwd( fromroot, sizeof(fromroot));
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
				getcwd( toroot, sizeof(toroot));
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
		if ( ! strchr( types, *statline)) continue;

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

		if (entries[entnum].islink) {
			fake_link( "", from[ NAME], entry_currency);
			write_statline( from, entry_currency);
			continue;
		}

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

	while( dp = readdir( dirp)) {
		if (strcmp(dp->d_name, ".") == 0)
		  continue;
		if (strcmp(dp->d_name, "..") == 0)
		  continue;

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
			sprintf(errmsg,"can't lstat comparison-file %s.\n",
				c[ ROOT]);
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
		nptr = strchr(ptr,'\n');
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
			"entry %d:\n\tislink-- %d\tfromfile-- %s\n",
			i,
			e->islink,
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
