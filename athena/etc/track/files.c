/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 3.0 1988-03-09 13:17:41 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.2  88/01/29  18:23:52  don
 * bug fixes. also, now track can update the root.
 * 
 * Revision 2.1  87/12/03  17:33:39  don
 * fixed lint warnings.
 * 
 * Revision 2.0  87/11/30  15:19:24  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 * Revision 1.1  87/02/12  21:14:49  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 3.0 1988-03-09 13:17:41 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
 * Check to see if a file is older than a certain amount of time.
 */

int
too_old( name, maxtime)
char *name;
long maxtime;
{
	struct stat sbuf;
	long now;
	int retval;

	time(&now);
	if(stat(name,&sbuf) == -1){
		sprintf(errmsg,
			"can't find file %s in routine too_old()",
			name);
		do_panic();
	}

	retval = ((sbuf.st_mtime + maxtime) <= now);

	if (debug)
		printf("too_old(%s,%ld): %d\n",name,maxtime,retval);

	return (retval);
}

char *follow_link( name)
char *name;
{
        static char target[ LINELEN];
        int cc;
        if (0 >= ( cc = readlink( name, target, LINELEN))) {
                sprintf( errmsg, "can't read link: %s\n", name);
                do_gripe();
                return( NULL);
        }
        target[ cc] = '\0';

        if ( verboseflag)
                fprintf( stderr, "following link: %s -> %s\n", name, target);

        return( target);
}

char *resolve(name)
char *name;
{
	/* only called during parsing,
	 * to trace links provided as cmpfiles,
	 * and then only if the sublist entry starts with '!'.
	 */
	static char path[ LINELEN];
	static char home[ LINELEN] = "";
	char *end, *linkval = path;
	struct stat sbuf;

	sprintf( path, "%s/%s", fromroot, name);
	if( stat( path, &sbuf)) {
		sprintf( errmsg, "can't stat %s/%s\n", fromroot, name);
		do_gripe();
		return("");
	}
	if ( ! *home) getwd( home);

	while ( 1) {
		if ( lstat( path, &sbuf)) {
			sprintf( errmsg, "can't lstat %s\n", path);
			do_gripe();
			chdir( home);
			return("");
		}
		if ( S_IFLNK != TYPE( sbuf))
			break;

		if ( NULL == ( linkval = follow_link( path))) {
			/* back out. something's broken */
			chdir( home);
			return( name);
		}
		if ( *linkval != '/' && ( end = rindex( path, '/'))) {
			/* linkval isn't an absolute pathname, and
			 * we're not already in path's parent dir.
			 * relocate to the parent, so we can lstat linkval:
			 */
			*end = '\0';
			chdir( path);
		}
		strcpy( path, linkval);
	}
	/* reduce  linkval to its optimal absolute pathname:
	 * we do this by chdir'ing to linkval's parent-dir,
	 * so that getwd() will optimize for us.
	 */
	if ( *linkval == '/');
	else if ( end = rindex( linkval, '/')) {
		/* make relative path to parent
		 */
		*end = '\0';
		getwd(  path);
		strcat( path, "/");
		strcat( path, linkval);
		if ( chdir( path)) {
			sprintf( errmsg, "can't resolve link %s/%s\n",
				 fromroot, name);
			do_gripe();
			chdir( home);
			return("");
		}
		getwd( path);
		*end = '/';
		strcat( path, end);
		linkval = path;
	}
	else {			/* path is linkval's parent */
		getwd(  path);
		strcat( path, "/");
		strcat( path, linkval);
		linkval = path;
	}
	if ( strncmp( linkval, fromroot, strlen( fromroot))) {
		sprintf( errmsg, "link %s->%s value not under mountpoint %s\n",
			 name, linkval, fromroot);
		do_gripe();
		linkval = "";
	}
	linkval += strlen( fromroot) + 1;
	chdir( home);
	return( linkval);
}

/*
 * if the parent-dir exists, returns 0.
 * returns the length of the parent's pathname, otherwise.
 * this weird returned-value allows makepath
 * to recurse without lots of strcpy's.
 */
int
findparent(path)
char *path;
{
	char *tmp;
	int retval;

	/*
	**	assume / exists
	*/
	if (! strcmp( path,"/"))
		return(-1);

	if (!( tmp = rindex( path,'/'))) {
		sprintf(errmsg,"checkroot can't find / in %s", path);
		do_gripe();
		return(-1);
	}
	if ( tmp == path) return( 0);	/* root is the parent-dir */

	*tmp = '\0';
	retval = access( path, 0) ? tmp - path - 1 : 0;
	*tmp = '/';
	return( retval);
}

int makepath( path,s)
char *path;
struct stat *s;
{
	char parent[ LINELEN];
	int n, usave;

	if ( 0 < ( n = findparent( path))) {
		parent[ n] = '\0';
		if ( makepath( parent, s)) return( -1);
		parent[ n] = '/';
	}
	else if ( 0 > n) return( -1);

        if ( verboseflag)
                fprintf( stderr,"making root directory %s mode %o\n",
                         path, MODE( *s));

        usave = umask( 022);

        if ( mkdir( path, MODE( *s))) {
                sprintf(errmsg, "can't create directory %s", path);
                do_gripe();
                umask( usave);
                return(-1);
        }
        umask( usave);

        if ( set_prots( path, s)) return(-1);

        return (0);
}

int
removeit(name, type)
char *name;
unsigned int type;
{
	struct direct *next;
	DIR *dirp;
	char *leaf, *type_str = "";
	struct stat sbuf;

	/* caller can pass us the file-type, if he's got it:
	 */
	if ( type != 0);
	else if ( (*statf)( name, &sbuf)) {
		sprintf( errmsg, "(removeit) can't %s %s\n", statn, name);
		do_gripe();
		return(-1);
	}
	else type = TYPE( sbuf);

	if (verboseflag) {
		switch( type) {
		case S_IFBLK: type_str = "block special device";	break;
		case S_IFCHR: type_str = "char special device";		break;
		case S_IFDIR: type_str = "directory";			break;
		case S_IFREG: type_str = "regular file";		break;
		case S_IFLNK: type_str = "symbolic link";		break;
		}
		fprintf(stderr,"removing %s %s\n",type_str,name);
	}
	if ( type == S_IFDIR);
	else if ( unlink( name)) {
		sprintf( errmsg, "can't remove %s %s", type_str, name);
		do_gripe();
		return(-1);
	}
	else return(0);

	if (!(dirp = (DIR *) opendir( name))) {
		sprintf(errmsg, "removeit: error from opendir of %s", name);
		do_gripe();
		return(-1);
	}
	readdir( dirp); readdir( dirp); /* skip . & .. */

	strcat( name,"/");		/* XXX: don't copy for recursive call */
	leaf =  name + strlen( name);
	for( next = readdir(dirp); next != NULL; next = readdir(dirp)) {
		strcpy( leaf, next->d_name);	/* changes name[] */
		removeit( name, 0);
	}
	leaf[-1] = '\0';	/* XXX: see strcat, above */

	if ( rmdir( name) == -1) {
		sprintf(errmsg, "can't remove directory %s",name);
		do_gripe();
		return(-1);
	}
	return(0);
}
