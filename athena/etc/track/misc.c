/*
 *	$Id: misc.c,v 4.9 1999-08-13 00:15:12 danw Exp $
 */

#ifndef lint
static char *rcsid_header_h = "$Id: misc.c,v 4.9 1999-08-13 00:15:12 danw Exp $";
#endif

#include "bellcore-copyright.h"
#include "mit-copyright.h"

#include "track.h"

/*
 * diagnostic stuff: used throughout track
 */
void
printmsg( filep) FILE *filep;
{
	int i;
	char *s;

	if ( filep);
	else if ( nopullflag) return;
	else if ( logfile = fopen( logfilepath, "w+")) {
		(void) fchmod( fileno(logfile), 0664);
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

	fprintf( filep, "   system error is '%s'.\n", strerror( errno));

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

void
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

void
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

void
doreset()
{
	*linebuf = '\0';
	*wordbuf = '\0';
}

void
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

	e->islink	=	  0;
       *e->sortkey	=	'\0';
	e->keylen	=	  0;
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

void
savestr(to,from)
char **to, *from;
{
	if (!(*to = malloc(( unsigned) strlen( from)+1))) {
		sprintf(errmsg,"ran out of memory during parse");
		do_panic();
	}
	strcpy(*to,from);
}

/* Convert mode (file type) bits to a string */

char *mode_to_string(mode)
int mode;
{
  switch(mode & S_IFMT)
    {
    case S_IFDIR:
      return "directory";
    case S_IFCHR:
      return "char-device";
    case S_IFBLK:
      return "block-device";
    case S_IFREG:
      return "file";
#ifdef S_IFIFO
    case S_IFIFO:
      return "fifo";
#endif
    case S_IFLNK:
      return "symlink";
    case S_IFSOCK:
      return "socket";
#ifdef S_IFMPX
    case S_IFMPX:
      return "multi char-device";
#endif
    default:
      return "nonexistent";
    }
}

char *mode_to_fmt(mode)
int mode;
{
  switch(mode & S_IFMT)
    {
    case S_IFDIR:
      return "d%s %c%d(%d.%d.%o)\n";
#ifdef S_IFMPX
    case S_IFMPX:
#endif
    case S_IFCHR:
      return "c%s %c%d(%d.%d.%o)\n";
    case S_IFBLK:
      return "b%s %c%d(%d.%d.%o)\n";
    case S_IFREG:
      return "f%s %c%x(%d.%d.%o)%ld\n";
    case S_IFLNK:
      return "l%s %c%s\n";
#ifdef S_IFIFO
    case S_IFIFO:
#endif
    case S_IFSOCK:
      return "*ERROR (write_statline): can't track socket %s.\n";
    default:
      return "*ERROR (write_statline): %s's file type is unknown.\n";
    }
}

char *mode_to_rfmt(mode)
int mode;
{
  switch(mode & S_IFMT)
    {
#ifdef S_IFMPX
    case S_IFMPX:
#endif
    case S_IFDIR:
    case S_IFCHR:
    case S_IFBLK:
      return "%d(%d.%d.%o)\n";
    case S_IFREG:
      return "%x(%d.%d.%o)%ld\n";
    default:
      return "";
    }
}
