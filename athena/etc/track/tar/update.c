/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/update.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/update.c,v 1.1 1993-10-12 04:48:56 probe Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 4.4  88/06/13  16:27:24  don
 * fixed a bug in device-handling: if device major/minor #'s are wrong,
 * have to delete the node & call mknode().
 * 
 * Revision 4.3  88/06/10  15:55:29  don
 * fixed a bug in device-handling: update  was triggered by differences in
 * st_dev, rather than st_rdev.
 * 
 * Revision 4.2  88/05/27  20:15:21  don
 * fixed two bugs at once: the nopulflag bug, and -I wasn't preserving
 * mode-bits at all. see update().
 * 
 * Revision 4.1  88/05/25  21:42:09  don
 * added -I option. needs compatible track.h & track.c.
 * 
 * Revision 4.0  88/04/14  16:43:30  don
 * this version is not compatible with prior versions.
 * it offers, chiefly, link-exporting, i.e., "->" systax in exception-lists.
 * it also offers sped-up exception-checking, via hash-tables.
 * a bug remains in -nopullflag support: if the entry's to-name top-level
 * dir doesn't exist, update_file doesn't get over it.
 * the fix should be put into the updated() routine, or possibly dec_entry().
 * 
 * Revision 3.0  88/03/09  13:17:18  don
 * this version is incompatible with prior versions. it offers:
 * 1) checksum-handling for regular files, to detect filesystem corruption.
 * 2) more concise & readable "updating" messages & error messages.
 * 3) better update-simulation when nopullflag is set.
 * 4) more support for non-default comparison-files.
 * finally, the "currentness" data-structure has replaced the statbufs
 * used before, so that the notion of currency is more readily extensible.
 * note: the statfile format has been changed.
 * 
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
*rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/update.c,v 1.1 1993-10-12 04:48:56 probe Exp $";
#endif lint

#include "bellcore-copyright.h"
#include "mit-copyright.h"

#include "track.h"
#include <sys/errno.h>

/* this array converts stat()'s type-bits to a character string:
 * to make the index, right-shift the st_mode field by 13 bits.
 */
static char *type_str[] = { "ERROR", "char-device", "directory",
	"block-device", "file", "symlink", "socket(ERROR)", "nonexistent"
};

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

	if ( ignore_prots) s->st_mode = UID( *s) = GID( *s) = 0;
	else {
	    s->st_mode	= DIFF( *l, *r, st_mode & ~S_IFMT);	/* prot bits */
	    UID( *s)	= DIFF( *l, *r, st_uid);
	    GID( *s)	= DIFF( *l, *r, st_gid);
	}
	s->st_mode |= DIFF( *l, *r, st_mode & S_IFMT) << 12;	/* type bits */

	diff = UID( *s) || GID( *s) || s->st_mode;

	TIME( *s) = 0;
	RDEV( *s) = 0;
	*d.link = '\0';
	d.cksum = 0;

	switch ( TYPE( r->sbuf)) {

	case S_IFREG:
		TIME( *s) = DIFF( *l, *r, st_mtime);
		diff |= (uflag == DO_CLOBBER) || TIME( *s);
		diff |= d.cksum = ( cksumflag && l->cksum != r->cksum);
	case S_IFDIR: break;

	case S_IFLNK:
		*d.link = (char)( 0 != strcmp( r->link, l->link));
		diff = (int) *d.link;
		break;

	case S_IFCHR:
	case S_IFBLK:
		RDEV( *s) = DIFF( *l, *r, st_rdev);
		diff |= RDEV( *s);
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
	struct stat lstat, *local_statp;
	struct timeval *timevec;
	static List_element *missing_dirs;
	List_element *p;
	unsigned int oumask;
	unsigned int exists, local_type, remote_type, same_name;

	diff = currency_diff( l, r);

	/* if the cmpfile doesn't exist, and tofile != cmpfile,
	 * give up, since we want to be conservative about updating.
	 */
	same_name = ! strcmp( l->name, lpath[ NAME]);

	if ( S_IFMT == TYPE( l->sbuf) && ! same_name) {
		sprintf( errmsg, "%s%s.\n%s%s.\n",
			 "nonexistent local comparison-file ", l->name,
			 "trying to update from ", rpath[ ROOT]);
		errno = 0;
		do_gripe();
		return( -1);
	}
	/* either: cmpfile != tofile, and cmpfile exists,
	 *     or: cmpfile == tofile, & the file might exist or not.
	 */
	local_statp = same_name ? &l->sbuf :
		     (*statf)( lpath[ ROOT], &lstat) ? NULL : &lstat;
	local_type = local_statp ? TYPE( *local_statp) : S_IFMT;
	exists = local_type != S_IFMT;

	/* that diff == NULL doesn't mean localname exists,
	 * since l may represent a different file.
	 */
	if ( ! diff && exists) return( -1);

	if ( verboseflag) banner( remotename, localname, r, l, diff);

	/* speed hack: when we can, we avoid re-extracting remote currentness.
	 * reasons that we can't:
	 * 1) if fromfile != the remote cmpfile,
	 * we need to extract fromfile's real currency info.
	 * 2) if nopullflag is set, we need to simulate real update of files.
	 * 3) if fromfile is a dir or device, we need to ensure that it's real.
	 * XXX: for detecting case 1, dec_statfile() makes
	 *	strcmp( r->name, rpath[ NAME]) unnecessary:
	 *      if '~' was in statline, r->name contains "".
	 *      if '=' was in statline, r->name contains rpath[ NAME].
	 */
	remote_type = TYPE( r->sbuf);
	do {
	    if ( ! *r->name); /* remote_type isn't necessarily rpath's type. */
	    else switch( remote_type) {
		 case S_IFREG: if ( nopullflag) break;	 /* call get_curr(). */
		 case S_IFLNK: continue; /* leave do block, skip get_curr(). */
		 default:      break;	 /* dir's & dev's must exist. */
	    }
	    if ( get_currentness( rpath, r)) {
		sprintf( errmsg,
			 "master-copy doesn't exist:\n\t%s should be a %s.\n",
			 rpath[ ROOT], type_str[ remote_type >> 13]);
		do_gripe();

		/* for nopullflag, 
		 * maintain list of dir's whose creation would fail,
		 * so we can see whether lpath's parent "exists".
		 * the only predictable reason that we wouldn't create a dir
		 * is if the corresponding remote-dir doesn't exist.
		 */
		if ( remote_type == S_IFDIR) {
			 pushpath( lpath, "");	/* append slash */
			 add_list_elt( lpath[ ROOT], 0, &missing_dirs);
			 poppath( lpath);	/* remove slash */
		}
		return( -1);
	    }
	    else remote_type = TYPE( r->sbuf);
	} while( 0); /* just once */

	/* if cmpfile == tofile, then we're updating the cmpfile.
	 * just in case this cmpfile is some entry's top-level cmpfile,
	 * we need to update that entry's currency-info for dec_entry().
	 * because dec_entry() reuses each entry's currencies repeatedly.
	 */
	if ( same_name) updated( l, r);

	if ( ! nopullflag);
	else if ( S_IFDIR == remote_type || exists)
	    return(-1);
	else {
	    /* simulate findparent():
	     * search missing_dirs list for ancestors of lpath.
	     * we must search the whole list,
	     * because we can't assume that the list is in recognizable order:
	     * the list contains tofile's, sorted in order of decreasing
	     * fromfile-name.
	     */
	    for ( p = missing_dirs; p; p = NEXT( p))
		if ( ! strncmp( lpath[ ROOT], TEXT( p), strlen( TEXT( p)))) {
		    sprintf(errmsg,"%s %s,\n\t%s %s.\n",
			    "would't find parent directory for", localname,
			    "because track previously failed to create",
			    TEXT( p));
		    do_gripe();
		    break;
		}
	    return(-1);
	}
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

	switch ( remote_type) {

	case S_IFREG:
		/* the stat structure happens to contain
		 * a timevec structure, because of the spare integers
		 * that follow each of the time fields:
		 */
		r->sbuf.st_atime = TIME( r->sbuf);
		timevec = (struct timeval *) &r->sbuf.st_atime; /* XXX */

		/* only transfer the file if the contents seem to differ.
		 * only call utimes() if copy_file() succeeds.
		 * it's important to call utimes before set_prots.
		 * if the file doesn't exist, always use remote protections.
		 */
		if ( !( diff->cksum || TIME( diff->sbuf)));
		else if ( copy_file( remotename, localname)) return( -1);
		else      utimes(    localname, timevec);

		/* if we're ignoring protections (-I option),
		 * restore the old local file-protections,
		 * unless this update created the file.
		 */
		return( exists && ignore_prots ?
			set_prots( localname, local_statp) :
			set_prots( localname, &r->sbuf));

	case S_IFLNK:
                if ( exists && removeit( localname, S_IFLNK))
			return( -1);

		oumask = umask(0); /* Symlinks don't really have modes */
		if ( symlink( r->link, localname)) {
			sprintf(errmsg,"can't create symbolic link %s -> %s\n",
				localname, r->link);
			do_gripe();
			umask(oumask);
			return(-1);
		}
		umask(oumask);
		return( 0);

	case S_IFDIR:
		/* if we're ignoring protections (-I option),
		 * keep the old local file-protections,
		 * unless this update creates the dir.
		 */
		return(   ! exists ?	   makepath(  localname, &r->sbuf)
			: ! ignore_prots ? set_prots( localname, &r->sbuf)
			: 0 );
	case S_IFBLK:
	case S_IFCHR:
		/* need to recompute the device#-difference,
		 * in case r or l has been recomputed since currency_diff()
		 * first computed it.
		 */
		if ( ! exists || ! DIFF( *r, *l, st_rdev));
		else if ( exists = removeit( localname, local_type))
			return( -1);

		if ( !exists && mknod( localname, remote_type,RDEV( r->sbuf))){
			sprintf(errmsg, "can't make device %s\n", localname);
			do_gripe();
			return(-1);
		}
		/* if we're ignoring protections (-I option),
		 * keep the old local file-protections,
		 * unless this update creates the device.
		 */
		return( exists && ignore_prots ?
			0 : set_prots( localname, &r->sbuf));

	case S_IFMT: /* should have been caught already. */
		sprintf( errmsg, "fromfile %s doesn't exist.\n", remotename);
		do_gripe();
		return( -1);
	default:
		sprintf(errmsg, "unknown file-type in update_file()\n");
		do_gripe();
		return(-1);
	}
	/*NOTREACHED*/
	/*
	sprintf( errmsg, "ERROR (update_file): internal error.\n");
	do_panic();
	return( -1);
	*/
}

updated( cmp, fr) struct currentness *cmp, *fr; {

	if ( ! fr) return( ! TYPE( cmp->sbuf));

	else if ( nopullflag) {
		/* simulate cmpfile's update for dec_entry().
		 * we do this for every cmpfile, because we can't
		 * tell whether this cmpfile represents an entry.
		 */
		cmp->sbuf.st_mode  = fr->sbuf.st_mode;
		cmp->sbuf.st_uid   = fr->sbuf.st_uid;
		cmp->sbuf.st_gid   = fr->sbuf.st_gid;
		cmp->sbuf.st_rdev  = fr->sbuf.st_rdev;
		cmp->sbuf.st_mtime = fr->sbuf.st_mtime;
		cmp->cksum	   = fr->cksum;
		strcpy( cmp->link,   fr->link);
	}
	else	/* usual case. mark the currency as "out-of-date",
		 * dec_entry() will refresh the entry's currency,
		 * if it sees this mark.
		 * this an efficiency hack; dec_entry() will seldom
		 * see this mark, because most cmpfiles aren't at top-level.
		 */
		cmp->sbuf.st_mode &= ~S_IFMT;

	return( 0);
}

get_currentness( path, c) char **path; struct currentness *c; {
        strcpy( c->name, path[ NAME]);
        c->cksum = 0;
        *c->link = '\0';
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
                if ( follow_link( path[ ROOT], c->link)) {
                        *c->link = '\0';
                        return( -1);
                }
                break;
        default:
                break;
        }
        return( 0);
}

clear_stat( sp) struct stat *sp; {
	int *p;

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
	else if	  (     *d->link) {
		n = (int)l->link;
		m = (int)r->link;
		format =	"%s %s%s %s ( symlink -> %s)\n"; }
	else if   ( d->cksum) {
		n = l->cksum;
		m = r->cksum;
		format =	"%s %s%s %s ( file-cksum=%4.x)\n"; }
	else if   (  RDEV( *ds)) {
		n =  RDEV( *ls);
		m =  RDEV( *rs);
		format =	"%s %s%s %s ( device-type=%d)\n"; }
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

	fprintf( stderr, format, "Updating", ltype, lfill, lname, n);
	fprintf( stderr, format, "    from", rtype, rfill, rname, m);
}
