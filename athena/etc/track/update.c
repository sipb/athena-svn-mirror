/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 3.0 1988-03-09 13:17:18 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.2  88/01/29  18:24:18  don
 * bug fixes. also, now track can update the root.
 * 
 * Revision 2.1  87/12/03  17:33:18  don
 * fixed lint warnings.
 * 
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
*rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/update.c,v 3.0 1988-03-09 13:17:18 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"
#include <sys/errno.h>

#define DIFF( l, r, field) (short)(((l).sbuf.field) != ((r).sbuf.field))

struct currentness *
currency_diff( l, r) struct currentness *l, *r; {
	static struct currentness d;
	struct stat *s;
	int diff;

	/* we fill the difference structure d with boolean flags,
	 * a field being set indicating that l & r differ in that field.
	 */
	s = &d.sbuf;

	d.sbuf.st_mode =
		   DIFF( *l, *r, st_mode & ~S_IFMT) |		/* prot bits */
		   DIFF( *l, *r, st_mode &  S_IFMT) << 12;	/* type bits */
	UID( *s) = DIFF( *l, *r, st_uid);
	GID( *s) = DIFF( *l, *r, st_gid);

	diff = UID( *s) || GID( *s) || d.sbuf.st_mode;

	TIME( *s) = 0;
	DEV( *s) = 0;
	d.link = NULL;
	d.cksum = 0;

	switch ( TYPE( r->sbuf)) {

	case S_IFREG:
		TIME( *s) = DIFF( *l, *r, st_mtime);
		diff |= (uflag == DO_CLOBBER) || TIME( *s);
		diff |= d.cksum = ( cksumflag && l->cksum != r->cksum);
	case S_IFDIR: break;

	case S_IFLNK:
		d.link = (char *)( 0 != strcmp( r->link, l->link));
		diff = (int) d.link;
		break;

	case S_IFCHR:
	case S_IFBLK:
		if (! incl_devs) return( NULL);
		DEV( *s) = DIFF( *l, *r, st_dev);
		diff != DEV( *s);
		break;

	case S_IFMT:
		return ( NULL);
	default:
		sprintf(errmsg,"bad string passed to update\n");
		do_panic();
		break;
	}

	return( diff? &d : NULL);
}
int
update_file(l, lpath,
	    r, rpath)
char **lpath, **rpath;
struct currentness *r, *l;
{
	char *remotename = rpath[ ROOT], *localname = lpath[ ROOT];
	struct currentness *diff;
	struct stat lstat;
	struct timeval *timevec;
	unsigned int oumask;
	unsigned int exists, local_type, remote_type, same_name;

	/* if the cmpfile doesn't exist, and tofile != cmpfile,
	 * give up, since we want to be conservative about updating.
	 */
	same_name = ! strcmp( lpath[ NAME], l->name);

	if ( S_IFMT == TYPE( l->sbuf) && ! same_name) {
		sprintf( errmsg, "nonexistent comparison-file %s\n", l->name);
		errno = 0;
		do_gripe();
		return( -1);
	}
	/* either: cmpfile != tofile, and cmpfile exists,
	 *     or: cmpfile == tofile, & the file might exist or not.
	 */
	local_type = same_name ? TYPE( l->sbuf) :
		     (*statf)( lpath[ ROOT], &lstat) ? S_IFMT : TYPE( lstat);

	exists = S_IFMT != local_type;

	diff = currency_diff( l, r);

	/* that diff == NULL doesn't mean localname exists,
	 * since l may represent a different file.
	 */
	if ( ! diff && exists) {
		return( -1);
	}

	if ( verboseflag) banner( remotename, localname, r, l, diff);

	/* if fromfile != the remote cmpfile, 
	 * we need to extract fromfile's real currency info.
	 * note that we compare the "unmounted" version of remotename,
	 * since r->name comes from the statfile, and so doesn't
	 * include the mountpoint path-component.
	 */
	if ( strcmp( r->name,    rpath[ NAME]))
		get_currentness( rpath, r);

	remote_type = TYPE( r->sbuf);

	/* same_name == 1 means that we're updating the cmpfile.
	 * if this cmpfile is the entry's top-level cmpfile,
	 * then we need to update the entry's currency-info.
	 */
	if ( same_name && nopullflag) { /* simulate cmpfile's update */
		l->sbuf.st_mode  = r->sbuf.st_mode;
		l->sbuf.st_uid   = r->sbuf.st_uid;
		l->sbuf.st_gid   = r->sbuf.st_gid;
		l->sbuf.st_rdev  = r->sbuf.st_rdev;
		l->sbuf.st_mtime = r->sbuf.st_mtime;
		l->link		 = r->link;
		l->cksum	 = r->cksum;
	}
	else if ( same_name) /* nopullflag off; usual case */
		/* mark the currency as "out-of-date",
		 * in case it's an entry's currency, because
		 * dec_entry() reuses each entry's currencies repeatedly.
		 * dec_entry() will refresh the entry's currency,
		 * if it sees this mark.
		 */
		l->sbuf.st_mode = 0;

	if ( nopullflag) return(-1);

	/* if tofile is supposed to be a dir,
	 * we can create its whole path if necessary;
	 * otherwise, its parent must exist:
	 */
	if ( S_IFDIR == remote_type || exists);
	else if ( findparent( localname)) {
		sprintf(errmsg,"can't find parent directory for %s",
			localname);
		do_gripe();
		return(-1);
	}
	/* if fromfile & tofile aren't of the same type,
	 * delete tofile, and record the deed.
	 */
	if ( local_type == remote_type);
	else if ( exists && removeit( localname, local_type)) return( -1);
	else exists = 0;

	/* at this stage, we know that if localfile still exists,
	 * it has the same type as its remote counterpart.
	 */

	switch ( TYPE( r->sbuf)) {

	case S_IFREG:
		/* the stat structure happens to contain
		 * a timevec structure, because of the spare integers
		 * that follow each of the time fields:
		 */
		r->sbuf.st_atime = TIME( r->sbuf);
		timevec = (struct timeval *) &r->sbuf.st_atime; /* XXX */

		/* it's important to call utimes before set_prots:
		 */
		if ( copy_file(   remotename, localname)) return( -1);
		else    utimes(    localname, timevec);
		return( set_prots( localname, &r->sbuf));

	case S_IFLNK:
                if ( exists && removeit( localname, S_IFLNK))
			return( -1);

		oumask = umask(0); /* Symlinks don't really have modes */
		if ( symlink( r->link, localname)) {
			sprintf(errmsg, "can't create symbolic link %s -> %s\n",
				localname, r->link);
			do_gripe();
			umask(oumask);
			return(-1);
		}
		umask(oumask);
		return( 0);

	case S_IFDIR:
		return( exists	? set_prots( localname, &r->sbuf)
				: makepath(  localname, &r->sbuf));
	case S_IFBLK:
	case S_IFCHR:
		if ( !exists && mknod( localname, remote_type, DEV( r->sbuf))) {
			sprintf(errmsg, "can't make device %s\n", localname);
			do_gripe();
			return(-1);
		}
		return( set_prots( localname, &r->sbuf));

	case S_IFMT: /* this message could ask the user whether to continue: */
		sprintf( errmsg, "fromfile %s doesn't exist.\n", remotename);
		do_gripe();
		return( -1);
	default:
		sprintf(errmsg, "unknown file-type in update_file()\n");
		do_gripe();
		return(-1);
	}
	sprintf( errmsg, "ERROR (update_file): internal error.\n");
	do_panic();
	/*NOTREACHED*/
	return( -1);
}

get_currentness( path, c) char **path; struct currentness *c; {
        strcpy( c->name, path[ NAME]);
        c->cksum = 0;
        c->link = "";
        if ( (*statf)( path[ ROOT], &c->sbuf)) {
		clear_stat( &c->sbuf);
                c->sbuf.st_mode = S_IFMT;       /* XXX */
		if ( errno != ENOENT) {
			sprintf( errmsg,"can't %s comparison-file %s\n",
				 statn, path[ ROOT]);
			do_gripe();
		}
                return( -1);
        }
        switch ( TYPE( c->sbuf)) {
        case S_IFREG:
		if ( writeflag || cksumflag)
			c->cksum = in_cksum( path[ ROOT], &c->sbuf);
                break;
        case S_IFLNK:
                if ( !( c->link = follow_link( path[ ROOT]))) {
                        c->link = "";
                        return( -1);
                }
                break;
        default:
                break;
        }
        return( 0);
}

clear_stat( sp) struct stat *sp; {
	int *p;;

	/* it happens that  a stat is 16 long integers;
	 * we exploit this fact for speed.
	 */
	for ( p = (int *)sp + sizeof( struct stat) / 4; --p >= (int *)sp;)
		*p = 0;
}

int
set_prots( name, r)
char *name;
struct stat *r;
{
	struct stat sbuf;
	int error = 0;

	if ( (*statf)( name, &sbuf)) {
		sprintf( errmsg, "(set_prots) can't %s &s\n", statn, name);
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
	int cc, fdf, fdt;

	fdf = open(from,O_RDONLY);
	if ( 0 > fdf) {
		sprintf(errmsg,"can't open input file %s\n",from);
		do_gripe();
		return(-1);
	}
	sprintf( temp,"%s_trk.tmp",to);

	if ( 0 <= ( fdt = open( temp, O_WRONLY | O_CREAT)));

	else if ( errno == ENOSPC) {
		/* creates will fail before writes do,
		 * when the disk is full.
		 */
		sprintf( errmsg, "no room for temp file %s\n", temp);
		do_panic();
	}
	else {
		sprintf(errmsg,"can't open temporary file %s\n",temp);
		do_gripe();
		close( fdf);
		return(-1);
	}
	while ( 0 < ( cc  = read(  fdf, buf, sizeof buf)))
		if  ( cc != write( fdt, buf, cc))
			break;

	close(fdf);
	close(fdt);

	switch( SIGN( cc)) {
	case 0: break;
	case -1:
		unlink( temp);
		sprintf( errmsg,"error while reading file %s\n",from);
		do_gripe();
		return(-1);
	case 1:
		unlink( temp);
		sprintf( errmsg,"error while writing file %s\n",temp);
		do_panic();
	}
	if ( rename( temp, to)) { /* atomic! */ 
		sprintf( errmsg, "rename( %s, %s) failed!\n", temp, to);
		do_panic();
	}
	return (0);
}

/* this array converts stat()'s type-bits to a character string:
 * to make the index, right-shift the st_mode field by 13 bits.
 */
static char *type_str[] = { "ERROR", "char-device", "directory",
	"block-device", "file", "symlink", "socket(ERROR)", "nonexistent"
};

banner( rname, lname, r, l, d)
char *rname, *lname;
struct currentness *r, *l, *d;
{
	unsigned int n = 0, m = 0;
	struct stat *ds = &d->sbuf, *ls = &l->sbuf, *rs = &r->sbuf;
	char *format, *ltype, *rtype, *p;
	char fill[ LINELEN], *lfill = "", *rfill = "";
	int dlen;

	ltype = type_str[ TYPE( *ls) >> 13];
	rtype = type_str[ TYPE( *rs) >> 13];

	dlen =	strlen( lname) - strlen( rname) +
		strlen( ltype) - strlen( rtype);
	
	/* we need to align the ends of the filenames,
	 * "Updating ... lname" &
	 * "    from ... rname".
	 */
	
	if ( dlen < 0) {
		dlen *= -1;
		lfill = fill;
	}
	else	rfill = fill;

	for ( p = fill ; dlen > 0; --dlen) *p++ = ' ';
	*p = '\0';

	if	  ( TYPE( *ds)) {
		format =	"%s %s%s %s\n"; }
	else if   ( TIME( *ds)) {
		n = TIME( *ls);
		m = TIME( *rs);
		format =	"%s %s%s %s ( mod-time=%d)\n"; }
	else if	  (      d->link) {
		n = (int)l->link;
		m = (int)r->link;
		format =	"%s %s%s %s ( symlink -> %s)\n"; }
	else if   ( d->cksum) {
		n = l->cksum;
		m = r->cksum;
		format =	"%s %s%s %s ( file-cksum=%4.x)\n"; }
	else if   ( MODE( *ds)) {
		n = MODE( *ls);
		m = MODE( *rs);
		format =	"%s %s%s %s ( mode-bits=%4.o)\n"; }
	else if   (  UID( *ds)) {
		n =  UID( *ls);
		m =  UID( *rs);
		format =	"%s %s%s %s ( user-id=%d)\n"; }
	else if   (  GID( *ds)) {
		n =  GID( *ls);
		m =  GID( *rs);
		format =	"%s %s%s %s ( group-id=%d)\n"; }
	else if   (  DEV( *ds)) {
		n =  DEV( *ls);
		m =  DEV( *rs);
		format =	"%s %s%s %s ( device-type=%d)\n"; }

	fprintf( stderr, format, "Updating", ltype, lfill, lname, n);
	fprintf( stderr, format, "    from", rtype, rfill, rname, m);
}
