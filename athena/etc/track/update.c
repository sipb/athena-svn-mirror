/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 2.1 1987-12-03 17:33:18 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.1  87/12/01  20:56:26  don
 * robustified file-updating, so that more failure-checking is done,
 * and so that if either utimes() or  set_prots() fails, the other will
 * still run.  made double-sure that protections & time don't
 * change, unless copy-file succeeds.
 * 
 * Revision 2.0  87/11/30  15:19:35  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 * Revision 1.2  87/09/02  17:44:20  shanzer
 * Aborts if We get a write error when copying a file.. 
 * 
 * Revision 1.1  87/02/12  21:16:00  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char
*rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 2.1 1987-12-03 17:33:18 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

int
update_file(r, remlink, remotename,
	    l, loclink, localname)
struct stat *r, *l;
char *remlink, *loclink, *remotename, *localname;
{
	int exists, oumask;
	int diff, type_diff;
	struct timeval *timevec;

	diff =  UID( *r)  != UID( *l) ||
		GID( *r)  != GID( *l) ||
		MODE( *r) != MODE( *l);

	type_diff = TYPE( *r) != TYPE( *l);

	switch ( TYPE( *r)) {

	case S_IFREG:
		diff |= (uflag     == DO_CLOBBER)  ||
			 TIME( *r) != TIME( *l); /*    ||
			 strcmp( remotename, localname); */
	case S_IFDIR: break;

	case S_IFCHR:
	case S_IFBLK:
		if (! incl_devs) return(-1);
		type_diff |= ( DEV( *r) != DEV( *l));
		break;

	case S_IFLNK:
		diff = strcmp( remlink, loclink);
		break;
	case S_IFMT:
		return (-1);
	default:
		sprintf(errmsg,"bad string passed to update\n");
		do_panic();
		break;
	}

	if (!diff && !type_diff) return(-1);

	if ( verboseflag) {
		fprintf(stderr,"Updating: source - %s\n",
			make_name( remotename, r, remlink));
		fprintf(stderr,"            dest - %s\n",
			make_name( localname,  l, loclink));
	}
	if ( nopullflag)
		return(-1);

	exists = !access( localname, 0);

	if ( S_IFDIR != TYPE( *l) && !exists && findparent( localname)) {
		sprintf(errmsg,"can't find parent directory for %s",
			localname);
		do_gripe();
		return(-1);
	}

	if ( type_diff && exists && removeit( localname, TYPE( *l))) {
		sprintf( errmsg,"can't remove %s\n",localname);
		do_gripe();
		return(-1);
	}

	switch ( TYPE( *r)) {

	case S_IFREG:
		/* the stat structure happens to contain
		 * a timevec structure, becuse of the spare integers
		 * that follow each of the time fields:
		 */
		(*r).st_atime = TIME( *r);
		timevec = (struct timeval *) &(*r).st_atime; /* XXX */

		/* it's important to call utimes before set_prots:
		 */
		if ( copy_file( remotename, localname)) return( -1);
		else    utimes(  localname, timevec);
		return( set_prots( localname, r));

	case S_IFDIR:
		return( exists	? set_prots( localname, r)
				: makepath(  localname, r));

	case S_IFBLK:		/* XXX: stat.st_mode != MODE() macro */
	case S_IFCHR:
		if ( !exists &&
		     mknod( localname, MODE( *r), DEV( *r))) {
			sprintf(errmsg,"can't make device %s\n",localname);
			do_gripe();
			return(-1);
		}
		return( set_prots( localname, r));

	case S_IFLNK:
		oumask = umask(0); /* Symlinks don't really have modes */
		if ( symlink( remlink, localname)) {
			sprintf(errmsg, "can't create symbolic link %s -> %s\n",
				localname, remlink);
			do_gripe();
			return(-1);
		}
		umask(oumask);
		return( 0);
	default:
		sprintf(errmsg, "unknown file-type in update_file()\n");
		do_gripe();
		return(-1);
	}
}

int
set_prots( name, r)
char *name;
struct stat *r;
{
	struct stat sbuf;
	int error = 0;

	if ( lstat( name, &sbuf)) {
		sprintf( errmsg, "can't lstat file &s\n", name);
		do_gripe();
		return(-1);
	}
	if (( UID( sbuf) != UID( *r) || GID( sbuf) != GID( *r)) &&
	     chown( name,   UID( *r),   GID( *r)) ) {
		sprintf( errmsg, "can't chown file %s %d %d\n",
			 name, UID( *r), GID( *r));
		do_gripe();
		error = 1;
	}
	if ( MODE( sbuf) != MODE( *r) && chmod( name,  MODE( *r)) ) {
		sprintf( errmsg, "can't chmod file %s %o\n", name, MODE( *r));
		do_gripe();
		error = 1;
	}
	return( error);
} 

copy_file(from,to)
char *from,*to;
{
	char buf[MAXBSIZE],temp[LINELEN];
	int fdf,fdt,n;

	fdf = open(from,O_RDONLY);
	if ( 0 > fdf) {
		sprintf(errmsg,"can't open input file %s\n",from);
		do_gripe();
		return (1);
	}
	sprintf( temp,"%s_trk.tmp",to);
	fdt = open( temp,O_WRONLY|O_CREAT);
	if ( 0 > fdt) {
		sprintf(errmsg,"can't open temporary file %s\n",temp);
		do_gripe();
		return (1);
	}

	for (;;) {
		n = read(fdf,buf,sizeof buf);
		if (!n)
			break;
		if (n < 0) {
			sprintf(errmsg,"error while reading file %s\n",from);
			do_gripe();
			close(fdf);
			close(fdt);
			unlink( temp);
			return (1);
		}
		if (write(fdt,buf,n) != n) {
			sprintf(errmsg,"error while writing file %s\n",temp);
			do_gripe();
			close(fdf);
			close(fdt);
			unlink( temp);
			exit(1);
		}
	}
	close(fdf);
	close(fdt);
	if ( rename( temp, to)) { /* atomic! */ 
		sprintf( errmsg, "rename( %s, %s) failed!\n", temp, to);
		do_panic();
	}
	return (0);
}

/* can't be called twice in one printf,
 * because it stores its result in static buffer.
 */
char *make_name( name, s, link)
char *name, *link;
struct stat *s;
{
	static char buff[LINELEN];

	switch ( TYPE( *s)) {
	case S_IFREG:
		sprintf(buff,"file %s (uid %d, gid %d, mode %04o)",
			name, UID( *s), GID( *s), MODE( *s));
		break;

	case S_IFDIR:
		sprintf(buff,"dir %s (uid %d, gid %d, mode %04o)",
			name, UID( *s), GID( *s), MODE( *s));
		break;

	case S_IFCHR:
	case S_IFBLK:
		sprintf( buff,
			"device %s (uid %d, gid %d, mode %03o) maj %d min %d",
			name, UID( *s), GID( *s), MODE( *s),
			major( DEV( *s)), minor( DEV( *s)));
		break;

	case S_IFLNK:
		sprintf(buff,"link %s pointing to %s", name, link);
		break;

	case S_IFMT:
		sprintf(buff,"nonexistant %s",name);
		break;
	}
	return (buff);
}
