/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 1.1 1987-02-12 21:15:04 rfrench Exp $
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/misc.c,v 1.1 1987-02-12 21:15:04 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

printmsg()
{
	fprintf(stderr,
		"%s, errno = %d -- %s\nWorking on list named %s and item %s\n",
		prgname,
		errno,
		errmsg,
		subname,
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


mycpy(to,from)
char *to, *from;
{
	while ((*from != '\0') && isprint(*from) && (!isspace(*from)))
		*to++ = *from++;
	*to = '\0';
}

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

doreset()
{
	strcpy(linebuf,"");;
	wordcnt = 0;
}

clear_ent()
{
	int i;
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
	for(i=0;*eptr != '\0';i++)
	{
		mycpy(ebuf,eptr);
		savestr(&entries[entrycnt].exceptions[i],ebuf);
		skipword(&eptr);
		skipspace(&eptr);
	}
	
	/*
	**	and clear the rest of the exceptions
	*/
	for(;i<WORDMAX;i++)
		entries[entrycnt].exceptions[i] = (char*) 0;
	entries[entrycnt].cmdbuf      = (char*) 0;
}

parseinit()
{
	yyin = subfile;
	yyout = stderr;
	doreset();
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

mapname(theline,from,to)
char *theline,*from,*to;
{
	char buf[LINELEN],word1[LINELEN],word2[LINELEN],*ptr;

	if (debug)
		printf("mapname(%s,%s,%s)\n",theline,from,to);

	strcpy(buf,theline);
	if (!strcmp(from,to))
		return(0);
		
	/*
	**	start from first slash
	*/
	if (!(ptr = index(buf,'/'))) {
		sprintf(errmsg,"can't find / in :%s:\n",buf);
		do_gripe();
		return(0);
	}
	/*
	**	skip first character on the line
	*/
	mycpy(word1,ptr);
	strcpy(word2,"");
	strncpy(word2,buf,ptr-buf);
	word2[ptr-buf] ='\0';
	if (!strncmp(word1,from,strlen(from)))
		sprintf(theline,"%s%s%s",
		       word2,to,ptr+strlen(from));

	if (debug)
		printf("mapname returns %s\n",theline);

	return(0);
}
