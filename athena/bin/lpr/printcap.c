/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)printcap.c	5.1 (Berkeley) 6/6/85";
#endif not lint

#define MAXHOP	32	/* max number of tc= indirections */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef HESIOD
#include <hesiod.h>
#include "lp.local.h"
#endif
#ifndef BUFSIZ
#define	BUFSIZ	1024
#endif

/*
 * termcap - routines for dealing with the terminal capability data base
 *
 * BUG:		Should use a "last" pointer in tbuf, so that searching
 *		for capabilities alphabetically would not be a n**2/2
 *		process when large numbers of capabilities are given.
 * Note:	If we add a last pointer now we will screw up the
 *		tc capability. We really should compile termcap.
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 *
 * 4/13/87	Modified to use Hesiod name server iff HESIOD
 * SDyer	is defined.  If no printer capability is found in the
 * Athena	/etc/printcap file, Hesiod is queried for an alias for
 *		the given printer name and its capability.  In looking
 *		up the alias, Hesiod may also be queried for the host's
 *		cluster name.
 */

#define PRINTCAP

#ifdef PRINTCAP
#define tgetent	pgetent
#define tskip	pskip
#define tgetstr	pgetstr
#define tdecode pdecode
#define tgetnum	pgetnum
#define	tgetflag pgetflag
#define tdecode pdecode
#define tnchktc	pnchktc
#define	tnamatch pnamatch
#ifdef E_TERMCAP
#undef E_TERMCAP
#endif
#define E_TERMCAP "/etc/printcap"
#define V6
#endif

static	FILE *pfp = NULL;	/* printcap data base file pointer */
static	char *tbuf;
static	int hopcount;		/* detect infinite loops in termcap, init 0 */
char	*tskip();
char	*tgetstr();
char	*tdecode();
char	*getenv();

#ifdef HESIOD
/* Ask Hesiod if printer name is an alias.  If so,
 * put new name in "name" and return 1.  If not, return 0.
 * format:  -Pprinter[.cluster]
 */

int pralias(buf, name)
	char buf[];		/* buffer to put printer alias in */
	char *name;		/* name of printer to look up */
{
	char *e, temp[BUFSIZ/2];
#ifdef OLD_DEFAULT_ALIAS
	char **hv;
#endif
	char *getclus();
	
	strcpy(temp, name);
	/* If printer name == "default" then lookup based on LPR cluster info*/
	if(!strcmp(name, DEFLP)) {
	    if ((e = getclus()) != NULL) {
		strcpy(buf, e);
		return(1);
	    }
	}

#ifdef OLD_DEFAULT_ALIAS
	/* if not in "printer.cluster" format, look up cluster and append */
	if (index(name, '.') == NULL)
		if ((e = getenv("LPR")) != NULL) {
			strcat(temp, ".");
			strcat(temp, e);
		} else if ((e = getclus()) != NULL) {
			strcat(temp, ".");
			strcat(temp, e);
		}
	if ((hv = hes_resolve(temp, "lpralias")) != NULL &&
	    strlen(*hv) < BUFSIZ/2) {
		strcpy(buf, *hv);
		return(1);
	}
#endif
	return (0);
}

/* Find this host's cluster name */
char *
getclus()
{
	static char cluster[BUFSIZ/2];
	char host[32];
	char **hv;
	int len = 4;  /* length of string "lpr " */

	gethostname(host, sizeof (host));
	if ((hv = hes_resolve(host, "cluster")) == NULL)
		return NULL;
	while (*hv) {
		if (strncmp(*hv, "lpr ", len) == 0) {
			strcpy(cluster, *hv + len);
			return cluster;
			}
		++hv;
	}
	return NULL;
}
	
/* 
 * Look for printer capability in Hesiod.
 * If eventually found, copy into "line" and return 1, otherwise
 * return 0.
 */

hpgetent(line, print)
        char *line;
        char *print;
{
	register char **hv;
	char **hes_resolve();

	tbuf = line;
	if ( (hv = hes_resolve(print, "pcap")) != NULL) {
		strcpy(line, *hv);
		return(tnchktc());
	}
	return 0;
}

#endif HESIOD

/*
 * Similar to tgetent except it returns the next entry instead of
 * doing a lookup.
 */
getprent(bp)
	register char *bp;
{
	register int c, skip = 0;

	if (pfp == NULL && (pfp = fopen(E_TERMCAP, "r")) == NULL)
		return(-1);
	tbuf = bp;
	for (;;) {
		switch (c = getc(pfp)) {
		case EOF:
			fclose(pfp);
			pfp = NULL;
			return(0);
		case '\n':
			if (bp == tbuf) {
				skip = 0;
				continue;
			}
			if (bp[-1] == '\\') {
				bp--;
				continue;
			}
			*bp = '\0';
			return(1);
		case '#':
			if (bp == tbuf)
				skip++;
		default:
			if (skip)
				continue;
			if (bp >= tbuf+BUFSIZ) {
				write(2, "Termcap entry too long\n", 23);
				*bp = '\0';
				return(1);
			}
			*bp++ = c;
		}
	}
}

endprent()
{
	if (pfp != NULL)
		fclose(pfp);
	pfp = NULL;
}

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */

tgetent(bp, name)
	char *bp, *name;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[BUFSIZ];
#ifndef V6
	char *cp2;
#endif
	int tf;

	tbuf = bp;
	tf = 0;
#ifndef V6
	cp = getenv("TERMCAP");
	/*
	 * TERMCAP can have one of two things in it. It can be the
	 * name of a file to use instead of /etc/termcap. In this
	 * case it better start with a "/". Or it can be an entry to
	 * use so we don't have to read the file. In this case it
	 * has to already have the newlines crunched out.
	 */
	if (cp && *cp) {
		if (*cp!='/') {
			cp2 = getenv("TERM");
			if (cp2==(char *) 0 || strcmp(name,cp2)==0) {
				strcpy(bp,cp);
				return(tnchktc());
			} else {
				tf = open(E_TERMCAP, 0);
			}
		} else
			tf = open(cp, 0);
	}
	if (tf==0)
		tf = open(E_TERMCAP, 0);
#else
	tf = open(E_TERMCAP, 0);
#endif
	if (tf < 0)
		return (-1);
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZ);
				if (cnt <= 0) {
					close(tf);
					return (0);
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+BUFSIZ) {
				write(2,"Termcap entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (tnamatch(name)) {
			close(tf);
			return(tnchktc());
		}
	}
}

/*
 * tnchktc: check the last entry, see if it's tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
tnchktc()
{
	register char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[BUFSIZ];
	char *holdtbuf = tbuf;
	int l;

	p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
	while (*--p != ':')
		if (p<tbuf) {
			write(2, "Bad termcap entry\n", 18);
			return (0);
		}
	p++;
	/* p now points to beginning of last field */
	if (p[0] != 't' || p[1] != 'c')
		return(1);
	strcpy(tcname,p+3);
	q = tcname;
	while (*q && *q != ':')/* was while (q && ... *//* SPD Athena 4/15/87 */
		q++;
	*q = 0;
	if (++hopcount > MAXHOP) {
		write(2, "Infinite tc= loop\n", 18);
		return (0);
	}
#ifdef HESIOD
	if (hpgetent(tcbuf, tcname) != 1)
#else
	if (tgetent(tcbuf, tcname) != 1)
#endif
		return(0);
	for (q=tcbuf; *q != ':'; q++)
		;
	l = p - holdtbuf + strlen(q);
	if (l > BUFSIZ) {
		write(2, "Termcap entry too long\n", 23);
		q[BUFSIZ - (p-tbuf)] = 0;
	}
	strcpy(p, q+1);
	tbuf = holdtbuf;
	return(1);
}

/*
 * Tnamatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
tnamatch(np)
	char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
tskip(bp)
	register char *bp;
{

	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (bp);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
tgetnum(id)
	char *id;
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
tgetflag(id)
	char *id;
{
	register char *bp = tbuf;

	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return(0);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
tgetstr(id, area)
	char *id, **area;
{
	register char *bp = tbuf;
	
	for (;;) {
		bp = tskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(0);
		if (*bp != '=')
			continue;
		bp++;
		return (tdecode(bp, area));
	}
}

/*
 * Tdecode does the grung work to decode the
 * string capability escapes.
 */
static char *
tdecode(str, area)
	register char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}
