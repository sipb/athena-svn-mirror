/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 3.0 1988-03-09 13:17:34 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 2.3  88/01/29  18:23:59  don
 * bug fixes. also, now track can update the root.
 * 
 * Revision 2.2  87/12/03  17:34:00  don
 * fixed lint warnings.
 * 
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 3.0 1988-03-09 13:17:34 don Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
 * diagnostic stuff: used throughout track
 */
printmsg()

{
	int i;
	char *s;

	fprintf(stderr, "***%s: %s", prgname, errmsg);

	if ( '\n' != errmsg[ strlen( errmsg) - 1])
		putc('\n', stderr);

	perror("errno is");

	if	( entnum >= 0)	i = entnum;	/* passed parser */
	else if ( entrycnt >= 0)i = entrycnt;	/* in parser */
	else			i = -1;		/* hard to tell */

	if ( *subfilepath)	s = subfilepath;
	else if ( *subfilename)	s = subfilename;
	else			s = "<unknown>";

	fprintf(stderr, "Working on list named %s", s);

	if ( i < 0) fprintf(stderr," before parsing a list-elt.\n");
	else	    fprintf(stderr," & entry '%s'\n", entries[ i].fromfile);
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
	entrycnt = 0;
	clear_ent();
}

clear_ent()
{
	int i;
	struct currentness *c = &entries[ entrycnt].currency;

       *entries[ entrycnt].sortkey	=	'\0';
	entries[ entrycnt].keylen	=	  0;
	entries[ entrycnt].followlink	=         0;
	entries[ entrycnt].fromfile	= (char*) 0;
	entries[ entrycnt].tofile	= (char*) 0;
	entries[ entrycnt].cmpfile	= (char*) 0;
	entries[ entrycnt].cmdbuf	= (char*) 0;
       *c->name  =	  '\0';
	c->link  =  (char*) 0;
	c->cksum =	    0;
	clear_stat( &c->sbuf);

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
