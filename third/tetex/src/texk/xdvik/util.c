/*========================================================================*\

Copyright (c) 1990-1999  Paul Vojta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
PAUL VOJTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

NOTE:
	xdvi is based on prior work, as noted in the modification history
	in xdvi.c.

\*========================================================================*/

#include "xdvi-config.h"
#include "kpathsea/c-ctype.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-vararg.h"
#include "HTEscape.h"

#include <errno.h>

#ifndef HAVE_MEMICMP
#include <ctype.h>
#else
#include <string.h> /* this might define memicmp() */
#endif

#include <pwd.h>

#include <X11/Xmd.h>	/* get WORD64 and LONG64 */

#if PS
# include <sys/stat.h>
#endif

#ifndef WORD64
# ifdef LONG64
typedef unsigned int xuint32;
# else
typedef unsigned long xuint32;
# endif
#endif

#ifdef VMS
#include <rmsdef.h>
#endif /* VMS */


#ifdef	X_NOT_STDC_ENV
extern int errno;
extern void *malloc();
extern void *realloc();
#endif

#if defined(macII) && !__STDC__	/* stdlib.h doesn't define these */
extern void *malloc();
extern void *realloc();
#endif

#ifdef	DOPRNT	/* define this if vfprintf gives you trouble */
#define	vfprintf(stream, message, args)	_doprnt(message, args, stream)
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef MAXSYMLINKS		/* Workaround for Linux libc 4.x/5.x */
#define MAXSYMLINKS 5
#endif

/*
 *	General utility routines.
 */

static void
exit_clean_windows(void)
{
    unsigned char *window_list;
    size_t window_list_len;
    unsigned char *window_list_end;
    unsigned char *wp;

#ifndef WORD64
# define W(p)	(*((xuint32 *) p))
#else
# if WORDS_BIGENDIAN
#  define W(p)	(((unsigned long) p[0] << 24) | ((unsigned long) p[1] << 16) \
		  | ((unsigned long) p[2] << 8) | (unsigned long) p[3])
# else
#  define W(p)	(((unsigned long) p[3] << 24) | ((unsigned long) p[2] << 16) \
		  | ((unsigned long) p[1] << 8) | (unsigned long) p[0])
# endif
#endif

    window_list_len = property_get_data(DefaultRootWindow(DISP),
					ATOM_XDVI_WINDOWS, &window_list,
					XGetWindowProperty);
    if (window_list_len == 0)
	return;

    if (window_list_len % 4 != 0) {
	XDeleteProperty(DISP, DefaultRootWindow(DISP), ATOM_XDVI_WINDOWS);
	return;
    }

    /* Loop over list of windows.  */

    window_list_end = window_list + window_list_len;
    
    for (wp = window_list; wp < window_list_end; wp += 4) {
	if (debug & DBG_CLIENT) {
	    Window w;
#ifndef WORD64
	    w = *((xuint32 *) wp);
#else
#if WORDS_BIGENDIAN
	    w = ((unsigned long)wp[0] << 24) | ((unsigned long)wp[1] << 16)
		| ((unsigned long)wp[2] << 8) | (unsigned long)wp[3];
#else
	    w = ((unsigned long)wp[3] << 24) | ((unsigned long)wp[2] << 16)
		| ((unsigned long)wp[1] << 8) | (unsigned long)wp[0];
#endif
#endif
	    TRACE_CLIENT((stderr, "Window list pos %d: 0x%x", (unsigned int)(wp - window_list), (unsigned int)w));
	}
	if (W(wp) == XtWindow(top_level)) {
	    TRACE_CLIENT((stderr, "wp == top_level; len: %d, end: %p\n", window_list_len, window_list_end));
	    window_list_len -= 4;
	    window_list_end -= 4;
	    bcopy(wp + 4, wp, window_list_end - wp);

	    if (window_list_len == 0)
		XDeleteProperty(DISP, DefaultRootWindow(DISP),
				ATOM_XDVI_WINDOWS);
	    else
		XChangeProperty(DISP, DefaultRootWindow(DISP),
				ATOM_XDVI_WINDOWS, ATOM_XDVI_WINDOWS, 32,
				PropModeReplace, (unsigned char *)window_list,
				window_list_len / 4);

	    XFlush(DISP);
	    return;
	}
    }
    TRACE_CLIENT((stderr, "Couldn't find window 0x%x in list when exiting.\n",
		  (unsigned int)XtWindow(top_level)));

    return;
}

#undef	W


/*
 *	This routine should be used for all exits, except for really early ones.
 */

NORETURN void
xdvi_exit(int status)
{
    /* Clean up the "xdvi windows" property in the root window.  */
    if (top_level)
	exit_clean_windows();
#if PS
    ps_destroy();
#endif
#ifdef HTEX
    htex_cleanup();
#endif
    exit(status);
}


/*
 *	invoked on SIGSEGV: try to stop gs before aborting, to prevent gs
 *	running on with 100% CPU consumption - this can be annoying during
 *	testing.
 */

NORETURN void
xdvi_abort(void)
{
    /* Clean up the "xdvi windows" property in the root window.  */
    if (top_level)
	exit_clean_windows();
#if PS
    ps_destroy();
#endif
#ifdef HTEX
    htex_cleanup();
#endif
    abort();
}

/*
 *	Print error message and quit.
 */

int lastwwwopen;


NORETURN void
oops(_Xconst char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    fprintf(stderr, "%s: ", prog);

    (void)vfprintf(stderr, fmt, argp);
    va_end(argp);
    putc('\n', stderr);
    xdvi_exit(1);
}

/* realpath implementation if no implementation available.
   Try to canonicalize path, removing `.' and `..' and expanding symlinks.
   Adopted from wu-ftpd's fb_realpath (in realpath.c), but without the
   seteuid(0) stuff (which we don't need, since we never change into
   directories or read files the user has no permissions for).

   resolved should be a buffer of size PATH_MAX.
*/

char *
my_realpath(const char *path, char *resolved)
{
    struct stat sb;
    int n;
    char *base;
    char tmpbuf[MAXPATHLEN];
    int symlinks = 0;
#ifdef HAVE_FCHDIR
    int fd;
#else
    char cwd[MAXPATHLEN];
#endif

    /* Save cwd for going back later */
#ifdef HAVE_FCHDIR
    if ((fd = open(".", O_RDONLY)) < 0)
	return NULL;
#else /* HAVE_FCHDIR */
    if (
# ifdef HAVE_GETCWD
	getcwd(cwd, MAXPATHLEN)
# else
	getwd(cwd)
# endif
	== NULL)
	return NULL;
#endif /* HAVE_FCHDIR */

    if (strlen(path) + 1 > MAXPATHLEN) {
	errno = ENAMETOOLONG;
	return NULL;
    }
    strcpy(resolved, path);

    for (;;) { /* loop for resolving symlinks in base name */
	/* get base name and dir name components */
	char *p = strrchr(resolved, DIR_SEPARATOR);
	if (p != NULL) {
	    base = p + 1;
	    if (p == resolved) {
		/* resolved is in root dir; this must be treated as a special case,
		   since we can't chop off at `/'; instead, just use the `/':
		 */
		p = "/";
	    }
	    else {
		/* not in root dir; chop off path name at slash */
		while (p > resolved && *p == DIR_SEPARATOR) /* for multiple trailing slashes */
		    p--;
		*(p + 1) = '\0';
		p = resolved;
	    }

	    /* change into that dir */
	    if (chdir(p) != 0)
		break;
	}
	else /* no directory component */
	    base = resolved;

	/* resolve symlinks or directory names (not used in our case) in basename */
	if (*base != '\0') {
	    if (
#ifdef HAVE_LSTAT
		lstat(base, &sb)
#else
		stat(base, &sb)
#endif
		== 0) {
#ifdef HAVE_LSTAT
		if (S_ISLNK(sb.st_mode)) { /* if it's a symlink, iterate for what it links to */
		    if (++symlinks > MAXSYMLINKS) {
			errno = ELOOP;
			break;
		    }

		    if ((n = readlink(base, resolved, MAXPATHLEN)) < 0)
			break;

		    resolved[n] = '\0';
		    continue;
		}
#endif /* HAVE_LSTAT */
		if (S_ISDIR(sb.st_mode)) { /* if it's a directory, go there */
		    if (chdir(base) != 0)
			break;

		    base = "";
		}
	    }
	}

	/* Now get full pathname of current directory and concatenate it with saved copy of base name */
	strcpy(tmpbuf, base); /* cannot overrun, since strlen(base) <= strlen(path) < MAXPATHLEN */
	if (
#ifdef HAVE_GETCWD
	    getcwd(resolved, MAXPATHLEN)
#else
	    getwd(resolved)
#endif
	    == NULL)
	    break;

	/* need to append a slash if resolved is not the root dir */
	if (!(resolved[0] == DIR_SEPARATOR && resolved[1] == '\0')) {
	    if (strlen(resolved) + 2 > MAXPATHLEN) {
		errno = ENAMETOOLONG;
		break;
	    }
	    strcat(resolved, "/");
	}

	if (*tmpbuf) {
	    if (strlen(resolved) + strlen(tmpbuf) + 1 > MAXPATHLEN) {
		errno = ENAMETOOLONG;
		break;
	    }
	    strcat(resolved, tmpbuf);
	}

	/* go back to where we came from */
#ifdef HAVE_FCHDIR
	fchdir(fd);
	close(fd);
#else
	chdir(cwd);
#endif
	return resolved;
    }

    /* arrive here in case of error: go back to where we came from, and return NULL */
#ifdef HAVE_FCHDIR
    fchdir(fd);
    close(fd);
#else
    chdir(cwd);
#endif
    return NULL;
}

#ifndef KPATHSEA

/*
 *	Either (re)allocate storage or fail with explanation.
 */

void *
xmalloc(unsigned size)
{
    /* Avoid malloc(0), though it's not clear if it ever actually
       happens any more.  */
    void *mem = malloc(size ? size : 1);

    if (mem == NULL)
	oops("! Out of memory (allocating %u bytes).\n", size);
    return mem;
}

void *
xrealloc(void *where, unsigned size)
{
    void *mem = realloc(where, size);

    if (mem == NULL)
	oops("! Out of memory (reallocating %u bytes).\n", size);
    return mem;
}


/*
 *	Allocate a new string.
 */

char *
xstrdup(_Xconst char *str)
{
    size_t len;
    char *new;

    len = strlen(str) + 1;
    new = xmalloc(len);
    bcopy(str, new, len);
    return new;
}


/*
 *	Allocate a new string.  The second argument is the length.
 */

char *
xmemdup(_Xconst char *str, size_t len)
{
    char *new;

    new = xmalloc(len);
    bcopy(str, new, len);
    return new;
}

#endif /* not KPATHSEA */

/*
 *	Expand the matrix *ffline to at least the given size.
 */

void
expandline(size_t n)
{
    size_t newlen = n + 128;

    ffline = (ffline == NULL) ? xmalloc(newlen) : xrealloc(ffline, newlen);
    ffline_len = newlen;
}


/*
 *	Allocate bitmap for given font and character
 */

void
alloc_bitmap(struct bitmap *bitmap)
{
    unsigned int size;

    /* width must be multiple of <arch-defined> bits for raster_op */
    bitmap->bytes_wide = ROUNDUP((int)bitmap->w, BMBITS) * BMBYTES;
    size = bitmap->bytes_wide * bitmap->h;
    bitmap->bits = xmalloc(size != 0 ? size : 1);
}


#ifndef KPATHSEA

/*
 *	Put a variable in the environment or abort on error.
 */

extern char **environ;

void
xputenv(_Xconst char *var, _Xconst char *value)
{

#if HAVE_PUTENV

    char *buf;
    int len1, len2;

    len1 = strlen(var);
    len2 = strlen(value) + 1;
    buf = xmalloc((unsigned int)len1 + len2 + 1);
    bcopy(var, buf, len1);
    buf[len1++] = '=';
    bcopy(value, buf + len1, len2);
    if (putenv(buf) != 0)
	oops("! Failure in setting environment variable.");
    return;

#elif HAVE_SETENV

    if (setenv(var, value, True) != 0)
	oops("! Failure in setting environment variable.");
    return;

#else /* not HAVE_{PUTENV,SETENV} */

    int len1;
    int len2;
    char *buf;
    char **linep;
    static Boolean did_malloc = False;

    len1 = strlen(var);
    len2 = strlen(value) + 1;
    buf = xmalloc((unsigned int)len1 + len2 + 1);
    bcopy(var, buf, len1);
    buf[len1++] = '=';
    bcopy(value, buf + len1, len2);
    for (linep = environ; *linep != NULL; ++linep)
	if (memcmp(*linep, buf, len1) == 0) {
	    *linep = buf;
	    return;
	}
    len1 = linep - environ;
    if (did_malloc)
	environ = xrealloc(environ, (unsigned int)(len1 + 2) * sizeof(char *));
    else {
	linep = xmalloc((unsigned int)(len1 + 2) * sizeof(char *));
	bcopy((char *)environ, (char *)linep, len1 * sizeof(char *));
	environ = linep;
	did_malloc = True;
    }
    environ[len1++] = buf;
    environ[len1] = NULL;

#endif /* not HAVE_{PUTENV,SETENV} */

}

#endif /* not KPATHSEA */


#ifndef HAVE_MEMICMP
/*
 *	Hopefully a self-explanatory name.  This code assumes the second
 *	argument is lower case.
 */

int
memicmp(_Xconst char *s1, _Xconst char *s2, size_t n)
{
    while (n > 0) {
	int i = tolower(*s1) - *s2;
	if (i != 0)
	    return i;
	++s1;
	++s2;
	--n;
    }
    return 0;
}
#endif

/*
 *	Close the pixel file for the least recently used font.
 */

static void
close_a_file(void)
{
    struct font *fontp;
    unsigned short oldest = ~0;
    struct font *f = NULL;

    if (debug & DBG_OPEN)
	Puts("Calling close_a_file()");

    for (fontp = font_head; fontp != NULL; fontp = fontp->next)
	if (fontp->file != NULL && fontp->timestamp <= oldest) {
	    f = fontp;
	    oldest = fontp->timestamp;
	}
    if (f == NULL)
	oops("Can't find an open pixel file to close");
    Fclose(f->file);
    f->file = NULL;
    ++n_files_left;
}

/*
 *	This is necessary on some systems to work around a bug.
 */

#if SUNOS4
static void
close_small_file(void)
{
    struct font *fontp;
    unsigned short oldest = ~0;
    struct font *f = NULL;

    if (debug & DBG_OPEN)
	Puts("Calling close_small_file()");

    for (fontp = font_head; fontp != NULL; fontp = fontp->next)
	if (fontp->file != NULL && fontp->timestamp <= oldest
	    && (unsigned char)fileno(fontp->file) < 128) {
	    f = fontp;
	    oldest = fontp->timestamp;
	}
    if (f == NULL)
	oops("Can't find an open pixel file to close");
    Fclose(f->file);
    f->file = NULL;
    ++n_files_left;
}
#else
#define	close_small_file	close_a_file
#endif

#ifdef HTEX

/*
 *	Localize a local URL.  This is done in place, but the pointer
 *	returned probably points to somewhere _WITHIN_ the filename.
 *	So the caller needs to use the returned pointer and the returned
 *	pointer is unsuitable for free operations - janl 31/1/1999
 */

char *
urlocalize(char *filename)
{
    if (strncmp(filename, "file:", 5) == 0) {

	if (debug & DBG_HYPER)
	    fprintf(stderr, "Shortcircuiting local file url: %s\n", filename);

	/* If it's a file: URL just remove the file: part, then
	   we'll open it localy as a normal local file */
	filename += 5;
	/* If what we're left with starts in // then we need to remove
	   a hostname part too */
	if (strncmp(filename, "//", 2) == 0)
	    filename = strchr(filename + 2, DIR_SEPARATOR);

	HTUnEscape(filename);

	if (debug & DBG_HYPER)
	    fprintf(stderr, "Local filename is: %s\n", filename);

    }

    return filename;
}

#endif /* HTEX */

/*
 *	Open a file in the given mode.  URL AWARE.
 */

FILE *
xdvi_xfopen
#ifndef	VMS
	(_Xconst char *filename, _Xconst char *type)
#define	TYPE	type
#else
        (_Xconst char *filename, _Xconst char *type, _Xconst char *type2)
#define	TYPE	type, type2
#endif /* VMS */
{
    FILE *f;

    /* Try not to let the file table fill up completely.  */
    if (n_files_left <= 10)
	close_a_file();
#ifdef HTEX

    filename = urlocalize((char *)filename);

    if (URL_aware && (((URLbase != NULL) && htex_is_url(urlocalize(URLbase))) ||
		      (htex_is_url(filename)))) {
	int i;
	i = fetch_relative_url(URLbase, filename);
	if (i < 0)
	    return NULL;
	/* Don't bother waiting right now... */
	wait_for_urls();
	/* This needs to be set somehow... */
	f = fopen(htex_file_at_index(i), TYPE);
	if (debug & DBG_HYPER)
	    fprintf(stderr, "Opening %s for %s\n",
		    htex_file_at_index(i), htex_url_at_index(i));
	lastwwwopen = i;
    }
    else
#endif /* HTEX */
	f = fopen(filename, TYPE);
    /*
     * Interactive Unix 2.2.1 doesn't set errno to EMFILE
     * or ENFILE even when it should, but if we do this
     * unconditionally, then giving a nonexistent file on the
     * command line gets the bizarre error `Can't find an open pixel
     * file to close' instead of `No such file or directory'.
     */
    if (f == NULL &&
#ifndef	VMS
	(errno == EMFILE || errno == ENFILE)
#else /* VMS */
	errno == EVMSERR && vaxc$errno == RMS$_ACC
#endif /* VMS */
	) {
	n_files_left = 0;
	close_a_file();
	f = fopen(filename, TYPE);
    }
#ifdef	F_SETFD
    if (f != NULL)
	(void)fcntl(fileno(f), F_SETFD, 1);
#endif
    return f;
}

#undef	TYPE

/*
 *	Open a file, but temporary disable URL awareness.
 */

#ifdef HTEX
FILE *
xfopen_local(_Xconst char *filename, _Xconst char *type)
{
    FILE *f;
    int url_aware_save;
    url_aware_save = URL_aware;
    URL_aware = FALSE;
    f = xfopen(filename, type);
    URL_aware = url_aware_save;
    return f;
}
#endif /* HTEX */

/*
 *	Create a pipe, closing a file if necessary.
 */

int
xpipe(int *fd)
{
    int retval;

    for (;;) {
	retval = pipe(fd);
	if (retval == 0 || (errno != EMFILE && errno != ENFILE))
	    break;
	n_files_left = 0;
	close_a_file();
    }
    return retval;
}

/*
 *	Perform tilde expansion, updating the character pointer unless the
 *	user was not found.
 */

_Xconst struct passwd *
ff_getpw(_Xconst char **pp, _Xconst char *p_end)
{
    _Xconst char *p = *pp;
    _Xconst char *p1;
    unsigned len;
    _Xconst struct passwd *pw;
    int count;

    ++p;	/* skip the tilde */
    p1 = p;
    while (p1 < p_end && *p1 != DIR_SEPARATOR)
	++p1;
    len = p1 - p;

    if (len != 0) {
	if (len >= ffline_len)
	    expandline(len);
	bcopy(p, ffline, len);
	ffline[len] = '\0';
    }

    for (count = 0;; ++count) {
	if (len == 0)	/* if no user name */
	    pw = getpwuid(getuid());
	else
	    pw = getpwnam(ffline);

	if (pw != NULL) {
	    *pp = p1;
	    return pw;
	}

	/* On some systems, getpw{uid,nam} return without setting errno,
	 * even if the call failed because of too many open files.
	 * Therefore, we play it safe here.
	 */
	if (count >= 2 && len != 0 && getpwuid(getuid()) != NULL)
	    return NULL;

	close_small_file();
    }
}


/*
 *
 *      Read size bytes from the FILE fp, constructing them into a
 *      signed/unsigned integer.
 *
 */

unsigned long
num(FILE *fp, int size)
{
    long x = 0;

    while (size--)
	x = (x << 8) | one(fp);
    return x;
}

long
snum(FILE *fp, int size)
{
    long x;

#if	__STDC__
    x = (signed char)getc(fp);
#else
    x = (unsigned char)getc(fp);
    if (x & 0x80)
	x -= 0x100;
#endif
    while (--size)
	x = (x << 8) | one(fp);
    return x;
}


size_t
property_get_data(Window w,
		  Atom a,
		  unsigned char **ret_buf,
		  int (*x_get_property)(Display *, Window, Atom, long,
					long, Bool, Atom, Atom *,
					int *, unsigned long *,
					unsigned long *, unsigned char **))
{
    /* all of these are in 8-bit units */
    long byte_offset = 0;
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret;
    unsigned long bytes_after_ret = 0;
    unsigned char *prop_ret = NULL;

    /*
     * buffer for collecting returned data; this is static to
     * avoid expensive malloc()s at every call (which is often!)
     */
    static unsigned char *buffer = NULL;
    static size_t buffer_len = 0;

    while (x_get_property(DISP, w,
			  a, byte_offset / 4, (bytes_after_ret + 3) / 4, False,
			  a, &type_ret, &format_ret, &nitems_ret,
			  &bytes_after_ret, &prop_ret)
	   == Success) {

	if (type_ret != a || format_ret == 0)
	    break;

	nitems_ret *= (format_ret / 8);	/* convert to bytes */

	while ((byte_offset + nitems_ret) >= buffer_len) {
	    buffer_len += 256;
	    buffer = (buffer == NULL ? xmalloc(buffer_len)
		      : xrealloc(buffer, buffer_len));
	}

	/* the +1 captures the extra '\0' that Xlib puts after the end.  */
	memcpy(buffer + byte_offset, prop_ret, nitems_ret + 1);
	byte_offset += nitems_ret;

	XFree(prop_ret);
	prop_ret = NULL;

	if (bytes_after_ret == 0)	/* got all data */
	    break;
    }

    if (prop_ret != NULL)
	XFree(prop_ret);

    *ret_buf = buffer;
    return byte_offset;
}

/* TODO: switch to #define HTEX 0 (or 1)
   instead of #define HTEX
   since non-k xdvi uses mostly the latter.
*/
#if PS || defined(HTEX)

/*
 *	Create a temporary file and return its fd.  Also put its filename
 *	in str.  Create str if it's NULL.
 */

#ifndef P_tmpdir
#define	P_tmpdir	"/tmp"
#endif

static _Xconst char tmp_suffix[] = "/xdvi-XXXXXX";

int
xdvi_temp_fd(char **str)
{
    int fd;
    char *p;
    size_t len;
    static _Xconst char *template = NULL;
#if !HAVE_MKSTEMP
    static unsigned long seed;
    static char letters[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._";
    char *p1;
#endif

    if (*str != NULL) {
	p = *str;
	if (n_files_left == 0)
	    close_a_file();
	/* O_EXCL is there for security (if root runs xdvi) */
	fd = open(p, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1 && (errno == EMFILE || errno == ENFILE)) {
	    n_files_left = 0;
	    close_a_file();
	    fd = open(p, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	}
	if (!(fd == -1 && errno == EEXIST))
	    return fd;
#if HAVE_MKSTEMP
	memcpy(p + strlen(p) - 6, "XXXXXX", 6);
#endif
    }
    else {
	if (template == NULL) {
	    _Xconst char *ourdir;

	    ourdir = getenv("TMPDIR");
	    if (ourdir == NULL || access(ourdir, W_OK) < 0) {
		ourdir = P_tmpdir;
		if (access(ourdir, W_OK) < 0)
		    ourdir = ".";
	    }
	    len = strlen(ourdir);
	    if (len > 0 && ourdir[len - 1] == DIR_SEPARATOR)
		--len;
	    template = p = xmalloc(len + sizeof tmp_suffix);
	    memcpy(p, ourdir, len);
	    memcpy(p + len, tmp_suffix, sizeof tmp_suffix);
#if !HAVE_MKSTEMP
	    seed = 123456789 * time(NULL) + 987654321 * getpid();
#endif
	}
	*str = p = xstrdup(template);
    }

    if (n_files_left == 0)
	close_a_file();

#if HAVE_MKSTEMP
    fd = mkstemp(p);
    if (fd == -1 && (errno == EMFILE || errno == ENFILE)) {
	n_files_left = 0;
	close_a_file();
	memcpy(p + strlen(p) - 6, "XXXXXX", 6);
	fd = mkstemp(p);
    }
#else
    p1 = p + strlen(p) - 6;
    for (;;) {
	unsigned long s = ++seed;
	char *p2;

	for (p2 = p1 + 5; p2 >= p1; --p2) {
	    *p2 = letters[s & 63];
	    s >>= 6;
	}
	fd = open(p, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1 && (errno == EMFILE || errno == ENFILE)) {
	    n_files_left = 0;
	    close_a_file();
	    fd = open(p, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	}
	if (!(fd == -1 && errno == EEXIST))
	    break;
    }
#endif

    return fd;
}

#endif /* PS */

/*
 * access functions for name (and arguments) of last forked command
 * (rather lame, not thread safe etc.)
 */
#define BUF_SIZE 1024

static char save_fork_cmd[BUF_SIZE];
static int err_pipe[2];

/*
 * fork a child process, collecting its error messages
 * into a pipe for GUI display
 */

void
fork_process(_Xconst char *proc_name, char *_Xconst argv[])
{
    int i;
    int fork_cmd_len = 0;

    /* save the command for later error messages */
    strcpy(save_fork_cmd, "");
    for (i = 0; argv[i] != NULL; i++) {
	fork_cmd_len += strlen(argv[i]) + 4; /* for `...\n' */
	if (fork_cmd_len > BUF_SIZE) {
	    strcat(save_fork_cmd, "...");
	}
	else {
	    strcat(save_fork_cmd, argv[i]);
	    strcat(save_fork_cmd, " ");
	}
    }

    Fflush(stdout);	/* to avoid double buffering */
    Fflush(stderr);
    
    if (pipe(err_pipe) < 0) {
	perror("err pipe");
	exit(1);
    }
    
    switch (vfork()) {
    case -1:	/* forking error */
	perror("vfork");
	close(err_pipe[0]);
	close(err_pipe[1]);
	break;
    case 0:	/* child */
	close(err_pipe[0]);	/* no reading from stderr */
	
	/* redirect stderr */
	if (dup2(err_pipe[1], fileno(stderr)) < 0) {
	    perror("dup2 for stderr");
	    _exit(1);
	}
	
	execvp(proc_name, argv);
	
	/* arrive here only if execvp failed */
	Fprintf(stderr, "%s: Execution of %s failed.\n", prog, proc_name);
	Fflush(stderr);
	close(err_pipe[1]);
	_exit(1);
    default:	/* parent */
	close(err_pipe[1]);	/* no writing to stderr */
	break;
    }
}

/* print error messages of forked childs */

void
print_child_error(void)
{
    int bytes;
    int buf_old_size = 0, buf_new_size = 0;
    char tmp_buf[BUF_SIZE];
    char *err_buf = NULL;

    /* collect stderr messages into err_buf */
    while ((bytes = read(err_pipe[0], tmp_buf, BUF_SIZE - 1)) > 0) {
	buf_old_size = buf_new_size;
	buf_new_size += bytes;
	err_buf = xrealloc(err_buf, buf_new_size + 1);
	memcpy(err_buf + buf_old_size, tmp_buf, buf_new_size - buf_old_size);
	err_buf[buf_new_size] = '\0';
    }
    close(err_pipe[0]);

    if (bytes == 0) {
 	err_buf = xstrdup("");
    }

    XBell(DISP, 20);

    do_popup_message(MSG_ERR,
		     NULL,
		     "Error executing:\n%s\n%s",
		     save_fork_cmd, err_buf);
    free(err_buf);
}


/* determine average width of a font */
int
get_avg_font_width(XFontStruct *font)
{
    int width;

    assert(font != NULL);
    width = font->max_bounds.width + font->min_bounds.width / 2;
    if (width == 0) {
	/* if min_bounds.width = -max_bounds.width, we probably
	   have a scalable TT font; try to determine its actual
	   width by measuring the letter `x':
	*/
	width = XTextWidth(font, "x", 1);
    }
    if (width == 0) { /* last resort */
	width = font->max_bounds.width / 2;
    }
    return width;
    
}

/* hashtable functions */
/*
  If key is in hashtable, return True and the integer value in val;
  else return False (and leave val untouched).
*/
Boolean
find_str_int_hash(hash_table_type *hashtable, const char *key, int *val)
{
    string *ret;
#ifdef KPSE_DEBUG
    if (KPSE_DEBUG_P (KPSE_DEBUG_HASH))
	kpse_debug_hash_lookup_int = true;
#endif
    ret = hash_lookup(*hashtable, key);
#ifdef KPSE_DEBUG
    if (KPSE_DEBUG_P (KPSE_DEBUG_HASH))
	kpse_debug_hash_lookup_int = false;
#endif

    if (ret != NULL) {
	long l = (long)*ret;
	*val = l;
/* 	fprintf(stderr, "found key: %s -> %d\n", key, l); */
	return True;
    }
    return False;
}

void
put_str_int_hash(hash_table_type *hashtable, const char *key, int val)
{
    long ptr = (long)val;
    hash_insert(hashtable, key, (const string)ptr);
}
