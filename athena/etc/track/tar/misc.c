/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/misc.c,v 1.1 1993-10-12 04:48:49 probe Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 4.1  88/05/17  18:59:17  don
 * fixed another bug in GLOBAL handling, by simplifying pattern-list
 * traversal. now, global-list is chained onto end of each entry's list.
 * 
 * Revision 4.0  88/04/14  16:42:53  don
 * this version is not compatible with prior versions.
 * it offers, chiefly, link-exporting, i.e., "->" systax in exception-lists.
 * it also offers sped-up exception-checking, via hash-tables.
 * a bug remains in -nopullflag support: if the entry's to-name top-level
 * dir doesn't exist, update_file doesn't get over it.
 * the fix should be put into the updated() routine, or possibly dec_entry().
 * 
 * Revision 3.0  88/03/09  13:17:34  don
 * this version is incompatible with prior versions. it offers:
 * 1) checksum-handling for regular files, to detect filesystem corruption.
 * 2) more concise & readable "updating" messages & error messages.
 * 3) better update-simulation when nopullflag is set.
 * 4) more support for non-default comparison-files.
 * finally, the "currentness" data-structure has replaced the statbufs
 * used before, so that the notion of currency is more readily extensible.
 * note: the statfile format has been changed.
 * 
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
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/misc.c,v 1.1 1993-10-12 04:48:49 probe Exp $";
#endif lint

#include "bellcore-copyright.h"
#include "mit-copyright.h"

#include "track.h"

/*
 * diagnostic stuff: used throughout track
 */
printmsg( filep) FILE *filep;
{
	int i;
	char *s;
	extern int sys_nerr;
	extern char *sys_errlist[];

	if ( filep);
	else if ( nopullflag) return;
	else if ( logfile = fopen( logfilepath, "w+")) {
		fchmod( logfile, 0664);
		filep = logfile;
	}
	else {
		fprintf( stderr, "can't open logfile %s.\n", logfilepath);
		perror("system error is: ");
		clearlocks();
		exit(1);
	}
	fprintf( filep, "\n***%s: %s", prgname, errmsg);

	if ( '\n' != errmsg[ strlen( errmsg) - 1])
		putc('\n', filep);

	if ( errno < sys_nerr) 
	     fprintf( filep, "   system error is '%s'.\n", sys_errlist[ errno]);
	else fprintf( filep, "   system errno is %d ( not in sys_errlist).\n",
		      errno);

	if	( entnum >= 0)	i = entnum;	/* passed parser */
	else if ( entrycnt >= 0)i = entrycnt;	/* in parser */
	else			i = -1;		/* hard to tell */

	if ( *subfilepath)	s = subfilepath;
	else if ( *subfilename)	s = subfilename;
	else			s = "<unknown>";

	fprintf( filep, "   Working on list named %s", s);

	if ( i < 0) fprintf( filep," before parsing a list-elt.\n");
	else	    fprintf( filep," & entry #%d: '%s'\n",
			    i, entries[ i].fromfile);

	/* a nuance of formatting:
	 * we want to separate error-msgs from the update banners
	 * with newlines, but if the update banners aren't present,
	 * we don't want the error-msgs to be double-spaced.
	 */
	if ( verboseflag && filep == stderr)
		fputc('\n', filep);
}

do_gripe()
{
	printmsg( logfile);
	if ( quietflag) return;
	printmsg( stderr);

	if ( ! verboseflag) {
		/* turn on verbosity, on the assumption that the user
		 * now needs to know what's being done.
		 */
		verboseflag = 1;
		fprintf(stderr, "Turning on -v option. -q suppresses this.\n");
	}
}

do_panic()
{
	printmsg( logfile);
	printmsg( stderr);
	clearlocks();
	exit(1);
}

/*
 * parser-support routines:
 */

doreset()
{
	*linebuf = '\0';
	*wordbuf = '\0';
}

parseinit( subfile) FILE *subfile;
{
	yyin = subfile;
	yyout = stderr;
	doreset();
	entrycnt = 0;
	clear_ent();
	errno = 0;
}

Entry *
clear_ent()
{
	Entry* e = &entries[ entrycnt];
	struct currentness *c = &e->currency;

       *e->sortkey	=	'\0';
	e->keylen	=	  0;
	e->followlink	=         0;
	e->fromfile	= (char*) 0;
	e->tofile	= (char*) 0;
	e->cmpfile	= (char*) 0;
	e->cmdbuf	= (char*) 0;

       *c->name  =	  '\0';
       *c->link  =	  '\0';
	c->cksum =	    0;
	clear_stat( &c->sbuf);

	e->names.table	= (List_element**) 0;
	e->names.shift  = 0;
	e->patterns     = entries[ 0].patterns; /* XXX global patterns */

	return( e);
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
