/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 2.4 1988-01-29 18:24:02 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.3  87/12/03  19:50:02  don
 * moved SIGN macro to track.h.
 * 
 * Revision 2.2  87/12/03  17:31:15  don
 * fixed rt-port bug in dec_statfile's use of sscanf():
 * can't fill a short int directly, because sscanf will interpret it
 * as a long-int, and on the rt, short* gets converted to int* via
 * truncation!
 * 
 * Revision 2.1  87/12/01  16:44:54  don
 * fixed bugs in readstat's traversal of entries] and statfile:
 * cur_ent is no longer global, but is now part of get_next_match's
 * state. also, last_match() was causing entries[]'s last element to be
 * skipped.
 * 
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/stamp.c,v 2.4 1988-01-29 18:24:02 don Exp $";
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
	char  *linebuf, *linkval, *name, type;
	struct stat fromstat;
	unsigned size;

	if ( S_IFMT == TYPE( *s))	/* couldn't stat cmpfile */
		s = &fromstat;

	if ( (*statf)( path[ ROOT], &fromstat)) {
		sprintf(errmsg,"can't stat %s\n", path[ ROOT]);
		do_gripe();
		return( S_IFMT);
	}
	if ( cur_line >= maxlines) {
		maxlines += MAXLINES;
		size = maxlines * sizeof statfilebufs[0];
		statfilebufs =
			(Statline *) ( cur_line ?
				       realloc( (char *) statfilebufs, size)
				      : malloc( size));
		if ( ! statfilebufs) {
			sprintf( errmsg, "alloc failed: %d statfile lines\n",
				 cur_line);
			do_panic();
		}
	}
	/* root is a special case:
	 * its "relative path" is "", which dec_statfile() can't read.
	 */
	name = path[ NAME];
	if ( !*name) name = "/";

	KEYCPY( statfilebufs[ cur_line].sortkey, name);

	/* XXX: see type_char[]
	 */
	type = type_char[ TYPE( fromstat) >> 13 ];

	linebuf = statfilebufs[ cur_line].line;

	switch( TYPE( fromstat)) {
	case S_IFREG:
	case S_IFDIR:
                sprintf( linebuf, "%c%s %d.%d.%o.%ld\n", type, name,
		 	 UID( *s), GID( *s), MODE( *s), TIME( *s));
		break;
	case S_IFBLK:
	case S_IFCHR:
                sprintf( linebuf, "%c%s %d.%d.%o.%d\n", type, name,
		 	 UID( *s), GID( *s), MODE( *s), DEV( *s));
		break;
	case S_IFLNK:
		if ( ! ( linkval = follow_link( path[ ROOT]))) {
			type = '*';	/* XXX */
			linkval = "";
		}
		sprintf( linebuf, "%c%s %s\n", type, name, linkval);
		break;
	case S_IFMT:
		sprintf( linebuf,"%c%s\n", type, name);
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
        qsort( (char *) statfilebufs, cur_line,
		sizeof( statfilebufs[ 0]), strcmp);

        for ( i = 0; i < cur_line; i++) {
                fputs( statfilebufs[ i].line, statfile);
	}
}

/* for sort_entries' use:
 */
exceptcmp( p, q) char **p, **q; {
        return( strcmp( *p, *q));
}

sort_entries() {
	char **e, *key, *tail;
	int i, j, k;

	/* NOTE: we assume that each entry begins with a sortkey string.
         * don't include entries[ 0] in the sort:
         */
        qsort( (char *)& entries[ 1], entrycnt - 1,
               sizeof(   entries[ 1]), strcmp);

	/* for each entry's fromfile (call it A),
	 * look for A's children amongst the subsequent entries,
	 * and add any that you find to A's exception-list.
	 */
	for ( i = 1; i < entrycnt; i++) {
	     for ( j = i + 1; j < entrycnt; j++) {
		  key = entries[ j].sortkey;
		  switch( keyncmp( key, i)) {
		  case -1:
		       break; /* get next i */
		  case 0:
		       tail = entries[j].fromfile + entries[i].keylen;
		       while( '/' == *tail) tail++;
		       for ( k=0; k < WORDMAX; k++) {
			    e = &entries[ i].exceptions[ k];
			    if ( ! strcmp( tail, *e));	/* j already here */
			    else if ( *e) continue;	/* keep looking */
			    else savestr( e, tail);	/* j not here */
			    break;	/* leave exceptions loop, goto case 1 */
		       }
		  case 1:
		       continue;
		  }
		  break; /* leave j loop, get next i */
	     }
	     for ( k = 0;    entries[ i].exceptions[ k]; k++);

	     qsort( (char *)&entries[ i].exceptions[ 0], k,
		     sizeof( entries[ 1].exceptions[ 0]),
		     exceptcmp);
	}
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
	char *time_format = "%*c%s %d.%d.%o.%ld";
	char *dev_format  = "%*c%s %d.%d.%o.%d";
	int d = 0, u = 0, g = 0, m = 0;

	/* these long-int temps are necessary for pc/rt compatibility:
	 * sscanf cannot scan into a short, though it may sometimes succeed
	 * in doing so. the difficulty is that it can't know about the
	 * target-integer's length, so it assumes that it's long.
	 * since the rt will truncate a short's addr, in order to treat
	 * it as a long, sscanf's data will often get lost.
	 * the solution is to give scanf longs, and then to convert these
	 * longs to shorts, explicitly.
	 */

	*link = '\0';

	switch( *line) {
	case 'f':
		sscanf( line, time_format, name, &u, &g, &m, &s.st_mtime);
		s.st_mode = S_IFREG;
		break;
	case 'l':	/* more common than dir's */
		sscanf( line, "%*c%s %s", name, link);
		s.st_mode = S_IFLNK;
		break;
	case 'd':
		sscanf( line, time_format, name, &u, &g, &m, &s.st_mtime);
		s.st_mode = S_IFDIR;
		break;
	case 'b':
		sscanf( line, dev_format,  name, &u, &g, &m, &d);
		s.st_mode = S_IFBLK;
		break;
	case 'c':
		sscanf( line, dev_format,  name, &u, &g, &m, &d);
		s.st_mode = S_IFCHR;
		break;
	case '*':
		sscanf( line, "%*c%s", name);
		s.st_mode = S_IFMT;
		break;
	default:
		sprintf( errmsg, "garbled statfile: bad line = %s\n", line);
		do_panic();
	}
	/* in the  statfile, which contains only relative pathnames,
	 * "/" is the only pathname which can begin with a slash.
	 * in entries[], the root appears as "", which is more natural,
	 * because "" is "/"'s pathname relative to the mount-point fromroot.
	 * and because pushpath() prepends slashes in the right places, anyway.
	 * "" is hard to read with sscanf(), so we handle "/" specially:
	 */
	if ( '/' != *name);
	else if ( ! name[ 1]) *name = '\0';
	else {
		sprintf(errmsg, "statfile passed an absolute pathname: %s\n",
			name);
		do_gripe();
	}
	s.st_uid   = (short) u;
	s.st_gid   = (short) g;
	s.st_mode |= (short) m & 07777;
	s.st_dev   = (short) d;
	return( &s);
}

/* the match abstractions implement a synchronized traversal
 * of the entries[] array & the statfile. both sets of data
 * are sorted by strcmp() on their sortkeys, which are made
 * via the KEYCPY macro.
 */

int cur_ent = 0;	/* not global! index into entries[]. */

init_next_match() {
	cur_ent = 1;
}

int
get_next_match( name)
char *name;
{
	int i = cur_ent;
	char key[ LINELEN];

	KEYCPY( key, name);

	for ( ; cur_ent < entrycnt; cur_ent++)
		switch ( keyncmp( key, cur_ent)) {
		case 0:  return( cur_ent);
		case -1: return( 0); 		/* advance name's    key */
		case 1:  continue;		/* advance cur_ent's key */
		}

	return( -1); /* out of entries */
}

/* last_match:
 * we know that path begins with entnum's fromfile,
 * but path may also match other entries.
 * for example, the path /a/b/c matches the entries /a & /a/b.
 * we want the longest entry that matches.
 * look-ahead to find last sub-list entry that matches
 * the current pathname: this works because by keyncmp,
 *	    /	is greater than		/Z...
 *	   |	matches			/a
 *	   |	is greater than		/a/a
 *	   \	matches			/a/b
 * a/b/c   <	is greater than		/a/b/a
 *	   /	matches			/a/b/c
 *	   |	is less than		/a/b/c/a
 *	   |	is less than		/a/b/d
 *	    \	is less than		/a/c...
 * NOTE that the right-hand column is sorted by key,
 * and that the last match is the longest.
 */
int
last_match( path, entnum) char *path; int entnum; {
	int i;
	char key[ LINELEN];

        KEYCPY( key, path);
        for ( i = entnum + 1; i < entrycnt; i++) {
                switch ( keyncmp( key, i)) {
		case -1: break;		/* quit at   /a/b/d */
		case 0:  entnum = i;	/* remember  /a or /a/b */
		case 1:  continue;	/* skip over /a/a */
		}
		break; /* quit loop */
	}
	return( entnum);
}

/* compare the path r with i's sortkey in the following way:
 * if the key is a subpath of r, return 0, as a match.
 * if the key is < or > r, return 1 or -1  respectively.
 */
int
keyncmp( r, i) char *r; int i; {
	char *l;
	int diff, n;

	l = entries[ i].sortkey;
	n = entries[ i].keylen;

	diff = SIGN( strncmp( r, l, n));

	/* if diff == 0 & n != 0 ( l isn't root) &
	 * r[n] == '\0' or '\001', we have a match.
	 * if n == 0, l is root, which matches everything.
	 */
	return( diff? diff: n ? ( (unsigned) r[n] > '\001') : 0);   /* XXX */
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
	
	/* this function-var is global, and used generally.
	 */
	statf = entries[ entnum].followlink ? stat : lstat;

	if ( (*statf)( cmp[ ROOT], &sbuf)) {
		sbuf.st_mode = S_IFMT;	/* klooge for bad type */
		sprintf( errmsg, "can't stat comparison-file %s", cmp[ ROOT]);
		do_gripe();
	}
	prev_ent = entnum;
	return( &sbuf);
}

/* these abstractions handle a stack of pointers into a character-string,
 * which typically is a UNIX pathname.
 * the first element CNT of the stack points to a depth-counter.
 * the second element ROOT points to the beginning of the pathname string.
 * subsequent elements point to terminal substrings of the pathname.
 * a slash precedes each element.
 */
#define COUNT(p) (*(int*)p[CNT])
char **
initpath( name) char *name; {
	char **p;

	/* for each stack-element, alloc a pointer 
	 * and 15 chars for a filename:
	 */
	p =      (char **) malloc( stackmax * sizeof NULL);
	p[ CNT] = (char *) malloc( stackmax * 15 + sizeof ((int) 0));
	COUNT( p) = 1;
	p[ ROOT] = p[ CNT] + sizeof ((int) 1);
	strcpy( p[ ROOT], name);
	p[ NAME] = p[ ROOT] + strlen( name);
	return( p);
}
int
pushpath( p, name) char **p; char *name; {
	char *top;

	if ( ! p) return( -1);
	if ( ++COUNT( p) >= stackmax) {
		sprintf( errmsg, "%s\n%s\n%s %d.\n",
			"path stack overflow: directory too deep:", p[ ROOT],
			"use -S option, with value >", stackmax);
		do_panic();
	}
	*p[ COUNT( p)]++ = '/';
	top = p[ COUNT( p)];
	strcpy( top, name);
	p[ COUNT( p) + 1] = top + strlen( top);
	return( COUNT( p));
}
poppath( p) char **p; {
	if ( ! p);
	else if ( 1 <  COUNT( p)) /* remove element's initial slash */
		*--p[  COUNT( p)--] = '\0';
	else {
		sprintf(errmsg,"can't pop root from path-stack");
		do_panic();
	}
	return;
}
