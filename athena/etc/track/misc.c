/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 2.1 1987-12-01 16:44:49 don Exp $
 *
 *	$Log: not supported by cvs2svn $
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 2.1 1987-12-01 16:44:49 don Exp $";
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
 * parser / lexer support routines:
 */

skipword(theptr)
char **theptr;
{
	while((**theptr != '\0') && isprint(**theptr) && (!isspace(**theptr)))
		(*theptr)++;
}

skipspace(theptr)
char **theptr;
{
	while((**theptr != '\0') && isspace(**theptr))
		(*theptr)++;
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
	extern char *malloc();

	if (!(*to = malloc(strlen(from)+1))) {
		sprintf(errmsg,"ran out of memory during parse");
		do_panic();
	}
	strcpy(*to,from);
}
