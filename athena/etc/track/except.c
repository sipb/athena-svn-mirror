/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 1.1 1987-02-12 21:14:36 rfrench Exp $
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_header_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/except.c,v 1.1 1987-02-12 21:14:36 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include "track.h"

/*
**	routine to implement exception lists
**	returns 1 if the filename passed in theline is
**		matches the string passed in rootstring
**		and does NOT match any of the exceptions
**		found in the entry pointed at by entnum
**	returns 0 otherwise
**	
*/

goodname(theline,rootstring,entnum)
char *rootstring;
char *theline;
int entnum;
{
	extern char *index(), *re_conv();
	char buf[LINELEN],buf2[LINELEN],*ptr,*ptr2;
	int i;

	strcpy(buf,theline);
	/*
	**	chop off newline if there is one
	*/
	if (*buf && (buf[strlen(buf)-1] == '\n'))
		buf[strlen(buf)-1] = '\0';

	if (debug)
		printf("goodname(%s,%s,%d)\n",buf,rootstring,entnum);

	/*
	**	find the first /
	*/
	if (!(ptr = index(buf,'/'))) {
		sprintf(errmsg,"can't find / in %s\n", ptr);
		do_panic();
	}

	/*
	**	and chop off any trailing white space
	*/
	for(ptr2 = ptr; *ptr2;ptr2++)
		if (isspace(*ptr2)) {
			*ptr2 = '\0';
			break;
		}

	/*
	**	does the beginning of the string match the head pattern
	*/
	if (!headmatch(ptr,rootstring))
		return(0);

	/*
	** if this is the end of word, it's a match.
	*/
	if (strlen(ptr) == strlen(rootstring))
		return(1);

	/*
	**	the head matches, so now we have to check the tail
	**	get just the tail in buf2
	**	SKIPPING THE / SEPERATING THE HEAD AND TAIL
	*/
	strcpy(buf2,ptr+strlen(rootstring)+1);
	
	/*
	**	compare the tail with each exception
	**	if there is a match, the return(0) else return(1)
	*/
	for(i=0;entries[entnum].exceptions[i];i++) {
		if (nofunny(entries[entnum].exceptions[i])) {
			if(!strcmp(buf2,entries[entnum].exceptions[i]))
				return(0);
		}
		else {
			if( re_comp(re_conv(entries[entnum].exceptions[i]))) {
				sprintf(errmsg,"%s bad regular expression\n",
						entries[entnum].exceptions[i]);
				do_panic();
			}
			if (re_exec(buf2))
				return(0) ;
		}
	}

	return(1);
}

nofunny(ptr)
char *ptr;
{
	return( ! (index(ptr,'*') ||
		   index(ptr,'[') ||
		   index(ptr,']') ||
		   index(ptr,'?')));
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

headmatch(s1,root)
char *s1,*root;
{
	char nextchar;

	if (!strncmp(s1,root,strlen(root))) {
		nextchar = *(s1+strlen(root));
		if ((nextchar == '\0') || (nextchar == '/'))
			return(1);
		else
			return(0);
	}

	return(0);
}
