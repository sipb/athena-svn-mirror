/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v 1.4 1987-03-05 19:50:40 rfrench Exp $
 */

#ifndef lint
static char *rcsid_track_h = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/track.h,v 1.4 1987-03-05 19:50:40 rfrench Exp $";
#endif lint

#include "mit-copyright.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/file.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>

/* Default root for source of transfer - just "" if root */
#define DEF_FROMROOT	"/srvd"

/* Default root for destination of transfer - just "" if root */
#define DEF_TOROOT	""

/* Default from working directory - under from root */
#define	DEF_FROMWDIR	"/etc/athena/lib"

/* Default to working directory - under to root */
#define DEF_TOWDIR	"/etc/athena/lib"

/* Default binary directory - under real root */
#define DEF_BINDIR	"/etc/athena"

/* Default administrator */
#define DEF_ADM		"treese"

/* Default subscription list */
#define DEF_SUB 	"sys_rvd"

/* Default directory containing subscription lists under working dir */
#define DEF_SUBDIR	"/slists"

/* Default directory containing stat files under working dir */
#define DEF_STATDIR	"/stats"

/* Default directory containing lock files under real root */
#define DEF_LOCKDIR	"/tmp"

/* Default exceptions */
#define DEF_EXCEPT	"#* *~"

/* Default log file */
#define DEF_LOG		"logfiles/lib.log"

/* Default shell */
#define DEF_SHELL	"/bin/sh"

/* Default command to set a shell variable */
#define DEF_SETCMD	""

#define BUFLEN 1024
#define BLOCKSIZE 1024
#define LINELEN 256
#define WORDLEN 20
#define WORDMAX 128
#define ENTRYMAX 256

char *gets(),*index(),*rindex();

extern FILE *popen();
extern FILE *subfile;
extern char *make_name();
extern char errmsg[];
extern char g_except[];
typedef struct ent {
	int followlink;
	char *fromfile;
	char *tofile;
	char *cmpfile;
	char *exceptions[WORDMAX];
	char *cmdbuf;
} Entry ;
extern Entry entries[];

extern int errno;
extern int quietflag;
extern int uflag;
extern int forceflag;
extern int verboseflag;
extern int dirflag;
extern int entrycnt;
extern int cur_ent;

#define	IS_LIST		1
#define NO_LIST		0

#define LOCK_TIME ((long)(60 * 60 * 6))	/* amount of time to let a lockfile
						sit before trying again
						i.e.  6 hours */
extern char binarydir[];
extern char myname[];
extern char fromroot[];
extern char toroot[];
extern char cwd[];
extern char subname[];
extern char prgname[];
extern int via;
extern int tcphung();
extern int child_id;
extern int debug;

#define	DO_CLOBBER	1
#define NO_CLOBBER	0
extern int clobber;

extern int inpipe;
extern int outpipe;

/* parser stuff */
extern char linebuf[];
extern int wordcnt;
extern FILE *yyin,*yyout;

/* Structure definitions */

struct stamp {
	char type;
	char name[LINELEN];
	char link[LINELEN];
	int uid,gid,mode,dev;
	long ftime;
};
