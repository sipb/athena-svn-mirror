/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 1.2 1987-11-12 16:51:18 don Exp $
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/02/12  21:15:04  rfrench
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 1.2 1987-11-12 16:51:18 don Exp $";
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
		entries[cur_ent].fromfile);
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

parseinit()
{
	if ( *subfilepath);
        else sprintf( subfilepath, "%s%s/%s", twdir, DEF_SUBDIR, subfilename);
        if (!( yyin = fopen( subfilepath, "r"))) {
                sprintf( errmsg, "Can't open %s\n", subfilepath);
                do_panic();
        }
	yyout = stderr;
	doreset();
}

clear_ent()
{
	int i,j;
	char ebuf[WORDLEN],*eptr;

	entries[entrycnt].followlink  =         0;
	entries[entrycnt].fromfile    = (char*) 0;
	entries[entrycnt].tofile      = (char*) 0;
	entries[entrycnt].cmpfile     = (char*) 0;
	/*
	**	add global exceptions
	*/
	eptr = g_except;
	skipspace(&eptr);
	for(i=0; *eptr != '\0';i++)
	{
		/* copy eptr's first word to ebuf:
		 */
		for ( j = 0; eptr[ j]; j++) {
			if ( !isprint( eptr[ j]) ||
			      isspace( eptr[ j])) break;
			ebuf[ j] = eptr[ j];
		}
		ebuf[ j + 1] = '\0';

		savestr(&entries[entrycnt].exceptions[i],ebuf);
		skipword(&eptr);
		skipspace(&eptr);
	}
	
	/*
	**	and clear the rest of the exceptions
	*/
	for(;i<WORDMAX;i++)
		entries[entrycnt].exceptions[i] = (char*) 0;
	entries[ entrycnt].cmdbuf      = (char*) 0;
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
