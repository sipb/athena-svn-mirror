/*
 * $Id: except.c,v 4.11 1999-01-22 23:15:58 ghudson Exp $
 */

#ifndef lint
static char *rcsid = "$Id: except.c,v 4.11 1999-01-22 23:15:58 ghudson Exp $";
#endif lint

#include "mit-copyright.h"
#include "bellcore-copyright.h"

#include "track.h"

/*
**	routine to implement exception lists:
*/

gettail( pathp, type, entnum)
char **pathp;
unsigned short type;
int entnum;
{
	static char exc_dir[ LINELEN] = "";
	List_element **except, *q;
	char retval = NORMALCASE, *tail, *wholename;
	unsigned int k;
	Entry *e = &entries[ entnum], *g = &entries[ 0];

	if (( type == S_IFBLK || type == S_IFCHR) && ! incl_devs)
		return( DONT_TRACK);

	/* strip fromfile from path:
	 * path   == fromfile/tail
	 * keylen == strlen( fromfile).
	 */
	wholename = *pathp;
	*pathp += e->keylen;

	if ( ! **pathp) return( NORMALCASE);
	while( *++*pathp == '/');	

	/* if exc_dir is not empty, then it holds the most recently excepted
	 * subdir of the current entry. if the wholename starts with exc_dir,
	 * then we count it as an exception, too.
	 * excepted links are handled this way, too, in case they target
	 * directories. exc_dir is a speed hack.
	 */
	if ( ! *exc_dir);
	else if ( strncmp( wholename, exc_dir, strlen( exc_dir)))
		*exc_dir = '\0';
	else	return( DONT_TRACK);

	k = hash( tail = *pathp);

	if ( e->names.table && ( except = lookup( tail, k, &e->names))) {
		retval = FLAG( *except);
	}
	else if ( g->names.table && ( except = lookup( tail, k, &g->names))) {
		retval = FLAG( *except);
	}
	/*	compare the tail with each pattern in both entries' lists:
	 *	the global list ( g->patterns),
	 *	and the current entry's exception patterns.
	 *	the global list is chained to the end of e->patterns,
	 *	during subscription-list parsing.
	 */
	else for( q = e->patterns; q; q = NEXT( q))
		if ( match( TEXT( q), tail)) {
			retval = FLAG( q);
			break;
		}
	/* now, retval has been computed.
	 * if it's not NORMALCASE, we've had a match:
	 * either tail is to be a link to the exporter's filesystem,
	 * or tail is an exception. in either case, if tail is  a dir or
	 * a link to a dir, we don't want to update tail's children.
	 */
	if ( retval == NORMALCASE);
	else if ( type == S_IFLNK || type == S_IFDIR)
		sprintf( exc_dir, "%s/", wholename);

	return( retval);
}

List_element **
lookup( name, hashval, ht) char *name; unsigned long hashval; Table *ht; {
	List_element **ptop;

	/* *ptop is the first element of an alphabetically-sorted
	 * linked-list of relative pathnames.
	 * for speed, we access each list-elt via the beginning of its string;
	 * the elt's first 4 bytes hold the addr of the next list-elt.
	 */
	for ( ptop = &ht->table[ hashval >> ht->shift];
	     *ptop;
	      ptop = PNEXT( *ptop)) {
		switch ( SIGN( strcmp( *ptop, name))) {
		case -1: continue;
		case 1:  break;
		case 0:  return( ptop);
		}
		break;
	}
	return( NULL);
}

list2hashtable( p) Table *p; {
	List_element *names, *wordp;
	unsigned int tbl_size;

	if ( ! p->shift) {
		sprintf(errmsg,"parser error: non-empty table has 0 count\n");
		do_panic();
	}
	/* convert names list to hash-table,
	 * using count of list-elements to allocate table:
	 * size is power of 2, s.t. table is at most 2/3 full.
	 * XXX: store hashing shift in caller's copy of list-count.
	 *	parser stored the list-count as a negative #,
	 *	to tell justshow() how to dump.
	 */
	p->shift *= -1;
	p->shift = 31 - log2( p->shift + ( p->shift >> 1));
	names    = (List_element *) p->table;
	tbl_size = (unsigned) 0x80000000 >> p->shift - 1;
	if ( ! tbl_size)
		sprintf( errmsg, "(list2hashtable) null hash table request\n");

	p->table = (List_element **) calloc( tbl_size, sizeof( List_element*));
	if ( ! p->table) {
		sprintf( errmsg, "(list2hashtable) calloc( %d, %d) failed.\n",
			 tbl_size, sizeof( List_element *));
		do_panic();
	}
	do {
	    wordp = names;
	    names = NEXT( names);
	    store( wordp, p);
	} while ( names);
	return;
}
/* get integer-part of log(base2) of the table-length,
 * by doing a binary-search for the high-order bit:
 */
unsigned short mask[] =
{       0xffff,0xfffe,0xfffc,0xfff8,0xfff0,0xffe0,0xffc0,0xff80,
	0xff00,0xfe00,0xfc00,0xf800,0xf000,0xe000,0xc000,0x8000};
log2( len) unsigned short len; {
	int i, n = 0;
	for ( i = 16; i >>= 1; ) if ( len & mask[ n + i]) n += i;
	return( n);
}

store( list_elt, p) List_element *list_elt; Table *p; {
	List_element **collisionp, **nextp;

	collisionp = &p->table[ hash( TEXT( list_elt)) >> p->shift];
	if ( ! *collisionp) {
	    *collisionp = list_elt;	/* store list-elt in hash table */
	    NEXT( list_elt) = NULL;
	    return;
	}
	/* keep list of colliding elt's in alphabetical order.
	 * if list_elt is already present in the table,
	 * ensure that FORCE_LINK overrides DONT_TRACK.
	 */ 
	for ( nextp = collisionp; *nextp; nextp = PNEXT( *nextp)) {
	    switch( SIGN( strcmp( *nextp, list_elt))) {
	    case -1: continue;
	    case  1: break;				/* switch */
	    case  0: FLAG( *nextp) |= FLAG( list_elt);
		     return; /* infequent case last */
	    }
	    break;	/* loop */
	}
	NEXT( list_elt) = *nextp;
	*nextp = list_elt;
	return;
}
List_element *
add_list_elt( str, flag, top) char *str; char flag; List_element **top; {
	List_element *p;

	/* this alloc will waste some char's,
	 * since List_element{}'s header is padded to end on a 
	 * fullword-boundary.
	 */
	p = (List_element *) malloc( sizeof( List_element) + strlen( str));
	if ( !p) {
		sprintf( errmsg, "(add_list_element) malloc( %d) failed.\n",
			 sizeof( List_element) + strlen( str));
		do_panic();
	}
	p = (List_element *) p->first_char;

	if ( top) {
		NEXT( p) = *top;
		*top = p;
	}
	else NEXT( p) = NULL;
	FLAG( p)   = flag;
	strcpy( p, str);
	return( p);
}
#define RATIO ((unsigned)(0.6125423371 * 0x80000000))
/* this is a multiplicative hash code. for an explanation of RATIO,
 * see T. A. Standish, Data Structure Techniques, p161-3, or
 * Knuth, Art of Comp. Prog., v3, p512.
 * the shifting makes a 17-bit key from several 6-bit bytes.
 * ( for alphabetic ASCII, the top 2 bits of each byte are redundant. )
 * the numbers 4, 5 & 12 are carefully chosen, from trial & error.
 * this hash code has the following advantages over mod-prime hash:
 * collides slightly less frequently, especially when table is overfull;
 * doesn't need prime-sized table, so table is easier to build.
 */
unsigned long
hash( p) char *p; {
	unsigned long key = 0;
	short s = 4;
	for ( ; *p ; s += 5) key ^= ( *p++ & 077) << s % 12;
	return( key * RATIO);
}
char *
next_def_except() {
	static char *g_except[] = DEF_EXCEPT; /* default GLOBAL exceptions */
	static int next = 0;

	if ( next < sizeof( g_except) / 4)
		return ( g_except[ next++]);
	else return( NULL);
}

file_pat( ptr)
char *ptr;
{
	return( (strchr(ptr,'*') ||
		 strchr(ptr,'[') ||
		 strchr(ptr,']') ||
		 strchr(ptr,'?')));
}

#define FASTEQ( a, b) (*(a) == *(b) && a[1] == b[1])

duplicate( word, entnum) char *word; int entnum; {
	List_element *re;

	for ( re = entries[ entnum].patterns; re; re = NEXT( re)) {
		    if ( FASTEQ( word, TEXT( re)) &&
		       ! strcmp( word, TEXT( re)))
                        return( 1);
        }
	/* check the global exceptions list
	 */
	if ( entnum) return( duplicate( word, 0));
	return( 0);
}

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int match(pattern, string)
	char *pattern;
	char *string;
{
	char *p, *q, c;

	q = string;
	p = pattern;
	for (; *p;) {
		c = *p++;
		switch (c) {
		case '\\':
			if (*q++ != *p++)
				return 0;
			break;
		case '?':
			if (*q++ == '\0')
				return 0;
			break;
		case '*':
			c = *p;
			if (c != '\\' && c != '?' && c != '*' && c != '[') {
				while (*q != c) {
					if (*q == '\0')
						return 0;
					q++;
				}
			}
			do {
				if (match(p, q))
					return 1;
			} while (*q++ != '\0');
			return 0;
		case '[': {
			char *endp;
			int invert, found;
			char chr;

			endp = p;
			if (*endp == '!')
				endp++;
			for (;;) {
				if (*endp == '\0')
					goto dft;	/* no matching ] */
				if (*endp == '\\')
					endp++;
				if (*++endp == ']')
					break;
			}
			invert = 0;
			if (*p == '!') {
				invert++;
				p++;
			}
			found = 0;
			chr = *q++;
			if (chr == '\0')
				return 0;
			c = *p++;
			do {
				if (c == '\\')
					c = *p++;
				if (*p == '-' && p[1] != ']') {
					p++;
					if (*p == '\\')
						p++;
					if (chr >= c && chr <= *p)
						found = 1;
					p++;
				} else {
					if (chr == c)
						found = 1;
				}
			} while ((c = *p++) != ']');
			if (found == invert)
				return 0;
			break;
		}
dft:	        default:
			if (*q++ != c)
				return 0;
			break;
		}
	}
	if (*q != '\0')
		return 0;
	return 1;
}
