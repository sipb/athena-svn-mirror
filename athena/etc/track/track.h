/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v 2.3 1988-02-29 20:33:04 don Exp $
 */

#ifndef lint
static char *rcsid_track_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v 2.3 1988-02-29 20:33:04 don Exp $";
#endif lint

#include "mit-copyright.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/file.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>

/* Default root for source of transfer */
#define DEF_FROMROOT	"/srvd"

/* Default root for destination of transfer */
#define DEF_TOROOT	"/"

/* Default working directory - under to- or from- root */
#define DEF_WORKDIR	"/etc/athena/lib"

/* Default binary directory - under real root */
#define DEF_BINDIR	"/etc/athena"

/* Default administrator */
#define DEF_ADM		"treese"

/* Default subscription list */
#define DEF_SUB 	"sys_rvd"

/* Default directory containing subscription lists under working dir */
#define DEF_SLISTDIR	"slists"

/* Default directory containing stat files under working dir */
#define DEF_STATDIR	"stats"

/* Default directory containing lock files under real root */
#define DEF_LOCKDIR	"/tmp"

/* Default global exceptions */
#define DEF_EXCEPT	{ "#*", "*~" }

/* Default log file */
#define DEF_LOG		"logfiles/lib.log"

/* Default shell */
#define DEF_SHELL	"/bin/sh"

/* Default command to set a shell variable */
#define DEF_SETCMD	""

#define BUFLEN 1024
#define BLOCKSIZE 1024
#define LINELEN 256
#define MAXLINES 300
#define WORDLEN 20
#define WORDMAX 128
#define ENTRYMAX 256
#define STACKMAX 50

#define CNT  0
#define ROOT 1
#define NAME 2

typedef struct stl {
	char sortkey[ LINELEN];
	char line[ LINELEN];
} Statline ;

extern Statline *statfilebufs;
extern int cur_line;
extern FILE *statfile;

typedef struct ent {
	char sortkey[ LINELEN];
	int keylen;
	int followlink;
	char *fromfile;
	char *tofile;
	char *cmpfile;
	char *exceptions[WORDMAX];
	char *cmdbuf;
} Entry ;
extern Entry entries[];

extern int errno;
extern int dirflag;
extern int forceflag;
extern int incl_devs;
extern int nopullflag;
extern int quietflag;
extern int uflag;
extern int verboseflag;
extern int writeflag;
extern int entnum;
extern int entrycnt;
extern unsigned stackmax;
extern unsigned maxlines;

#define	IS_LIST		1
#define NO_LIST		0

#define LOCK_TIME ((long)(60 * 60 * 6))	/* amount of time to let a lockfile
						sit before trying again
						i.e.  6 hours */
extern char binarydir[];
extern char fromroot[];
extern char toroot[];
extern char twdir[];
extern char cwd[];
extern char subfilename[];
extern char subfilepath[];
extern char prgname[];
extern int debug;

#define	DO_CLOBBER	1
#define NO_CLOBBER	0
extern int clobber;

/* parser stuff */
extern char linebuf[];
extern int wordcnt;
extern FILE *yyin,*yyout;

#define TYPE( statbuf) ((int )(statbuf).st_mode & S_IFMT)
#define MODE( statbuf) ((int )(statbuf).st_mode & 07777)
#define TIME( statbuf) ((long)(statbuf).st_mtime)
#define UID( statbuf)  ((int )(statbuf).st_uid)
#define GID( statbuf)  ((int )(statbuf).st_gid)
#define DEV( statbuf)  ((int )(statbuf).st_rdev)

extern int access();

extern char errmsg[];
char *gets(),*malloc(),*realloc(),*re_comp();
char *index(),*rindex(),*strcat(),*strncat(),*strcpy(),*strncpy();
int strcmp(),strncmp(),strlen();
long time();

int stat(), lstat();
extern int (*statf)();

/* track's internal functions which need decl's */

extern char *next_def_except();
extern struct stat *dec_entry(), *dec_statfile();
extern int entrycmp(), statlinecmp();
extern char *follow_link();
extern char *goodname();
extern char **initpath();
extern char *make_name();
extern FILE *opensubfile();
extern char *re_conv();
extern char *resolve();

#define SIGN( i) (((i) > 0)? 1 : ((i)? -1 : 0))

/* make a sortkey out of a pathname.
 * because several printing characters, notably '.',
 * come before '/' in the standard ascii sort-order,
 * we need a non-standard sorting order:
 * /etc
 * /etc/blah
 * /etc/whoop
 * /etc.athena ...
 * this requires that slashes get mapped to low-ranking
 * non-printing characters, for the purposes of the sort.
 * this macro allows us to do it fast.
 */
#define KEYCPY( key, name) \
{char *k,*p; k=key; for (p=name;*p;p++) *k++ = (*p=='/') ? '\001' : *p; *k= *p;}
