#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987,1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psutil.c,v 1.2 1996-10-14 05:04:43 ghudson Exp $";
#endif
/* psutil.c
 *
 * Copyright (C) 1985,1987,1990,1991,1992 Adobe Systems Incorporated. All
 *  rights reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * common utility subroutines
 *
 * RCSLOG:
 * $/Log: psutil.c,v/$
 * Revision 3.10  1993/05/25  21:42:27  snichols
 * cleanup for Solaris
 *
 * Revision 3.9  1992/11/25  20:17:05  snichols
 * Quotestring should be set back to false if no quote if found.
 *
 * Revision 3.8  1992/11/23  23:17:00  snichols
 * fixed bug in multi-line values.
 *
 * Revision 3.7  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.6  1992/07/14  22:11:41  snichols
 * Updated copyrights.
 *
 * Revision 3.5  1992/06/01  21:32:58  snichols
 * fixed bug in composing ppd file name under certain circumstances.
 *
 * Revision 3.4  1992/05/18  20:31:13  snichols
 * Support for SymbolValues in PPD files.
 *
 * Revision 3.3  1992/04/21  21:00:17  snichols
 * Fixed infinite loop bug in GetPPD.
 *
 * Revision 3.2  1992/04/10  16:28:27  snichols
 * exported ParseLine so print panel code could use it.
 *
 * Revision 3.1  1992/01/16  23:27:27  snichols
 * added a null return at end of parseppd; this was the source of
 * unexplained core dumps on sparcs.
 *
 * Revision 3.0  1991/06/17  16:52:41  snichols
 * Release 3.0
 *
 * Revision 2.12  1991/06/11  23:25:38  snichols
 * needed to compare to *mainkey, not mainkey, in ParseLine.
 *
 * Revision 2.11  1991/05/13  23:54:36  snichols
 * slashes inside quotes do not signal translations.
 *
 * Revision 2.10  1991/04/19  22:56:06  snichols
 * initialize "slash" before using it in ParseLine; not all architectures
 * like it otherwise.
 *
 * Revision 2.9  1991/03/28  23:47:32  snichols
 * isolated code for finding PPD files to one routine, in psutil.c.
 *
 * Revision 2.8  1991/03/14  22:33:49  snichols
 * special case code for distinguishing filename slashes from translation
 * slashes in PPD files.
 *
 * Revision 2.7  1991/03/06  21:53:03  snichols
 * left out arg to strncmp.
 *
 * Revision 2.6  1991/02/07  13:52:01  snichols
 * rewrote parser; now handles 4.0 PPD Files.
 *
 * Revision 2.5  90/12/12  10:46:26  snichols
 * Changed everything to use strchr instead of index.
 * 
 * Revision 2.4  90/10/11  15:05:26  snichols
 * fixed minor bug in processing mainkeyword.
 * 
 * Revision 2.3  90/08/08  17:34:34  snichols
 * Added parseppd routine, which parses printer description files.
 * 
 * Revision 2.2  87/11/17  16:52:49  byron
 * Release 2.1
 * 
 * Revision 2.1.1.2  87/11/12  13:42:10  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.1  87/04/23  10:26:58  byron
 * Copyright notice.
 * 
 * Revision 2.1  85/11/24  11:51:18  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:57:02  shore
 * Support for System V
 * fixed bug in copyfile
 * added envget and Sys V gethostname
 * 
 * Revision 1.2  85/05/14  11:28:42  shore
 * 
 * 
 *
 */

#include "config-defs.h"
#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <string.h>
#include "transcript.h"

extern char *getenv();

/* copy from file named fn to stream stm */
/* use read and write in hopes that it goes fast */
copyfile(fn, stm)
	char *fn;
	register FILE *stm;
{
    	int fd, fo;
	register int pcnt;
	char buf[BUFSIZ];

	VOIDC fflush(stm);
	fo = fileno(stm);
	if ((fd = open(fn, O_RDONLY, 0)) < 0) return(-1);
	while ((pcnt = read(fd, buf, sizeof buf)) > 0) {
	      if (write(fo, buf, (unsigned) pcnt) != pcnt) return(-2);
	}
	if (pcnt < 0) {perror("copyfile"); return (-1);}
	VOIDC close(fd);
	VOIDC fflush(stm);
	return(0);
}

/* exit with message and code */
VOID pexit(message, code)
char *message;
int code;
{
    perror(message);
    exit(code);
}
/* exit with message and code */
VOID pexit2(prog, message, code)
char *prog, *message;
int code;
{
    fprintf(stderr,"%s: ",prog);
    VOIDC perror(message);
    VOIDC exit(code);
}

/* concatenate s1 and s2 into s0 (of length l0), die if too long
 * returns a pointer to the null-terminated result
 */
char *mstrcat(s0,s1,s2,l0)
char *s0,*s1,*s2;
int	l0;
{
    if (((int)strlen(s1) + (int)strlen(s2)) >= l0) {
	fprintf(stderr,"concatenate overflow %s%s\n",s1,s2);
	VOIDC exit(2);
    }
    return strcat(((s0 == s1) ? s0 : strcpy(s0,s1)),s2);
}

/* envget is a getenv
 * 	if the variable is not present in the environment or
 *	it has the null string as value envget returns NULL
 *	otherwise it returns the value from the environment
 */

char *envget(var)
char *var;
{
    register char *val;
    if (((val = getenv(var)) == NULL) || (*val == '\0'))
    	return ((char *) NULL);
    else return (val);
}


/* System V specific compatibility stuff */

#ifdef SYSV

#include <sys/types.h>
#include <sys/utsname.h>

#define SYSVNAMELIMIT 9

gethostname(name, namelen)
char *name;
int namelen;
{
    struct utsname uts;
    uname(&uts);
    VOIDC strncpy(name,uts.sysname,SYSVNAMELIMIT);
    return(0);
}

#endif

FILE *GetPPD(printer)
    char *printer;
{
    FILE *fp;
    char ppdfile[TSPATHSIZE];
    char *ppd;
    char *p, *q;

    if (printer == NULL)
	return NULL;
    ppd = getenv("PPDPATH");
    if (ppd) {
	q = ppd;
	while (q != NULL) {
	    p = strchr(q, ':');
	    if (p) {
		*p = '\0';
		p++;
	    }
	    strncpy(ppdfile, q, TSPATHSIZE);
	    strncat(ppdfile, "/", TSPATHSIZE);
	    strncat(ppdfile,printer,TSPATHSIZE);
	    strncat(ppdfile,".ppd",TSPATHSIZE);
	    if ((fp = fopen(ppdfile, "r")) != NULL)
		return fp;
	    if ((int)strlen(printer) > 10) {
		strncpy(ppdfile, q, TSPATHSIZE);
		strncat(ppdfile, "/", TSPATHSIZE);
		strncat(ppdfile, printer, 10);
		strncat(ppdfile, ".ppd", TSPATHSIZE);
		if ((fp = fopen(ppdfile, "r")) != NULL)
		    return fp;
	    }
	    q = p;
	}
    }
    else {
	strncpy(ppdfile,PPDDir,TSPATHSIZE);
	strncat(ppdfile,printer,TSPATHSIZE);
	strncat(ppdfile,".ppd",TSPATHSIZE);
	if ((fp = fopen(ppdfile,"r")) != NULL)
	    return fp;
	/* try shorter name */
	if ((int)strlen(printer) > 10) {
	    strncpy(ppdfile,PPDDir,TSPATHSIZE);
	    strncat(ppdfile,printer,10);
	    strncat(ppdfile,".ppd",TSPATHSIZE);
	    if ((fp = fopen(ppdfile, "r")) != NULL)
		return fp;
	}
    }
    return NULL;
}

int ParseLine(line, mainkey, option, opttran, val, valtran)
    char *line;
    char **mainkey;
    char **option;
    char **opttran;
    char **val;
    char **valtran;
{
    char *blank, *colon, *value, *slash;
    char *endofquote;
    char *q;

    blank = strchr(line,' ');
    colon = strchr(line,':');
    if (blank) {
	*blank = '\0';
	blank++;
    }
    else return -1;
    if (colon) {
	*colon = '\0';
	colon++;
    }
    else return -2;

    *mainkey = line;
    *option = NULL;
    *opttran = NULL;
    *val = NULL;
    *valtran = NULL;

    slash = NULL;

    if (colon < blank) {
	/* no option, get value */
	value = colon;
	value++;		/* need to skip over extra null */
	while (*value == ' ') value++;
	if (*value == '"') {
	    q = value;
	    q++;
	    if (q)
		endofquote = strchr(q,'"');
	}
	else endofquote = value;
	if (strcmp(*mainkey, "*Include")) {
	    /* special case include, and don't search for value
	       translations */
	    if (endofquote)
		slash = strchr(endofquote,'/');
	    if (slash) {
		/* value translation */
		*slash = '\0';
		slash++;
		*valtran = slash;
	    }
	}
	*val = value;
	return 1;
    }
    /* there is an option keyword */
    while (*blank == ' ') blank++;
    /* blank is now pointing to the beginning of option */
    *option = blank;
    while (*colon == ' ') colon++;
    /* colon is now pointing to beginning of value */
    value = colon;
    slash = strchr(blank,'/');
    if (slash) {
	/* option translation */
	*slash = '\0';
	slash++;
	*opttran = slash;
    }
    if (*value == '"') {
	q = value;
	q++;
	if (q)
	    endofquote = strchr(q,'"');
    }
    else endofquote = value;
    if (endofquote)
	slash = strchr(endofquote,'/');
    if (slash) {
	/* value translation */
	*slash = '\0';
	slash++;
	*valtran = slash;
    }
    *val = value;
    return 1;
}



char *parseppd(fptr,mainkeyword, optionkeyword)
    FILE *fptr;
    char *mainkeyword, *optionkeyword;
{
    /* looks for mainkeyword optionkeyword, returns the value, without the
       quote marks */
    
    char buf[255];
    char mkey[255];
    char *p,*q;
    char *vstring;
    char *key, *option, *value, *otran, *vtran;
    int ret;
    int multiline;
    int quotestring;
    int sizeRemaining;
    FILE *incfile;
    char incname[TSPATHSIZE];
    char *temp;
    char *beg, *end;

    if ((vstring = (char *)malloc(255)) == NULL)
	return NULL;
    vstring[0] = '\0';
    sizeRemaining = 255;
    quotestring = FALSE;
    multiline = FALSE;
    strncpy(mkey, mainkeyword, 255);
    while (fgets(buf,255,fptr)) {
	if (buf[0] == '*') {
	    if (buf[1] == '%')	/* discard comments */
		continue;
	    p = strchr(buf,'\n');
	    if (p) *p = '\0';
	    ret = ParseLine(buf, &key, &option, &otran, &value, &vtran);
	    if (ret > 0) {
		if (!strncmp(key,"*Include",8)) {
		    /* check include files, too */
		    /* +++ check on file name definition */
		    /* strip off delimiters */
		    beg = value;
		    beg++;
		    end = strrchr(beg, *value);
		    if (end) *end = '\0';
		    if (*beg != '/') {
			/* tack on ppddir */
			strncpy(incname, PPDDir, TSPATHSIZE);
			strncat(incname, beg, TSPATHSIZE);
		    }
		    else
			strncpy(incname, beg, TSPATHSIZE);
		    if ((incfile = fopen(incname,"r")) == NULL)
			continue;
		    /* oooo, recursion! */
		    temp = parseppd(incfile, mkey, optionkeyword);
		    if (temp)
			return temp;
		    continue;
		}
		if (!strncmp(key, "*End", 4)) {
		    if (multiline) 
			return vstring;
		}
		if (*value == '"') {
		    /* if there's no closing ", multi-line value */
		    q = value; q++;
		    if ((p = strrchr(q,'"')) == NULL)
			quotestring = TRUE;
		    else
			*p = '\0';
		    *value = '\0';
		    value++;
		}
		else {
		    quotestring = FALSE;
		}
		if (!strcmp(key, mkey)) {
		    /* matched main keyword */
		    if (optionkeyword) {
			if (option) {
			    if (!strcmp(option, optionkeyword)) {
				/* matched both, set vstring */
				if (*value == '^') {
				    /* SymbolValue, actual value later in
				       file */
				    temp = parseppd(fptr, "*SymbolValue",
						    value);
				    if (temp)
					return temp;
				    else return NULL;
				}
				else {
				    strcpy(vstring,value);
				    sizeRemaining -= (int)strlen(value);
				    if (quotestring)
					multiline = TRUE;
				    else
					return vstring;
				}
			    }
			    else if (otran) {
				if (!strcmp(optionkeyword,otran)) {
				    /* matched translation */
				    strcpy(vstring,value);
				    sizeRemaining -= (int)strlen(value);
				    if (quotestring)
					multiline = TRUE;
				    else
					return vstring;
				}
			    }
			    else				    
				continue;
			}
			else return NULL;
		    }
		    else {
			/* no options */
			strcpy(vstring,value);
			sizeRemaining -= (int)strlen(value);
			if (quotestring)
			    multiline = TRUE;
			else
			    return vstring;
		    }
		}
		else if (!strncmp(key, "*OpenUI", 7)) {
		    /* check for match to translation of mkey */
		    if (vtran) {
			if (!strcmp(mkey, vtran)) {
			    /* matched translation, replace with real key */
			    strncpy(mkey, value, 255);
			    continue;
			}
		    }
		}
		else continue;
	    }
	    else continue;
	}
	else {
	    if (multiline) {
		/* check for closing ", and dump into vstring */
		if (p = strrchr(buf,'"')) {
		    *p = '\0';
		    multiline = FALSE;
		}
		if ((int)strlen(buf) >= sizeRemaining) {
		    vstring = (char *)realloc(vstring,
					      (int)strlen(vstring)+255); 
		    sizeRemaining = 255;
		}
		strncat(vstring, buf, 255);
		sizeRemaining -= (int)strlen(buf);
		if (!multiline)
		    return vstring;
	    }
	}
    }
    return NULL;
}



