/*
 * Copyright (c) 1996 Paul Vojta.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 *	filefind.c - Routines to perform file searches.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <pwd.h>

#include "filf_app.h"		/* application-related declarations & defs */
#include "filefind.h"

#ifndef	XtNumber
#define	XtNumber(array)	(sizeof(array) / sizeof(*array))
#endif

#ifndef	ST_NLINK_TRICK
#if	defined(VMS)
#define	ST_NLINK_TRICK	0
#else
#define	ST_NLINK_TRICK	1
#endif
#endif	/* !defined(ST_NLINK_TRICK) */

extern	char	*newstring ARGS((_Xconst char *, int));

	/* these are defined so that the editor can match braces properly. */
#define	LBRACE	'{'
#define	RBRACE	'}'

/*	Local copy of findrec record. */

static	struct findrec	lrec;

/*
 *	Additional data structures.
 */

struct	statrec {		/* status vis-a-vis laying down the file name */
	int	pos;			/* position in ffline */
	int	flags;
	char	quickchar;		/* '\0' or 'q' or 'Q' */
	int	quickpos;		/* value of pos (above) if 'q' or 'Q' */
};

struct	texmfrec {		/* list of texmf components */
	struct texmfrec	*next;
	_Xconst	char	*home;
	_Xconst	char	*str;
	int		len;
	int		flags;
};

#if	!CFG_FILE
static	_Xconst char	default_texmf_path[]	= DEFAULT_TEXMF_PATH;
#endif

/*	These are global variables. */

_Xconst	char			*fF_values[MAX_N_OPTS];	/* not static */

static	FILE			*file_found;
static	jmp_buf			got_it;
static	struct steprec		**bracepp;
static	struct steprec		**steppp;
static	struct findrec		*frecp;		/* used to distinguish ls-Rs */
static	int			seqno;
static	struct texmfrec		*texmfhead	= NULL;
static	struct rootrec		**rootpp;
static	_Xconst	struct atomrec	*treeatom;
#ifdef	PK_AND_GF
static	int			gflags;		/* global flags */
static	Boolean			pk_second_pass;
#endif

	/* dummy records for handling %s after // or wild cards */
static	struct atomrec	pct_s_dummy = {NULL, NULL, NULL, F_PCT_S};
static	struct atomrec	pct_s_dummy_slash
			= {NULL, NULL, NULL, F_PCT_S | F_SLASH_SLASH};

	/* flag value for directories not read yet */
static	struct treerec	not_read_yet = {NULL, NULL, NULL};

/*
 *	The following expands in 128-byte increments to be as long as is
 *	necessary.  We don't often use pointers to refer to elements of this
 *	array, since it may move when it is enlarged.
 */

#ifndef	EXTERN_FFLINE
static	char	*ffline		= NULL;
static	int	ffline_len	= 0;	/* current length of the array */
#endif


/*
 *	ls-R database hash table
 */

struct	lsr {
	struct lsr	*next;		/* next in hash chain */
	struct findrec	*frecp;		/* which search type? */
	short		seqno;		/* which ls-R invocation we are using */
	short		keylen;		/* length of key */
	_Xconst	char	*key;
	_Xconst	char	*value;
};

static	struct lsr	*lsrtab[1024];	/* hash table */


/*
 *	Forward references.
 */

static	void		dobrace();
static	struct steprec	*scan_pct_s();
static	void		atomize_pct_s();


#ifndef	EXTERN_FFLINE

/*
 *	Expand ffline[] to at least the given size.
 */

static	void
expandline(n)
	int	n;
{
	int	newlen	= n + 128;

	ffline = (ffline == NULL) ? xmalloc(newlen, "space for file paths")
		: xrealloc(ffline, newlen, "space for file paths");
	ffline_len = newlen;
}

#endif	/* !EXTERN_FFLINE */


/*
 *	prehash() - hash function (before modding).
 */

unsigned int
prehash(str, len)
	_Xconst	char	*str;
	int		len;
{
	_Xconst	char	*p;
	unsigned int	hash;

	hash = 0;
	for (p = str + len; p > str; )
	    hash = hash * 5 + *--p;
	return hash;
}


#ifndef	EXTERN_GETPW

/*
 *	Look up the home directory.
 */

static	_Xconst	struct passwd *
ff_getpw(pp, p_end)
	_Xconst	char	**pp;
	_Xconst	char	*p_end;
{
	_Xconst	char		*p	= *pp;
	_Xconst	char		*p1;
	int			len;
	_Xconst	struct passwd	*pw;

	++p;	/* skip the tilde */
	p1 = p;
	while (p1 < p_end && *p1 != '/') ++p1;
	len = p1 - p;
	if (len == 0)	/* if no user name */
	    pw = getpwuid(getuid());
	else {
	    if (len >= ffline_len)
		expandline(len);
	    bcopy(p, ffline, len);
	    ffline[len] = '\0';
	    pw = getpwnam(ffline);
	}
	if (pw != NULL)
	    *pp = p1;
	return pw;
}

#endif	/* !EXTERN_GETPW */


static	void
gethome(pp, p_end, homepp)
	_Xconst	char	**pp;
	_Xconst	char	*p_end;
	_Xconst	char	**homepp;
{
	_Xconst	struct passwd	*pw;

	pw = ff_getpw(pp, p_end);
	if (pw != NULL)
	    *homepp = newstring(pw->pw_dir, -1);
}


#if	CFGFILE

struct envrec		*envtab[128];	/* hash table for local environment */

struct cfglist {		/* linked list of config files we've done */
	struct cfglist	*next;
	_Xconst	char	*value;
};

struct cfglist	*cfghead;	/* head of that linked list */

/* global stuff for the fancy loop over config files. */

static	_Xconst	struct envrec	*lastvar		= NULL;
static	_Xconst	char		*deflt			= DEFAULT_CONFIG_PATH;

/*
 *	Get the node containing a local environment variable.
 */

struct envrec *
ffgetenv(key)
	_Xconst	char	*key;
{
	int		keylen;
	struct envrec	*envp;

	keylen = strlen(key);
	envp = envtab[prehash(key, keylen) % XtNumber(envtab)];
	++keylen;
	for (;;) {
	    if (envp == NULL ||
	      (envp->key != NULL && memcmp(envp->key, key, keylen) == 0))
		break;
	    envp = envp->next;
	}
	return envp;
}


/*
 *	Store the value of a local environment variable.
 */

static	void
ffputenv(key, keylen, value, flag)
	_Xconst	char	*key;
	int		keylen;
	_Xconst	char	*value;
	Boolean		flag;
{
	struct envrec	**envpp;
	struct envrec	*envp;
	_Xconst	char	*key1;

	envpp = envtab + prehash(key, keylen) % XtNumber(envtab);
	++keylen;
	for (;;) {	/* loop to find the key in the hash chain */
	    envp = *envpp;
	    if (envp == NULL) {		/* if no existing entries */
		key1 = newstring(key, keylen);
		break;
	    }
	    if (envp->key != NULL && memcmp(key, envp->key, keylen) == 0) {
		/* skip to the end of the chain of identical keys */
		while (envp->next != NULL && envp->next->key == NULL)
		    envp = envp->next;
		/* If we already defined this variable this time around. */
		if (envp->flag)
		    return;
		/* If this is a getenv() placeholder. */
		if (envp->value == NULL) {
		    envp->value = value;
		    envp->flag = flag;
		    return;
		}
		envpp = &envp->next;
		key1 = NULL;
		break;
	    }
	    envpp = &envp->next;
	}

	envp = (struct envrec *) xmalloc(sizeof(*envp), "Local env record");
	envp->next = *envpp;
	envp->key = key1;
	envp->value = value;
	envp->flag = flag;
	*envpp = envp;
}


#ifdef	SELFAUTO

	/* string containing values of SELFAUTODIR and SELFAUTOPARENT */
static	_Xconst	char	*selfautostr	= NULL;
static	int		selfautodirlen;
static	int		selfautoparentlen;

/*
 *	Resolve a symlink.  Returns the length of the new string.
 */

static	int
getrealname(pos, len)
	int	pos;	/* position in ffline[] of beginning of filename */
	int	len;	/* length of string in ffline[] */
{
	struct stat	statbuf;
	char		*buffer;
	int		bufsize;
	char		*buf1;
	int		pos1;
	int		len1;

	for (;;) {	/* loop over symlinks in chain */
	    if (lstat(ffline + pos, &statbuf) != 0) {
		perror(ffline + pos);
		return len;
	    }
	    if (!S_ISLNK(statbuf.st_mode))
		break;
	    buffer = xmalloc(statbuf.st_size + 1, "symlink contents");
	    bufsize = readlink(ffline + pos, buffer, statbuf.st_size + 1);
	    if (bufsize < 0 || bufsize > statbuf.st_size) {
		perror(ffline + pos);
		return len;
	    }
	    buffer[bufsize] = '\0';
	    buf1 = buffer;
	    if (buffer[0] == '/')	/* if absolute path, just replace */
		pos1 = pos;	/* copy it to the beginning */
	    else {
		pos1 = pos + len;
		/* find preceding slash */
		while (pos1 > pos && ffline[--pos1] != '/') ;
		/* get rid of multiple slashes */
		while (pos1 > pos && ffline[pos1 - 1] == '/') --pos1;
		for (;;) {
		    if (buf1[0] == '.' && buf1[1] == '/')
			buf1 += 2;
		    else if (buf1[0] == '.' && buf1[1] == '.' && buf1[2] == '/')
		    {
			buf1 += 3;
			ffline[pos1] = '\0';
			pos1 = getrealname(pos, pos1 - pos) + pos;
			/* back up to preceding slash */
			while (pos1 > pos && ffline[--pos1] != '/') ;
			/* get rid of multiple slashes */
			while (pos1 > pos && ffline[pos1 - 1] == '/') --pos1;
		    }
		    else break;
		    while (*buf1 == '/') ++buf1;
		}
		++pos1;	/* put the slash back */
	    }
	    len1 = buffer + bufsize + 1 - buf1;
	    if (pos1 + len1 >= ffline_len)
		expandline(pos1 + len1);
	    bcopy(buf1, ffline + pos1, len1);
	    free(buffer);
	    len = pos1 + len1 - pos - 1;
	}
	return len;
}


/*
 *	Initialize SELFAUTODIR and SELFAUTOPARENT.
 */

static	void
selfautoinit(pos)
	int	pos;
{
	_Xconst	char	*p;
	int		len;
	_Xconst	char	*path;
	int		argv0len;
	int		pos1;
	struct stat	statbuf;

	if (index(argv0, '/') != NULL) {  /* if program was called directly */
	    len = strlen(argv0);
	    if (pos + len >= ffline_len)
		expandline(pos + len);
	    bcopy(argv0, ffline + pos, len);
	}
	else {			/* try to find it in $PATH */
	    path = getenv("PATH");
	    argv0len = strlen(argv0) + 1;
	    len = 0;
	    if (path != NULL)
		for (;;) {
		    p = index(path, ':');
		    if (p == NULL) p = path + strlen(path);
		    len = p - path;
		    pos1 = pos + len;
		    if (pos + len + argv0len >= ffline_len)
			expandline(pos + len + argv0len);
		    bcopy(path, ffline + pos, len);
		    ffline[pos1] = '/';
		    bcopy(argv0, ffline + pos1 + 1, argv0len);
		    len += argv0len;
		    if (stat(ffline + pos, &statbuf) == 0
		      && S_ISREG(statbuf.st_mode)
		      && (statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
			break;
		    if (*p == '\0') {
			len = 0;
			break;
		    }
		    path = p + 1;
		}
	}

	/* Now we have:  file name starting in ffline[pos], and len = length. */

	len = getrealname(pos, len);
	p = ffline + pos + len;
	while (p > ffline + pos && *--p != '/') ;
	len = p - (ffline + pos);
	selfautostr = newstring(ffline + pos, len);
	p = selfautostr + len;
	while (p > selfautostr && *--p != '/') ;
	selfautodirlen = p - selfautostr;
	while (p > selfautostr && *--p != '/') ;
	selfautoparentlen = p - selfautostr;

	if (FFDEBUG) {
	    fputs("SELFAUTODIR = ", stdout);
	    fwrite(selfautostr, 1, selfautodirlen, stdout);
	    fputs("\nSELFAUTOPARENT = ", stdout);
	    fwrite(selfautostr, 1, selfautoparentlen, stdout);
	    putchar('\n');
	}
}

#endif	/* SELFAUTO */


/*
 *	Do a dollar substitution of this variable.  Return position in ffline.
 */

static	int	envexpand();

static	int
dollarsub(key, keylen, pos, percent)
	_Xconst	char	*key;
	int		keylen;
	int		pos;
	char		percent;
{
	_Xconst	char	*env_value;
	struct envrec	*env_rec;

#ifdef	SELFAUTO
	if (keylen >= 11 && memcmp(key, "SELFAUTO", 8) == 0
	  && ((keylen == 11 && memcmp(key + 8, "DIR", 3) == 0)
	  || (keylen == 14 && memcmp(key + 8, "PARENT", 6) == 0))) {
	    int len;

	    if (selfautostr == NULL)
		selfautoinit(pos);
	    len = (keylen == 11 ? selfautodirlen : selfautoparentlen);
	    if (pos + len >= ffline_len)
		expandline(pos + len);
	    bcopy(selfautostr, ffline + pos, len);
	    return pos + len;
	}
#endif

	if (pos + keylen >= ffline_len)
	    expandline(pos + keylen);
	bcopy(key, ffline + pos, keylen);
	ffline[pos + keylen] = '\0';
	env_value = getenv(ffline + pos);
	env_rec = ffgetenv(ffline + pos);
	if (env_rec == NULL)
	    if (env_value == NULL)
		return pos;	/* no value found */
	    else {		/* create a placeholder record */
		ffputenv(ffline + pos, keylen, NULL, False);
		env_rec = ffgetenv(ffline + pos);
	    }

	if (env_rec->flag)	/* if recursive call */
	    return pos;
	if (env_value == NULL) env_value = env_rec->value;
	env_rec->flag = True;
	pos = envexpand(env_value, env_value + strlen(env_value), pos, percent);
	env_rec->flag = False;
	return pos;
}


/*
 *	Expand all environment references in a string and add the result to
 *	ffline.  Return the length of ffline.
 */

#define	LBRACE	'{'
#define	RBRACE	'}'

static	int
envexpand(p, p_end, pos, percent)
	_Xconst	char	*p;
	_Xconst	char	*p_end;
	int		pos;
	char		percent;
{
	_Xconst	char	*p1;
	_Xconst	char	*p2;

	for (;;) {	/* transfer this to ffline */
	    for (p2 = p;;) {	/* find the next $ */
		_Xconst char *p3;

		p1 = memchr(p2, '$', p_end - p);
		if (p1 == NULL) {
		    p1 = p_end;
		    break;
		}
		p3 = p1;
		while (p3-- > p2 && *p3 == percent) ;
		/* if preceded by an even number of percents */
		if ((p3 - p1) % 2 != 0)
		    break;
		p2 = p1 + 1;
	    }
	    if (p1 > p) {
		if (pos + (p1 - p) >= ffline_len)
		    expandline(pos + (p1 - p));
		bcopy(p, ffline + pos, p1 - p);
		pos += p1 - p;
	    }
	    if (p1 >= p_end)
		break;
	    ++p1;
	    if (*p1 == LBRACE) {
		++p1;
		for (p = p1;; ++p) {
		    if (p >= p_end) /* if syntax error */
			break;
		    if (*p == RBRACE) {
			pos = dollarsub(p1, p - p1, pos, percent);
			++p;
			break;
		    }
		}
	    }
	    else {
		p = p1;
		while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')
		  || (*p >= '0' && *p <= '9') || *p == '_')
		    ++p;
		pos = dollarsub(p1, p - p1, pos, percent);
	    }
	}
	return pos;
}


/*
 *	Do all expansions up to the next colon or '\0'.
 */

static	int
form_path_part(pathpp, percent)
	_Xconst	char	**pathpp;
	char		percent;
{
	_Xconst	char	*path;
	_Xconst	char	*p;
	_Xconst	char	*p_end;
	int		pos;

	path = *pathpp;
	p_end = p;
	for (p = path;;) {	/* find end of path part */
	    _Xconst char *p1;

	    p_end = index(p, ':');
	    if (p_end == NULL) {
		p_end = p + strlen(p);
		break;
	    }
	    p1 = p_end;
	    while (p1 > p && *--p1 == percent) ;
	    /* if preceded by an even number of percents */
	    if ((p_end - p1) % 2 != 0)
		break;
	    p = p_end + 1;
	}
	*pathpp = p_end;

	return envexpand(path, p_end, 0, percent);
}

/*
 *	Read a config file.
 */

static	void
read_config_file(f)
	FILE	*f;
{
	int		pos;
	char		*p;
	char		*keybegin, *keyend, *valend;
	struct envrec	*envp;
	struct envrec	**envpp;

	for (;;) {	/* loop over input lines */

	    /*
	     * Read a line into ffline[].  It can be arbitrarily long.
	     * We may assume here that ffline_len > 0 since ffline[] was used
	     * to hold the full name of the config file.
	     */

	    pos = 0;
	    for (;;) {
		int	len;

		if (fgets(ffline + pos, ffline_len - pos, f) == NULL) break;
		len = strlen(ffline + pos);
		if (len == 0) break;
		pos += len;
		if (ffline[pos - 1] == '\n')
		    break;
		if (pos + 2 >= ffline_len)
		    expandline(pos);
	    }

	    if (pos == 0) break;	/* if end of file */

	    if (ffline[pos - 1] == '\n')	/* trim off trailing \n */
		ffline[--pos] = '\0';

	    p = ffline;
	    while (*p == ' ' || *p == '\t') ++p;
	    if (*p == '\0') continue;	/* blank line */
	    if (*p == '%' || *p == '#') continue;	/* comment */

	    keybegin = p;
	    while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '='
	      && *p != '.')
		++p;
	    keyend = p;
	    while (*p == ' ' || *p == '\t') ++p;
	    if (*p == '.') {	/* if qualified by a program name */
		_Xconst char	*progbegin;

		++p;
		while (*p == ' ' || *p == '\t') ++p;
		progbegin = p;
		while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '=')
		    ++p;
		if (memcmp(progbegin, prog, p - progbegin) != 0
		  || prog[p - progbegin] != '\0')
		  /* if program name does not match */
		    continue;
		while (*p == ' ' || *p == '\t') ++p;
	    }
	    if (*p != '=') {	/* syntax error */
		if (FFDEBUG)
		    printf("Config line rejected (bad syntax):  %s\n", ffline);
		continue;
	    }
	    do
		++p;
	    while (*p == ' ' || *p == '\t');
	    valend = ffline + pos;
	    while (valend > p && (valend[-1] == ' ' || valend[-1] == '\t'))
		--valend;
	    *keyend = *valend = '\0';
	    ffputenv(keybegin, keyend - keybegin, newstring(p, valend - p + 1),
	      True);
	}

	/*
	 * Clear all the flag fields.  This indicates that the variables are
	 * no longer new.
	 */

	for (envpp = envtab; envpp < envtab + XtNumber(envtab); ++envpp)
	    for (envp = *envpp; envp != NULL; envp = envp->next)
		envp->flag = False;
}


/*
 *	Read a config file path list.
 */

static	void
rd_cfg(paths)
	_Xconst	char		*paths;
{
	int		pos;
	FILE		*f;
	struct cfglist	**cfgpp;

	for (;;) {
	    if (*paths == ':' || *paths == '\0') {	/* do the default */
		if (deflt != NULL) {
		    _Xconst char *d1 = deflt;

		    deflt = NULL;
		    rd_cfg(d1);		/* do the compiled-in default */
		}
	    }
	    else {
		pos = form_path_part(&paths, '\0');
		if (ffline[0] == '~') {
		    _Xconst char *home = NULL;
		    char *ffp = ffline;

		    gethome(&ffp, ffline + pos, &home);
		    if (home != NULL) {
			int	homelen	= strlen(home);
			int	pos1	= ffp - ffline;

			pos -= pos1;
			if (pos + homelen >= ffline_len)
			    expandline(pos + homelen);
			bcopy(ffline + pos1, ffline + homelen, pos);
			bcopy(home, ffline, homelen);
			pos += homelen;
			free((char *) home);
		    }
		}
	        if (pos > 0 && ffline[pos - 1] == '/')
		    --pos;	/* trim trailing slash */
		if (pos + 11 >= ffline_len)
		    expandline(pos);
		ffline[pos] = '\0';
		cfgpp = &cfghead;
		for (;;) {			/* check for duplicates */
		    struct cfglist	*cfgp;

		    if (*cfgpp == NULL) {	/* if not a duplicate */
			/* Add it to the list */
			cfgp = (struct cfglist *)
			  xmalloc(sizeof(*cfgp), "Linked list node");
			cfgp->next = NULL;
			cfgp->value = newstring(ffline, pos + 1);
			*cfgpp = cfgp;

			bcopy("/texmf.cnf", ffline + pos, 11);
			if (FFDEBUG)
			    printf("Reading config file %s\n", ffline);
			f = fopen(ffline, "r");
			if (f == NULL) {
			    if (FFDEBUG) perror(ffline);
			}
			else {
			    read_config_file(f);
			    fclose(f);
			    /* Did it define a new TEXMFCNF value? */
			    /* If so, utilize its value. */
			    if (lastvar == NULL) {
				lastvar = ffgetenv("TEXMFCNF");
				if (lastvar != NULL)
				    rd_cfg(lastvar->value);
			    }
			    else if (lastvar->next != NULL
			      && lastvar->next->key == NULL) {
				lastvar = lastvar->next;
				rd_cfg(lastvar->value);
			    }
			}
			break;
		    }
		    cfgp = *cfgpp;
		    if (strcmp(cfgp->value, ffline) == 0) {
			if (FFDEBUG)
			    printf(
			      "Skipped duplicate config file %s/texmf.cnf\n",
			      ffline);
			break;
		    }
		    cfgpp = &cfgp->next;
		}
	    }
	    if (*paths == '\0') break;
	    ++paths;
	}
}


/*
 *	Read the config file (public routine).
 */

void
#ifndef	CFG2RES
readconfig()
#else
readconfig(cfg2reslist, cfg2resend, appres, appres_end)
	_Xconst	struct cfg2res	*cfg2reslist;
	_Xconst	struct cfg2res	*cfg2resend;
	XtResource		*appres;
	XtResource		*appres_end;
#endif
{
	_Xconst	char	*paths;
#ifdef	CFG2RES
	_Xconst	struct cfg2res	*cfgp;
#endif

	paths = getenv("TEXMFCNF");
	if (paths == NULL) {	/* if no env variable set */
	    paths = deflt;	/* use compiled-in default */
	    deflt = NULL;	/* mark it as used */
	}
	rd_cfg(paths);

#ifdef	CFG2RES
	for (cfgp = cfg2reslist; cfgp < cfg2resend; ++cfgp) {
	    _Xconst struct envrec *envp;
	    _Xconst char *value;
	    XtResource *resp;

	    envp = ffgetenv(cfgp->cfgname);
	    if (envp == NULL) continue;
	    value = envp->value;
	    if (value == NULL) continue;	/* if getenv() marker record */
	    for (resp = appres; resp < appres_end; ++resp)
		if (strcmp(cfgp->resname, resp->resource_name) == 0) {
		    resp->default_addr = cfgp->numeric ? (XtPointer) atoi(value)
		      : (XtPointer) value;
		    break;
		}
	}
#endif	/* CFG2RES */
}

#endif	/* if CFGFILE */


/*
 *	Scan the path part from // or wild cards, to the end.
 *	This assumes that {} have already been taken care of.
 */

static	void
atomize(p0, sp)
	_Xconst	char	*p0;
	struct steprec	*sp;
{
	struct atomrec	*head;
	struct atomrec	**linkpp;
	int		flags;		/* F_SLASH_SLASH and F_WILD */
	Boolean		percent_s;
	_Xconst	char	*p	= p0;
	_Xconst	char	*p1;
	_Xconst	char	*p2;

	/*
	 * First check the string for braces.
	 */

	for (p1 = p; ;++p1) {
	    if (*p1 == '\0' || *p1 == ':')
		break;
	    if (*p1 == LBRACE) {
		/* Take care of braces first.  This code should only be reached
		 * if atomize() is called from prescan(). */
		int n;

		bracepp = &sp->nextstep;
		n = p1 - p0;
		if (n >= ffline_len)
		    expandline(n);
		bcopy(p0, ffline, n);
		dobrace(p1, n, 1);
		return;
	    }
	    if (*p1 == '%' && p1[1] != '\0')
		++p1;
	}

	/*
	 * No braces.  Split it up into atoms (delimited by / or //).
	 */

	linkpp = &head;
	flags = 0;
	percent_s = False;
	for (;;) {
	    if (*p == '/') {
		++p;
		flags |= F_SLASH_SLASH;
		/* a %[qQ] may occur following the first //. */
		if (*p == '%')
		    if (p[1] == 'q' || p[1] == 'Q') {
			if (p[1] == 'Q') flags |= F_QUICKONLY;
			flags |= F_QUICKFIND;
			p += 2;
		    }
	    }
	    p1 = p;
	    for (;; ++p) {
		if (*p == '\0' || *p == ':') {
		    p2 = NULL;
		    if (!(sp->flags & F_FILE_USED))
			p2 = lrec.no_f_str + 1;
		    break;
		}
		if (*p == '/') {
		    p2 = p + 1;
		    break;
		}
		if (*p == '%' && p[1] != '\0' && p[1] != '/') {
		    ++p;
		    if (*p == 'f') sp->flags |= F_FILE_USED;
#ifdef	PK_AND_GF
			/* set it in sp because we may never reach try_to_open*/
		    else if (*p == lrec.pk_opt_char) sp->flags |= F_PK_USED;
#endif
		    else if (*p == 's' && (p[1] == '\0' || p[1] == ':')) {
			--p;
			percent_s = True;
			if (lrec.v.pct_s_atom == NULL)
			    atomize_pct_s();
			break;
		    }
		}
		else if (*p == '*' || *p == '?' || *p == '[')
		    flags |= F_WILD;
	    }
	    if (p != p1) {	/* add on an atomrec */
		struct atomrec	*atomp;

		atomp = (struct atomrec *) xmalloc(sizeof(struct atomrec),
		    "Record for tail of path string");
		*linkpp = atomp;
		linkpp = &atomp->next;
		atomp->flags = flags;
		flags = 0;
		atomp->p = p1;
		atomp->p_end = p;
	    }
	    if (percent_s) {
		*linkpp = (flags & F_SLASH_SLASH) ? &pct_s_dummy_slash
		    : &pct_s_dummy;
		sp->flags |= F_PCT_S;
		break;
	    }
	    p = p2;
	    if (p == NULL) {
		*linkpp = NULL;
		break;
	    }
	}
	sp->atom = head;
}

/*
 *	prescan2() sets up the steprec information for a brace alternative.
 *	It also does much of the work in prescan().
 */

static	_Xconst	char *
prescan2(sp, p)
	struct steprec	*sp;
	_Xconst	char	*p;
{
	int		flags;

	/*
	 * Scan the string, looking for // or wild cards or braces.
	 * It takes the input / by / so that we can back up to the previous
	 * slash if there is a wild card (which may be hidden in braces).
	 */

	flags = sp->flags;
	for (;;) {	/* loop over slash-separated substrings */
	    sp->strend = p;
	    sp->flags = flags;
	    if (*p == '/') {	/* if it's a // */
		atomize(p, sp);
		return p;
	    }
	    for (;;) {	/* loop over characters in this subpart */
		if (*p == '\0' || *p == ':') {
		    sp->strend = p;
		    sp->flags = flags;
		    return p;
		}
		if (*p == '/')
		    break;
		if (*p == '%' && p[1] != '\0') {
		    ++p;
		    if (*p == 's' && (p[1] == '\0' || p[1] == ':')) {
			sp->strend = p - 1;
			sp->flags = flags;
			sp->nextstep = scan_pct_s();
			return p;
		    }
		    else if (*p == 'f')
			flags |= F_FILE_USED | F_VARIABLE;
		    else if (*p == 'F' || *p == lrec.x_var_char)
			flags |= F_VARIABLE;
#ifdef	PK_AND_GF
		    else if (*p == lrec.pk_opt_char)
			flags |= F_PK_USED;
#endif
		    /* %c and %C are sorted out later. */
		}
		else if (*p == '*' || *p == '?' || *p == '[') {
		    atomize(sp->strend, sp);
		    return p;
		}
		else if (*p == LBRACE) {
		    int n;

		    bracepp = &sp->nextstep;
		    n = p - sp->strend;
		    if (n >= ffline_len)
			expandline(n);
		    bcopy(sp->strend, ffline, n);
		    dobrace(p, n, 1);
		    return p;
		}
		++p;
	    }
	    ++p;
	}
}

/*
 *	dobrace() is the recursive routine for handling {} alternatives.
 *	bracepp (global) = address of where to put next brace item on the
 *		linked list.  It has to be global because dobrace() is called
 *		recursively and leaves its droppings at varying levels.
 *	p = input string pointer; it should point to the opening brace.
 *	pos = position within ffline[] for output string.
 *	level = nesting depth.
 */

static	void
dobrace(p, pos0, level)
	_Xconst	char	*p;
	int		pos0;
	int		level;
{
	int		lev;
	int		level1;
	_Xconst	char	*p1;
	int		pos;

	for (;;) {	/* loop over the alternatives */
	    lev = 0;
	    ++p;	/* skip left brace or comma */
	    pos = pos0;
	    for (;;) {	/* loop over characters */
		if (*p == '\0' || *p == ':') {
			/* keep the braces matched:  { */
		    fprintf(stderr, "xdvi: Missing } in %s path.\n", lrec.type);
		    return;
		}
		else if (*p == '%') {
		    if (p[1] != '\0') {
			if (pos >= ffline_len)
			    expandline(pos);
			ffline[pos++] = *p++;
		    }
		}
		else if (*p == LBRACE) {
		    dobrace(p, pos, level + 1);
		    /* skip to the next matching comma or right brace */
		    for (;;) {
			if (*p == '\0' || *p == ':')
			    return;	/* this has already been reported */
			else if (*p == '%') {
			    if (p[1] != '\0') ++p;
			}
			else if (*p == LBRACE)
			    ++lev;
			else if (*p == ',') {
			    if (lev == 0)
				break;
			}
			else if (*p == RBRACE) {
			    if (lev == 0)
				break;
			    --lev;
			}
			++p;
		    }
		    goto out2;
		}
		else if (*p == ',' || *p == RBRACE)
		    break;
		if (pos >= ffline_len)
		    expandline(pos);
		ffline[pos++] = *p++;
	    }
	    p1 = p;
	    level1 = level;
	    for (;;) {
		/* skip until matching right brace */
		for (;;) {
		    if (*p1 == '\0' || *p1 == ':')
			goto out2;	/* this has already been reported */
		    if (*p1 == '%') {
			if (p1[1] != '\0') ++p1;
		    }
		    else if (*p1 == LBRACE)
			++lev;
		    else if (*p1 == RBRACE) {
			if (lev == 0) {
			    ++p1;
			    break;
			}
			--lev;
		    }
		    ++p1;
		}
		--level1;
		/* now copy until a qualifying comma */
		for (;;) {
		    if (*p1 == '\0' || *p1 == ':') {
			if (level1 == 0) {
			    struct steprec *bp;

			    if (pos >= ffline_len)
				expandline(pos);
			    ffline[pos++] = '\0';
			    *bracepp = bp = (struct steprec *)
				xmalloc(sizeof(struct steprec),
				    "Font search brace record");
			    bp->next = NULL;
			    bp->atom = NULL;
			    bp->nextstep = NULL;
			    bracepp = &bp->next;
			    bp->str = newstring(ffline, pos);
			    bp->flags = 0;
			    prescan2(bp, bp->str);
			}
			/* else:  this has already been reported */
			goto out2;
		    }
		    if (*p1 == '%') {
			if (p1[1] != '\0') {
			    if (pos >= ffline_len)
				expandline(pos);
			    ffline[pos++] = '%';
			    ++p1;
			}
		    }
		    else if (*p1 == LBRACE) {
			dobrace(p1, pos, level1 + 1);
			goto out2;
		    }
		    else if (*p1 == ',' && level1 > 0)
			break;
		    else if (*p1 == RBRACE && level1 > 0) {
			--level1;
			++p1;
			continue;
		    }
		    if (pos >= ffline_len)
			expandline(pos);
		    ffline[pos++] = *p1++;
		}
	    }
	    out2:
	    if (*p != ',') break;
	}
}


/*
 *	Prescan the '%s' string.  This is done only once.
 */

static	struct steprec *
scan_pct_s()
{
	struct steprec	**headpp;
	_Xconst	char	*p;
	struct steprec	*sp;

	if (lrec.v.pct_s_head != NULL)	/* if we've already done this */
	    return lrec.v.pct_s_head;

	headpp = &lrec.v.pct_s_head;
	p = lrec.pct_s_str;
	for (p = lrec.pct_s_str; ; ++p) {
	    ++lrec.v.pct_s_count;
	    *headpp = sp = (struct steprec *) xmalloc(sizeof(struct steprec),
		"record for %s in path search code");
	    sp->atom = NULL;
	    sp->nextstep = NULL;
	    sp->flags = 0;
	    sp->str = p;
	    headpp = &sp->next;
	    prescan2(sp, p);
	    for (; *p != ':' && *p != '\0'; ++p)
		if (*p == '%' && p[1] != '\0') ++p;
	    if (*p == '\0') break;
	}
	*headpp = NULL;
	return lrec.v.pct_s_head;
}

/*
 *	Pre-atomize the '%s' strings.  Again, done only once.
 */

static	void
atomize_pct_s()
{
	_Xconst	struct atomrec	**app;
	struct steprec		*sp;

	(void) scan_pct_s();	/* make sure the count is valid */

	app = lrec.v.pct_s_atom = (_Xconst struct atomrec **)
		xmalloc((unsigned) lrec.v.pct_s_count * sizeof(*app),
		"list of atomized chains for %s in path search code");

	for (sp = lrec.v.pct_s_head; sp != NULL; sp = sp->next, ++app) {
	    static struct steprec dummy;

	    dummy.flags = 0;
	    atomize(sp->str, &dummy);
	    *app = dummy.atom;
#ifdef	PK_AND_GF
	    dummy.atom->flags |= dummy.flags;
#endif
	}
}


/*
 *	Init_texmf - Initialize the linked list of options for %t.
 */

static	void		/* here's a small subroutine */
add_to_texmf_list(pp)
	_Xconst	char	**pp;
{
	static	struct texmfrec	**tpp	= &texmfhead;
	struct texmfrec		*tp;
	_Xconst	char		*p;
	_Xconst	char		*p_end;
	_Xconst	char		*home	= NULL;
	int			flags	= 0;
#if	CFGFILE
	_Xconst	char		*p0;
	int			pos;
#endif

#if	CFGFILE
	p0 = *pp;
	pos = form_path_part(pp, '\0');
	p = ffline;
	p_end = ffline + pos;
	ffline[pos] = '\0';
#else
	p = *pp;
	p_end = index(p, ':');
	if (p_end == NULL) p_end = p + strlen(p);
	*pp = p_end;
#endif	/* CFGFILE */

	if (*p == '!' && p[1] == '!') {
	    p += 2;
	    flags = F_QUICKONLY;
	}

	if (*p == '~')
	    gethome(&p, p_end, &home);

#if	CFGFILE
	pos -= p - ffline;
	if (*pp - p0 >= pos && memcmp(p, *pp - pos, pos) == 0)
	    p = *pp - pos;
	else
	    p = newstring(p, pos + 1);
#endif	/* CFGFILE */

	tp = *tpp = (struct texmfrec *)
	    xmalloc(sizeof(struct texmfrec), "texmf record");
	tp->home = home;
	tp->str = p;
#if	CFGFILE
	tp->len = pos;
#else
	tp->len = p_end - p;
#endif
	tp->flags = flags;
	tp->next = NULL;
	tpp = &tp->next;
}


#if	CFGFILE

static	void
intexmf(env_value, env_ptr, dflt)
	_Xconst	char		*env_value;
	_Xconst	struct envrec	*env_ptr;
	_Xconst	char		*dflt;
{
	_Xconst	char		*path;
	Boolean			did_next_default;

	if (env_value != NULL) {
	    path = env_value;
	    env_value = NULL;
	}
	else if (env_ptr != NULL && env_ptr->value != NULL) {
	    path = env_ptr->value;
	    env_ptr = env_ptr->next;
	    if (env_ptr != NULL && env_ptr->key != NULL)
		env_ptr = NULL;
	}
	else if (dflt != NULL) {
	    path = dflt;
	    dflt = NULL;
	}
	else return;

	did_next_default = False;

	for (;;) {
	    if (*path == '\0' || *path == ':') {
		if (!did_next_default) {
		    intexmf(env_value, env_ptr, dflt);
		    did_next_default = True;
		}
	    }
	    else
		add_to_texmf_list(&path);
	    if (*path == '\0')
		break;
	    ++path;
	}
}

#endif	/* if CFGFILE */


static	void
init_texmf()
{
#if	!CFGFILE
	_Xconst	char		*texmf;
	_Xconst	char		*texmf2;
#endif
	_Xconst	struct texmfrec	*tp;
	int			n;

	if (texmfhead != NULL)
	    return;

#if	CFGFILE

	intexmf(getenv("TEXMF"), ffgetenv("TEXMF"), DEFAULT_TEXMF_PATH);

#else	/* !CFGFILE */

	texmf = getenv("TEXMF");
	texmf2 = default_texmf_path;
	if (texmf == NULL) {
	    texmf = default_texmf_path;
	    texmf2 = NULL;
	}

	for (;;) {
	    if (*texmf == '\0' || *texmf == ':') {
		if (texmf2 != NULL)
		    for (;;) {
			while (*texmf2 == ':') ++texmf2;
			if (*texmf2 == '\0') break;
			add_to_texmf_list(&texmf2);
		    }
	    }
	    else
		add_to_texmf_list(&texmf);
	    if (*texmf == '\0')
		break;
	    ++texmf;
	}

#endif

	/* Make sure ffline[] is long enough for these. */

	n = 0;
	for (tp = texmfhead; tp != NULL; tp = tp->next)
	    if (tp->len > n)
		n = tp->len;

	if (n > ffline_len)
	    expandline(n);
}


/*
 *	Scan this path part.  This is done at most once.
 *	We look for // or wild cards or {}.
 */

static	_Xconst	char *
prescan(p)
	_Xconst	char	*p;
{
	struct steprec	*sp;

	*steppp = sp = (struct steprec *) xmalloc(sizeof(struct steprec),
	    "main info. record in path search code.");
	sp->next = NULL;
	sp->flags = 0;
	sp->atom = NULL;
	sp->nextstep = NULL;

	/*
	 * Scan the string, looking for // or wild cards or braces.
	 */

	sp->str = p;
	if (*p == '/') ++p;	/* take care of an initial slash */
	return prescan2(sp, p);
}

/*
 *	This routine handles the translation of the '%' modifiers.
 *	It returns the new number string length (pos).
 *	It makes sure that at least one free byte remains in ffline[].
 */

static	int
xlat(stat_in, stat_ret, pos0, p, lastp)
	_Xconst	struct statrec	*stat_in;
	struct statrec		*stat_ret;
	int			pos0;
	_Xconst	char		*p;
	_Xconst	char		*lastp;
{
	struct statrec		status;
	_Xconst	char		*q;
	int			l;

	status.pos = pos0;
	if (stat_ret != NULL)
	    status = *stat_in;

	while (p < lastp) {
	    q = memchr(p, '%', lastp - p);
	    l = (q != NULL ? q : lastp) - p;
	    if (status.pos + l >= ffline_len)
		expandline(status.pos + l);
	    bcopy(p, ffline + status.pos, l);
	    status.pos += l;
	    p = q;
	    if (p == NULL) break;
	    do {
		++p;
		if (p >= lastp) break;
		q = index(lrec.fF_etc, *p);
		if (q != NULL) {
		    _Xconst char *str = fF_values[q - lrec.fF_etc];

		    if (str != NULL) {
			l = strlen(str);
			if (status.pos + l >= ffline_len)
			    expandline(status.pos + l);
			bcopy(str, ffline + status.pos, l);
			status.pos += l;
		    }
		    else {	/* eliminate a possible double slash */
			if (p[1] == '/' && (status.pos == 0
				|| ffline[status.pos - 1] == '/'))
			    --status.pos;
		    }
		}
		else if (*p == 'q' || *p == 'Q') {
		    status.quickchar = *p;
		    status.quickpos = status.pos;
		}
		else {
		    if (status.pos + 1 >= ffline_len)
			expandline(ffline_len);
		    ffline[status.pos++] = *p;
		}
		++p;
	    }
	    while (p < lastp && *p == '%');
	}

	if (stat_ret != NULL)
	    *stat_ret = status;
	return status.pos;
}


/*
 *	TRY_TO_OPEN - Try to open the file.  Exit via longjmp() if success.
 */

#ifdef	PK_AND_GF
#define	TRY_TO_OPEN(pos, flags)	try_to_open(pos, flags)
#else
#define	TRY_TO_OPEN(pos, flags)	try_to_open(pos)
#endif

static	void
TRY_TO_OPEN(pos, flags)
	int	pos;
#ifdef	PK_AND_GF
	int	flags;
#endif
{
#ifdef	PK_AND_GF
	gflags |= flags;
	if (pk_second_pass && !(flags & F_PK_USED))
	    return;
#endif
	ffline[pos] = '\0';

	if (FFDEBUG)
	    printf("Trying %s file %s\n", lrec.type, ffline);

	file_found = xfopen(ffline, OPEN_MODE);
	if (file_found != NULL)
	    longjmp(got_it, 1);
}


/*
 *	lsrgetline - Read a line from the (ls-R) file and return:
 *		minus its length if it ends in a colon (including the colon);
 *		its length	 otherwise;
 *		zero		 for end of file.
 *	Blank lines are skipped.
 */

static	int
lsrgetline(f, pos0)
	FILE	*f;	/* ls-R file */
	int	pos0;	/* relative position in line to start reading */
{
	int	pos	= pos0;

	for (;;) {
	    if (pos + 4 >= ffline_len)
		expandline(pos);
	    if (fgets(ffline + pos, ffline_len - pos, f) == NULL)
		break;
	    pos += strlen(ffline + pos);
	    if (pos > pos0 && ffline[pos - 1] == '\n') {
		--pos;		/* trim newline */
		if (pos > pos0) break;	/* if ffline still nonempty */
	    }
	}
	if (pos > pos0 && ffline[pos - 1] == ':') {
	    ffline[pos - 1] = '/';	/* change trailing : to slash */
	    return pos0 - pos;
	}
	else
	    return pos - pos0;
}


/*
 *	lsrput - Put a key/value pair into the ls-R database.
 */

static	void
lsrput(key, keylen, value)
	_Xconst	char	*key;
	int		keylen;
	_Xconst	char	*value;
{
	struct lsr	**lpp;
	struct lsr	*lp;

#if 0
	fputs("lsrput:  key = `", stdout);
	fwrite(key, 1, keylen, stdout);
	printf("'; value = `%s'.\n", value);
#endif

	    /* make the new record */
	lp = (struct lsr *) xmalloc(sizeof(*lp), "ls-R record");
	lp->next = NULL;
	lp->frecp = frecp;
	lp->seqno = seqno;
	lp->keylen = keylen;
	lp->key = newstring(key, keylen);
	lp->value = value;

	    /* insert it */
	lpp = lsrtab + prehash(key, keylen) % XtNumber(lsrtab);
	while (*lpp != NULL)	/* skip to end of chain (performance reasons) */
	    lpp = &(*lpp)->next;
	*lpp = lp;
}


/*
 *	init_quick_find - Read ls-R database for this particular search.
 */

static	void
init_quick_find(root, pos, atom, pos1)
	struct rootrec		*root;	/* pointer to the governing rootrec */
	int			pos;	/* position of %[qQ] in ffline[] */
	_Xconst	struct atomrec	*atom;	/* the beginning of the atomlist */
	int			pos1;	/* next free byte in ffline[] */
{
#define	LS_R_LEN	5		/* length of the string "ls-R\0" */
	char		tmpsav[LS_R_LEN];
	int		keypos;		/* position of start of key pattern */
	int		keyend;		/* last + 1 character of the key */
	int		keypos_a;	/* position + 1 of last slash */
	int		pos2;		/* end + 1 of the decoded thing */
	int		nslashes;	/* how many slashes in bbbcccddd */
	FILE		*f;		/* ls-R file */
	_Xconst	char	*fullpat;	/* fields in *root */
	_Xconst	char	*keypat;	/* " */
	_Xconst	char	*keypat_end;	/* " */
	_Xconst	char	*fullpat_end;	/* " */
	_Xconst	char	*keypat_a;	/* position of end+1 of restricted
					 * initial fixed part */
	int		retval;		/* return value of lsrgetline */

	/*
	 *	Reencode the string of atoms into a character string
	 *	When we're done, ffline[] contains:
	 *	/aa/aa/a...a/ bbb/bb...bb/bbb %f/cc/c...c/%d ddd/ddd/.../ddd
	 *
	 *	where the aaa part is what's already in ffline[], and
	 *	the rest is the decoded atom chain.  The ccc part of this
	 *	string contains all the variable specifiers (not replaced by
	 *	anything), and the bbb and ddd parts are character strings that
	 *	should occur verbatim in the file name.
	 */

	root->flags &= ~F_QUICKFIND;	/* clear it for now; set it later */

	if (atom->flags & F_PCT_S)
	    atom = treeatom;

	if (atom->flags & F_WILD) return;	/* not allowed */

	pos2 = pos1;
	keypos = -1;
	nslashes = 0;

	for (;;) {
	    _Xconst char *p;

	    keypos_a = pos2;
		/* translate the atom, leaving %f and maybe %d alone */
	    for (p = atom->p; p < atom->p_end; ++p) {
		if (p >= atom->p_end) break;
		if (*p == '%') {
		    _Xconst char *q;

		    ++p;
		    if (p >= atom->p_end) break;
		    q = index(lrec.fF_etc, *p);
		    if (q != NULL) {
			if (q - lrec.fF_etc < lrec.n_var_opts) {
			    /* it's a variable specifier (%f or ...) */
			    --p;	/* copy it over */
			    if (keypos < 0) keypos = pos2;
			    keyend = pos2 + 2;
			}
			else {
			    _Xconst char *str	= fF_values[q - lrec.fF_etc];

			    if (str != NULL) {
				int	l	= strlen(str);

				if (pos2 + l >= ffline_len)
				    expandline(pos2 + l);
				bcopy(str, ffline + pos2, l);
				pos2 += l;
			    }
			    continue;
			}
		    }
		}
		if (pos2 + 1 >= ffline_len)
		    expandline(pos2);
		ffline[pos2++] = *p;
	    }

	    atom = atom->next;
	    if (atom == NULL) break;

	    if (atom->flags & F_PCT_S) atom = treeatom;
	    if (atom->flags & (F_SLASH_SLASH | F_WILD))
		return;		/* not supported */

	    if (pos2 == keypos_a) continue;	/* if /%m/ with no mode */
	    ffline[pos2++] = '/';
	    ++nslashes;
	}

	if (keypos_a > keypos) keypos_a = keypos;

	root->flags |= F_QUICKFIND;	/* it's eligible */

	if (FFDEBUG) {
#define	DDD(a,b,c)	\
		fputs(a, stdout); \
		fwrite(ffline + (b), 1, (c) - (b), stdout); \
		puts("'");

	    DDD("\nReading ls-R.\nOriginal string = `", 0, pos1);
	    DDD("Restricted initial fixed part = `", pos1, keypos_a);
	    DDD("Initial fixed part = `", pos1, keypos);
	    DDD("Key pattern = `", keypos, keyend);
	    DDD("Final fixed part = `", keyend, pos2);
#undef	DDD
	}

	/*
	 * Open ls-R and start reading it.
	 */

	bcopy(ffline + pos, tmpsav, sizeof(tmpsav));
	bcopy("ls-R", ffline + pos, sizeof(tmpsav));
	if (FFDEBUG) printf("Opening ls-R file %s\n", ffline);
	f = xfopen(ffline, OPEN_MODE);
	bcopy(tmpsav, ffline + pos, sizeof(tmpsav));
	if (f == NULL) {
	    if (FFDEBUG) perror("xfopen");
	    return;
	}

	fullpat = newstring(ffline + pos1, pos2 - pos1);
	keypat = fullpat + (keypos - pos1);
	keypat_a = fullpat + (keypos_a - pos1);
	keypat_end = fullpat + (keyend - pos1);
	fullpat_end = fullpat + (pos2 - pos1);

	do {			/* skip initial comment lines */
	    retval = lsrgetline(f, pos1);
	    if (retval == 0) {
		fclose(f);
		return;		/* premature end of file */
	    }
	} while (ffline[pos1] == '%');

	/* First take care of file names without a directory: these are
	 * those in the same directory as the ls-R file */

	while (retval > 0) {
	    if (nslashes == 0	/* if there are no slashes in the pattern */
				/* and if ls-R is in the same directory
				 * as the //: */
		    && pos1 == pos
				/* and if the initial fixed part matches: */
		    && memcmp(ffline + pos1, fullpat, keypat - fullpat) == 0
				/* and if the final fixed part matches: */
		    && memcmp(ffline + pos1 + retval
				- (fullpat_end - keypat_end),
				keypat_end, fullpat_end - keypat_end) == 0)
				/* then create a directory entry */
		lsrput(ffline + pos1 + (keypat - fullpat),
			retval - (keypat - fullpat)
			- (fullpat_end - keypat_end), "");
	    retval = lsrgetline(f, pos1);	/* read next line */
	}

	/*
	 * Now do whole directories.
	 */

	for (;;) {
	    int		i;
	    _Xconst char *p;
	    _Xconst char *p_begin;
	    _Xconst char *p_end;
	    int		pos3;
	    int		pos4;
	    int		pos5;
	    int		retpos;
	    char	*value;

	    while (retval > 0)		/* skip to next directory */
		retval = lsrgetline(f, pos1);
	    if (retval == 0) break;	/* if end of file */
	    retval = -retval;		/* length of the string */
	    /* also, retval is now > 0, so a continue statement won't lead to
	     * an infinite loop. */
	    retpos = pos1 + retval;
	    if (ffline[pos1] == '/') {	/* if absolute path */
			/* these should be referring to the same directory */
		if (retval < pos1 || memcmp(ffline + pos1, ffline, pos1) != 0)
		    continue;
		pos2 = pos1 + pos1;
	    }
	    else {		/* relative path */
		pos2 = pos1;
		if (retval >= 2 && ffline[pos1] == '.'
		  && ffline[pos1 + 1] == '/')
		    pos2 += 2;	/* eliminate "./" */
			/* these should be referring to the same directory */
		if (retpos - pos2 < pos1 - pos
			|| memcmp(ffline + pos2, ffline + pos, pos1 - pos) != 0)
		    continue;
		pos2 += pos1 - pos;
	    }
	    /* now pos2 points to what should follow the // */
	    /* count backwards from the end of the string to just after the
	       (nslashes + 1)-st slash */
	    p_begin = ffline + pos2;
	    p_end = ffline + retpos;
	    i = nslashes;
	    for (p = p_end; p > p_begin; )
		if (*--p == '/') {
		    if (i == 0) {
			++p;
			break;
		    }
		    else --i;
		}
	    if (i > 0) continue;	/* if too few slashes */
	    pos3 = p - ffline;
	    /* the next part of the directory name should match the initial
	       fixed part of the pattern */
	    if (p_end - p < keypat_a - fullpat
	      || memcmp(p, fullpat, keypat_a - fullpat) != 0)
		continue;
	    pos4 = pos3 + (keypat - fullpat);	/* start of key */
	    value = NULL;
	    for (;;) {		/* read the files in this directory */
		retval = lsrgetline(f, retpos);
		if (retval <= 0) {	/* if done with list*/
		    bcopy(ffline + retpos, ffline + pos1, -retval);
		    break;
		}
		    /* check the remaining part of the initial fixed part */
		    /* (the first test is only for performance) */
		if (keypat_a != keypat
		  && memcmp(ffline + retpos, keypat_a, keypat - keypat_a) != 0)
		    continue;
		    /* end of key */
		pos5 = retpos + retval - (fullpat_end - keypat_end);
		    /* check the final fixed part */
		if (pos5 < pos4 || (fullpat_end != keypat_end
			&& memcmp(ffline + pos5, keypat_end,
			fullpat_end - keypat_end) != 0))
		    continue;
		if (value == NULL) {
		    value = newstring(ffline + pos2, pos3 - pos2 + 1);
		    value[pos3 - pos2] = '\0';
		}
		lsrput(ffline + pos4, pos5 - pos4, value);
	    }
	}

	fclose(f);	/* close the file */

	    /* Fill in the requisite entries in *root:  we have ls-R. */
	root->fullpat = fullpat;
	root->fullpat_end = fullpat_end;
	root->keypat = keypat;
	root->keypat_end = keypat_end;
}

/*
 *	wildmatch - Tell whether the string matches the pattern.
 *
 *		pat		pointer to first character of pattern
 *		pat_end		pointer to last + 1 character of pattern
 *		candidate	the string
 *		allowvar	how many characters of fF_etc should be treated
 *				as *
 */

static	Boolean
wildmatch(pat, pat_end, candidate, allowvar)
	_Xconst	char	*pat;
	_Xconst	char	*pat_end;
	_Xconst	char	*candidate;
	int		allowvar;	/* How many of %f, %F, %d, ... to */
					/* treat as '*' */
{
	_Xconst	char	*p;
	_Xconst	char	*q	= candidate;

	for (p = pat; p < pat_end; ++p)
	    switch (*p) {

		case '%': {
		    _Xconst char *q1;

		    if (p + 1 >= pat_end) continue;
		    ++p;
		    q1 = index(lrec.fF_etc, *p);
		    if (q1 == NULL) {		/* %[ or something */
			if (*q != *p)
			    return False;
			++q;
			break;
		    }

		    if (q1 - lrec.fF_etc < allowvar) {	/* treat it as '*' */
			for (q1 = q + strlen(q); q1 >= q; --q1)
			    if (wildmatch(p + 1, pat_end, q1, allowvar))
				return True;
			return False;
		    }
		    else {
			_Xconst char	*str	= fF_values[q1 - lrec.fF_etc];
			int		l;

			if (str == NULL) str = "";
			l = strlen(str);
			if (bcmp(q, str, l) != 0)
			    return False;
			q += l;
		    }
		    break;
		}

	        case '*': {
		    _Xconst char *q1;

		    for (q1 = q + strlen(q); q1 >= q; --q1)
			if (wildmatch(p + 1, pat_end, q1, allowvar))
			    return True;
		    return False;
		}

	        case '?':
		    if (*q == '\0')
			return False;
		    ++q;
		    break;

	        case '[': {
		    char c;
		    Boolean reverse = False;

		    c = *q++;
		    if (c == '\0')
			return False;

		    ++p;
		    if (*p == '^') {
			reverse = True;
			++p;
		    }

		    for (;;) {
			char	c1;
			char	c2;

			if (p >= pat_end)
			    return False;	/* syntax error */
			c1 = *p;
			if (c1 == ']')
			    if (reverse)
				break;		/* success */
			    else
				return False;
			c2 = c1;
			++p;
			if (*p == '-' && p[1] != ']') {
			    ++p;
			    c2 = *p++;
			    if (c2 == '%') c2 = *p++;
			}
			if (p >= pat_end)
			    return False;	/* syntax error */
			if (c >= c1 && c <= c2)
			    if (reverse)
				return False;
			    else {		/* success */
				while (*p != ']') {
				    if (*p == '%') ++p;
				    ++p;
				    if (p >= pat_end)
					return False;	/* syntax error */
				}
				break;
			    }
		    }
		    break;
		}

		default:
		    if (*q != *p)
			return False;
		    ++q;
	    }

	if (*q != '\0')
	    return False;
	return True;
}


/*
 *	Read in the given directory.
 */

static	void
filltree(pos, treepp, atom, slashslash)
	int		pos;
	struct treerec	**treepp;
	struct atomrec	*atom;
	Boolean		slashslash;
{
	DIR		*f;
	struct dirent	*dp;

	if (pos == 0) {
	    if (ffline_len == 0)
		expandline(2);
	    ffline[0] = '.';
	    ffline[1] = '\0';
	}
	else
	    ffline[pos - 1] = '\0';

	if (FFDEBUG)
	    printf("Opening directory %s\n", ffline);

	f = xopendir(ffline);
	if (f == NULL) {
	    if (FFDEBUG) perror(ffline);
	    *treepp = NULL;
	    return;
	}

	if (pos != 0) ffline[pos - 1] = '/';
	for (;;) {
	    char		*line1;
	    struct stat		statbuf;
	    struct treerec	*leaf;
	    Boolean		isdir;
#if	ST_NLINK_TRICK
	    struct treerec	*child;
#endif

	    dp = readdir(f);
	    if (dp == NULL) break;	/* done */

	    if (pos + dp->d_reclen >= ffline_len)
		expandline(pos + dp->d_reclen);
	    line1 = ffline + pos;
	    bcopy(dp->d_name, line1, dp->d_reclen);
	    line1[dp->d_reclen] = '\0';
	    if (*line1 == '.' && (line1[1] == '\0'
		    || (line1[1] == '.' && line1[2] == '\0')))
		continue;	/* skip . and .. */

	    if (stat(ffline, &statbuf) != 0) {
		fputs("xdvi/filltree/stat: ", stderr);
		perror(ffline);
		continue;
	    }
	    isdir = False;
#if	ST_NLINK_TRICK
	    child = &not_read_yet;
#endif
	    if (S_ISDIR(statbuf.st_mode)) {
		isdir = True;
		if (!slashslash
			&& (atom->next == NULL
			|| !wildmatch(atom->p, atom->p_end, line1,
				lrec.n_var_opts)))
		    continue;
#if	ST_NLINK_TRICK
		if (statbuf.st_nlink <= 2)
		    child = NULL;
#endif
	    }
	    else if (S_ISREG(statbuf.st_mode)) {
		if (atom->next != NULL
			|| (slashslash && !(atom->flags & F_WILD))
			|| !wildmatch(atom->p, atom->p_end, line1,
				lrec.n_var_opts))
		    continue;
	    }
	    else continue;	/* something else */

	    leaf = (struct treerec *) xmalloc(sizeof(struct treerec),
		    "leaf for subdir searching");
	    {
		char	*str;

		str = newstring(dp->d_name, dp->d_reclen + 1);
		str[dp->d_reclen] = '\0';
		leaf->dirname = str;
	    }
#if	ST_NLINK_TRICK
	    leaf->child = child;
#else
	    leaf->child = &not_read_yet;
#endif
	    leaf->isdir = isdir;
	    *treepp = leaf;
	    treepp = &leaf->next;
	}
	closedir(f);

	*treepp = NULL;
}


/*
 *	do_tree_search() - recursive routine for // and wild card searches.
 */

static	void
do_tree_search(treepp, atom, goback, pos, flags)
	struct treerec		**treepp;
	_Xconst	struct atomrec	*atom;
	_Xconst	struct atomrec	*goback;
	int			pos;
	int			flags;
{
	int		aflags;
	struct treerec	*tp;

	/*
	 * If we're at the end of the chain of atoms, try to open the file.
	 */

	if (atom == NULL) {
	    TRY_TO_OPEN(pos - 1, flags);
	    return;
	}

	/*
	 * Otherwise, we're still forming the path.
	 */

	aflags = atom->flags;
	if (aflags & F_PCT_S) {		/* if it's a dummy record */
	    atom = treeatom;
	    aflags |= atom->flags;
	}

	if (aflags & F_SLASH_SLASH)
	    goback = atom;

	if (goback == NULL) {		/* wild card search, no // yet */
	    /*
	     * If the next atom is neither wild nor variable, then we don't
	     * need to do a readdir() on the directory.  Just add the
	     * appropriate characters to the path and recurse.
	     */
	    if (!(atom->flags & (F_WILD | F_VARIABLE))) {
		pos = xlat(NULL, NULL, pos, atom->p, atom->p_end);
		ffline[pos++] = '/';
		do_tree_search(treepp, atom->next, goback, pos,
		    flags | atom->flags);
		return;
	    }
	    /*
	     * Otherwise, go down one level in the tree.
	     */
	    if (*treepp == &not_read_yet)	/* if readdir() not done yet */
		filltree(pos, treepp, atom, False);	/* do it */

	    for (tp = *treepp; tp != NULL; tp = tp->next)
		if (wildmatch(atom->p, atom->p_end, tp->dirname, 0)) {
		    int len = strlen(tp->dirname);
		    bcopy(tp->dirname, ffline + pos, len);
		    ffline[pos + len++] = '/';
		    do_tree_search(&tp->child, atom->next, goback, pos + len,
			flags | atom->flags);
		}

	    return;
	}

	/*
	 * If we got here, then we're past a //
	 */

	if (atom->next == NULL && !(atom->flags & F_WILD))
		/* if we can try a file */
	    TRY_TO_OPEN(xlat(NULL, NULL, pos, atom->p, atom->p_end),
		flags | atom->flags);

	if (*treepp == &not_read_yet)		/* if readdir() not done yet */
	    filltree(pos, treepp, atom, True);	/* do it */

	for (tp = *treepp; tp != NULL; tp = tp->next) {
	    tp->tried_already = False;
	    if ((atom->next != NULL || (atom->flags & F_WILD))
		    && wildmatch(atom->p, atom->p_end, tp->dirname, 0)) {
		int len = strlen(tp->dirname);
		bcopy(tp->dirname, ffline + pos, len);
		ffline[pos + len++] = '/';
		do_tree_search(&tp->child, atom->next, goback, pos + len,
		    flags | atom->flags);
		tp->tried_already = True;
	    }
	}

	for (tp = *treepp; tp != NULL; tp = tp->next)
	    if (!tp->tried_already && tp->isdir) {
		int len = strlen(tp->dirname);
		bcopy(tp->dirname, ffline + pos, len);
		ffline[pos + len++] = '/';
		do_tree_search(&tp->child, goback, goback, pos + len, flags);
	    }
}


/*
 *	begin_tree_search() - start a wild card or // search.
 */

static	void
begin_tree_search(status, oneatom, twoatom)
	_Xconst	struct statrec	*status;
	_Xconst	struct atomrec	*oneatom;	/* initial atom */
	_Xconst	struct atomrec	*twoatom;	/* if F_PCT_S set */
{
	struct treerec		**tpp;

#ifdef	PK_AND_GF
	{
	    int flags = status->flags;

	    if (twoatom != NULL)
		flags |= twoatom->flags;
	    if (pk_second_pass && !(flags & F_PK_USED))
		return;
	}
#endif

#if 0
	{
	    _Xconst struct atomrec	*atom;

	    puts("begin_tree_search():");
	    fwrite(ffline, 1, status->pos, stdout);
	    putchar('\n');
	    atom = oneatom;
	    for (;;) {
		fputs("+ ", stdout);
		if (atom->flags & F_SLASH_SLASH) fputs("(//) ", stdout);
		if (atom->flags & F_PCT_S) atom = twoatom;
		fwrite(atom->p, 1, atom->p_end - atom->p, stdout);
		if (atom->flags & F_WILD) fputs(" (wild)", stdout);
		putchar('\n');
		atom = atom->next;
		if (atom == NULL) break;
	    }
	    putchar('\n');
	}
#endif

	++seqno;
	treeatom = twoatom;		/* save this globally */

	/*
	 * Find the record controlling this mess.  Create one, if necessary.
	 */

	if (status->flags & F_VARIABLE) {
	    struct vrootrec {
		struct rootrec	*next;	/* link to next in hash chain */
		struct treerec	*tree;	/* the tree */
		_Xconst char	*path;
		int		seqno;
	    };
	    struct rtab {
		struct rtab	*next;
		int		seqno;
		int		len;
		_Xconst char	*tag;
		struct vrootrec	*r;
	    };
	    static struct rtab	*varroot[32];
	    struct rtab		**tpp2;
	    struct rtab		*t;
	    struct vrootrec	*r;

	    tpp2 = varroot + prehash(ffline, status->pos) % XtNumber(varroot);
	    for (;;) {
		t = *tpp2;
		if (t == NULL) {
		    t = (struct rtab *) xmalloc(sizeof(*t), "root record");
		    t->seqno = seqno;
		    t->len = status->pos;
		    t->tag = newstring(ffline, status->pos);
		    r = (struct vrootrec *) xmalloc(sizeof(*r), "root record");
		    r->next = NULL;
		    r->tree = &not_read_yet;	/* readdir() not done yet */
		    t->r = r;
		    t->next = NULL;
		    *tpp2 = t;
		    break;
		}
		if (t->seqno == seqno && t->len == status->pos
			&& memcmp(t->tag, ffline, t->len) == 0) {
		    r = t->r;
		    break;
		}
		tpp2 = &t->next;
	    }
	    tpp = &r->tree;
	}
	else {
	    struct rootrec *r;

	    r = *rootpp;
	    if (r == NULL) {
		r = (struct rootrec *) xmalloc(sizeof(*r), "root record");
		r->next = NULL;
		r->tree = &not_read_yet;	/* readdir() not done yet */
		r->fullpat = NULL;		/* no ls-R yet */
		*rootpp = r;
		r->flags = status->flags |
		    ((oneatom->flags & F_PCT_S) ? twoatom : oneatom) ->flags;
		if (r->flags & F_QUICKFIND)
		    init_quick_find(r, status->pos, oneatom, status->pos);
		else if (status->quickchar != '\0') {
		    r->flags |= (status->quickchar == 'Q' ? F_QUICKONLY : 0);
		    /* (init_quick_find() will set F_QUICKFIND in r->flags) */
		    init_quick_find(r, status->quickpos, oneatom, status->pos);
		}
	    }
	    rootpp = &r->next;
	    tpp = &r->tree;
		/* do ls-R search, if appropriate */
	    if ((r->flags & F_QUICKFIND) && r->fullpat != NULL) {
		int		pos;
		struct lsr	*lp;

		pos = xlat(NULL, NULL, status->pos, r->keypat, r->keypat_end);
		lp = lsrtab[prehash(ffline + status->pos, pos - status->pos)
		    % XtNumber(lsrtab)];
		for (;;) {
		    if (lp == NULL) break;	/* no match */
		    if (lp->frecp == frecp && lp->seqno == seqno
		      && lp->keylen == pos - status->pos
		      && memcmp(lp->key, ffline + status->pos, lp->keylen) == 0)
		      {		/* if match */
			struct statrec	stat1;
			int		len = strlen(lp->value);

			stat1 = *status;
			if (stat1.pos + len > ffline_len)
			    expandline(stat1.pos + len);
			bcopy(lp->value, ffline + stat1.pos, len);
			stat1.pos += len;
			xlat(&stat1, &stat1, 0, r->fullpat, r->fullpat_end);
			TRY_TO_OPEN(stat1.pos, stat1.flags);
			return;
		    }
		    lp = lp->next;
		}
		    /* if there's an ls-R database, and our file is not there,
		     * then we don't look recursively this time. */
		return;
	    }
	    if ((r->flags & (F_QUICKFIND | F_QUICKONLY))
		    == (F_QUICKFIND | F_QUICKONLY)) {
#ifdef	PK_AND_GF
		gflags |= status->flags;
#endif
		return;
	    }
	}

	do_tree_search(tpp, oneatom, NULL, status->pos, status->flags);
}


/*
 *	dostep() - Process a steprec record.
 */

static	void
dostep(sp, stat0)
	_Xconst	struct steprec	*sp;
	_Xconst	struct statrec	*stat0;
{
	struct statrec		status;

	xlat(stat0, &status, 0, sp->str, sp->strend);

	status.flags |= sp->flags;

	if (sp->atom != NULL) {		/* if wild cards or // */
	    if (sp->flags & F_PCT_S) {
		int	i;

		for (i = 0; i < lrec.v.pct_s_count; ++i)
		    begin_tree_search(&status, sp->atom, lrec.v.pct_s_atom[i]);
	    }
	    else
		begin_tree_search(&status, sp->atom, NULL);
	}
	else if (sp->nextstep != NULL) {	/* if {} list */
	    struct steprec *sp1;

	    for (sp1 = sp->nextstep; sp1 != NULL; sp1 = sp1->next)
		dostep(sp1, &status);
	}
	else {	/* end of string */
	    if (!(status.flags & F_FILE_USED)) {
		xlat(&status, &status, 0, lrec.no_f_str, lrec.no_f_str_end);
#ifdef	PK_AND_GF
		status.flags |= lrec.no_f_str_flags;
#endif
	    }
	    TRY_TO_OPEN(status.pos, status.flags);
	}
}


/*
 *	fixbegin() - Handle !!, %t, %S, and ~ at the beginning of a specifier.
 */

static	void
fixbegin(sp)
	struct steprec	*sp;
{
	_Xconst	char	*p	= sp->str;

	/*
	 * Initial !!.
	 */

	if (*p == '!' && p[1] == '!' && sp->strend >= p + 2) {
	    sp->flags |= F_QUICKONLY;
	    p += 2;
	}

	/*
	 * Take care of %S, %t, and ~.
	 */

	sp->home = NULL;

	if (*p == '%') {
	    if (p[1] == 'S' && sp->strend == p + 2 && sp->atom == NULL
	      && sp->nextstep == NULL) {	/* %S */
		init_texmf();
		sp->flags |= F_PCT_T;
		p = "/";
		sp->strend = p + 1;
		sp->nextstep = scan_pct_s();
	    }
	    if (p[1] == 't' && sp->strend >= p + 2) {	/* %t */
		init_texmf();
		sp->flags |= F_PCT_T;
		p += 2;
	    }
	}
	else if (*p == '~')
	    gethome(&p, sp->strend, &sp->home);

	sp->str = p;
}


/*
 *	pathpart() - Handle one (colon-separated) part of the search path.
 */

static	_Xconst	char *
pathpart(p)
	_Xconst	char	*p;
{
	struct steprec			*sp;
	static	_Xconst	struct statrec	stat_ini	= {0, 0, '\0', 0};
	struct statrec			status;

	sp = *steppp;
	if (sp == NULL) {	/* do prescanning (lazy evaluation) */
	    _Xconst char *p_end;
#if	CFGFILE
	    _Xconst char *p1;
	    int		pos;
#endif

#if	CFGFILE
	    p_end = p;
	    pos = form_path_part(&p_end, '%');
	    ffline[pos] = '\0';
	    if (pos <= p_end - p && memcmp(ffline, p_end - pos, pos) == 0)
		p1 = p_end - pos;
	    else
		p1 = newstring(ffline, pos + 1);
	    (void) prescan(p1);
#else	/* !CFGFILE */
	    p_end = prescan(p);
		/* skip to next colon or end of string */
	    for (; *p_end != '\0' && *p_end != ':'; ++p_end)
		if (*p_end == '%' && p_end[1] != '\0')
		    ++p_end;
#endif	/* CFGFILE */

	    sp = *steppp;
	    if (sp->str == sp->strend && sp->nextstep != NULL) {
		/* If it's braces right off the bat, then pretend these are */
		/* all different specifiers (so %t, %S, !!, and ~ can work). */
		/* There's a memory leak here, but it's bounded. */
		struct steprec	*sp1	= sp->nextstep;

		for (;;) {
		    fixbegin(sp1);
		    if (sp1->next == NULL) {
			sp1->nextpart = p_end;
			break;
		    }
		    sp1->nextpart = p;
		    sp1 = sp1->next;
		}
		*steppp = sp = sp->nextstep;	/* relink the chain */
	    }
	    else {
		fixbegin(sp);
		sp->nextpart = p_end;
	    }
	}
	steppp = &sp->next;

	if (sp->flags & F_PCT_T) {
	    struct texmfrec	*tp;

	    for (tp = texmfhead; tp != NULL; tp = tp->next) {
		status = stat_ini;
		status.flags = tp->flags;
		if (tp->home != NULL) {
		    status.pos = strlen(tp->home);
		    if (status.pos + tp->len >= ffline_len)
			expandline(status.pos + tp->len);
		    bcopy(tp->home, ffline, status.pos);
		}
		bcopy(tp->str, ffline + status.pos, tp->len);
		status.pos += tp->len;
#ifndef	PK_AND_GF
		dostep(sp, &status);
#else
		if (lrec.pk_gf_addr != NULL) *lrec.pk_gf_addr = "pk";
		pk_second_pass = False;
		gflags = 0;
		dostep(sp, &status);
		if (gflags & F_PK_USED) {
		    *lrec.pk_gf_addr = "gf";
		    pk_second_pass = True;
		    dostep(sp, &status);
		}
#endif
	    }
	}
	else {
	    status = stat_ini;
	    if (sp->home != NULL) {
		int	len = strlen(sp->home);

		if (len >= ffline_len)
		    expandline(len);
		bcopy(sp->home, ffline, len);
		status.pos = len;
	    }
#ifndef	PK_AND_GF
	    dostep(sp, &status);
#else
	    if (lrec.pk_gf_addr != NULL) *lrec.pk_gf_addr = "pk";
	    pk_second_pass = False;
	    gflags = 0;
	    dostep(sp, &status);
	    if (gflags & F_PK_USED) {
		*lrec.pk_gf_addr = "gf";
		pk_second_pass = True;
		dostep(sp, &status);
	    }
#endif
	}

	return sp->nextpart;
}


#if	CFGFILE

/*
 *	Recursive routine for searching in config file path variables.
 */

static	void
ffrecurse(env_ptr, dflt)
	_Xconst	struct envrec	*env_ptr;
	_Xconst	char		*dflt;
{
	_Xconst	char		*p;
	Boolean			did_next_default;

	if (env_ptr != NULL) {
	    p = env_ptr->value;
	    env_ptr = env_ptr->next;
	    if (env_ptr != NULL && env_ptr->key != NULL)
		env_ptr = NULL;
	}
	else if (dflt != NULL) {
	    p = dflt;
	    dflt = NULL;
	}
	else return;

	did_next_default = False;
	for (;;) {
	    if (*p == '\0' || *p == ':') {
		if (!did_next_default) {
		    ffrecurse(env_ptr, dflt);
		    did_next_default = True;
		}
	    }
	    else
		p = pathpart(p);
	    if (*p == '\0')
		break;
	    ++p;
	}
}

#endif	/* CFGFILE */


/*
 *	This is the main search routine.  It calls pathpath() for each
 *	component of the path.  Substitution for the default path is done here.
 */

FILE *
filefind(name, srchtype, path_ret)
	_Xconst	char	*name;		/* name of the font or file */
	struct findrec	*srchtype;	/* what type of search to perform */
	_Xconst	char	**path_ret;	/* put the name of the file here */
{
	lrec = *srchtype;
	fF_values[0] = fF_values[1] = name;

	if (setjmp(got_it) == 0) {
	    if (*name == '/' || *name == '~') {		/* if absolute path */
		int			pos;
		int			pos0	= 0;
		_Xconst struct passwd	*pw	= NULL;

		if (*name == '~')
		    pw = ff_getpw(&name, name + strlen(name));
		if (pw != NULL) {
		    pos0 = strlen(pw->pw_dir);
		    if (pos0 >= ffline_len)
			expandline(pos0);
		    bcopy(pw->pw_dir, ffline, pos0);
		    fF_values[0] = fF_values[1] = name;
		}
#ifndef	PK_AND_GF
		pos = xlat(NULL, NULL, pos0, lrec.abs_str,
			lrec.abs_str + strlen(lrec.abs_str));
		TRY_TO_OPEN(pos, 0);
#else
		if (lrec.pk_gf_addr != NULL) *lrec.pk_gf_addr = "pk";
		pk_second_pass = False;
		pos = xlat(NULL, NULL, pos0, lrec.abs_str,
			lrec.abs_str + strlen(lrec.abs_str));
		TRY_TO_OPEN(pos, lrec.abs_str_flags);
		if (lrec.abs_str_flags & F_PK_USED) {
		    *lrec.pk_gf_addr = "gf";
		    pk_second_pass = True;
		    pos = xlat(NULL, NULL, pos0, lrec.abs_str,
			    lrec.abs_str + strlen(lrec.abs_str));
		    TRY_TO_OPEN(pos, lrec.abs_str_flags);
		}
#endif
	    }
	    else {
		_Xconst char	*p;

		steppp = &lrec.v.stephead;
		frecp = srchtype;
		seqno = 0;
		rootpp = &lrec.v.rootp;

#if	CFGFILE

		if (lrec.path1 == NULL)
		    ffrecurse(lrec.envptr, lrec.path2);
		else {
		    Boolean did_the_default = False;

		    for (p = lrec.path1;;) {
			if (*p == '\0' || *p == ':') {
				/* bring in the default path */
			    if (!did_the_default) {
				ffrecurse(lrec.envptr, lrec.path2);
				did_the_default = True;
			    }
			}
			else
			    p = pathpart(p);
			if (*p == '\0') break;
			++p;	/* skip the colon */
		    }
		}

#else	/* !CFGFILE */

		for (p = lrec.path1;;) {
		    if (*p == '\0' || *p == ':') {
			    /* bring in the default path */
			if (lrec.path2 != NULL) {
			    _Xconst char	*p1	= lrec.path2;

			    for (;;) {
				if (*p1 == '\0') break;
				if (*p1 == ':') continue;
				p1 = pathpart(p1);
				if (*p1 == '\0') break;
				++p1;	/* skip the colon */
			    }
			}
		    }
		    else
			p = pathpart(p);
		    if (*p == '\0') break;
		    ++p;	/* skip the colon */
		}

#endif	/* CFGFILE */

		file_found = NULL;
	    }
	}
	else {
	    /* it longjmp()s here when it finds the file */
	    if (FFDEBUG) puts("--Success--\n");
	    if (path_ret != NULL)
		*path_ret = newstring(ffline, -1);
	}

	srchtype->v = lrec.v;	/* restore volatile parameters of *srchtype */

	return file_found;
}
