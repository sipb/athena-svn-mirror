/*
 *	$Id: files.c,v 4.12 1999-01-22 23:15:59 ghudson Exp $
 */

#ifndef lint
static char *rcsid_header_h = "$Id: files.c,v 4.12 1999-01-22 23:15:59 ghudson Exp $";
#endif lint

#include "mit-copyright.h"
#include "bellcore-copyright.h"

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

follow_link( name, retval)
char *name, *retval;
{
        int cc;
        if (0 >= ( cc = readlink( name, retval, LINELEN))) {
                sprintf( errmsg, "can't read link: %s\n", name);
                do_gripe();
                return( -1);
        }
        retval[ cc] = '\0';

	/*
        if ( verboseflag)
                fprintf( stderr, "following link: %s -> %s\n", name, retval);
	*/

        return( 0);
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

	if (!( tmp = strrchr( path,'/'))) {
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
#ifdef POSIX
	struct dirent *next;
#else
	struct direct *next;
#endif
	DIR *dirp;
	char *leaf, *type_str = "";
	struct stat sbuf;

	/* caller can pass us the file-type, if he's got it:
	 */
	if ( type != 0);
	else if ( lstat( name, &sbuf)) {
		sprintf( errmsg, "(removeit) can't lstat %s\n", name);
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

	strcat( name,"/");		/* XXX: don't copy for recursive call */
	leaf =  name + strlen( name);
	for( next = readdir(dirp); next != NULL; next = readdir(dirp)) {
		if ((! next->d_name[0]) ||	/* "" */
		    ((next->d_name[0] == '.') &&
		     ((! next->d_name[1]) ||	/* "." */
		      ((next->d_name[1] == '.') &&
		       (! next->d_name[2])))))	/* ".." */
			continue;
		strcpy( leaf, next->d_name);	/* changes name[] */
		removeit( name, 0);
	}
	leaf[-1] = '\0';	/* XXX: see strcat, above */

	(void) closedir(dirp);

	if ( rmdir( name) == -1) {
		sprintf(errmsg, "can't remove directory %s",name);
		do_gripe();
		return(-1);
	}
	return(0);
}
