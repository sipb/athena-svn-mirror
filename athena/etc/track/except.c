/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 2.2 1987-12-03 17:33:54 don Exp $
 *
 *	$Log: not supported by cvs2svn $
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 2.2 1987-12-03 17:33:54 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
**	routine to implement exception lists
**	returns 1 if the filename matches the fromfile in entnum's entry,
**			and does NOT match any of entnum's exceptions.
**	returns 0 otherwise
*/

int
goodname( tail, entnum)
char *tail;
int entnum;
{
	char *pattern;
	int i;

	if (debug)
		printf("goodname(%s,%d); fromname=%s\n",
			tail,entnum, entries[entnum].fromfile);

	switch( ( unsigned) *tail) {
	case '\0': return( 1);
	case '/':  break;
	default: sprintf( errmsg, "bad tail value: %s\n", tail);
		 do_gripe();
		 return(0);
	}

	/* skip the leading slash: */
	tail++;
	
	/*	compare the tail with each exception in both lists:
	 *	the global list ( entries[0].exceptions[] ),
	 *	and the current entry's exceptions list.
	 */
	for( i=0; pattern = entries[ 0     ].exceptions[ i]; i++)
		if ( match( pattern, tail)) return( 0);

	for( i=0; pattern = entries[ entnum].exceptions[ i]; i++)
		if ( match( pattern, tail)) return( 0);

	return(1);
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
	if ( *p != '^') return( ! strcmp( p, fname));
	if ( re_comp( p)) {
		sprintf(errmsg, "%s bad regular expression\n", re_comp( p));
		do_panic();
	}
	switch( re_exec( fname)) {
	case 0: return( 0);
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

#define FASTEQ( a, b) (*((short *)(a)) == *((short*)(b)))

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
