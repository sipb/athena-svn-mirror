/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 2.0 1987-11-30 15:19:24 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/02/12  21:14:49  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/files.c,v 2.0 1987-11-30 15:19:24 don Exp $";
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

/* 
isdir(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'd');

	if (debug)
		printf("isdir(%s): %d\n",name,retval);

	return(retval);
}

isfile(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'f');

	if (debug)
		printf("isfile(%s): %d\n",name,retval);

	return (retval);
}

zerolen(name)
char *name;
{
	struct stat sbuf;
	int retval;

	if(stat(name,&sbuf) == -1) {
		sprintf(errmsg,
			"can't find file %s in routine zerolen()\n",
			name);
		do_panic();
	}

	retval = sbuf.st_size;

	if (debug)
		printf("zerolen(%s): %d\n",name,retval);

	return (retval);
}

gettype(name)
char *name;
{
	struct stat sbuf;
	int retval;

	if (!lstat(name,&sbuf)) {
		switch(sbuf.st_mode & S_IFMT) {
			case S_IFDIR:
				retval = 'd';
				break;
			case S_IFREG:
				retval = 'f';
				break;
			case S_IFLNK:
				retval = 'l';
				break;
			case S_IFCHR:
				retval = 'c';
				break;
			case S_IFBLK:
				retval = 'b';
				break;
			default:
				sprintf(errmsg,
					"gettype -- bad file mode for %s\n",
					name);
				do_panic();
		}
	}
	else
		retval = '*';

	if (debug)
		printf("gettype(%s): %c\n",name,retval);

	return (retval);
}

islink(name)
char *name;
{
	int retval;

	retval = (gettype(name) == 'l');

	if (debug)
		printf("islink(%s): %d\n",name,retval);

	return (retval);
}
*/

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
	char *linkval = name;	/* return unqualified name if it's not link */
	struct stat sbuf;

	sprintf( path, "%s%s", writeflag ? fromroot : toroot, name);

	while ( 1) {
		if ( lstat( path, &sbuf)) {
			sprintf( errmsg, "can't lstat %s\n", path);
			do_gripe();
			return("");
		}
		if ( S_IFLNK != TYPE( sbuf))
			return( linkval);

		if ( NULL == ( linkval = follow_link( path)))
			return( name);	/* back out. something's broken */

		strcpy( path, linkval);
	}
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
	else if ( lstat( name, &sbuf)) {
		sprintf( errmsg, "can't lstat %s\n", name);
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
