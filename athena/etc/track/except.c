/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 3.0 1988-03-09 13:53:42 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.4  88/01/29  18:22:53  don
 * bug fixes. also, now track can update the root.
 * 
 * Revision 2.3  87/12/03  20:36:59  don
 * fixed a portability bug in FASTEQ macro, and made yacc sort each
 * exceptions-list, so that goodname can run faster.
 * 
 * Revision 2.2  87/12/03  17:33:54  don
 * fixed lint warnings.
 * 
 * Revision 2.2  87/12/01  20:22:31  don
 * fixed a bug in match(): it was usually compiling exception-names,
 * even if they weren't regexp's. this seems to have led to some bogus
 * matches. at least, "track -w" doesn't miss files anymore.
 * this bug was created during the rewrite.
 * 
 * Revision 2.1  87/12/01  16:41:36  don
 * fixed bugs in readstat's traversal of entries] and statfile:
 * cur_ent is no longer global, but is now part of get_next_match's
 * state. also, last_match() was causing entries[]'s last element to be
 * skipped.
 * 
 * Revision 2.0  87/11/30  15:19:17  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 * Revision 1.1  87/02/12  21:14:36  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 3.0 1988-03-09 13:53:42 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
**	routine to implement exception lists:
**	returns filename's addendum to entnum's fromfile
**		if the filename matches the fromfile
**		and does NOT match any of entnum's exceptions.
**	returns NULL otherwise
*/

char *
goodname( path, entnum)
char *path;
int entnum;
{
	char *pattern, *tail;
	int e, i;

	/* strip fromfile from path:
	 * path   == fromfile/tail
	 * keylen == strlen( fromfile).
	 */
	tail = path + entries[ entnum].keylen;

	if ( ! *tail) return( tail);

	while( *++tail == '/');

	/*	compare the tail with each exception in both lists:
	 *	the global list ( entries[0].exceptions[] ),
	 *	and the current entry's exceptions list.
	 *	both lists are sorted lexicographically, not by sortkey.
	 */
	for( e = 0; e <= entnum; e += entnum)
		for( i=0; pattern = entries[ e].exceptions[ i]; i++) {
			switch ( SIGN( strcmp( pattern, tail))) {
			case 1:  if ( (unsigned) *pattern > '^') break;
			case -1: if ( ! match( pattern, tail)) continue;
			case 0:  return( NULL);
			}
			/* all further patterns are strings
			 * which are lexic'ly greater than the tail.
			 */
			break;
		}
	return( tail);
}

char *
next_def_except() {
	static char *g_except[] = DEF_EXCEPT; /* default GLOBAL exceptions */
	static int next = 0;

	if ( next < sizeof g_except / 4)
		return ( g_except[ next++]);
	else return( NULL);
}

/*
**	if there is a match, then return(1) else return(0)
*/
match( p, fname) char *p, *fname;
{
	/* all our regexp's begin with ^,
	 * because re_conv() makes them.
	 */
	if ( *p != '^') return( 0);
	if ( re_comp( p)) {
		sprintf(errmsg, "%s bad regular expression\n", re_comp( p));
		do_panic();
	}
	switch( re_exec( fname)) {
	case 0: return( 0);		/* most frequent case */
	case 1: return( 1);
	case -1: sprintf( errmsg, "%s bad regexp\n", p);
		 do_panic();
	}
	sprintf( errmsg, "bad value from re_exec\n");
	do_panic();
	return( -1);
}

file_pat( ptr)
char *ptr;
{
	return( (index(ptr,'*') ||
		 index(ptr,'[') ||
		 index(ptr,']') ||
		 index(ptr,'?')));
}

#define FASTEQ( a, b) (*(a) == *(b) && a[1] == b[1])

duplicate( word, entnum) char *word; int entnum; {
        int i;
        char *re;

        for (i=0; re = entries[ entnum].exceptions[ i]; i++) {
                if ( FASTEQ( word, re) &&
                   ! strcmp( word, re))
                        return( 1);
        }
	/* check the global exceptions list
	 */
	if ( entnum) return( duplicate( word, 0));
	return( 0);
}

/*
**	convert shell type regular expressions to ex style form
*/
char *re_conv(from)
char *from;
{
	static char tmp[LINELEN];
	char *to;

	to = tmp;
	*to++ = '^';
	while(*from) {
		switch (*from) {
		case '.':
			*to++ = '\\';
			*to++ = '.';
			from++;
			break;

		case '*':
			*to++ = '.';
			*to++ = '*';
			from++;
			break;

		case '?':
			*to++ = '.';
			from++;
			break;
			
		default:
			*to++ = *from++;
			break;
		}
	}

	*to++ = '$';
	*to++ = '\0';
	return(tmp);
}
