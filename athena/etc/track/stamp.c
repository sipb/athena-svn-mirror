/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 2.1 1987-12-01 16:44:54 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.0  87/11/30  15:19:43  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 * Revision 1.1  87/02/12  21:15:36  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 2.1 1987-12-01 16:44:54 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/* XXX
 * convert right-shifted st_mode type-bits to corresponding characters:
 * S_IFCHR = 0020000 gets mapped to elt 1 of the array, which is 'c'.
 * S_IFDIR = 0040000 => elt 2 of the array, which is 'd'.
 * S_IFBLK = 0060000 => elt 3 of the array, which is 'b'.
 * S_IFREG = 0100000 => elt 4 of the array, which is 'f'.
 * S_IFLNK = 0120000 => elt 5 of the array, which is 'l'.
 * S_IFSOCK= 0140000 => elt 6 of the array, which is 's' ( only for errors).
 * S_IFMT  = 0170000 => elt 7 of the array, which is '*' ( dropping 1 bit).
 */
char type_char[] = " cdbfls*89ABCDEF";

/*
 * Place a time-stamp line in the format:
 *       type file uid.gid.mode.time
 * or a suitable derivate thereof
 */

write_statline( path, s)
char **path; struct stat *s;
{
	char  *linebuf, *linkval, type;
	struct stat fromstat;
	int size;

	if ( lstat( path[ ROOT], &fromstat)) {
		sprintf(errmsg,"can't stat %s\n", path[ ROOT]);
		do_gripe();
		return( S_IFMT);
	}
	if ( cur_line >= maxlines) {
		maxlines += MAXLINES;
		size = maxlines * sizeof statfilebufs[0];
		statfilebufs = (Statline *) ( cur_line ?
					       realloc( statfilebufs, size)
					      : malloc( size));
		if ( ! statfilebufs) {
			sprintf( errmsg, "alloc failed: %d statfile lines\n",
				 cur_line);
			do_panic();
		}
	}
	KEYCPY( statfilebufs[ cur_line].sortkey, path[ NAME]);

	/* XXX: see type_char[]
	 */
	type = type_char[ TYPE( *s) >> 13 ];

	linebuf = statfilebufs[ cur_line].line;

	switch( TYPE( *s)) {
	case S_IFREG:
	case S_IFDIR:
                sprintf( linebuf, "%c%s %d.%d.%o.%ld\n", type, path[ NAME],
		 	 UID( *s), GID( *s), MODE( *s), TIME( *s));
		break;
	case S_IFBLK:
	case S_IFCHR:
                sprintf( linebuf, "%c%s %d.%d.%o.%d\n", type, path[ NAME],
		 	 UID( *s), GID( *s), MODE( *s), DEV( *s));
		break;
	case S_IFLNK:
		if ( ! ( linkval = follow_link( path[ ROOT]))) {
			type = '*';	/* XXX */
			linkval = "";
		}
		sprintf( linebuf, "%c%s %s\n", type, path[ NAME], linkval);
		break;
	case S_IFMT:
		sprintf( linebuf,"%c%s\n", type, path[ NAME]);
		break;
	default:
		sprintf( errmsg, "bad type for inode %d. apparent type = %c\n",
		         ( *s).st_ino, type);
		do_panic();
	}
	if ( verboseflag)
		fputs( linebuf, stderr);

	cur_line++;
	return( TYPE( fromstat));
}

sort_stat() {
	int i;

	/* NOTE: this qsort call assumes that each statfilebufs[] element
	 * begins with a sortkey as its first field.
	 */
        qsort( statfilebufs, cur_line, sizeof( statfilebufs[ 0]), strcmp);

        for ( i = 0; i < cur_line; i++) {
                fputs( statfilebufs[ i].line, statfile);
	}
}

sort_entries() {

	/* NOTE: we assume that each entry begins with a sortkey string.
         * don't include entries[ 0] in the sort:
         */
        qsort( (char *)& entries[ 1], entrycnt - 1,
               sizeof(   entries[ 1]), strcmp);
}

/*
 * Decode a statfile line into its individual fields.
 * setup TYPE(), UID(), GID(), MODE(), TIME(), & DEV() contents.
 */

struct stat *
dec_statfile( line, name, link)
char *line, *name, *link;
{
	static struct stat s;

	*link = '\0';

	switch( *line) {
	case 'f':
		sscanf( line, "%*c%s %d.%d.%o.%ld", name,
			&s.st_uid, &s.st_gid, &s.st_mode, &(long) s.st_mtime);
		s.st_mode |= S_IFREG;
		break;
	case 'l':
		sscanf( line, "%*c%s %s", name, link);
		s.st_mode = S_IFLNK;
		break;
	case 'd':
		sscanf( line, "%*c%s %d.%d.%o.%ld", name,
			&s.st_uid, &s.st_gid, &s.st_mode, &(long) s.st_mtime);
		s.st_mode |= S_IFDIR;
		break;
	case 'b':
		sscanf( line, "%*c%s %d.%d.%o.%d", name,
			&s.st_uid, &s.st_gid, &s.st_mode, &s.st_dev);
		s.st_mode |= S_IFBLK;
		break;
	case 'c':
		sscanf( line, "%*c%s %d.%d.%o.%d", name,
			&s.st_uid, &s.st_gid, &s.st_mode, &s.st_dev);
		s.st_mode |= S_IFCHR;
		break;
	case '*':
		sscanf( line, "%*c%s", name);
		s.st_mode = S_IFMT;
		break;
	default:
		sprintf( errmsg, "garbled statfile: bad line = %s\n", line);
		do_panic();
	}
	return( &s);
}

/* the match abstractions implement a synchronized traversal
 * of the entries[] array & the statfile. both sets of data
 * are sorted by strcmp() on their sortkeys, which are made
 * via the KEYCPY macro.
 */

/* since keys are homologous to pathnames,
 * we can speak of one key containing another,
 * just as paths do: path-components have to match exactly,
 * but r can have more components that l:
 */
#define SUBPATH( r, l, n) ('\001' == (r)[n] || '\0' == (r)[n])

int cur_ent = 0;	/* not global! index into entries[]. */

init_next_match() {
	cur_ent = 1;
}
int
last_match( remkey, i) char *remkey; int i; {
	char *key;
	int len;

	/* look-ahead to find last sub-list entry that matches
	 * the current statline:
	 */
	for ( ; i < entrycnt; i++) {
		key = entries[ i].sortkey;
		len = entries[ i].keylen;
		if ( strncmp( remkey, key, len) ||
	           ! SUBPATH( remkey, key, len))
			return( i - 1);
	}
	return( entrycnt - 1);
}
#define SIGN( i) (((i) > 0)? 1 : ((i)? -1 : 0))

int
get_next_match( remstatline)
char *remstatline;
{
	char *lkey, rkey[ LINELEN], remname[ LINELEN];
	int klen;

	while ( NULL != fgets( remstatline, LINELEN, statfile)) {
		/* extract just the filename from the statline:
		 */
		sscanf( remstatline, "%*c%s ", remname);
		KEYCPY( rkey, remname);

		/* find the subscription entry corresponding to the
		 * current statline.
		 * if we reach an entry which is lexicographically greater
		 * than the current statline, get a new statline.
		 * both entries[] & statfile must be sorted by sortkey!
		 */
		for ( ; cur_ent < entrycnt; cur_ent++) {
			lkey = entries[ cur_ent].sortkey;
			klen = entries[ cur_ent].keylen;
			switch ( SIGN( strncmp( rkey, lkey, klen))) {
			case 0: if (   SUBPATH( rkey, lkey, klen))
					return( last_match( rkey, cur_ent + 1));
			case -1: break; 
			case 1: continue;
			}
			break; /* entries array is ahead of statfile.
				* go to outer loop & read statfile.  */
		}
		/* XXX: when the inner loop walks off the end of entries[],
		 * entries[ cur_ent] is garbage; but, the loop is faster
		 * this way. the caller shouldn't be looking at cur_ent's value,
		 * since we're saying "the party's over", but let's clean up,
		 * anyway.
		 */
		if ( cur_ent >= entrycnt) {
			cur_ent = 0;
			return( 0);
		}
	}
	return( 0);
}

struct stat *dec_entry( entnum, fr, to, cmp)
int entnum; char *fr[], *to[], *cmp[]; {
	static int prev_ent = 0;
	static struct stat sbuf;

	if ( entnum == prev_ent) return( &sbuf); /* usual case */
	if ( prev_ent) {
		poppath( fr); poppath( to); poppath( cmp);
	}
	pushpath( fr,  entries[ entnum].fromfile);
	pushpath( to,  entries[ entnum].tofile);
	pushpath( cmp, entries[ entnum].cmpfile);
	
	if ( lstat( cmp[ ROOT], &sbuf)) {
		sbuf.st_mode = S_IFMT;	/* klooge for bad type */
	}
	prev_ent = entnum;
	return( &sbuf);
}

/* these abstractions handle a stack of pointers into a character-string,
 * which typically is a UNIX pathname.
 * the first element CNT of the stack points to a depth-counter.
 * the second element ROOT points to the beginning of the pathname string.
 * subsequent elements point to substrings of the pathname.
 */
char **
initpath( ) {
	char **p;

	p =      (char **) malloc( stackmax * sizeof NULL);
	p[ CNT] = (char *) malloc( stackmax * 15 + sizeof ((int) 0));
	p[ ROOT] = p[ CNT] + sizeof ((int) 0);
	*( int*)p[ CNT] = 0;
	return( p);
}
int
pushpath( p, name) char **p; char *name; {
	int i;
	if ( ! p) return( -1);
	if ( stackmax <= *( int*)p[ CNT]) {
		sprintf( errmsg, "%s\n%s\n%s %s.\n",
			"path stack overflow: directory too deep:", p[ ROOT],
			"use -S option, with value >", stackmax);
		do_panic();
	}
	i = ++*( int*)p[ CNT];
	strcpy( p[ i], name);
	p[ i + 1] = p[ i] + strlen( name);
	return( i);
}
poppath( p) char **p; {
	if ( ! p) return;
	if ( *(int*)p[ CNT]) --*(int*)p[ CNT];
}

entrycmp( i, j) int *i, *j; {
        return( strcmp( entries[ *i].sortkey,
                        entries[ *j].sortkey));
}
