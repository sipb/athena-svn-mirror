/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * NOTE:
 *	xdvi is based on prior work, as noted in the modification history
 *	in xdvi.c.
 */

#include "xdvi.h"
#include <errno.h>

#ifdef	X_NOT_STDC_ENV
extern	int	errno;
extern	char	*getenv ARGS((_Xconst char *));
#endif

/*
 *	If you think you have to change DEFAULT_TAIL, then you haven't read the
 *	documentation closely enough.
 */
#ifndef	VMS
#define	PATH_SEP	':'
#define	DEFAULT_TAIL	"/%f.%d%p"
#define	DEFAULT_VF_TAIL	"/%f.vf"
#else	/* VMS */
#define	PATH_SEP	'/'
#define	DEFAULT_TAIL	":%f.%d%p"
#define	DEFAULT_VF_TAIL	":%f.vf"
#endif	/* VMS */

static	_Xconst char	*font_path;
static	_Xconst char	*default_font_path	= DEFAULT_FONT_PATH;
static	_Xconst char	*vf_path;
static	_Xconst char	*default_vf_path	= DEFAULT_VF_PATH;
#ifdef	MAKEPK
static	_Xconst	char	*makepkcmd		= NULL;
#endif

#if	PS
struct findrec	figfind		= {DEFAULT_FIG_PATH, NULL};
struct findrec	headerfind	= {DEFAULT_HEADER_PATH, NULL};
#endif

#ifdef	SEARCH_SUBDIRECTORIES
static	char	default_subdir_path[]	= DEFAULT_SUBDIR_PATH;
#endif
static	int	*sizes, *sizend;
static	char	default_size_list[]	= DEFAULT_FONT_SIZES;

#ifdef	_POSIX_SOURCE
#include <limits.h>
#ifdef	PATH_MAX
#define	FILENAMESIZE	PATH_MAX
#endif
#endif

#ifndef	FILENAMESIZE
#define	FILENAMESIZE 512
#endif

#if	defined(sun) && BSD
char	*sprintf();
#endif

#ifndef	EX_OK
#ifndef	VMS
#define	EX_OK	0
#else
#define	EX_OK	1
#endif
#endif

#ifdef	SEARCH_SUBDIRECTORIES
/* We will need some system include files to deal with directories.  */
/* <sys/types.h> was included by xdvi.h.  */

#include <sys/stat.h>

static	int	is_dir ();

#if	defined(SYSV) || defined(_POSIX_SOURCE)
#include <dirent.h>
typedef	struct dirent	*directory_entry_type;
#else
#include <sys/dir.h>
typedef	struct direct	*directory_entry_type;
#endif

/* Declare the routine to get the current working directory.  */

#ifdef	HAVE_GETWD
extern	char	*getwd ();
#define	GETCWD(b, len)	((b) ? getwd (b) : getwd (xmalloc (len, "getwd")))
#else
/* POSIX says getcwd result is undefined if the pointer is NULL; at least
   on a Convex, the result is a coredump.  Hence the GETCWD macro
   below is defined, as it works regardless of what getcwd() does
   with a NULL pointer  */
#define	GETCWD(b, len)	((b) ? getcwd (b,len) \
			: getcwd (xmalloc (len, "getcwd"),len))
#ifndef	_POSIX_SOURCE
#if	NeedFunctionPrototypes
/* extern	char	*getcwd (char *, int); */
#else
/* extern	char	*getcwd (); */
#endif	/* not NeedFunctionPrototypes */
#endif	/* not _POSIX_SOURCE */
#endif	/* not HAVE_GETWD */

static	char	*cwd;

/* The following is a data structure containing the precomputed names of
   subdirectories to be recursively searched. */

static	struct subdir_entry {
	char	*name;		/* partial string */
	_Xconst char	*index;	/* reference point in {,default_}font_path */
	struct subdir_entry *next;	/* link in list */
}
	*subdir_head	= NULL,
	*next_subdir;

#ifndef	S_ISDIR
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

/* Return true if FN is a directory or a symlink to a directory,
   false if not. */

static	int
is_dir (fn)
	char	*fn;
{
	struct stat	stats;

	return stat (fn, &stats) == 0 && S_ISDIR (stats.st_mode);
}

/*
 *	Compute extra length of subdirectory entries, including a star for each.
 */

static	int
extra_len(str1, str2)
	char	*str1, *str2;
{
	int	bias	= 0;
	char	*p	= str1;
	char	*q;

	do {
	    q = index(p, PATH_SEP);
	    if (q == NULL) q = p + strlen(p);
	    if (q == p) {
		if (str2 != NULL) {
		    bias += extra_len(str2, (char *) NULL);
		    str2 = NULL;
		}
	    }
	    else ++bias;	/* don't forget the star */
	    p = q + 1;
	}
	while (p[-1] != '\0');
	return bias + p - str1;
}

/*
 *	Add the specifiers (and double stars) for the given strings (user
 *	string, plus default string) to the destination string.
 */

static	void
add_subdir_paths(dst, dst_first, src, src_default)
	char	*dst, *dst_first, *src, *src_default;
{
	char	*q;

	do {
	    q = index(src, PATH_SEP);
	    if (q == NULL) q = src + strlen(src);
	    if (q == src) {
		if (src_default != NULL) {
		    add_subdir_paths(dst, dst_first, src_default, (char *)NULL);
		    dst += strlen(dst);
		    src_default = NULL;
		}
	    }
	    else {
		if (dst != dst_first) *dst++ = PATH_SEP;
		bcopy(src, dst, q - src);
		dst += q - src;
		*dst++ = '*';
	    }
	    src = q + 1;
	}
	while (src[-1] != '\0');
	*dst = '\0';
}

/*
 *	Make a subdirectory entry.
 */

static	struct subdir_entry *
make_subdir_entry(index, name)
	_Xconst char	*index;
	char		*name;
{
	struct subdir_entry		*new_entry;
	static	struct subdir_entry	**subdir_tail = &subdir_head;

	*subdir_tail = new_entry = (struct subdir_entry *)
	    xmalloc(sizeof(struct subdir_entry), "subdirectory list entry");
	subdir_tail = &(new_entry->next);
	new_entry->name = strcpy(xmalloc(strlen(name) + 1,
	    "subdirectory entry string"), name);
	new_entry->index = index;
	new_entry->next = NULL;
	return new_entry;
}

/*
 *	Create the subdirectory linked list for the given initial string
 */

static	void
add_subdirs(str, len, recurs)
	_Xconst char	*str;
	int	len;
	Boolean	recurs;
{
	int	len1 = len;
	char	temp[FILENAMESIZE];
	struct subdir_entry *next_subdir;
	DIR	*dir;
	directory_entry_type	e;

	bcopy(str, temp, len);
	if (len > 0 && temp[len - 1] != '/') temp[len1++] = '/';
	temp[len1] = '\0';
	next_subdir = make_subdir_entry(str, temp + len);
	do {
	    /* By changing directories, we save a bunch of string
	       concatenations (and make the pathnames the kernel looks up
	       shorter).  */
	    Strcpy(temp + len, next_subdir->name);
	    if (chdir (temp) != 0) continue;

	    dir = opendir (".");
	    if (dir == NULL) continue;

	    len1 = strlen(temp);
	    if (len1 == 0 || temp[len1 - 1] != '/') temp[len1++] = '/';
	    while ((e = readdir (dir)) != NULL) {
		if (is_dir (e->d_name) && strcmp (e->d_name, ".") != 0
			&& strcmp (e->d_name, "..") != 0) {
		    Strcpy(temp + len1, e->d_name);
		    (void) make_subdir_entry(str, temp + len);
		}
	    }
	    (void) closedir (dir);


	    /* Change back to the current directory, in case the path
	       contains relative directory names.  */
	    if (chdir (cwd) != 0) {
		perror (cwd);
#if	PS
		ps_destroy();
#endif
		exit (errno);
	    }
	}
	while (recurs && (next_subdir = next_subdir->next) != NULL);
}

/*
 *	Recursively figure out the subdirectory tree and precompute the
 *	list of subdirectories to search.
 */

static	void
compute_subdir_paths(fp, fp_default)
	_Xconst char	*fp;
	_Xconst char	*fp_default;
{
	_Xconst char	*star_loc = NULL;
	_Xconst char	*endp;

	do {
	    if (star_loc == NULL) {
		star_loc = index(fp, '*');
		if (star_loc == NULL) star_loc = fp + strlen(fp);
	    }
	    endp = index(fp, PATH_SEP);
	    if (endp == NULL) endp = fp + strlen(fp);
	    if (endp == fp) {
		if (fp_default != NULL) {
		    compute_subdir_paths(fp_default, (char *) NULL);
		    fp_default = NULL;
		}
	    }
	    else if (star_loc < endp) {
		add_subdirs(fp, star_loc - fp, star_loc[1] == '*');
		star_loc = NULL;
	    }
	    fp = endp + 1;
	}
	while (fp[-1] != '\0');
}
#endif	/* SEARCH_SUBDIRECTORIES */

static	void
get_sizes(size_list, spp)
	char	*size_list;
	int	**spp;
{
	if (*size_list == PATH_SEP) ++size_list;
	for (;;) {
	    *(*spp)++ = atoi(size_list);
	    size_list = index(size_list, PATH_SEP);
	    if (size_list == NULL) return;
	    ++size_list;
	}
}

void
init_font_open()
{
	char	*size_list;
	int	*sp, *sp1, *sp2;
	unsigned int n;
	char	*p;
#ifdef	SEARCH_SUBDIRECTORIES
	char	*q;
#endif

	if ((font_path = getenv("XDVIFONTS")) == NULL
#ifndef	XDVIFONTS_ONLY
		&& (font_path = getenv("PKFONTS")) == NULL
		&& (font_path = getenv("TEXPKS")) == NULL
		&& (font_path = getenv("TEXFONTS")) == NULL
#endif
		) {
	    font_path = default_font_path;
	    default_font_path = NULL;
	}

#ifdef	SEARCH_SUBDIRECTORIES
	p = getenv ("TEXFONTS_SUBDIR");
	if (p == NULL) p = "";
	q = xmalloc((unsigned) strlen(font_path)
	    + extra_len(p, default_subdir_path) + 1,
	    "initializing font searching");
	Strcpy(q, font_path);
	add_subdir_paths(q + strlen(q), q, p, default_subdir_path);
	font_path = q;

	/* Unfortunately, we can't look in the environment for the current
	   directory, because if we are running under a program (let's say
	   Emacs), the PWD variable might have been set by Emacs' parent
	   to the current directory at the time Emacs was invoked.  This
	   is not necessarily the same directory the user expects to be
	   in.  So, we must always call getcwd(3) or getwd(3), even though
	   they are slow and prone to hang in networked installations.  */
	cwd = GETCWD ((char *) NULL, FILENAMESIZE + 2);
	if (cwd == NULL) {
	    perror ("getcwd");
#if	PS
	    ps_destroy();
#endif
	    exit (errno);
	}
	compute_subdir_paths(font_path, default_font_path);
#endif

	if ((vf_path = getenv("XDVIVFS")) == NULL
#ifndef	XDVIFONTS_ONLY
		&& (vf_path = getenv("VFFONTS")) == NULL
#endif
		) {
	    vf_path = default_vf_path;
	    default_vf_path = NULL;
	}

	size_list = getenv("XDVISIZES");
	n = 1;	/* count number of sizes */
	if (size_list == NULL || *size_list == PATH_SEP)
	    for (p = default_size_list; (p = index(p, PATH_SEP)) != NULL; ++p)
		++n;
	if (size_list != NULL)
	    for (p = size_list; (p = index(p, PATH_SEP)) != NULL; ++p) ++n;
	sizes = (int *) xmalloc(n * sizeof(int), "size list");
	sizend = sizes + n;
	sp = sizes;	/* get the actual sizes */
	if (size_list == NULL || *size_list == PATH_SEP)
	    get_sizes(default_size_list, &sp);
	if (size_list != NULL) get_sizes(size_list, &sp);

	/* bubble sort the sizes */
	sp1 = sizend - 1;	/* extent of this pass */
	do {
	    sp2 = NULL;
	    for (sp = sizes; sp < sp1; ++sp)
		if (*sp > sp[1]) {
		    int i = *sp;
		    *sp = sp[1];
		    sp[1] = i;
		    sp2 = sp;
		}
	}
	while ((sp1 = sp2) != NULL);
}

static	FILE *
formatted_open(path, font, pk_or_gf, dpi, name, first_try, tail)
	_Xconst char	*path;
	_Xconst char	*font;
	_Xconst char	*pk_or_gf;
	int	dpi;
	char	**name;
	Boolean	first_try;
	_Xconst	char	*tail;
{
	_Xconst char	*p = path;
	char	nm[FILENAMESIZE];
	char	*n = nm;
	char	c;
	Boolean	f_used = False;
	Boolean	p_used = False;
	FILE	*f;

#ifdef	SEARCH_SUBDIRECTORIES
	if (next_subdir != NULL && next_subdir->index == p) {
	    int len = index(p, '*') - p;

	    bcopy(p, n, len);
	    p += len;
	    n += len;
	    Strcpy(n, next_subdir->name);
	    n += strlen(n);
	    ++p;
	    if (*p == '*') ++p;
	    if (*p != '/') *n++ = '/';
	}
#endif
	for (;;) {
	    c = *p++;
	    if (c==PATH_SEP || c=='\0') {
		if (f_used) break;
		p = tail;
		continue;
	    }
	    if (c=='%') {
		c = *p++;
		switch (c) {
		    case 'f':
			f_used = True;
		    case 'F':
			Strcpy(n, font);
			break;
		    case 'p':
			p_used = True;
			Strcpy(n, pk_or_gf);
			break;
		    case 'd':
			Sprintf(n, "%d", dpi);
			break;
		    case 'b':
			Sprintf(n, "%d", pixels_per_inch);
			break;
		    case 'm':
			Strcpy(n, resource.mfmode != NULL ? resource.mfmode
			    : "default");
			break;
		    default:
			*n++ = c;
			*n = '\0';
		}
		n += strlen(n);
	    }
	    else *n++ = c;
	}
	if (!p_used && !first_try) return NULL;
	*n = '\0';
	if (debug & DBG_OPEN) Printf("Trying font file %s\n", nm);
	f = xfopen(nm, OPEN_MODE);
	if (f != NULL) {
	    *name = xmalloc((unsigned) (n - nm + 1), "font file name");
	    Strcpy(*name, nm);
	}
	return f;
}

/*
 *	Try a given size
 */

static	FILE *
try_size(font, dpi, name, x_font_path, x_default_font_path)
	_Xconst char	*font;
	int	dpi;
	char	**name;
	_Xconst char	*x_font_path;
	_Xconst char	*x_default_font_path;
{
	_Xconst char	*p	= x_font_path;
	FILE	*f;

	/*
	 * loop over paths
	 */
#ifdef	SEARCH_SUBDIRECTORIES
	next_subdir = subdir_head;
#endif
	for (;;) {
	    if (*p == PATH_SEP || *p == '\0') {
		if (x_default_font_path != NULL &&
			(f = try_size(font, dpi, name, x_default_font_path,
			(_Xconst char *) NULL)) != NULL)
		    return f;
		if (*p == '\0') break;
	    }
	    else {
#define	FIRST_TRY True
#ifdef	USE_PK
		if ((f = formatted_open(p, font, "pk", dpi, name, FIRST_TRY,
			DEFAULT_TAIL)) != NULL)
		    return f;
#undef	FIRST_TRY
#define	FIRST_TRY False
#endif
#ifdef	USE_GF
		if ((f = formatted_open(p, font, "gf", dpi, name, FIRST_TRY,
			DEFAULT_TAIL)) != NULL)
		    return f;
#endif
#ifdef	SEARCH_SUBDIRECTORIES
		if (next_subdir != NULL && next_subdir->index == p) {
		    next_subdir = next_subdir->next;
		    if (next_subdir != NULL && next_subdir->index == p)
			continue;
		}
#endif
		p = index(p, PATH_SEP);
		if (p == NULL) break;
	    }
	    ++p;
	}
	return NULL;
}

/*
 *	Try a virtual font
 */

static	FILE *
try_vf(font, name, x_vf_path, x_default_vf_path)
	_Xconst char	*font;
	char	**name;
	_Xconst char	*x_vf_path;
	_Xconst char	*x_default_vf_path;
{
	_Xconst char	*p	= x_vf_path;
	FILE	*f;

	/*
	 * loop over paths
	 */
	for (;;) {
	    if (*p == PATH_SEP || *p == '\0') {
		if (x_default_vf_path != NULL &&
			(f = try_vf(font, name, x_default_vf_path,
			(_Xconst char *) NULL)) != NULL)
		    return f;
		if (*p == '\0') break;
	    }
	    else {
		if ((f = formatted_open(p, font, "vf", 0, name, True,
			DEFAULT_VF_TAIL)) != NULL)
		    return f;
		p = index(p, PATH_SEP);
		if (p == NULL) break;
	    }
	    ++p;
	}
	return NULL;
}

#ifdef	MAKEPK
#ifndef	MAKEPKCMD
#define	MAKEPKCMD	"MakeTeXPK"
#endif
#define	NOBUILD		29999
#define	MKPKSIZE 	256
#endif	/* MAKEPK */

/*
 *	Try a given font name
 */

#ifndef	MAKEPK
#define	PRE_FONT_OPEN(font, fdpi, dpi_ret, name, ignore) \
		pre_font_open(font, fdpi, dpi_ret, name)
#else
#define	PRE_FONT_OPEN	pre_font_open
#endif

static	FILE *
PRE_FONT_OPEN(font, fdpi, dpi_ret, name, magstepval)
	_Xconst char	*font;
	double	fdpi;
	int	*dpi_ret;
	char	**name;
#ifdef	MAKEPK
	int	magstepval;
#endif
{
	FILE	*f;
	int	*p1, *p2;
	int	dpi	= fdpi + 0.5;
	int	tempdpi;
#ifndef	VMS
	_Xconst char	*path_to_use;
	_Xconst char	*vf_path_to_use;
#endif

	/*
	 * Loop over sizes.  Try actual size first, then closest sizes.
	   If the pathname is absolutely or explicitly relative, don't
	   use the usual paths to search for it; just look for it in the
	   directory specified.
	 */

#ifndef	VMS
	path_to_use = (_Xconst char *) NULL;
	if (*font == '/') path_to_use = "/";
	else if (*font == '.' && (*(font + 1) == '/'
		|| (*(font + 1) == '.' && *(font + 2) == '/')))
	    path_to_use = ".";
	vf_path_to_use = path_to_use;
	if (path_to_use == NULL) {
	    path_to_use = font_path;
	    vf_path_to_use = vf_path;
	}
#else	/* VMS */
#define	path_to_use	font_path
#define	vf_path_to_use	vf_path
#endif	/* VMS */

	if ((f = try_size(font, *dpi_ret = dpi, name, path_to_use,
		default_font_path)) != NULL)
	    return f;

	/* Try at one away from the size we just tried, to account
	   for rounding error.  */
	tempdpi = dpi + (dpi < fdpi ? 1 : -1);
	if ((f = try_size(font, tempdpi, name, path_to_use, default_font_path))
		!= NULL) {
	    *dpi_ret = tempdpi;
	    return f;
	}

	/* Try a virtual font. */
	if ((f = try_vf(font, name, vf_path_to_use, default_vf_path)) != NULL)
	    return f;

#ifdef	MAKEPK
	/* Try to create the font. */
	if (magstepval != NOBUILD && resource.makepk) {
	    char mkpk[MKPKSIZE];
	    Boolean used_fontname = False;
	    Boolean used_mfmode = False;
	    _Xconst char *p;
	    char *q;

	    if (makepkcmd == NULL) {
		makepkcmd = getenv("XDVIMAKEPK");
		if (makepkcmd == NULL) makepkcmd = MAKEPKCMD;
	    }
	    p = makepkcmd;
	    q = mkpk;
	    for (;;) {
		if (*p == '%') {
		    switch (*++p) {
		    case 'n':
			Strcpy(q, font);
			used_fontname = True;
			break;
		    case 'd':
			Sprintf(q, "%d", dpi);
			break;
		    case 'b':
			Sprintf(q, "%d", pixels_per_inch);
			break;
		    case 'm':
			if (magstepval == NOMAGSTP)
			    Sprintf(q, "%d+%d/%d", dpi / pixels_per_inch,
				dpi % pixels_per_inch, pixels_per_inch);
			else if (magstepval < 0)
			    Sprintf(q, "magstep\\(-%d%s\\)", -magstepval / 2,
				magstepval % 2 ? ".5" :"");
			else
			    Sprintf(q, "magstep\\(%d%s\\)", magstepval / 2,
				magstepval % 2 ? ".5" :"");
			break;
		    case 'o':
			Strcpy(q, resource.mfmode != NULL ? resource.mfmode
			    : "default");
			used_mfmode = True;
			break;
		    case '%':
			*q++ = '%';
			*q = '\0';
			break;
		    case '\0':
			--p;
			*q = '\0';
			break;
		    default:
			*q++ = '%';
			*q++ = *p;
			*q = '\0';
			break;
		    }
		    q += strlen(q);
		}
		else if (*p == '\0')
		    if (used_fontname) break;
		    else {
			p = " %n %d %b %m";
			continue;
		    }
		else *q++ = *p;
		++p;
	    }
	    if (resource.mfmode != NULL && !used_mfmode) {
		*q++ = ' ';
		Strcpy(q, resource.mfmode);
	    }
	    else *q = '\0';

	    Printf("- %s\n", mkpk);
	    if (system(mkpk) == EX_OK
		&& (f = try_size(font, dpi, name, path_to_use,
			default_font_path))
		    != NULL)
		return f;
	}
#endif	/* MAKEPK */

	/* Now try at all the sizes. */
	for (p2 = sizes; p2 < sizend; ++p2) if (*p2 >= dpi) break;
	p1 = p2;
	for (;;) {
		/* find another resolution */
	    if (p1 <= sizes)
		if (p2 >= sizend) return NULL;
		else tempdpi = *p2++;
	    else if (p2 >= sizend || (long) dpi * dpi <= (long) p1[-1] * *p2)
		    tempdpi = *--p1;
		else tempdpi = *p2++;
	    if ((f = try_size(font, *dpi_ret = tempdpi, name, path_to_use,
		    default_font_path)) != NULL)
		return f;
	}
}

/* ARGSUSED */
FILE *
font_open(font, font_ret, dpi, dpi_ret, magstepval, name)
	_Xconst char	*font;
	char	**font_ret;
	double	dpi;
	int	*dpi_ret;
	int	magstepval;
	char	**name;
{
	FILE	*f;
	int	actual_pt, low_pt, high_pt, trial_pt;
	char	fn[50], *fnend;

	f = PRE_FONT_OPEN(font, dpi, dpi_ret, name, magstepval);
	if (f != NULL) {
	    *font_ret = NULL;
	    return f;
	}
	Strcpy(fn, font);
	fnend = fn + strlen(fn);
	while (fnend > fn && fnend[-1] >= '0' && fnend[-1] <= '9') --fnend;
	actual_pt = low_pt = high_pt = atoi(fnend);
	if (actual_pt) {
	    low_pt = actual_pt - 1;
	    high_pt = actual_pt + 1;
	    for (;;) {
		if (2 * low_pt >= actual_pt &&
		    (low_pt * high_pt > actual_pt * actual_pt ||
		    high_pt > actual_pt + 5))
			trial_pt = low_pt--;
		else if (high_pt > actual_pt + 5) break;
		else trial_pt = high_pt++;
		Sprintf(fnend, "%d", trial_pt);
		f = PRE_FONT_OPEN(fn, dpi * actual_pt / trial_pt, dpi_ret,
		    name, NOBUILD);
		if (f != NULL) {
		    *font_ret = strcpy(xmalloc((unsigned) strlen(fn) + 1,
			"name of font used"), fn);
		    return f;
		}
	    }
	}
	if (alt_font != NULL) {
	    f = PRE_FONT_OPEN(alt_font, dpi, dpi_ret, name, magstepval);
	    if (f != NULL)
		*font_ret = strcpy(xmalloc((unsigned) strlen(alt_font) + 1,
		    "name of font used"), alt_font);
	}
	return f;
}
