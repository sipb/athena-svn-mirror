/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 2.0 1987-11-30 15:19:17 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/02/12  21:14:36  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 2.0 1987-11-30 15:19:17 don Exp $";
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
			tail,entnum, entries[cur_ent].fromfile);

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
	if ( *p != '^' && !strcmp( p, fname))
		return(1);

	if ( re_comp( p)) {
		sprintf(errmsg, "%s bad regular expression\n", p);
		do_panic();
	}
	if ( re_exec( fname)) return(1) ;
	return( 0);
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
