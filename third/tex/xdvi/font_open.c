/*
 * Copyright (c) 1996 Paul Vojta.  All rights reserved.
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

#include "filf_app.h"	/* application-related defs, etc. */
#include "filefind.h"

#include <errno.h>

#if	!defined(X_NOT_POSIX) || BSD
#include <sys/wait.h>
#endif

#ifdef	VFORK
#if	VFORK == include
#include <vfork.h>
#endif
#else
#define	vfork	fork
#endif

#ifdef	X_NOT_STDC_ENV
extern	int	errno;
extern	char	*getenv ARGS((_Xconst char *));
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

#ifdef	MAKEPK
static	_Xconst	char	*makepkcmd		= NULL;
#endif

/*
 *	Information on how to search for pk and gf files.
 */

static	_Xconst	char	no_f_str_pkgf[]	= DEFAULT_TAIL;

static	struct findrec			search_pkgf	= {
	/* path1	*/	NULL,
#if	CFGFILE
	/* envptr	*/	NULL,
#endif
	/* path2	*/	DEFAULT_FONT_PATH,
	/* type		*/	"font",
	/* fF_etc	*/	"fFdbpm",
	/* x_var_char	*/	'd',
	/* n_var_opts	*/	3,
	/* no_f_str	*/	no_f_str_pkgf,
	/* no_f_str_end	*/	no_f_str_pkgf + sizeof(no_f_str_pkgf) - 1,
	/* abs_str	*/	"%f.%d%p",
#ifdef	PK_AND_GF
	/* no_f_str_flags */	F_FILE_USED | F_PK_USED,
	/* abs_str_flags */	F_FILE_USED | F_PK_USED,
	/* pk_opt_char	*/	'p',
	/* pk_gf_addr	*/	&fF_values[4],
#endif
	/* pct_s_str	*/
		"%qfonts/%p/%m//:%qfonts/%p/gsftopk//:%qfonts/%p/ps2pk//",
	{
	  /* v.stephead		*/	NULL,
	  /* v.pct_s_head	*/	NULL,
	  /* v.pct_s_count	*/	0,
	  /* v.pct_s_atom	*/	NULL,
	  /* v.rootp		*/	NULL,
	}
};

#ifdef	DOSNAMES

static	_Xconst	char	no_f_str_dos[]	= "/dpi%d/%f.%p";

static	struct findrec			search_pkgf_dos	= {
	/* path1	*/	NULL,
#if	CFGFILE
	/* envptr	*/	NULL,
#endif
	/* path2	*/	DEFAULT_FONT_PATH,
	/* type		*/	"font",
	/* fF_etc	*/	"fFdbpm",
	/* x_var_char	*/	'd',
	/* n_var_opts	*/	3,
	/* no_f_str	*/	no_f_str_dos,
	/* no_f_str_end	*/	no_f_str_dos + sizeof(no_f_str_dos) - 1,
	/* abs_str	*/	"%f.%d%p",
#ifdef	PK_AND_GF
	/* no_f_str_flags */	F_FILE_USED | F_PK_USED,
	/* abs_str_flags */	F_FILE_USED | F_PK_USED,
	/* pk_opt_char	*/	'p',
	/* pk_gf_addr	*/	&fF_values[4],
#endif
	/* pct_s_str	*/
		"%qfonts/%p/%m//:%qfonts/%p/gsftopk//:%qfonts/%p/ps2pk//",
	{
	  /* v.stephead		*/	NULL,
	  /* v.pct_s_head	*/	NULL,
	  /* v.pct_s_count	*/	0,
	  /* v.pct_s_atom	*/	NULL,
	  /* v.rootp		*/	NULL,
	}
};
#endif	/* DOSNAMES */

/*
 *	Information on how to search for vf files.
 */

static	_Xconst	char	no_f_str_vf[]	= DEFAULT_VF_TAIL;

static	struct findrec			search_vf	= {
	/* path1	*/	NULL,
#if	CFGFILE
	/* envptr	*/	NULL,
#endif
	/* path2	*/	DEFAULT_VF_PATH,
	/* type		*/	"vf",
	/* fF_etc	*/	"fF",
	/* x_var_char	*/	'f',		/* i.e., none */
	/* n_var_opts	*/	2,
	/* no_f_str	*/	no_f_str_vf,
	/* no_f_str_end	*/	no_f_str_vf + sizeof(no_f_str_vf) - 1,
	/* abs_str	*/	"%f.vf",
#ifdef	PK_AND_GF
	/* no_f_str_flags */	F_FILE_USED,
	/* abs_str_flags */	F_FILE_USED,
	/* pk_opt_char	*/	'f',		/* none */
	/* pk_gf_addr	*/	NULL,
#endif
	/* pct_s_str	*/	"%qfonts/vf//",
	{
	  /* v.stephead		*/	NULL,
	  /* v.pct_s_head	*/	NULL,
	  /* v.pct_s_count	*/	0,
	  /* v.pct_s_atom	*/	NULL,
	  /* v.rootp		*/	NULL,
	}
};

static	int	*sizes, *sizend;
static	char	default_size_list[]	= DEFAULT_FONT_SIZES;

static	char		bdpi_string[10];
static	char		dpi_string[10];

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

	sprintf(bdpi_string, "%d", pixels_per_inch);

#if	CFGFILE

#ifndef	XDVIFONTS_ONLY
	if ((search_pkgf.path1 = getenv("XDVIFONTS")) == NULL
	  && (search_pkgf.path1 = getenv("PKFONTS")) == NULL
	  && (search_pkgf.path1 = getenv("TEXPKS")) == NULL)
	    search_pkgf.path1 = getenv("TEXFONTS");
#else
	search_pkgf.path1 = getenv("XDVIFONTS");
#endif
#ifdef	DOSNAMES
	search_pkgf_dos.path1 = search_pkgf.path1;
#endif
	search_pkgf.envptr = ffgetenv("PKFONTS");
	/* clear it if it's a getenv() placeholder */
	if (search_pkgf.envptr != NULL && search_pkgf.envptr->value == NULL)
            search_pkgf.envptr = NULL;
#ifdef	DOSNAMES
	search_pkgf_dos.envptr = search_pkgf.envptr;
#endif

#else	/* !CFGFILE */

	if ((search_pkgf.path1 = getenv("XDVIFONTS")) == NULL
#ifndef	XDVIFONTS_ONLY
		&& (search_pkgf.path1 = getenv("PKFONTS")) == NULL
		&& (search_pkgf.path1 = getenv("TEXPKS")) == NULL
		&& (search_pkgf.path1 = getenv("TEXFONTS")) == NULL
#endif
		) {
	    search_pkgf.path1 = search_pkgf.path2;
	    search_pkgf.path2 = NULL;
	}
#ifdef	DOSNAMES
	search_pkgf_dos.path1 = search_pkgf.path1;
	search_pkgf_dos.path2 = search_pkgf.path2;
#endif

#endif	/* CFGFILE */

	/*
	 * pk/gf searching is the only kind that uses more than three
	 * characters in fF_etc, so these can be initialized once and then
	 * forgotten.
	 */

	fF_values[2] = dpi_string;
	fF_values[3] = bdpi_string;

#ifndef	PK_AND_GF
#ifdef	USE_PK
	fF_values[4] = "pk";
#else	/* !USE_PK */
#ifdef	USE_GF
	fF_values[4] = "gf";
#else
#error	You have to define at least one of USE_PK or USE_GF
#endif	/* USE_GF */
#endif	/* USE_PK */
#endif	/* PK_AND_GF */

	fF_values[5] = resource.mfmode;

#if	CFGFILE

#ifndef	XDVIFONTS_ONLY
	if ((search_vf.path1 = getenv("XDVIVFS")) == NULL)
	    search_vf.path1 = getenv("VFFONTS");
#else
	search_vf.path1 = getenv("XDVIVFS");
#endif
	search_vf.envptr = ffgetenv("VFFONTS");
	/* clear it if it's a getenv() placeholder */
	if (search_vf.envptr != NULL && search_vf.envptr->value == NULL)
            search_vf.envptr = NULL;

#else	/* !CFGFILE */

	if ((search_vf.path1 = getenv("XDVIVFS")) == NULL
#ifndef	XDVIFONTS_ONLY
		&& (search_vf.path1 = getenv("VFFONTS")) == NULL
#endif
		) {
	    search_vf.path1 = search_vf.path2;
	    search_vf.path2 = NULL;
	}

#endif	/* CFGFILE */

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

	/* bubble sort (||| AAGH!) the sizes */
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

/*
 *	Try a given size.
 */

static	FILE *
try_size(font, dpi, ret_path)
	_Xconst char	*font;
	int		dpi;
	_Xconst char	**ret_path;
{
#ifdef	DOSNAMES
	FILE		*retval;
#endif

	sprintf(dpi_string, "%d", dpi);

#ifdef	DOSNAMES
	retval = filefind(font, &search_pkgf, ret_path);
	if (retval != NULL) return retval;
	return filefind(font, &search_pkgf_dos, ret_path);
#else
	return filefind(font, &search_pkgf, ret_path);
#endif
}


#ifdef	MAKEPK

#ifndef	MAKEPKCMD
#define	MAKEPKCMD	"MakeTeXPK"
#endif
#define	NOBUILD		29999
#define	MKPKSIZE 	256

/*
 *	xdvisystem - same as system(), but read from a file descriptor.
 */

static	FILE *
makefont(font, dpi, name, magstepval)
	_Xconst char	*font;
	int		dpi;
	_Xconst char	**name;
	int		magstepval;
{
	int			pipefds[2];
	char			mkpk[MKPKSIZE];
	Boolean			used_fontname = False;
	Boolean			used_mfmode = False;
	int			redirect_to = 1;
	_Xconst	char		*p;
	char			*q;
	static	_Xconst	char	*argv[]	= {"/bin/sh", "-c", NULL, NULL};
	pid_t			pid;
	int			status;
	FILE			*f;
	int			pos;

	if (xpipe(pipefds) != 0) {	/* create the pipe */
	    perror("[xdvi] pipe");
	    return NULL;
	}

	/*
	 * Generate the MakeTeXPK command line.
	 */

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
		case 'r':
		    Strcpy(q, "'>&3'");
		    redirect_to = 3;
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
#ifdef	MKPK_REDIRECT
		    p = " %n %d %b %m %o '' %r";
#else
		    p = " %n %d %b %m";
#endif
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

	/*
	 * Create a child process.
	 */

	argv[2] = mkpk;
	Printf("- %s\n", mkpk);

	Fflush(stderr);		/* avoid double buffering */
	pid = vfork();
	if (pid == 0) {		/* if child */
	    (void) close(pipefds[0]);
	    if (redirect_to != pipefds[1]) {
		(void) dup2(pipefds[1], redirect_to);
		(void) close(pipefds[1]);
	    }
	    (void) execvp(argv[0], (char **) argv);
	    Fputs("Execvp of /bin/sh failed.\n", stderr);
	    Fflush(stderr);
	    _exit(1);
	}

	(void) close(pipefds[1]);

	if (pid == -1) {
	    (void) close(pipefds[0]);
	    perror("[xdvi] fork");
	    return NULL;
	}

	/*
	 * Now wait until the process terminates, reading whatever it writes
	 * to the pipe.  An eof on the pipe assumes that the child terminated.
	 */

	pos = 0;
	for (;;) {		/* read from pipe until EOF */
	    int bytes;

	    if (pos >= ffline_len)
		expandline(ffline_len);

	    bytes = read(pipefds[0], ffline + pos, ffline_len - pos);
	    if (bytes == -1) continue;
	    if (bytes == 0) break;
	    pos += bytes;
	}

	(void) close(pipefds[0]);

	for (;;) {		/* get termination status */
#if	X_NOT_POSIX
#if	!BSD
	    int retval;

	    retval = wait(&status);
	    if (retval == pid) break;
	    if (retval != -1) continue;
#else	/* BSD */
	    if (wait4(pid, &status, 0, (struct rusage *) NULL) != -1) break;
#endif	/* BSD */
#else	/* POSIX */
	    if (waitpid(pid, &status, 0) != -1) break;
#endif	/* POSIX */
	    if (errno == EINTR) continue;
	    perror("[xdvi] waitpid");
	    return NULL;
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != EX_OK) {
	    Fprintf(stderr, "%s failed.\n", makepkcmd);
	    return NULL;
	}

	if (pos != 0 && ffline[pos - 1] == '\n')	/* trim off last \n */
	    --pos;

		/* if no response, then it probably failed, but look anyway */
	if (pos == 0) {
	    if (debug & DBG_OPEN)
		Printf("No response from %s\n", makepkcmd);
	    return try_size(font, dpi, name);
	}

	if (pos >= ffline_len)
	    expandline(pos);
	ffline[pos++] = '\0';

	if (debug & DBG_OPEN)
	    Printf("%s ---> %s\n", makepkcmd, ffline);
	f = xfopen(ffline, OPEN_MODE);
	if (f == NULL) {
	    perror(ffline);
	    return NULL;
	}

	if (debug & DBG_OPEN)
	    puts("--Success--\n");
	*name = newstring(ffline, pos);
	return f;
}

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
	double		fdpi;
	int		*dpi_ret;
	_Xconst char	**name;
#ifdef	MAKEPK
	int	magstepval;
#endif
{
	FILE	*f;
	int	*p1, *p2;
	int	dpi	= fdpi + 0.5;
	int	tempdpi;

	/*
	 * Loop over sizes.  Try actual size first, then closest sizes.
	 * If the pathname is absolutely or explicitly relative, don't
	 * use the usual paths to search for it; just look for it in the
	 * directory specified.
	 */

	f = try_size(font, *dpi_ret = dpi, name);
	if (f != NULL)
	    return f;

	/* Try at one away from the size we just tried, to account
	   for rounding error.  */
	tempdpi = dpi + (dpi < fdpi ? 1 : -1);
	f = try_size(font, tempdpi, name);
	if (f != NULL) {
	    *dpi_ret = tempdpi;
	    return f;
	}

	/* Try a virtual font. */
	f = filefind(font, &search_vf, name);
	if (f != NULL)
	    return f;

#ifdef	MAKEPK
	/* Try to create the font. */
	if (magstepval != NOBUILD && resource.makepk) {
	    f = makefont(font, dpi, name, magstepval);
	    if (f != NULL)
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
	    f = try_size(font, *dpi_ret = tempdpi, name);
	    if (f != NULL)
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
		    *font_ret = newstring(fn, -1);
		    return f;
		}
	    }
	}
	if (alt_font != NULL) {
	    f = PRE_FONT_OPEN(alt_font, dpi, dpi_ret, name, magstepval);
	    if (f != NULL)
		*font_ret = newstring(alt_font, -1);
	}
	return f;
}
