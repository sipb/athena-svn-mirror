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
 */

#ifndef	lint
static	char	copyright[] =
"@(#) Copyright (c) 1994 Paul Vojta.  All rights reserved.\n";
#endif

#include "config.h"
#include <errno.h>
#include <ctype.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <setjmp.h>

#ifdef	POSIX_DIRENT
#include <dirent.h>
typedef	struct dirent	struct_dirent;
#else	/* !POSIX */
#include <sys/dir.h>
typedef	struct direct	struct_dirent;
#endif

#ifndef	GS
#define	GS	"gs"
#endif

#ifndef	atof
double	atof();
#endif
char	*getenv();

#ifdef	__GNUC__
#define	NORETURN	volatile
#else
#define	NORETURN	/* nothing */
#endif

#ifdef	I_STDARG
#define	NeedVarargsPrototypes	1
#include <stdarg.h>
#else
#define	NeedVarargsPrototypes	0
#include <varargs.h>
#endif

typedef	char	Boolean;
#define	True	1
#define	False	0

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	256
#endif

#define	PK_PRE	(char) 247
#define	PK_ID	89
#define	PK_POST	(char) 245
#define	PK_NOP	(char) 246

char	ident[]	= "gsftopk version 1.9";

typedef	unsigned char	byte;

FILE		*data_file;
FILE		*pk_file;
Boolean		quiet		= False;
int		col		= 0;		/* current column number */

/*
 *	Information from the .tfm file.
 */

int		tfm_lengths[12];
#define	lh	tfm_lengths[1]
#define	bc	tfm_lengths[2]
#define	ec	tfm_lengths[3]
#define	nw	tfm_lengths[4]

long		checksum;
long		design;
byte		width_index[256];
long		tfm_widths[256];

/*
 *	Information on the bitmap currently being worked on.
 */

byte		*bitmap;
int		width;
int		skip;
int		height;
int		hoff;
int		voff;
int		bytes_wide;
unsigned int	bm_size;
byte		*bitmap_end;
int		pk_len;

/*
 *	Print error message and quit.
 */

#if	NeedVarargsPrototypes
NORETURN void
oops(const char *message, ...)
#else
/* VARARGS */
NORETURN void
oops(va_alist)
	va_dcl
#endif
{
#if	!NeedVarargsPrototypes
	const char *message;
#endif
	va_list	args;

#if	NeedVarargsPrototypes
	va_start(args, message);
#else
	va_start(args);
	message = va_arg(args, const char *);
#endif
	vfprintf(stderr, message, args);
	va_end(args);
	putc('\n', stderr);
	exit(1);
}

/*
 *	Either allocate storage or fail with explanation.
 */

char *
xmalloc(size, why)
	unsigned	size;
	const char	*why;
{
	char *mem = (char *) malloc(size);

	if (mem == NULL)
	    oops("Cannot allocate %u bytes for %s.\n", size, why);
	return mem;
}

/*
 *	Either reallocate storage or fail with explanation.
 */

char *
xrealloc(oldp, size, why)
	char		*oldp;
	unsigned	size;
	const char	*why;
{
	char	*mem;

	mem = oldp == NULL ? (char *) malloc(size)
	    : (char *) realloc(oldp, size);
	if (mem == NULL)
	    oops("Cannot reallocate %u bytes for %s.\n", size, why);
	return mem;
}

/*
 *	Here's the patch searching stuff.  First the typedefs and variables.
 */

static	char	searchpath[MAXPATHLEN + 1];

#define	HUNKSIZE	(MAXPATHLEN + 2)

struct spacenode {	/* used for storage of directory names */
	struct spacenode	*next;
	char			*sp_end;	/* end of data for this chunk */
	char			sp[HUNKSIZE];
}
	firstnode;

static	jmp_buf	found_env;
static	FILE	*searchfile;
static	char	*searchname;
static	int	searchnamelen;

static	char *
find_dbl_slash(char *sp_bgn, char *sp_end)
{
	char *p;

	for (;;) {
	    p = memchr(sp_bgn, '/', sp_end - sp_bgn);
	    if (p == NULL) return sp_end;
	    if (p[1] == '/') return p;
	    sp_bgn = p + 1;
	}
}

static	void
main_search_proc(char *matpos, char *sp_pos, char *sp_slash, char *sp_end,
	Boolean skip_subdirs, struct spacenode *space, char *spacenext)
{
	char		*mp;
	struct stat	statbuf;
	DIR		*dir;
	struct_dirent	*entry;
	int		lenleft;
	int		len;
	struct spacenode *space1;
	char		*spacenext1;

	mp = matpos + (sp_slash - sp_pos);
	/* check length */
	if (mp + searchnamelen >= searchpath + sizeof(searchpath) - 2) return;
	memcpy(matpos, sp_pos, sp_slash - sp_pos);
	if (sp_slash == sp_end) {	/* try for a file */
	    *mp = '/';
	    strcpy(mp + (mp == searchpath || mp[-1] != '/'), searchname);
	    searchfile = fopen(searchpath, "r");
	    if (searchfile != NULL) longjmp(found_env, True);
	}
	else {/* try for a subdirectory */
	    *mp = '\0';
	    if (stat(searchpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
		*mp++ = '/';
		sp_slash += 2;
		main_search_proc(mp, sp_slash, find_dbl_slash(sp_slash, sp_end),
		    sp_end, statbuf.st_nlink <= 2, space, spacenext);
		return;
	    }
	}
	if (skip_subdirs) return;
	*matpos = '\0';
	dir = opendir(searchpath);
	if (dir == NULL) return;
	lenleft = searchpath + sizeof(searchpath) - matpos;
	space1 = space;
	spacenext1 = spacenext;
	for (;;) {
	    entry = readdir(dir);
	    if (entry == NULL) break;
	    len = strlen(entry->d_name) + 1;
	    if (len > lenleft) continue;	/* too long */
	    strcpy(matpos, entry->d_name);
	    if (*matpos == '.' && (matpos[1] == '\0' || (matpos[1] == '.'
		    && matpos[2] == '\0')))
		continue;		/* ignore . and .. */
	    if (stat(searchpath, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode))
		continue;		/* if not a directory */
	    if (statbuf.st_nlink > 2) ++len;
	    if (spacenext1 + len > space1->sp + HUNKSIZE) {
		space1->sp_end = spacenext1;
		if (space1->next == NULL) {
		    space1->next =
			(struct spacenode *) xmalloc(sizeof(struct spacenode),
			    "space for directory names");
		    space1->next->next = NULL;
		}
		space1 = space1->next;
		spacenext1 = space1->sp;
	    }
	    if (statbuf.st_nlink > 2) {
		*spacenext1++ = '/';
		--len;
	    }
	    memcpy(spacenext1, entry->d_name, len);
	    spacenext1 += len;
	}
	closedir(dir);
	for (;;) {
	    space1->sp_end = spacenext1;
	    if (spacenext == space->sp_end) {
		if (space == space1) break;
		space = space->next;
		spacenext = space->sp;
	    }
	    skip_subdirs = True;
	    if (*spacenext == '/') {
		++spacenext;
		skip_subdirs = False;
	    }
	    len = strlen(spacenext);
	    memcpy(matpos, spacenext, len);
	    matpos[len] = '/';
	    main_search_proc(matpos + len + 1, sp_pos, sp_slash, sp_end,
		skip_subdirs, space1, spacenext1);
	    spacenext += len + 1;
	}
}

static	FILE *
search(char *path, char *path_var, char *name)
{
	char	*env_path	= NULL;
	FILE	*f;

	if (path_var != NULL) {
	    if (*name == '/') {
		strcpy(searchpath, name);
		return fopen(searchpath, "r");
	    }
	    env_path = getenv(path_var);
	}
	if (env_path == NULL) {
	    env_path = path;
	    path = NULL;
	}
	searchname = name;
	searchnamelen = strlen(name);
	for (;;) {
	    char *p;

	    p = strchr(env_path, ':');
	    if (p == NULL) p = env_path + strlen(env_path);
	    if (p == env_path) {
		if (path != NULL) {
		    f = search(path, NULL, name);
		    if (f != NULL) return f;
		}
	    }
	    else {
		if (setjmp(found_env))
		    return searchfile;
		main_search_proc(searchpath,
		    env_path, find_dbl_slash(env_path, p), p,
		    True, &firstnode, firstnode.sp);
	    }
	    if (*p == '\0') return NULL;
	    env_path = p + 1;
	}
}

/*
 *	Add to dlstring
 */

char		*dlstring	= NULL;
unsigned int	dls_len		= 0;
unsigned int	dls_max		= 0;

void
addtodls(char *s)
{
	int	len	= strlen(s);

	if (dls_len + len >= dls_max) {
	    unsigned int newsize = dls_max + 80;

	    if (newsize <= dls_len + len) newsize = dls_len + len + 1;
	    dlstring = xrealloc(dlstring, dls_max = newsize, "download string");
	}
	strcpy(dlstring + dls_len, s);
	dls_len += len;
}



long
getlong(FILE *f)
{
	int	value;

	value = (int) ((byte) getc(f)) << 24;
	value |= (int) ((byte) getc(f)) << 16;
	value |= (int) ((byte) getc(f)) << 8;
	value |= (int) ((byte) getc(f));
	return value;
}


char	line[82];

void
expect(char *waitingfor)
{
	for (;;) {
	    if (fgets(line, sizeof(line), data_file) == NULL)
		oops("Premature end of file");
	    if (memcmp(line, waitingfor, strlen(waitingfor)) == 0) return;
	    fputs("gs: ", stdout);
	    for (;;) {
		fputs(line, stdout);
		if (*line == '\0' || line[strlen(line) - 1] == '\n') break;
		if (fgets(line, sizeof(line), data_file) == NULL)
		    oops("Premature end of file");
	    }
	}
}

void
whitespace(void)
{
	char	c;

	for (;;) {
	    c = getc(data_file);
	    if (c == '#')
		do c = getc(data_file); while (!feof(data_file) && c != '\n');
	    else if (!isspace(c)) {
		ungetc(c, data_file);
		break;
	    }
	}
}

int
getint(void)
{
	char	c;
	int	i	= 0;

	do c = getc(data_file); while (isspace(c));
	if (c < '0' || c > '9') oops("digit expected");
	do {
	    i = i * 10 + (c - '0');
	    c = getc(data_file);
	} while (c >= '0' && c <= '9');
	if (!feof(data_file)) ungetc(c, data_file);
	return i;
}

static	byte	masks[]	= {0, 1, 3, 7, 017, 037, 077, 0177, 0377};

byte	flag;
int	pk_dyn_f;
int	pk_dyn_g;
int	base;		/* cost of this character if pk_dyn_f = 0 */
int	deltas[13];	/* cost of increasing pk_dyn_f from i to i+1 */

/*
 *	Add up statistics for putting out the given shift count
 */

static	void
tallyup(int n)
{
	int	m;

	if (n > 208) {
	    ++base;
	    n -= 192;
	    for (m = 0x100; m != 0 && m < n; m <<= 4) base += 2;
	    if (m != 0 && (m = (m - n) / 15) < 13) deltas[m] += 2;
	}
	else if (n > 13) ++deltas[(208 - n) / 15];
	else --deltas[n - 1];
}

/*
 *	Routines for storing the shift counts
 */

static	Boolean	odd	= False;
static	byte	part;

static	void
pk_put_nyb(int n)
{
	if (odd) {
	    *bitmap_end++ = (part << 4) | n;
	    odd = False;
	}
	else {
	    part = n;
	    odd = True;
	}
}

static	void
pk_put_long(int n)
{
	if (n >= 16) {
	    pk_put_nyb(0);
	    pk_put_long(n / 16);
	}
	pk_put_nyb(n % 16);
}

static	void
pk_put_count(int n)
{
	if (n > pk_dyn_f) {
	    if (n > pk_dyn_g)
		pk_put_long(n - pk_dyn_g + 15);
	    else {
		pk_put_nyb(pk_dyn_f + (n - pk_dyn_f + 15) / 16);
		pk_put_nyb((n - pk_dyn_f - 1) % 16);
	    }
	}
	else pk_put_nyb(n);
}

static	void
trim_bitmap(void)
{
	byte	*p;
	byte	mask;

	/* clear out garbage bits in bitmap */
	if (width % 8 != 0) {
	    mask = ~masks[8 - width % 8];
	    for (p = bitmap + bytes_wide - 1; p < bitmap_end; p += bytes_wide)
		*p &= mask;
	}

	/*
	 *	Find the bounding box of the bitmap.
	 */

	/* trim top */
	skip = 0;
	mask = 0;
	for (;;) {
	    if (bitmap >= bitmap_end) {	/* if bitmap is empty */
		width = height = hoff = voff = 0;
		return;
	    }
	    p = bitmap + bytes_wide;
	    while (p > bitmap) mask |= *--p;
	    if (mask) break;
	    ++skip;
	    bitmap += bytes_wide;
	}
	height -= skip;
	voff -= skip;
#ifdef	DEBUG
	if (skip < 2 || skip > 3)
	    printf("Character has %d empty rows at top\n", skip);
#endif

	/* trim bottom */
	skip = 0;
	mask = 0;
	for (;;) {
	    p = bitmap_end - bytes_wide;
	    while (p < bitmap_end) mask |= *p++;
	    if (mask) break;
	    ++skip;
	    bitmap_end -= bytes_wide;
	}
	height -= skip;
#ifdef	DEBUG
	if (skip < 2 || skip > 3)
	    printf("Character has %d empty rows at bottom\n", skip);
#endif

	/* trim right */
	skip = 0;
	--width;
	for (;;) {
	    mask = 0;
	    for (p = bitmap + width / 8; p < bitmap_end; p += bytes_wide)
		mask |= *p;
	    if (mask & (0x80 >> (width % 8))) break;
	    --width;
	    ++skip;
	}
	++width;
#ifdef	DEBUG
	if (skip < 2 || skip > 3)
	    printf("Character has %d empty columns at right\n", skip);
#endif

	/* trim left */
	skip = 0;
	for (;;) {
	    mask = 0;
	    for (p = bitmap + skip / 8; p < bitmap_end; p += bytes_wide)
		mask |= *p;
	    if (mask & (0x80 >> (skip % 8))) break;
	    ++skip;
	}
	width -= skip;
	hoff -= skip;
#ifdef	DEBUG
	if (skip < 2 || skip > 3)
	    printf("Character has %d empty columns at left\n", skip);
#endif
	bitmap += skip / 8;
	skip = skip % 8;
}

/*
 *	Pack the bitmap using the rll method.  (Return false if it's better
 *	to just pack the bits.)
 */

static	Boolean
pk_rll_cvt(void)
{
	static	int	*counts		= NULL;	/* area for saving bit counts */
	static	int	maxcounts	= 0;	/* size of this area */
	unsigned int	ncounts;		/* max to allow this time */
	int	*nextcount;			/* next count value */
	int	*counts_end;			/* pointer to end */
	byte	*rowptr;
	byte	*p;
	byte	mask;
	byte	*rowdup;			/* last row checked for dup */
	byte	paint_switch;			/* 0 or 0xff */
	int	bits_left;			/* bits left in row */
	int	cost;
	int	i;

	/*
	 *	Allocate space for bit counts.
	 */

	ncounts = (width * height + 3) / 4;
	if (ncounts > maxcounts) {
	    if (counts != NULL) free(counts);
	    counts = (int *) xmalloc((ncounts + 2) * sizeof(int),
		"array for bit counts");
	    maxcounts = ncounts;
	}
	counts_end = counts + ncounts;

	/*
	 *	Form bit counts and collect statistics
	 */
	base = 0;
	bzero(deltas, sizeof(deltas));
	rowdup = NULL;	/* last row checked for duplicates */
	p = rowptr = bitmap;
	mask = 0x80 >> skip;
	flag = 0;
	paint_switch = 0;
	if (*p & mask) {
	    flag = 8;
	    paint_switch = 0xff;
	}
	bits_left = width;
	nextcount = counts;
	while (rowptr < bitmap_end) {	/* loop over shift counts */
	    int shift_count = bits_left;

	    for (;;) {
		if (bits_left == 0) {
		    if ((p = rowptr += bytes_wide) >= bitmap_end) break;
		    mask = 0x80 >> skip;
		    bits_left = width;
		    shift_count += width;
		}
		if (((*p ^ paint_switch) & mask) != 0) break;
		--bits_left;
		mask >>= 1;
		if (mask == 0) {
		    ++p;
		    while (*p == paint_switch && bits_left >= 8) {
			++p;
			bits_left -= 8;
		    }
		    mask = 0x80;
		}
	    }
	    if (nextcount >= counts_end) return False;
	    shift_count -= bits_left;
	    *nextcount++ = shift_count;
	    tallyup(shift_count);
	    /* check for duplicate rows */
	    if (rowptr != rowdup && bits_left != width) {
		byte	*p1	= rowptr;
		byte	*q	= rowptr + bytes_wide;
		int	repeat_count;

		while (q < bitmap_end && *p1 == *q) ++p1, ++q;
		repeat_count = (p1 - rowptr) / bytes_wide;
		if (repeat_count > 0) {
		    *nextcount++ = -repeat_count;
		    if (repeat_count == 1) --base;
		    else {
			++base;
			tallyup(repeat_count);
		    }
		    rowptr += repeat_count * bytes_wide;
		}
		rowdup = rowptr;
	    }
	    paint_switch = ~paint_switch;
	}

#ifdef	DEBUG
	/*
	 *	Dump the bitmap
	 */

	for (p = bitmap; p < bitmap_end; p += bytes_wide) {
	    byte *p1	= p;
	    int j;

	    mask = 0x80 >> skip;
	    for (j = 0; j < width; ++j) {
		putchar(*p1 & mask ? '@' : '.');
		if ((mask >>= 1) == 0) mask = 0x80, ++p1;
	    }
	    putchar('\n');
	}
	putchar('\n');
#endif

	/*
	 *	Determine the best pk_dyn_f
	 */

	pk_dyn_f = 0;
	cost = base += 2 * (nextcount - counts);
	for (i = 1; i < 14; ++i) {
	    base += deltas[i - 1];
	    if (base < cost) {
		pk_dyn_f = i;
		cost = base;
	    }
	}
	/* last chance to bail out */
	if (cost * 4 > width * height) return False;

	/*
	 *	Pack the bit counts
	 */

	pk_dyn_g = 208 - 15 * pk_dyn_f;
	flag |= pk_dyn_f << 4;
	bitmap_end = bitmap;
	*nextcount = 0;
	nextcount = counts;
	while (*nextcount != 0) {
	    if (*nextcount > 0) pk_put_count(*nextcount);
	    else
		if (*nextcount == -1) pk_put_nyb(15);
		else {
		    pk_put_nyb(14);
		    pk_put_count(-*nextcount);
		}
	    ++nextcount;
	}
	if (odd) {
	    pk_put_nyb(0);
	    ++cost;
	}
	if (cost != 2 * (bitmap_end - bitmap))
	    printf("Cost miscalculation:  expected %d, got %d\n", cost,
		2 * (bitmap_end - bitmap));
	pk_len = bitmap_end - bitmap;
	return True;
}

static	void
pk_bm_cvt(void)
{
	byte	*rowptr;
	byte	*p;
	int	blib1;		/* bits left in byte */
	int	bits_left;	/* bits left in row */
	byte	*q;
	int	blib2;
	byte	nextbyte;

	flag = 14 << 4;
	q = bitmap;
	blib2 = 8;
	nextbyte = 0;
	for (rowptr = bitmap; rowptr < bitmap_end; rowptr += bytes_wide) {
	    p = rowptr;
	    blib1 = 8 - skip;
	    bits_left = width;
	    if (blib2 != 8) {
		int	n;

		if (blib1 < blib2) {
		    nextbyte |= *p << (blib2 - blib1);
		    n = blib1;
		}
		else {
		    nextbyte |= *p >> (blib1 - blib2);
		    n = blib2;
		}
		blib2 -= n;
		if ((bits_left -= n) < 0) {
		    blib2 -= bits_left;
		    continue;
		}
		if ((blib1 -= n) == 0) {
		    blib1 = 8;
		    ++p;
		    if (blib2 > 0) {
			nextbyte |= *p >> (8 - blib2);
			blib1 -= blib2;
			bits_left -= blib2;
			if (bits_left < 0) {
			    blib2 = -bits_left;
			    continue;
			}
		    }
		}
		*q++ = nextbyte;
	    }
	    /* fill up whole (destination) bytes */
	    while (bits_left >= 8) {
		nextbyte = *p++ << (8 - blib1);
		*q++ = nextbyte | (*p >> blib1);
		bits_left -= 8;
	    }
	    /* now do the remainder */
	    nextbyte = *p << (8 - blib1);
	    if (bits_left > blib1) nextbyte |= p[1] >> blib1;
	    blib2 = 8 - bits_left;
	}
	if (blib2 != 8) *q++ = nextbyte;
	pk_len = q - bitmap;
}

static	void
putshort(short w)
{
	putc(w >> 8, pk_file);
	putc(w, pk_file);
}

static	void
putmed(long w)
{
	putc(w >> 16, pk_file);
	putc(w >> 8, pk_file);
	putc(w, pk_file);
}

static	void
putlong(long w)
{
	putc(w >> 24, pk_file);
	putc(w >> 16, pk_file);
	putc(w >> 8, pk_file);
	putc(w, pk_file);
}

static	void
putglyph(int cc)
{
	static	Boolean	have_first_line = False;
	static	int	llx, lly, urx, ury;
	static	float	char_width;
	static	byte	*area1	= NULL;
	static unsigned int size1 = 0;
	long	dm;
	long	tfm_wid;
	byte	*p;
	int	i;

	if (!quiet) {
	    int wid;
	    static char *s = "";

	    wid = (cc >= 100) + (cc >= 10) + 4;
	    if (col + wid > 80) {
		s = "\n";
		col = 0;
	    }
	    printf("%s[%d", s, cc);
	    fflush(stdout);
	    col += wid;
	    s = " ";
	}
	if (!have_first_line) {
	    expect("#^");
	    if (sscanf(line, "#^ %d %d %d %d %d %f\n", &i,
		    &llx, &lly, &urx, &ury, &char_width) != 6)
		oops("Cannot scanf first line");
	}
	if (i < cc) oops("Character %d received, %d expected", i, cc);
	if (i > cc) {
	    fprintf(stderr, "Character %d is missing.\n", cc);
	    have_first_line = True;
	    return;
	}
	have_first_line = False;
	hoff = -llx + 2;
	voff = ury + 2 - 1;
	expect("P4\n");
	whitespace();
	width = getint();
	whitespace();
	height = getint();
	(void) getc(data_file);
	if (width != urx - llx + 4 || height != ury - lly + 4)
	    oops("Dimensions do not match:  %d %d %d %d %d %d",
		llx, lly, urx, ury, width, height);
	bytes_wide = (width + 7) / 8;
	bm_size = bytes_wide * height;
	if (size1 < bm_size) {
	    if (area1 != NULL) free(area1);
	    area1 = (byte *) xmalloc(bm_size, "original bitmap");
	    size1 = bm_size;
	}
	for (p = area1 + (height - 1) * bytes_wide; p >= area1; p -= bytes_wide)
	    if (fread(p, 1, bytes_wide, data_file) != bytes_wide)
		oops("Cannot read bitmap of size %u", bm_size);
	bitmap = area1;
	bitmap_end = bitmap + bm_size;
	trim_bitmap();
	if (height == 0 || !pk_rll_cvt()) pk_bm_cvt();
	tfm_wid = tfm_widths[width_index[cc]];
	dm = (long) (char_width + 0.5) - (char_width < -0.5);
	if (pk_len + 8 < 4 * 256 && tfm_wid < (1<<24) &&
		dm >= 0 && dm < 256 && width < 256 && height < 256 &&
		hoff >= -128 && hoff < 128 && voff >= -128 && voff < 128) {
	    putc(flag | ((pk_len + 8) >> 8), pk_file);
	    putc(pk_len + 8, pk_file);
	    putc(cc, pk_file);
	    putmed(tfm_wid);
	    putc(dm, pk_file);
	    putc(width, pk_file);
	    putc(height, pk_file);
	    putc(hoff, pk_file);
	    putc(voff, pk_file);
	} else
	if (pk_len + 13 < 3 * 65536L && tfm_wid < (1<<24) &&
		dm >= 0 && dm < 65536L && width < 65536L && height < 65536L &&
		hoff >= -65536L && hoff < 65536L &&
		voff >= -65536L && voff < 65536L) {
	    putc(flag | 4 | ((pk_len + 13) >> 16), pk_file);
	    putshort(pk_len + 13);
	    putc(cc, pk_file);
	    putmed(tfm_wid);
	    putshort(dm);
	    putshort(width);
	    putshort(height);
	    putshort(hoff);
	    putshort(voff);
	}
	else {
	    putc(flag | 7, pk_file);
	    putlong(pk_len + 28);
	    putlong(cc);
	    putlong(tfm_wid);
	    putlong((long) (char_width * 65536.0 + 0.5) - (char_width < -0.5));
	    putlong(0);
	    putlong(width);
	    putlong(height);
	    putlong(hoff);
	    putlong(voff);
	}
	fwrite(bitmap, 1, pk_len, pk_file);
	if (!quiet) {
	    putchar(']');
	    fflush(stdout);
	}
}

int
main(int argc, char **argv)
{
	FILE	*config_file;
	FILE	*tfm_file;
	float	dpi;
	char	*fontname;
	int	fontlen;
	char	*configline;
	unsigned int	cflinelen;
	char	*p;
	char	*PSname		= NULL;
	char	*specinfo	= "";
	char	*xfilename;
	char	charlist[10*2 + 90*3 + 156*4 + 1];
	char	designstr[20];
	char	dpistr[20];
	int	pid;
	int	std_in[2];
	int	std_out[2];
	int	status;
	int	cc;
	int	ppp;
	int	i;

	if (argc > 1 && strcmp(argv[1], "-q") == 0) {
	    ++argv;
	    --argc;
	    quiet = True;
	}

	if (argc != 3 || (dpi = atof(argv[2])) <= 0.0) {
	    fputs("Usage: gsftopk [-q] <font> <dpi>\n", stderr);
	    exit(1);
	}
	fontname = argv[1];
	fontlen = strlen(fontname);

	if (!quiet) puts(ident);

	config_file = search(CONFIGPATH, "TEXCONFIG", "psfonts.map");
	if (config_file == NULL) oops("Cannot find file psfonts.map.");

	configline = (char *) xmalloc(cflinelen = 80, "Config file line");
	do {
	    int len	= 0;

	    if (fgets(configline, cflinelen, config_file) == NULL)
		oops("Cannot find font %s in config file.", fontname);
	    for (;;) {
		i = strlen(configline + len);

		len += i;
		if (len > 0 && configline[len - 1] == '\n') {
		    configline[--len] = '\0';
		    break;
		}
		if (len < cflinelen - 1) break;
		configline = xrealloc(configline, cflinelen += 80,
		    "config file line");
		fgets(configline + len, cflinelen - len, config_file);
	    }
	}
	while (memcmp(configline, fontname, fontlen) != 0
	    || (configline[fontlen] != '\0' && !isspace(configline[fontlen])));
	fclose(config_file);

	/*
	 * Parse the line from the config file.
	 */
	for (p = configline + fontlen; *p != '\0'; ++p) {
	    if (isspace(*p)) continue;
	    if (*p == '<') {
		char	*q	= ++p;
		char	endc;
		FILE	*f;

		addtodls(" (");
		while (*p != '\0' && !isspace(*p)) ++p;
		endc = *p;
		*p = '\0';
		f = search(HEADERPATH, "DVIPSHEADERS", q);
		if (f == NULL) oops("Cannot find font file %s", q);
		/* search() also sets searchpath */
		addtodls(searchpath);
		addtodls((char) getc(f) == '\200' ? ") brun" : ") run");
		fclose(f);
		if (endc == '\0') break;
		continue;
	    }
	    else if (*p == '"') {
		char	*q;

		specinfo = ++p;
		q = strchr(p, '"');
		if (q == NULL) break;
		p = q;
	    }
	    else {
		PSname = p;
		while (*p != '\0' && !isspace(*p)) ++p;
		if (*p == '\0') break;
	    }
	    *p = '\0';
	}

#ifdef	OLD_DVIPS
	/* Parse lines like `Symbol-Slanted "/Symbol .167 SlantFont"'. */
	if (*(p = specinfo) == '/') {
	    PSname = ++p;
	    while (*p && !isspace(*p)) ++p;
	    if (*p) *p++ = '\0';
	    specinfo = p;
	}
#endif	/* OLD_DVIPS */

	/*
	 *	Start up GhostScript.
	 */

	tfm_file = search(HEADERPATH, "DVIPSHEADERS", "render.ps");
	if (tfm_file == NULL)
	    oops("Cannot find PS driver file \"render.ps\".");
	fclose(tfm_file);

	sprintf(dpistr, "%f", dpi);

	if (pipe(std_in) != 0 || pipe(std_out) != 0) {
	    perror("pipe");
	    return 1;
	}

	fflush(stderr);		/* to avoid double flushing */
	pid = vfork();
	if (pid == 0) {
	    close(std_in[1]);
	    dup2(std_in[0], 0);
	    close(std_in[0]);
	    close(std_out[0]);
	    dup2(std_out[1], 1);
	    close(std_out[1]);
	    execlp(GS, "gs", "-DNODISPLAY", "-q", "--",
		/* render.ps */ searchpath,
		PSname != NULL ? PSname : fontname,
		dlstring != NULL ? dlstring : "", specinfo, dpistr, NULL);
	    perror("gs");
	    exit(1);
	}
	if (pid == -1) {
	    perror("fork");
	    exit(1);
	}

	/*
	 *	Open and read the tfm file.  If this takes a while, at least
	 *	it can overlap with the startup of GhostScript.
	 */

	fontlen = strlen(fontname);
	xfilename = xmalloc(fontlen + 10, "name of tfm/pk file");
	strcpy(xfilename, fontname);
	strcpy(xfilename + fontlen, ".tfm");
	tfm_file = search(TEXFONTS_DEFAULT, "TEXFONTS", xfilename);
	if (tfm_file == NULL) oops("Cannot find tfm file.");
	for (i = 0; i < 12; ++i) {
	    int j;

	    j = (int) ((byte) getc(tfm_file)) << 8;
	    tfm_lengths[i] = j | (int) ((byte) getc(tfm_file));
	}
	checksum = getlong(tfm_file);
	design = getlong(tfm_file);
	fseek(tfm_file, 4 * (lh + 6), 0);
	p = charlist;
	for (cc = bc; cc <= ec; ++cc) {
	    width_index[cc] = (byte) getc(tfm_file);
	    if (width_index[cc] != 0) {
		sprintf(p, "%d ", cc);
		p += strlen(p);
	    }
	    (void) getc(tfm_file);
	    (void) getc(tfm_file);
	    (void) getc(tfm_file);
	}
	for (i = 0; i < nw; ++i) tfm_widths[i] = getlong(tfm_file);
	fclose(tfm_file);
	p[-1] = '\n';

	/* write the design size and character list to the file */
	sprintf(designstr, "%f\n", (float) design / (1 << 20));
	write(std_in[1], designstr, strlen(designstr));
	write(std_in[1], charlist, p - charlist);
	close(std_in[1]);

/*
 *	Read the output from GhostScript.
 */

	if ((data_file = fdopen(std_out[0], "r")) == NULL) {
	    perror("GS_out");
	    exit(1);
	}

/*
 *	Create pk file and write preamble.
 */

	fflush(stdout);
	sprintf(xfilename + fontlen, ".%dpk", (int) (dpi + 0.5));
	if ((pk_file = fopen(xfilename, "w")) == NULL) {
	    perror(xfilename);
	    exit(1);
	}
	putc(PK_PRE, pk_file);
	putc(PK_ID, pk_file);
	i = strlen(ident);
	putc(i, pk_file);
	fwrite(ident, 1, i, pk_file);
	putlong(design);
	putlong(checksum);
	ppp = dpi / 72.27 * 65536.0 + 0.5;
	putlong(ppp);	/* hppp */
	putlong(ppp);	/* vppp */

/*
 *	Write the actual characters.
 */

	for (cc = bc; cc <= ec; ++cc)
	    if (width_index[cc] != 0)
		putglyph(cc);
	fclose(data_file);

	if (wait(&status) == -1) {
	    perror("wait");
	    exit(1);
	}
	if (status != 0)
	    if (status & 0377)
		oops("Call to gs stopped by signal %d", status & 0177);
	    else oops("Call to gs returned nonzero status %d", status >> 8);

	putc(PK_POST, pk_file);
	while (ftell(pk_file) % 4 != 0) putc(PK_NOP, pk_file);
	fclose(pk_file);
	if (!quiet) putchar('\n');
	return 0;
}
