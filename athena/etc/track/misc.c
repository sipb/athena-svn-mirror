/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 2.2 1987-12-03 17:34:00 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.1  87/12/01  16:44:49  don
 * fixed bugs in readstat's traversal of entries] and statfile:
 * cur_ent is no longer global, but is now part of get_next_match's
 * state. also, last_match() was causing entries[]'s last element to be
 * skipped.
 * 
 * Revision 2.0  87/11/30  15:19:30  don
 * general rewrite; got rid of stamp data-type, with its attendant garbage,
 * cleaned up pathname-handling. readstat & writestat now sort overything
 * by pathname, which simplifies traversals/lookup. should be comprehensible
 * now.
 * 
 * Revision 1.2  87/11/12  16:51:18  don
 * part of general rewrite.
 * 
 * Revision 1.1  87/02/12  21:15:04  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 2.2 1987-12-03 17:34:00 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
 * diagnostic stuff: used throughout track
 */
printmsg()

{
	fprintf(stderr,
		"%s, errno = %d -- %s\nWorking on list named %s and item %s\n",
		prgname,
		errno,
		errmsg,
		subfilename,
		entries[ entnum].fromfile);
}

do_gripe()
{
	if (!quietflag)
		printmsg();
}

do_panic()
{
	printmsg();
	clearlocks();
	exit(1);
}

/*
 * parser-support routines:
 */

doreset()
{
	strcpy(linebuf,"");;
	wordcnt = 0;
}

parseinit( subfile) FILE *subfile;
{
	yyin = subfile;
	yyout = stderr;
	doreset();
}

clear_ent()
{
	int i;
       *entries[ entrycnt].sortkey	=	'\0';
	entries[ entrycnt].keylen	=	  0;
	entries[ entrycnt].followlink	=         0;
	entries[ entrycnt].fromfile	= (char*) 0;
	entries[ entrycnt].tofile	= (char*) 0;
	entries[ entrycnt].cmpfile	= (char*) 0;
	entries[ entrycnt].cmdbuf	= (char*) 0;
	/* add global exceptions
	 */
	for(i=0;i<WORDMAX;i++)
		entries[ entrycnt].exceptions[ i] = (char *) 0;
}

savestr(to,from)
char **to, *from;
{
	if (!(*to = malloc(( unsigned) strlen( from)+1))) {
		sprintf(errmsg,"ran out of memory during parse");
		do_panic();
	}
	strcpy(*to,from);
}
