/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: globmatch.c,v 1.1.1.2 1999-05-04 18:50:42 mwhitson Exp $";

#include "ifhp.h"

/**** ENDINCLUDE ****/

static int glob_pattern( char *pattern, char *str )
{
	int result = 1;
	int len, c;
	char *glob;

	/* DEBUG4("glob_pattern: pattern '%s' to '%s'", pattern, str ); */
	if( (glob = strpbrk( pattern, "*?[" )) ){
		/* check the characters up to the glob length for a match */
		len = glob - pattern;
		c = *glob;
	 	/* DEBUG4("glob_pattern: meta '%c', len %d", c, len ); */
		if( (len == 0 || !strncasecmp( pattern, str, len )) ){
			/* matched: pattern xxxx*  to xxxx
			 * now we have to do the remainder. We simply check for rest
			 */
			pattern += len+1;
			str += len;
			/* DEBUG4("glob_pattern: rest of pattern '%s', str '%s'", pattern, str ); */
			/* check for trailing glob */
			if( c == '?' ){
				if( *str ){
					result = glob_pattern( pattern, str+1 ); 
				} else {
					result = 1;
				}
			} else if( c == '[' ){
				/* now we check for the characters in the pattern
				 * this can have the format N or N-M
				 */
				glob = strchr( pattern, ']' );
				if( glob == 0 ){
					return( 1 );
				}
				len = glob - pattern;
				c = 0;
				/* DEBUG4("globmatch: now case '%s', len %d", pattern, len ); */
				while( result && len > 0 ){
					/* DEBUG4("globmatch: c '0x%x', pat '%c', str '%c'",
						c, *pattern, *str ); */
					if( c && *pattern == '-' && len > 1 ){
						/* we have preceeding, get end */
						pattern++; --len;
						while( result && c <= *pattern ){
					/* DEBUG4("globmatch: range c '%c', pat '%c', str '%c'",
								c, *pattern, *str ); */
							result = (*str != c++);
						}
						pattern++; --len;
						c = 0;
					} else {
						c = *pattern++; --len;
						result = (*str != c);
					}
				}
				/* DEBUG4("globmatch: after pattern result %d", result); */
				if( result == 0 ){
					pattern += len+1;
					++str;
					/* at this point pattern is past */
					result = glob_pattern( pattern, str );
				}
			} else if( pattern[0] ){
				while( *str && (result = glob_pattern( pattern, str )) ){
					++str;
				}
			} else {
				result = 0;
			}
		}
	} else {
		result = strcasecmp( pattern, str );
	}
	return( result );
}

int Globmatch( char *pattern, char *str )
{
	int result;

	/* try simple test first: string compare */
	/* DEBUG4("Globmatch: pattern '%s' to '%s'", pattern, str ); */
	if( pattern == 0 ) pattern = "";
	if( str == 0 ) str = "";
	result = glob_pattern( pattern, str );
	DEBUG4("Globmatch: '%s' to '%s' result %d", pattern, str, result );
	return( result );
}
