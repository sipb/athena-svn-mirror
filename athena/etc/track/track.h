/*
 * $Id: track.h,v 4.9 1995-07-21 00:24:28 cfields Exp $
 */

#include "bellcore-copyright.h"
#include "mit-copyright.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <sys/param.h>
#include <sys/file.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>

/* Default working directory - under to- or from- root */
#define DEF_WORKDIR	"/usr/athena/lib"

/* Default administrator */
#define DEF_ADM		"root"

/* Default binary directory - under real root */
#define DEF_BINDIR	"/usr/athena/etc"

/* Default root for source of transfer */
#define DEF_FROMROOT	"/srvd"

/* Default subscription list */
#define DEF_LOG 	"/usr/tmp/TRACKLOG"

/* Default root for destination of transfer: "" == root. */
#define DEF_TOROOT	""

/* Default subscription list */
#define DEF_SUB 	"sys_rvd"

/* Default directory containing subscription lists under working dir */
#define DEF_SLISTDIR	"slists"

/* Default directory containing stat files under working dir */
#define DEF_STATDIR	"stats"

/* Default directory containing lock files under real root */
#define DEF_LOCKDIR	"/tmp"

/* Default global exceptions: filenames shouldn't contain whitespace */
#define DEF_EXCEPT	{ "#*", "*~", "*\t*", "* *", "*\n*" }

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

typedef struct currentness {
	char name[ LINELEN];
	unsigned int cksum;
	char link[ LINELEN];
	struct stat sbuf;
} Currentness;

typedef struct statline {
	char sortkey[ LINELEN];
	char line[ LINELEN];
} Statline ;

extern Statline *statfilebufs;
extern int cur_line;
extern FILE *statfile;

/* NEXT is defined weirdly,
 * so that it can appear as an lvalue.
 */
#define  FLAG(   list_elt)		     (((char *)( list_elt))[-1])
#define PNEXT(   list_elt) ((List_element **)&((char *)( list_elt))[-5])
#define  NEXT(   list_elt) *PNEXT( list_elt)
#define  TEXT(   list_elt) ((char*)(list_elt))
#define NORMALCASE	((char) 0)
#define FORCE_LINK	((char)-1)
#define DONT_TRACK	((char) 1)

typedef struct list_element {
	struct list_element *next;
	char flag;
	char first_char[1];
} List_element;

/* XXX: if shift field is negative, the table field contains a linked-list.
 * if shift is positive, the table is a hash-table.
 * this enables justshow() to dump an incompletely-parsed subscription-list.
 * LIST() macro should only be used when adding list-elts during parsing.
 */
#define LIST( tbl) ( (tbl).shift--, (List_element**)&(tbl).table)
typedef struct Tbl {
	List_element **table;
	short shift;
} Table;

typedef struct entry {
	char sortkey[ LINELEN];
	int keylen;
	int followlink;
	char *fromfile;
	char *tofile;
	char *cmpfile;
	Currentness currency;
	Table names;
	List_element *patterns;
	char *cmdbuf;
} Entry ;
extern Entry entries[];

extern int errno;
extern int cksumflag;
extern int forceflag;
extern int ignore_prots;
extern int incl_devs;
extern int nopullflag;
extern int parseflag;
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
extern char logfilepath[];
extern char subfilepath[];
extern char prgname[];
extern FILE *logfile;
extern int debug;

#define	DO_CLOBBER	1
#define NO_CLOBBER	0
extern int clobber;

/* parser stuff */
extern char wordbuf[];
extern char linebuf[];
extern FILE *yyin,*yyout;

#define TYPE( statbuf) ((statbuf).st_mode & S_IFMT)
#define MODE( statbuf) ((statbuf).st_mode & 07777)
#define TIME( statbuf) ((statbuf).st_mtime)
#define UID( statbuf)  ((statbuf).st_uid)
#define GID( statbuf)  ((statbuf).st_gid)
#define RDEV( statbuf) ((statbuf).st_rdev)

extern int access();

extern char errmsg[];
char *gets(),*re_comp();
long time();

int stat(), lstat();
extern int (*statf)();
extern char *statn;

/* track's internal functions which need decl's */

extern Entry *clear_ent();

extern FILE *opensubfile();

extern List_element *add_list_elt();
extern List_element **lookup();

extern char *dec_statfile();
extern char *goodname();
extern char **initpath();
extern char *next_def_except();
extern char *re_conv();
extern char *resolve();

extern int entrycmp(), statlinecmp();
extern unsigned long hash();
extern unsigned in_cksum();

extern struct currentness *dec_entry();
extern struct currentness *get_cmp_currency();

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
