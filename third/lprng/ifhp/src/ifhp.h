/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****
$Id: ifhp.h,v 1.1.1.1 1999-02-17 15:31:05 ghudson Exp $
 **** ENDHEADER ****/

#ifndef _IFHP_H_
#define _IFHP_H_ 1
#ifdef EXTERN
# undef EXTERN
# undef DEFINE
# define EXTERN
# define DEFINE(X) X
#else
# undef EXTERN
# undef DEFINE
# define EXTERN extern
# define DEFINE(X)
#endif

/*****************************************************************
 * get the portability information and configuration 
 *****************************************************************/

#include "portable.h"
#include "debug.h"
#include "errormsg.h"
#include "patchlevel.h"
#include "configfile.h"
#include "linelist.h"

/*****************************************************************
 * Global variables and routines that will be common to all programs
 *****************************************************************/

/*****************************************************
 * Internationalisation of messages, using GNU gettext
 *****************************************************/

#if HAVE_LOCALE_H
# include <locale.h>
#endif

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define _(Text) Text
#endif
#define N_(Text) Text

/*
 * data types and #defines
 */

/*
 * struct value - used to set and get values for variables
 */

struct keyvalue{
	char *varname;	/* variable name */
    char *key; /* name of the key */
    char **var; /* variable to set */
    int kind; /* type of variable */
#define INTV 0
#define STRV 1
#define FLGV 2
	char *defval;	/* default value, if any */
};

/*
 * dynamically controlled byte array for IO purposes 
 */
struct std_buffer {
	char *buf;		/* buffer */
	int end;		/* end of buffer */
	int start;		/* start of buffer */
	int max;		/* maximum size */
};

typedef void (*Wr_out)(char *);
typedef	int (*Builtin_func)(char *, char*, char *, Wr_out);

#define SMALLBUFFER 256
#define LARGEBUFFER 1024
#define OUTBUFFER 1024*10

/* get the character value at a char * location */

#define cval(x) (int)(*(unsigned char *)(x))
/* EXIT CODES */

#define JSUCC    0     /* done */
#define JFAIL    32    /* failed - retry later */
#define JABORT   33    /* aborted - do not try again, but keep job */
#define JREMOVE  34    /* failed - remove job */
#define JACTIVE  35    /* active server - try later */
#define JIGNORE  36    /* ignore this job */
/* from 1 - 31 are signal terminations */

#define UNKNOWN 0
#define PCL 1
#define PS 2
#define TEXT 3
#define RAW 4

/*
 * Standard function prototypes
 */

/* VARARGS3 */
#ifdef HAVE_STDARGS
int	plp_snprintf (char *str, size_t count, const char *fmt, ...);
int	vplp_snprintf (char *str, size_t count, const char *fmt, va_list arg);
#else
int plp_snprintf ();
int vplp_snprintf ();
#endif

/* dynamic memory allocation */

char *safestrdup3( const char *s1, const char *s2, const char *s3, const char *file, int line );
char *safestrdup2( const char *s1, const char *s2, const char *file, int line );
char *safestrdup (const char *p, const char *file, int line);
void *realloc_or_die( void *p, size_t size, const char *file, int line );
void *malloc_or_die( size_t size, const char *file, int line );

/*
 * User Function Prototypes
 */

extern char *Find_str_value( struct line_list *l, char *key, char *Value_sep );
extern int Find_flag_value( struct line_list *l, char *key, char *sep );

extern void Open_device( char *device );
extern void cleanup(int sig);
extern int Globmatch( char *pattern, char *str );
extern void Do_accounting(int start, int elapsed, int pagecounter, int npages );
extern char *Check_code( char *code_str );
extern void Put_outbuf_str( char *s );

void Text_banner(void);

/*
 * Constant Strings
 */

EXTERN char *Value_sep DEFINE( = " \t=#@" );
EXTERN char *Whitespace DEFINE( = " \t\n\f" );
EXTERN char *List_sep DEFINE( = "[] \t\n\f" );
EXTERN char *Linespace DEFINE( = " \t" );
EXTERN char *Filesep DEFINE( = " \t,;" );
EXTERN char *Argsep DEFINE( = ",;" );
EXTERN char *Namesep DEFINE( = "|:" );
EXTERN char *Line_ends DEFINE( = "\n\014\004\024" );

/*
 * setup values
 */

EXTERN struct line_list Zopts, Topts, Raw, Model,
	Devstatus, Pjl_only, Pjl_except,	/* option variables */
	Pjl_vars_set, Pjl_vars_except,
	Pcl_vars_set, Pcl_vars_except,
	User_opts, Pjl_user_opts, Pcl_user_opts, Ps_user_opts,
	Pjl_error_codes, Pjl_quiet_codes;
EXTERN char *Loweropts[26];	/* lower case options */
EXTERN char *Upperopts[26];	/* upper case options */
EXTERN char **Envp;			/* environment variables */
EXTERN char **Argv;			/* parms variables */
EXTERN int Argc;			/* we have the number of variables */

EXTERN time_t Start_time;	/* start time of program */

EXTERN int
	Accounting_fd,	/* accounting fd */
	Autodetect,	/* let printer autodetect type */
	Banner_name,	/* invoked with banner */
	Banner_parse_inputline,	/* parse inputline */
	Banner_suppressed,	/* suppress banner generation */
	Banner_user,	/* allow user to select a banner */
	Crlf,		/* only do CRLF */
	CTRL_D_at_start,	/* only do CRLF */
	Dev_retries,	/* number of retries on open */
	Dev_sleep,	/* wait between restries */
	Errorcode,		/* exit value */
	Force_status,	/* even if device is not socket or tty, allow status */
	Full_time,		/* use Full_time format */
	Initial_timeout,	/* initial timeout on first write */
	Job_timeout,	/* timeout for job */
	Logall,		/* log all information back from printer */
	Max_status_size DEFINE(=8),	/* status file max size */
	Min_status_size DEFINE(=2),	/* status file min size */
	No_PS_EOJ,			/* no PS eoj */
	No_PCL_EOJ,			/* no PS eoj */
	Null_pad_count,	/* null padding on PJL ENTER command */
	OF_Mode,			/* running in OF mode */
	Pagecount_interval,	/* pagecount polling interval */
	Pagecount_poll,	/* pagecount polling interval */
	Pagecount_timeout,	/* pagecount */
	Pcl,		/* has PCL support */
	Pjl,		/* has PJL support */
	Pjl_enter,	/* use the PJL ENTER command */
	Ps,			/* has PostScript support */
	Psonly,	/* only recognizes PostScript */
	Quiet,		/* suppress printer status messages */
	Status,		/* any type of status - off = write only */
	Status_fd DEFINE(=-2),	/* status reporting to remote site */
	Summary_fd DEFINE(=-2),	/* status reporting */
	Sync,		/* synchronize printer */
	Sync_interval,	/* sync interval */
	Sync_timeout,	/* sync timeout */
	Tbcp,		/* supports Postscript TBCP */
	Text,		/* supports test files */
	Trace_on_stderr, /* puts out trace on stderr as well */
	Ustatus_on, /* Ustatus was sent, need to send USTATUSOFF */
	Waitend_interval;	/* wait between sending end status requests */

EXTERN char 
	*Accountfile,		/* accounting file */
	*Accounting_script,	/* accounting script to use */
	*Banner,			/* banner type */
	*Banner_file,	/* banner file */
	*Banner_only,	/* parse inputline */
	*Config_file,	/* config file list */
	*Device,		/* device to open */
	*Model_id,		/* printer model */
	*Name,			/* program name */
	*Remove_ctrl,	/* remove these control chars */
	*Statusfile,	/* status file */
	*Stty_args,		/* if device is tty, stty values */
	*Summaryfile;	/* summary file to set */

/*
 * set by routines
 */
EXTERN char
	*Pagecount_ps_code;		/* how to do pagecount */

extern struct keyvalue Valuelist[], Builtin_values[];

#if defined DMALLOC
#	include <dmalloc.h>
#endif
#endif
