/* 
 * icombine:  combine multiple ispell dictionary entries into a single 
 *            entry with the options of all entries
 *
 * Author:  Gary Puckering
 *          Cognos, Inc.
 *
 * Written:  January 29, 1987
 * 
 * Notes:  Input lines consist of a word followed optionally by
 *         by one or more flags.  e.g CREATE/V/N/S
 *         
 *         No editing on flags is performed.
 *         The input line is upshifted.
 *         An improper flag, like /XN will be output as /X/N.
 *         Flags are output in alphabetical order.
 *         Non-letter appearing before the first "/" are retained,
 *           those after are dropped.
 */

#include <stdio.h>
#include <ctype.h>

#define MAXFLAGS 26     /* letters A-Z */
#define MAXLINE 255     /* maximum line size */

#define TRUE 1
#define FALSE 0
typedef int bool;

bool flagtbl[MAXFLAGS]; /* array of flag options */

char line[MAXLINE];     /* current line */
char lastword[MAXLINE]; /* previous word */
char word[MAXLINE];     /* current word */
char flags[MAXLINE];    /* current flags */
int  expand = 0;	/* if NZ, expand instead of combining */

extern char *strcpy ();


char *getline()
{
    char *rvalue;
    char *p;

    if (rvalue=gets(line))      
    { p = line;
      while (*p) { if (islower(*p)) *p=toupper(*p); p++; }
    }
    return(rvalue);
}

main(argc,argv)
int argc;
char *argv[];
{

    if (argc > 1  &&  strcmp (argv[1], "-e") == 0)
	expand = 1;
    if (getline()) 
    {
        parse(line,lastword,flags);
        getflags(flags);
    }
    else
	return 0;

    while (getline())
    {
        parse(line,word,flags);
        if (strcmp(word,lastword)!=0)   /* different word */
        {
            putword();
            (void) strcpy(lastword,word);
        }
        getflags(flags);
    }
    putword();
    return 0;
}

putword()
{
    printf("%s",lastword);
    putflags();
}

parse(ln,wrd,flgs)
    char ln[];
    char wrd[];
    char flgs[];
{
    register char *p, *q;

    /* copy line up to first "/" or to end */
    for (p=ln,q=wrd; *p && *p != '/'; p++,q++) *q = *p;
    *q = NULL;

    (void) strcpy(flgs,p);     /* copy from "/" to end */
}

getflags(flgs)
    char *flgs;
{
    register char *p;

    for (p=flgs; *p; p++) 
        if (*p != '/')
	{
	    if (islower (*p))
		*p = toupper (*p);
	    if (isupper(*p))
		flagtbl[(*p)-'A'] = TRUE;
	}
}

putflags()
{
    register int i;
    int slashout = 0;

    if (expand)
	putchar ('\n');

    for (i=0; i<MAXFLAGS; i++) 
        if (flagtbl[i]) 
        {
	    if (expand)
		printf("%s/%c\n", lastword, i + 'A');
            else
	    {
		if (!slashout)
		    putchar('/');
		slashout = 1;
		putchar(i+'A');
	    }
            flagtbl[i]=FALSE;
        }
    if (!expand)
	putchar('\n');
}
