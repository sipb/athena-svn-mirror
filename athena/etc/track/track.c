/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 2.2 1987-12-02 18:46:29 don Exp $
 *
 *	$Log: not supported by cvs2svn $
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.c,v 2.2 1987-12-02 18:46:29 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

char admin[WORDLEN] = DEF_ADM;		/* track administrator */
char twdir[LINELEN] = DEF_TOWDIR;	/* working directory for dest root
					 * where slists/statfiles etc. can be
					 * found */
char fwdir[LINELEN];			/* working directory for source root */
char binarydir[LINELEN] = DEF_BINDIR;	/* directory in working dir to
					 * find executables */
char fromroot[LINELEN] = DEF_FROMROOT;	/* Root directory for source */
char toroot[LINELEN] = DEF_TOROOT;	/* Root directory for destination */

char lockpath[LINELEN];			/* starting lock filename */
char subfilename[LINELEN] = DEF_SUB;	/* default subscription file */
char subfilepath[LINELEN] = "";		/* alternate subscription file */
char statfilepath[LINELEN];		/* pathname to statfile */
FILE *statfile;				/* the statfile! */
Statline *statfilebufs;			/* array of line buffers for sort() */
int cur_line;				/* index into statfilebufs	*/
int maxlines = 0;			/* max # lines in statfile buf array.
					 * write_statline maintains maxlines */
int stackmax = STACKMAX;		/* max depth of pathname-stack vars */

char prgname[LINELEN];

extern int errno;		/* global error number location */
extern int wait();
char errmsg[LINELEN];

int writeflag = 0;	/* if set, translate subscription list -> statfile,
			 * rather than pulling files */
int parseflag = 0;	/* if set, just parse the subscription list */
int forceflag = 0;	/* if set, will over-ride lock files */
int verboseflag = 0;	/* if set, files listed on stdout as they're updated */
int dirflag = 0;	/* if set, create directories when necessary */
int nopullflag = 0;	/* if set, find out the differences,
			 *	   but don't pull anything */
int quietflag = 0;	/* if set, don't print non-fatal error messages */
int interactive = 1;	/* if set, don't send errors via mail, print them */
int uflag = NO_CLOBBER;	/* if set, copy a older file on top of a newer one */
int debug = 0;		/* if set, print debugging information */
int incl_devs = 0;	/* if set, include devices in update */

Entry entries[ ENTRYMAX];		/* Subscription list entries */
int entrycnt = 0;			/* Number of entries */
int entnum = 0;			/* Current entry number */

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

	/* we spawn child processes which run shell-commands.
	 * don't let aborted children become zombies!
	 */
	signal( SIGCHLD, wait);
	signal( SIGINT, cleanup);

	for(i=1;i<argc;i++) {
		if (argv[i][0] != '-') {
			strcpy( subfilepath, argv[i]);
			continue;
		}
		switch (argv[i][1]) {
		/* -D
		 *    Create directories as necessary during updating.
		 */
		case 'D':
			dirflag = 1;
			verboseflag = 1;
			break;
		/* -F dirname
		 *    Specify source "root" directory.
		 */
		case 'F':
			get_arg(fromroot,argv,&i);
			break;
		/* -p
		 *    Parse only.  Display a detailed list of the fields in the
		 * subscription file.
		 */
		case 'p':
			parseflag = 1;
			break;
		/* -S stackmax
		 *    Specify deeper path stacks
		 */ 
		case 'S':
			get_arg(scratch,argv,&i);
			sscanf( scratch, "%d", stackmax);
			break;
		/* -T dirname
		 *    Specify destination "root" directory.
		 */ 
		case 'T':
			get_arg(toroot,argv,&i);
			break;
		/* -d dirname
		 *    Specify the working directory for
		 *    the destination root system.
		 */
		case 'd':
			get_arg(twdir,argv,&i);
			break;
		/* -f
		 *    Force updating regardless of locks.
		 */
		case 'f':
			forceflag = 1;
			break;
		/* -i
		 *    Include devices in an update.
		 */
		case 'i':
			incl_devs = 1;
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
		/* -q
		 *    Be quiet about warning messages.
		 */
		case 'q':
			quietflag = 1;
			break;
		/* -r dirname
		 *    Specify the working directory for the source root system.
		 * If -r is not specified, it will default to the -d directory.
		 */
		case 'r':
			get_arg(fwdir,argv,&i);
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
	/*
	 * Get the proper working directories.
	 * By default, use the working directory in both of the root systems.
	 */
	if (!*fwdir)
		strcpy(fwdir,twdir);

	sprintf(scratch,"%s%s",toroot,twdir);
	strcpy(twdir,scratch);

	sprintf(scratch,"%s%s",fromroot,fwdir);
	strcpy(fwdir,scratch);

	/*
	**	redirect yacc/lex i/o
	*/
	parseinit( opensubfile( writeflag? fwdir : twdir));
	if (yyparse()) {
		strcpy(errmsg,"parser error");
		do_panic();
	}
	if (debug)
		printf("parse worked\n");

	if (parseflag) {  /* -p: Just show the fields */
		justshow();
		cleanup();
	}

	sort_entries();

	setlock();

	openstat( writeflag);

	if ( writeflag)		/* -w: Write the initial statfile */
		writestat();
	else	readstat();

	closestat();

	clearlocks();
}			/* end of main() */

#define pathtail( p) p[1+*(int*)p[CNT]]

readstat() {
	struct stat *cmpstatp, *remstatp, treestat;
	char statline[ LINELEN], remname[ LINELEN], remlink[ LINELEN];
	char **from, **to, **cmp;
	char *cmplink, *tail;

	from = initpath();
	to   = initpath();
	cmp  = initpath();

	pushpath( from, fromroot);
	pushpath( to,   toroot);
	pushpath( cmp,  toroot);

	init_next_match();

	while ( entnum = get_next_match( statline)) {

		/* extract the currency data from the statline:
		 */
		remstatp = dec_statfile( statline, remname, remlink);
		if ( S_IFMT == TYPE( *remstatp))
			continue;

		/* remname begins with entnum's fromfile:
		 */
		tail = remname + strlen( entries[ entnum].fromfile);

		if ( ! goodname( tail, entnum)) continue;

		/* construct the current entry's full pathnames,
		 * and extract the corresponding currency data for
		 * the local cmpfile:
		 */
		cmpstatp = dec_entry( entnum, from, to, cmp);

		/* dec_entry pushes pathname-qualification onto
		 * from, to, and cmp:
		 * the element NAME of the path 'from' indicates the start of
		 * entnum's fromfile-name;
		 * the element pathtail() indicates the end of the pathname.
		 * the same holds for the corresp. elements of 'to' & 'cmp'.
		 */

		/* add the tail onto the pathnames: don't need to push.
		 */
		strcpy( pathtail( from), tail);
		strcpy( pathtail( to), tail);

		/* a directory can have a non-dir as its cmpfile:
		 * we append tail to cmp only if cmp is a dir.
		 * we change stat buffers, so as not to trash
		 * dec_entry's value, which we'll need again.
		 */
		if ( S_IFDIR == TYPE( *cmpstatp) && *tail) {
			strcpy( pathtail( cmp), tail);
			if ( lstat( cmp[ ROOT], &treestat))
				treestat.st_mode = S_IFMT;	/* XXX */
			cmpstatp = &treestat;
		}
		if ( S_IFLNK != TYPE( *cmpstatp) ||
		     NULL == ( cmplink = follow_link( cmp[ ROOT])))
			cmplink = "";

		if ( update_file( remstatp, remlink, from[ ROOT],
				  cmpstatp, cmplink,   to[ ROOT]))
			continue;

		do_cmds( entries[entnum].cmdbuf, to[ ROOT]);
	}
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
	if ( close( creat( lockpath,0))) {
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
		sprintf( errmsg, "can't remove lockfile %s", lockpath);
		do_gripe();
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
	char **from, **cmp;
	struct stat *cmpstatp;

	from = initpath();
	cmp  = initpath();

	pushpath( from, fromroot);
	pushpath( cmp,  fromroot);

	for( entnum = 1; entnum < entrycnt; entnum++) {

		/* dec_entry pushes pathname-qualification
		 * onto the paths 'from' & 'cmp'.
		 */
		cmpstatp = dec_entry( entnum, from, NULL, cmp);

		/* write_statline returns fromfile's type:
		 */
		if      ( S_IFDIR != write_statline( from, cmpstatp));
		else if ( S_IFDIR != TYPE( *cmpstatp )) {
			/* a directory can use a non-dir as its cmp-file.
			 * the type bits must match, though, if it's to mean
			 * anything.
			 */
			(*cmpstatp).st_mode &= 0x7777;
			(*cmpstatp).st_mode |= S_IFDIR;		/* XXX */

			walk_trees( from, NULL,	cmpstatp);
		}
		else    walk_trees( from, cmp,  cmpstatp);

		/* WARNING: walk_trees alters ALL of its arguments */
	}
	/* sort the statfile, and write it out
	 * to the correct directory:
	 */
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
walk_trees( f, c, cmpstatp)
char *f[], *c[];
struct stat *cmpstatp;
{
	DIR *dirp;
	struct direct *dp;

	pushpath( f, "/");
	pushpath( c, "/");

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

		/* f[ TAIL] was the end of entnum's fromfile-name,
		 * when writestat called walk_trees.
		 * since that call, walk_trees has recursively appended
		 * more pathname-qualification to f.
		 * now, f[ TAIL] shows just these additions, without the
		 * original fromfile-name.
		 */
		if (! goodname( f[ TAIL], entnum));

		else if ( c && lstat( c[ ROOT], cmpstatp)) {
			sprintf(errmsg,"can't stat %s\n", c[ ROOT]);
			do_gripe();
			/* give up, goto poppath() calls */
		}
		/* write_statline returns fromfile's type:
		 */
		else if ( S_IFDIR == write_statline( f, cmpstatp))
			walk_trees( f, c, cmpstatp);

		poppath( f);
		poppath( c);
	}
	closedir(dirp);

	/* remove slashes:
	 */
	poppath( f);
	poppath( c);
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
		sprintf( namebuf,"%s/%s",fwdir,DEF_LOG);
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
	int i,j;

	for (i = 0; i < entrycnt;i++) {
		fprintf(stderr,
			"entry %d:\n\tfollow -- %d\n\tfromfile--|%s|\n",
			i,
			entries[i].followlink,
			entries[i].fromfile);
		fprintf(stderr,
			"\tcmpfile--|%s|\n\ttofile--|%s|\n\texceptions--\n",
			entries[i].cmpfile,
			entries[i].tofile);
		for(j=0;entries[i].exceptions[j] != (char*)0;j++)
			fprintf(stderr,"\t\t|%s|\n", entries[i].exceptions[j]);
		fprintf(stderr,"\tcommand--|%s|\n",entries[i].cmdbuf);
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

FILE *
opensubfile( workdir) char *workdir; {
	FILE *subfile;
        if ( ! *subfilepath)
		sprintf( subfilepath, "%s%s/%s",
			 workdir, DEF_SUBDIR, subfilename);
	fprintf( stderr, "using %s as subscription-list\n", subfilepath);
        if ( ! ( subfile = fopen( subfilepath, "r"))) {
                sprintf( errmsg, "Can't open subscriptionlist %s\n",
			 subfilepath);
                do_panic();
        }
	return( subfile);
}

openstat( write) int write;
{
	char *root = write? fwdir : twdir;
	char *mode = write? "w"   : "r";
	FILE *std =  write? stdout : stdin;

        if ( ! *statfilepath)
		sprintf( statfilepath, "%s%s/%s",
			 root, DEF_STATDIR, subfilename);

	fprintf( stderr, "using %s as statfile\n", statfilepath);

	if ( ! strcmp( statfilepath, "-"))
		statfile = std;
	else if ( ! ( statfile = fopen( statfilepath, mode))) {
		sprintf( errmsg, "can't open statfile %s\n", statfilepath);
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
