/*
 * 	newsyslog - roll over selected logs at the appropriate time,
 * 		keeping the a specified number of backup files around.
 *
 * 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/newsyslog.c,v $
 * 	$Author: cfields $    $Revision: 1.7 $
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/newsyslog.c,v 1.7 1996-08-04 03:18:34 cfields Exp $";
#endif  /* lint */

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>  /* only VAXen don't have string.h --bert 15mar96 */
#endif
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>

#include "signames.h"

#ifdef sgi
/* Note: Irix has the "killall" command, which is like kill except
   that it takes a process name instead of an id. This command is
   not to be confused with the Solaris killall command, which kills
   just about everything. It would probably be nice just to go
   ahead and read the process table instead of execing an external
   program, but... */
#define ALLOW_PROCNAME
#define USE_PROCNAME
#endif

/*** defines ***/

#define CONF "/etc/athena/newsyslog.conf"	/* Configuration file */
#define SYSLOG_PID "/etc/syslog.pid"		/* Pidfile for syslogd */
#define SYSLOG_PNAME "syslogd"			/* Process name for syslogd */

/* COMPRESS is now defined in config.h */

#ifndef SLEEP_DELAY
#define SLEEP_DELAY 1	 /* Default delay used after restarting each daemon, */
#endif			         /* to give it time to clean up (in seconds) */

#define kbytes(size)  (((size) + 1023) >> 10)
#ifdef _IBMR2
/* Calculates (db * DEV_BSIZE) */
#define dbtob(db)  ((unsigned)(db) << UBSHIFT) 
#endif

/* special (i.e. non-configurable) flags for logfile entries */
#define CE_ACTIVE 1	/* Logfile needs to be turned over */
#define CE_BINARY 2	/* Logfile is in binary, don't add status messages */
#define CE_DATED  4	/* Mark the logfile with date, not number */

#define NONE -1

/* Definitions of the keywords for the logfile.
 * Should not be changed once we come up with good ones and document them. =)
 */
/* This is used to add another post-processing command, e.g. 'gzip -9' */
#define KEYWORD_EXEC "filter"
/* This describes a process to restart, other than syslogd */
#define KEYWORD_PID  "signal"
/* This works as "signal," only takes a process name rather than a pid file. */
#define KEYWORD_PNAME "signame"

/* Definitions of the predefined flag letters. */
#define FL_BINARY 'B'   /* reserved */
#define FL_DATED  'D'   /* reserved */
#define FL_COMPRESS 'Z' /* can be redefined */

/* Define the date format used for generating dated filenames. */
/***If you change the format, you must change the parsing in valid_date_ext***/
#define DATE_FORMAT "%Y%m%d"	/* YYYYMMDD, as used by strftime */
#define DATE_LENGTH 8

struct log_entry {
    char *log;			/* Name of the log */
    int  uid;			/* Owner of log */
    int  gid;			/* Group of log */
    int  numlogs;		/* Number of logs to keep */
    int  size;			/* Size cutoff to trigger trimming the log */
    int  hours;			/* Hours between log trimming */
    int  permissions;		/* File permissions on the log */
    int  flags;			/* Flags (CE_ACTIVE , CE_BINARY, CE_DATED) */
    char *exec_flags;		/* Flag letters for things to run */
    char *pid_flags;		/* Flag letters for things to restart */
    struct log_entry *next;	/* Linked list pointer */
};

/* The same structure is used for different kinds of flags. */
/* This wastes a little run-time space, but makes my life easier. */
struct flag_entry {
    char  option;		/* Option character used for this program */
    enum { EXEC, PID, PNAME } type;	/* Type of flag ('run' vs. 'signal') */

    /* EXEC flag options */
    char  *extension;		/* Extension added by this program */
    char  **args;		/* Program name and command-line arguments */
    int	  nargs;		/* Number of arguments not including args[0] */

    /* PID/PNAME flag options */
    char  *pidident;		/* Path to the PID file, or the process name */
    int	signal;			/* Signal to restart the process */
    int	ref_count;		/* Number of times used (if 0, no restart) */

    struct flag_entry *next;	/* Linked list pointer */
};

/* extern uid_t getuid(),geteuid(); */
/* extern time_t time(); */

/* extern's needed for getopt() processing */
extern int	optind;
extern char	*optarg;

/*** globals ***/

char	*progname;		/* name we were run as (ie, argv[0]) */
char	hostname[64];		/* hostname of this machine */

/* variables which correspond to the command-line flags */
int	verbose = 0;		/* Print out what's going on */
int	needroot = 1;		/* Root privs are necessary */
int	noaction = 0;		/* Don't do anything, just show it */
char	*conf = CONF;		/* Configuration file to use */
int	sleeptm = SLEEP_DELAY;  /* Time to wait after restarting a daemon */

time_t	timenow;		/* Time of the start of newsyslog */
char	*daytime;		/* timenow in human readable form */
char    datestamp[DATE_LENGTH+1]; /* timenow in the YYYTMMDD format */

struct flag_entry *flags;	/* List of all config-file flags */

/*** support functions ***/

/* Skip Over Blanks */
char *sob (register char *p)
{
    while (p && *p && isspace(*p))
	p++;
    return(p);
}

/* Skip Over Non-Blanks */
char *son (register char *p)
{
    while (p && *p && !isspace(*p))
	p++;
    return(p);
}

/* Check if string is actually a number */
int isnumber(char *string)
{
    while (*string != '\0') {
	if (! isdigit(*string)) return(0);
	string++;
    }
    return(1);
}

#ifndef HAVE_STRDUP
/* Duplicate a string using malloc */
char *strdup (register char *strp)
{
    register char *cp;

    if ((cp = malloc((unsigned) strlen(strp) + 1)) == NULL)
	abort();
    return(strcpy (cp, strp));
}
#endif

/* Tack a character to the end of a string. (Inefficient...) */
char *grow_option_string (char **strp, char new)
{
    size_t len;

    if (! strp)
	return NULL;
    else if (! *strp) {
	len = 0;
	(*strp) = (char*) malloc(2);
    } else {
	len = strlen(*strp);
	(*strp) = (char*) realloc(*strp, len + 2);
    }

    if (! *strp) abort();

    (*strp)[len] = new;
    (*strp)[len+1] = '\0';

    return *strp;
}

/*** finding the appropriate logfile, whatever its extension ***/

/* Check if the filename extension is composed of valid components */
int valid_extension(char* ext)
{
    struct flag_entry *cfl = flags;
    int len;

    if (ext && (*ext))
	while (cfl) {
	    if ((cfl->type == EXEC) && (cfl->extension)) {
		len = strlen(cfl->extension);

		/* if the current extension matches... */
		if (!strncmp(ext, cfl->extension, len)) {
		    /* exact match */
		    if (ext[len] == '\0')
			return 1;
		    /* match, but we have more to check */
		    else if (ext[len] == '.') {
			if (valid_extension(ext + len + 1))
			    return 1;
			/* if valid_ext returns false, try other possibilities;
			   that way, one proc can add >1 ext (.tar.gz) */
		    }
		}
	    }
	    cfl = cfl->next;
	}
    return 0;
}

/* Check if the string is empty, or a dot followed by a valid extension */
int valid_dot_ext(char* dot)
{
    if (!dot)
	return 0;
    else if (*dot == '\0')
	return 1;
    else if (*dot == '.')
	return valid_extension(dot + 1);

    return 0;
}

/* Check if the string is a valid date (YYYYMMDD) plus extension */
int valid_date_ext(char* date)
{
    /* if we're still compiling C code in 2099, something is horribly wrong */
    return (((date[0]=='1' && date[1]=='9')
	     || (date[0]=='2' && date[1]=='0')) &&
	    isdigit(date[2]) && isdigit(date[3]) &&
	    ((date[4]=='0' && (date[5]>='1' && date[5]<='9'))
	     || (date[4]=='1' && (date[5]>='0' && date[5]<='2'))) &&
	    ((date[6]=='0' && (date[7]>='1' && date[7]<='9'))
	     || ((date[6]=='1' || date[6]=='2') && isdigit(date[7]))
	     || (date[6]=='3' && (date[7]>='0' && date[7]<='1'))) &&
	    valid_dot_ext(date + 8)) ;
}

/* Find the file whose name matches one given, with a valid extension,
 * subject to the following rules:
 * (1) If a filename of the form "${name}${root}valid_ext" exists,
 *     then it is returned.  If there is more than one such file, then
 *     the file returned is the one whose entry occurs first in the directory.
 * (2) If not, and if cmp is non-NULL, then filenames of the form
 *     "${name}YYYYMMDD.valid_ext" are considered.  If there are less
 *     than ${limit} files, NULL is returned; otherwise the 'cmp-most'
 *     match is returned; i.e. the match for which cmp returned true
 *     for all the files it was compared to.
 * Look below for more directly useful functions that call this one.
 */
char *matching_file_compared (char* name, char* root,
			      int cmp(char*,char*,size_t), int limit)
{
    DIR *parent;
    struct dirent *dent;
    char *dirname, *namefrag, *fp, *filename;
    char *cmpmost = NULL;
    int fraglen, rootlen, have_path, mismatch, matches;

    if (root)
	rootlen = strlen(root);
    else
	rootlen = 0;

    /* extract the directory path and the filename fragment. */
    fp = dirname = strdup(name);
    namefrag = strrchr(dirname, '/');
    if (( have_path = (namefrag != NULL) )) {
	*(namefrag++) = '\0';
    } else {
	namefrag = dirname;
	dirname = ".";
    }
    fraglen = strlen(namefrag);

    if (!(parent = opendir(dirname))) {
	free(fp);
	return (char*)NULL;
    }

    /* WARNING: on Solaris, -lucb will link with readdir() which
       returns a *different* struct dirent.  libucb: just say no. */

    matches = 0;
    /* find matching directory entry */
    while ((dent=readdir(parent))
	   && ((mismatch = strncmp(dent->d_name, namefrag, fraglen))
	       || (root && strncmp(dent->d_name + fraglen, root, rootlen))
	       || !valid_dot_ext(dent->d_name + fraglen + rootlen))) {
	if (!mismatch && cmp && valid_date_ext(dent->d_name + fraglen)) {
	    matches++;
	    if ((cmpmost == NULL)
		|| cmp(dent->d_name+fraglen, cmpmost+fraglen, DATE_LENGTH))
	    {
		/* current entry is cmp'er than previous value of cmpmost */
		if (cmpmost)  free(cmpmost);
		cmpmost = strdup(dent->d_name);
	    }
	}
    }

    /* If we found an entry matching root, use that; if not, use a dated one */
    if (dent && dent->d_name)
	filename = dent->d_name;
    else if (matches >= limit)
	filename = cmpmost; /* possibly NULL */
    else
	filename = NULL;

    if (filename) {
	char *glued = (char*)malloc( strlen(dirname)+strlen(filename) + 3);
	if (!glued)
	    abort();

	/* reconstruct pathname+filename */
	if (have_path) {
	    sprintf(glued, "%s/%s", dirname, filename);
	} else
	    strcpy(glued, filename);

	closedir(parent);
	if (cmpmost)  free(cmpmost);
	free(fp);
	return glued;
    } else {
	closedir(parent);
	if (cmpmost)  free(cmpmost);
	free(fp);
	return (char*)NULL;
    }
}

/* Comparison functions: true if file1's date is earlier than file2's */
int cmp_earliest_date (char* file1, char* file2, size_t n)
{
    return ( strncmp(file1, file2, n) < 0 );
}

/* Comparison functions: true if file1's date is later than file2's */
int cmp_latest_date (char* file1, char* file2, size_t n)
{
    return ( strncmp(file1, file2, n) > 0 );
}

/* Find the file whose name matches one given, with a valid extension */
char *matching_file(char* name)
{
    return matching_file_compared (name, NULL, NULL, -1);
}

/* Find the newest logfile: ".0" if it exists, or the latest-dated one */
char *newest_log(char* file)
{
    char tmp[MAXPATHLEN];

    sprintf(tmp, "%s.", file);
    return matching_file_compared(tmp, "0", cmp_latest_date, -1);
}

/* Find the oldest logfile: ".${numlogs}" if it exists, or else the
 * first-dated file if there are more than ${numlogs} dated files.
 */
char *oldest_log(char* file, int numlogs)
{
    char tmp[MAXPATHLEN];
    char num[MAXPATHLEN];

    sprintf(tmp, "%s.", file);
    sprintf(num, "%d",  numlogs);
    return matching_file_compared(tmp, num, cmp_earliest_date, numlogs);
}


/*** examining linked lists of flags ***/

/* return the flag_entry for option letter (if it's currently used), or NULL */
struct flag_entry *get_flag_entry (char option, struct flag_entry *flags)
{
#ifdef DBG_FLAG_ENTRY
    { int debuggers_suck_when_breakpoint_is_a_while_loop = 1; }
#endif

    /* go down the list until we find a match. */
    while (flags) {
	if (option == flags->option)
	    return flags;
	flags = flags->next;
    }

    return (struct flag_entry*)NULL;  /* not found */
}

/* check if an option letter is already used for a flag */
int already_used (char option, struct flag_entry *flags)
{
    /* these letters cannot be redefined! */
    if ((option == FL_BINARY) || (option == FL_DATED))
	return 1;

    /* otherwise check the list */
    if ( get_flag_entry(option,flags) )
	return 1;

    return 0;   /* not found */
}

/*** command-line parsing ***/

void usage(void)
{
    fprintf(stderr,
	    "Usage: %s [-nrv] [-f config-file] [-t restart-time]\n", progname);
    exit(1);
}

/* Parse the command-line arguments */
void PRS(int argc, char **argv)
{
    int	c;
    char    *end;

    progname = argv[0];
    timenow = time((time_t *) 0);
    daytime = ctime(&timenow);
    daytime[strlen(daytime)-1] = '\0';
    if (!strftime(datestamp, DATE_LENGTH+1,DATE_FORMAT, localtime(&timenow))) {
	for (c=0; c<DATE_LENGTH; c++)
	    datestamp[c] = '0';
	datestamp[DATE_LENGTH] = '\0';
    }

    /* Let's get our hostname */
    /* WARNING: on Solaris, -lnsl has gethostname().
       Using -lucb may be dangerous to your foot. --bert 7nov1995 */
    if (gethostname(hostname, 64)) {
	perror("gethostname");
	(void) strcpy(hostname,"Mystery Host");
    }

    optind = 1;		/* Start options parsing */
    while ((c=getopt(argc,argv,"nrvf:t:")) != EOF)
	switch (c) {
	 case 'n':
	    noaction++; /* This implies needroot as off */
	    /* fall through */
	 case 'r':
	    needroot = 0;
	    break;
	 case 'v':
	    verbose++;
	    break;
	 case 'f':
	    conf = optarg;
	    break;
	 case 't':
	    sleeptm = strtol(optarg, &end, 0);
	    if (end && *end) /* arg contains non-numeric chars */
		usage();
	    break;
	 default:
	    usage();
	}
}

/*** config file parsing ***/

/* Complain if the first argument is NULL, return it otherwise. */
char *missing_field(char *p, char *errline)
{
    if (!p || !*p) {
	fprintf(stderr,"Missing field in config file:\n");
	fputs(errline,stderr);
	exit(1);
    }
    return(p);
}

/* Parse a logfile description from the configuration file and update
 * the linked list of logfiles
 */
void parse_logfile_line(char *line, struct log_entry **first,
			struct flag_entry *flags_list)
{
    char   *parse, *q;
    char	  *errline, *group = NULL;
    struct passwd *pass;
    struct group *grp;
    struct log_entry *working = (*first);
    struct flag_entry *opt;

    errline = strdup(line);

    if (!working) {
	(*first) = working =
	    (struct log_entry *) malloc(sizeof(struct log_entry));
	if (!working) abort();
    } else {
	while (working->next)
	    working = working->next;

	working->next = (struct log_entry *)malloc(sizeof(struct log_entry));
	working = working->next;
	if (!working) abort();
    }

    working->next = (struct log_entry *)NULL;

    q = parse = missing_field(sob(line),errline);
    *(parse = son(line)) = '\0';
    working->log = strdup(q);

    q = parse = missing_field(sob(++parse),errline);
    *(parse = son(parse)) = '\0';
    if ((group = strchr(q, '.')) != NULL) {
	*group++ = '\0';
	if (*q) {
	    if (!(isnumber(q))) {
		if ((pass = getpwnam(q)) == NULL) {
		    fprintf(stderr,
			    "Error in config file; unknown user:\n");
		    fputs(errline,stderr);
		    exit(1);
		}
		working->uid = pass->pw_uid;
	    } else
		working->uid = atoi(q);
	} else
	    working->uid = NONE;

	q = group;
	if (*q) {
	    if (!(isnumber(q))) {
		if ((grp = getgrnam(q)) == NULL) {
		    fprintf(stderr,
			    "Error in config file; unknown group:\n");
		    fputs(errline,stderr);
		    exit(1);
		}
		working->gid = grp->gr_gid;
	    } else
		working->gid = atoi(q);
	} else
	    working->gid = NONE;

	q = parse = missing_field(sob(++parse),errline);
	*(parse = son(parse)) = '\0';
    }
    else  /* next string does not contain a '.' */
	working->uid = working->gid = NONE;

    if (!sscanf(q,"%o",&working->permissions)) {
	fprintf(stderr,
		"Error in config file; bad permissions:\n");
	fputs(errline,stderr);
	exit(1);
    }

    q = parse = missing_field(sob(++parse),errline);
    *(parse = son(parse)) = '\0';
    if (!sscanf(q,"%d",&working->numlogs)) {
	fprintf(stderr,
		"Error in config file; bad number:\n");
	fputs(errline,stderr);
	exit(1);
    }

    q = parse = missing_field(sob(++parse),errline);
    *(parse = son(parse)) = '\0';
    if (isdigit(*q))
	working->size = atoi(q);
    else
	working->size = -1;

    q = parse = missing_field(sob(++parse),errline);
    *(parse = son(parse)) = '\0';
    if (isdigit(*q))
	working->hours = atoi(q);
    else
	working->hours = -1;

    q = parse = sob(++parse); /* Optional field */
    *(parse = son(parse)) = '\0';
    working->flags = 0;
    working->exec_flags = NULL;
    working->pid_flags = NULL;

    while (q && *q && !isspace(*q)) {
	char qq = toupper(*q);
	if (qq == FL_BINARY)
	    working->flags |= CE_BINARY;
	else if (qq == FL_DATED)
	    working->flags |= CE_DATED;
	else if (( opt = get_flag_entry(qq, flags_list) )) {
	    if ( opt->type == EXEC )
		grow_option_string( &(working->exec_flags), qq );
	    else if ( opt->type == PID || opt->type == PNAME )
		grow_option_string( &(working->pid_flags), qq );
	    else abort();
	} else {
	    fprintf(stderr,
		    "Illegal flag in config file -- %c\n",
		    *q);
	    exit(1);
	}
	q++;
    }

    free(errline);
}

/* Parse a program-to-run description from the configuration file and update
 * the linked list of flag entries
 */
void parse_exec_line(char *line, struct flag_entry **first)
{
    char *parse, *q;
    char *errline;
    char **arg;
    struct flag_entry *working = (*first);
    int num_args, i;

    errline = strdup(line);

    if (!working) {
	(*first) = working =
	    (struct flag_entry *) malloc(sizeof(struct flag_entry));
	if (!working) abort();
    } else {
	while (working->next)
	    working = working->next;

	working->next = (struct flag_entry*) malloc(sizeof(struct flag_entry));
	working = working->next;
	if (!working) abort();
    }
    working->next = (struct flag_entry *)NULL;
    working->type = EXEC;

    parse = missing_field(sob(line),errline);
    parse = missing_field(son(parse),errline);   /* skip over the keyword */

    parse = missing_field(sob(parse),errline);
    working->option = 0;   /* This is here so already_used doesn't break. */
    if (already_used(toupper(*parse), *first)) {
	fprintf(stderr,
		"Error in config file; option letter already in use:\n");
	fputs(errline,stderr);
	exit(1);
    }
    working->option = toupper(*parse);
    if (!isspace(*(++parse))) {
	fprintf(stderr,
		"Error in config file; more than one option letter:\n");
	fputs(errline,stderr);
	exit(1);
    }

    q = parse = missing_field(sob(parse),errline);
    if (*q == '.') {  /* extension */
	*(parse = son(parse)) = '\0';
	working->extension = strdup(++q);
	
	q = parse = missing_field(sob(++parse),errline);
    }
    else  /* no extension */
	working->extension = NULL;

    num_args = 0;
    while (q && (*q)) {
	num_args++;
	q = son(q);
	if (q && (*q))
	    q = sob(q);
    }
    arg = working->args = (char**) malloc((num_args+2)*sizeof(char*));
    if (! working->args) abort();
    for (i=0; i<num_args; i++) {
	q = missing_field(parse,errline);
	*(parse = son(parse)) = '\0';
      working->args[i] = strdup(q);
	parse = sob(++parse);
    }
    working->args[num_args] = NULL;    /* placeholder for log file name */
    working->args[num_args+1] = NULL;  /* end of array */
    working->nargs = num_args;
    
    free(errline);
}

/* Parse a process-to-restart description from the configuration file and
 * update the linked list of flag entries
 */
void parse_pid_line(char *line, struct flag_entry **first)
{
    char *parse, *q, *end;
    char	*errline;
    struct flag_entry *working = (*first);
    char oops;

    errline = strdup(line);

    if (!working) {
	(*first) = working =
	    (struct flag_entry *) malloc(sizeof(struct flag_entry));
	if (!working) abort();
    } else {
	while (working->next)
	    working = working->next;

	working->next = (struct flag_entry *)malloc(sizeof(struct flag_entry));
	working = working->next;
	if (!working) abort();
    }

    working->next = (struct flag_entry *)NULL;

    parse = missing_field(sob(line),errline);

    if (!strncasecmp(parse, KEYWORD_PID, sizeof(KEYWORD_PID)-1)
	&& isspace(parse[sizeof(KEYWORD_PID)-1]))
      working->type = PID;
    else
      working->type = PNAME;

    parse = missing_field(son(parse),errline);   /* skip over the keyword */

    parse = missing_field(sob(parse),errline);
    working->option = 0;   /* This is here so already_used doesn't break. */
    if (already_used(toupper(*parse), *first)) {
	fprintf(stderr,
		"Error in config file; option letter already in use:\n");
	fputs(errline,stderr);
	exit(1);
    }
    working->option = toupper(*parse);
    if (!isspace(*(++parse))) {
	fprintf(stderr,
		"Error in config file; more than one option letter:\n");
	fputs(errline,stderr);
	exit(1);
    }

    q = parse = missing_field(sob(++parse),errline);
    oops = *(parse = son(parse));
    (*parse) = '\0';
    working->pidident = strdup(q);
    (*parse) = oops;  /* parse may have been pointing at the end of
		       * the line already, so we can't just ++ it.
		       * (Falling off the edge of the world would be bad.) */

    /* the signal field is optional */
    q = parse = sob(parse);
    if (q && (*q)) {
	*(parse = son(parse)) = '\0';
	if (! (working->signal = signal_number(q))) {
	    working->signal = strtol(q, &end, 0);
	    if ((! working->signal) || (end != parse)) {
		fprintf(stderr,
			"Error in config file; bad signal number:\n");
		fputs(errline,stderr);
		exit(1);
	    }
	}
    } else {
	/* default signal value */
	working->signal = SIGHUP;
    }

    working->ref_count = 0;

    free(errline);
}

/* Add support for default flags to the linked list of flag entries.
 */
void add_default_flags(struct flag_entry **first)
{
    struct flag_entry *working = (*first);

    /* we find out if Z flag is used *before* we change the linked list */
    int have_Z = already_used(FL_COMPRESS, *first);

    /* we'll be adding at least one new entry. */
    if (!working) {
	(*first) = working =
	    (struct flag_entry *) malloc(sizeof(struct flag_entry));
	if (!working) abort();
    } else {
	while (working->next)
	    working = working->next;
	working->next = (struct flag_entry*) malloc(sizeof(struct flag_entry));
	working = working->next;
	if (!working) abort();
    }

    /* add 'Z' flag for running "compress" on the logfile. */
    if (! have_Z) {
	working->option = FL_COMPRESS;
	working->type = EXEC;
	working->extension = "Z";

	working->args = (char**) malloc(4*sizeof(char*));
	if (! working->args) abort();
	working->args[0] = strdup(COMPRESS);
	working->args[1] = strdup("-f");
	working->args[2] = NULL;          /* placeholder for log file name */
	working->args[3] = NULL;          /* end of array */
	working->nargs = 2;

	/* we used up an entry, so we allocate a new one. */
	working->next = (struct flag_entry*) malloc(sizeof(struct flag_entry));
	working = working->next;
	if (!working) abort();
    } else {
	fprintf(stderr, "warning: '%c' flag has been redefined.\n",
		FL_COMPRESS);
    }

    /* add unnamed flag for restarting syslogd. */
#ifdef USE_PROCNAME
    working->type = PNAME;
    working->pidident = SYSLOG_PNAME;
#else
    working->type = PID;
    working->pidident = SYSLOG_PID;
#endif
    working->option = 0;
    working->signal = SIGHUP;
    working->ref_count = 0;

    working->next = (struct flag_entry *)NULL;
}

/* Parse a configuration file and return all relevant data in several
 * linked lists
 */
void parse_file(struct log_entry **logfiles, struct flag_entry **flags)
{
    FILE	*f;
    char	line[BUFSIZ];
    int add_flags = 1;

    (*logfiles) = (struct log_entry *) NULL;
    (*flags) = (struct flag_entry *) NULL;

    if (strcmp(conf,"-"))
	f = fopen(conf,"r");
    else
	f = stdin;
    if (!f) {
	(void) fprintf(stderr,"%s: ",progname);
	perror(conf);
	exit(1);
    }
    while (fgets(line,BUFSIZ,f)) {
	if ((line[0]== '\n') || (line[0] == '#'))
	    continue;
	if (!strncasecmp(line, KEYWORD_EXEC, sizeof(KEYWORD_EXEC)-1)
	    && isspace(line[sizeof(KEYWORD_EXEC)-1])) {
	    parse_exec_line(line, flags);
	    continue;
	}
	if ((!strncasecmp(line, KEYWORD_PID, sizeof(KEYWORD_PID)-1)
	     && isspace(line[sizeof(KEYWORD_PID)-1]))
#ifdef ALLOW_PROCNAME
	    || (!strncasecmp(line, KEYWORD_PNAME, sizeof(KEYWORD_PNAME)-1)
		&& isspace(line[sizeof(KEYWORD_PNAME)-1]))) {
#else
	    ) {
#endif
	    parse_pid_line(line, flags);
	    continue;
	}

	/* Add records for some of the default flags.  Do it before
	 * the first file-to-turn-over line, so 'Z' can be redefined.
	 * Also, make sure to do it only once. */
	if (add_flags) {
	    add_default_flags(flags);
	    add_flags = 0;
	}

	parse_logfile_line (line, logfiles, *flags);
    }
    (void) fclose(f);
}

/*** support functions for turning logfiles over ***/

/* Return size in kilobytes of a file */
int sizefile (char *file)
{
    struct stat sb;

    if (stat(file,&sb) < 0)
	return(-1);
    return(kbytes(dbtob(sb.st_blocks)));
}

/* Return the age of old log file (file.0) */
int age_old_log (char *file)
{
    struct stat sb;
    char* last = newest_log(file);

    if (!last || (stat(last, &sb) < 0))
	return(-1);
    free(last);

#ifdef MINUTES    /* this is for debugging: times in minutes instead of hrs */
    return( (int) (timenow - sb.st_mtime + 30) / 60);
#else
    return( (int) (timenow - sb.st_mtime + 1800) / 3600);
#endif
}

/* Log the fact that the logs were turned over */
int log_trim(char *log)
{
    FILE	*f;
    if ((f = fopen(log,"a")) == NULL)
	return(-1);
    fprintf(f,"%s %s newsyslog[%d]: logfile turned over\n",
	    daytime, hostname, (int)getpid());
    if (fclose(f) == EOF) {
	perror("log_trim: fclose:");
	exit(1);
    }
    return(0);
}

/* Fork a program to compress or otherwise process the old log file */
void compress_log(char *log, struct flag_entry *flag)
{
    int	pid;
    int     i;
	
    if (noaction) {
	printf("flag '%c':\n ", flag->option);
	for (i=0; i < flag->nargs; i++)   printf(" %s", flag->args[i]);
	printf(" %s\n", log);
    } else {
	pid = fork();
	if (pid < 0) {
	    fprintf(stderr,"%s: ",progname);
	    perror("fork");
	    exit(1);
	} else if (!pid) {
	    flag->args[flag->nargs] = log;
#ifdef USE_EXECVP
	    /* execvp() looks for the program in PATH (unless path is given) */
	    (void) execvp(flag->args[0], flag->args);
#else
	    (void) execv(flag->args[0], flag->args);
#endif
	    fprintf(stderr,"%s: ",progname);
	    perror(flag->args[0]);
	    exit(1);
	}
    }
}

/* Restart the process whose PID is given in the specified file */
void restart_proc(char *pidfile, int signum)
{
    FILE	*f;
    char	line[BUFSIZ];
    int  pid = 0;
    char *signame;

    /* Let's find the pid of syslogd */
    f = fopen(pidfile, "r");
    if (f && fgets(line,BUFSIZ,f))
	pid = atoi(line);
    fclose(f);

    if (pid) {
	if (noaction) {
	    if (( signame = signal_name(signum) ))
		printf("  kill -%s %d (%s)\n", signame, pid, pidfile);
	    else
		printf("  kill -%d %d (%s)\n", signum, pid, pidfile);
	    printf("  sleep %d\n", sleeptm);
	} else {
	    if (kill(pid, signum)) {
		fprintf(stderr,"%s: %s: ", progname, pidfile);
		perror("warning - could not restart process");
	    }
	    if (sleeptm > 0)
		sleep(sleeptm);
	}
    } else {
	fprintf(stderr,"%s: %s: ", progname, pidfile);
	perror("warning - could not read pidfile");
    }
}

#ifdef ALLOW_PROCNAME
static int run(char *what)
{
  char cmd[1024], *args[20], *ptr;
  int i = 0, status;
  sigset_t mask, omask;
  pid_t pid;

  if (what == NULL)
    return -1;

  strcpy(cmd, what);
  ptr = cmd;
  while (*ptr != '\0')
    {
      args[i++] = ptr;
      while (*ptr != '\0' && *ptr != ' ') ptr++;
      if (*ptr == ' ')
        *ptr++ = '\0';
    }
  args[i] = NULL;

  /* Guard against kidnapping. */
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);

  if ((pid = fork()) == 0)
    {
      sigprocmask(SIG_SETMASK, &omask, NULL);
      execvp(args[0], args);
      exit(-1);
    }

  if (pid == -1)
    {
      sigprocmask(SIG_SETMASK, &omask, NULL);
      return -1;
    }

  while (pid != waitpid(pid, &status, 0));
  sigprocmask(SIG_SETMASK, &omask, NULL);

  if (WIFEXITED(status))
    return WEXITSTATUS(status);

  return -2;
}

int killproc(char *procname, int signum)
{
    char line[BUFSIZ];

    sprintf(line, "killall -%d %s", signum, procname);
    return run(line);
}

/* Restart the process whose name has been specified */
void restart_procname(char *procname, int signum)
{
    char *signame;

    if (noaction) {
       if (( signame = signal_name(signum) ))
	  printf("  killproc -%s %s\n", signame, procname);
       else
	  printf("  killproc -%d %s\n", signum, procname);
       printf("  sleep %d\n", sleeptm);
    } else {
       if (killproc(procname, signum)) {
	  fprintf(stderr,"%s: %s: ", progname, procname);
	  perror("warning - could not restart process");
       }
       if (sleeptm > 0)
	 sleep(sleeptm);
    }
}
#endif /* ALLOW_PROCNAME */

/*** various fun+games with logfile entries ***/

/* Examine an entry and figure out whether the logfile needs to be
 * turned over; if it does, mark it with CE_ACTIVE flag.
 */
void do_entry(struct log_entry *ent)
{
    int	size, modtime;

    if (verbose) {
	printf("%s <%d>: ",ent->log,ent->numlogs);
    }
    size = sizefile(ent->log);
    modtime = age_old_log(ent->log);
    if (size < 0) {
	if (verbose)
	    printf("does not exist.\n");
    } else {
	if (verbose && (ent->size > 0))
	    printf("size (Kb): %d [%d] ", size, ent->size);
	if (verbose && (ent->hours > 0))
	    printf(" age (hr): %d [%d] ", modtime, ent->hours);
	if (((ent->size > 0) && (size >= ent->size)) ||
	    ((ent->hours > 0) && ((modtime >= ent->hours)
				  || (modtime < 0)))) {
	    if (verbose)
		printf("--> trimming log....\n");
	    if (noaction && !verbose) {
		printf("%s <%d>: trimming\n",
		       ent->log,ent->numlogs);
	    }
	    ent->flags |= CE_ACTIVE;
	} else {
	    if (verbose)
		printf("--> skipping\n");
	}
    }
}

/* Turn over the logfiles and create a new one, in preparation for a
 * restart of the logging process(es)
 */
void do_trim(struct log_entry *ent)
{
    char    file1[MAXPATHLEN], file2[MAXPATHLEN];
    char    *zfile1, zfile2[MAXPATHLEN];
    char    *freep = NULL;
    int	fd;
    struct stat st;
    int	numdays = ent->numlogs;
    int	owner_uid = ent->uid;
    struct flag_entry *flg;

#ifdef _IBMR2
/* AIX 3.1 has a broken fchown- if the owner_uid is -1, it will actually */
/* change it to be owned by uid -1, instead of leaving it as is, as it is */
/* supposed to. */
	if (owner_uid == -1)
	  owner_uid = geteuid();
#endif

    /* Remove oldest log */
    zfile1 = freep = oldest_log(ent->log, numdays);
    while (zfile1) {
	if (noaction) {
	    printf("  rm -f %s ...\n", zfile1);
	    zfile1 = NULL;
	} else {
	    (void) unlink(zfile1);
	    if (freep) { free(freep);  freep = NULL; }
	    zfile1 = freep = oldest_log(ent->log, numdays);
	}
    }

    /* Note: if all of the logs are dated, the rolling-over code does nothing.
     * However, it does no harm, and it is vaguely useful in the transition.
     */

    /* Move down log files */
    (void) sprintf(file1,"%s.%d", ent->log, numdays);
    while (numdays--) {
	(void) strcpy(file2,file1);
	(void) sprintf(file1,"%s.%d",ent->log,numdays);

	if (!stat(file1, &st))
	    zfile1 = file1;
	else {
	    if (freep) { free(freep);  freep = NULL; }
	    zfile1 = freep = matching_file(file1);
	    if (!zfile1) continue;
	}

	/* the extension starts at (zfile1 + strlen(file1)) */
	(void) strcpy(zfile2, file2);
	(void) strcat(zfile2, (zfile1 + strlen(file1)));

	if (noaction) {
	    printf("  mv %s %s\n",zfile1,zfile2);
	    printf("  chmod %o %s\n", ent->permissions, zfile2);
	    printf("  chown %d.%d %s\n",
		   owner_uid, ent->gid, zfile2);
	} else {
	    (void) rename(zfile1, zfile2);
	    (void) chmod(zfile2, ent->permissions);
	    (void) chown(zfile2, owner_uid, ent->gid);
	}
    }
    if (freep) { free(freep);  freep = NULL; }

    if (ent->flags & CE_DATED) {    /* if logs are dated, generate new name  */
	(void) sprintf(file1, "%s.%s", ent->log, datestamp);
    }

    if (noaction) 
	printf("  mv %s %s\n",ent->log, file1);
    else
	(void) rename(ent->log, file1);
    if (noaction) 
	printf("Start new log...\n");
    else {
	fd = creat(ent->log, ent->permissions);
	if (fd < 0) {
	    perror("can't start new log");
	    exit(1);
	}		
	if (fchown(fd, owner_uid, ent->gid)) {
	    perror("can't chmod new log file");
	    exit(1);
	}
	(void) close(fd);
	if (!(ent->flags & CE_BINARY))
	    if (log_trim(ent->log)) {      /* Add status message */
		perror("can't add status message to log");
		exit(1);
	    }
    }
    if (noaction)
	printf("  chmod %o %s\n",ent->permissions,ent->log);
    else
	(void) chmod(ent->log,ent->permissions);

    /* Everything on the pid_flags list should get tagged to be restarted. */
    if (ent->pid_flags) {
	char* pid;
	for (pid = ent->pid_flags; *pid; pid++) {
	    flg = get_flag_entry(*pid, flags);
	    if (!flg || ((flg->type != PID) && (flg->type != PNAME))) abort();

	    flg->ref_count++;
	}
    } else {
	/* do the default restart. ('\0' is special.) */
	flg = get_flag_entry('\0', flags);
	if (!flg || ((flg->type != PID) && flg->type != PNAME)) abort();

	flg->ref_count++;
    }
}

/* Take the old logfile, mark it with current time, then run the
 * apropriate program(s) on it.
 */
void do_compress(struct log_entry *ent)
{
    struct flag_entry *flg;
    char* exec;
    char  old[MAXPATHLEN];

    
    if (ent->flags & CE_DATED) {
	sprintf(old, "%s.%s", ent->log, datestamp);
    } else {
	sprintf(old, "%s.0", ent->log);
    }

    if (!noaction && !(ent->flags & CE_BINARY)) {
	(void) log_trim(old);   /* add a date stamp to old log */
    }

    if ((ent->flags & CE_ACTIVE) && ent->exec_flags) {
	for (exec = ent->exec_flags; *exec; exec++) {
	    flg = get_flag_entry(*exec, flags);
	    if (!flg || (flg->type != EXEC)) abort();

	    compress_log(old, flg);

	    if (flg->extension) {
		strcat(old, ".");
		strcat(old, flg->extension);
	    }
	}
    }
}

/*** main ***/

int main(int argc, char **argv)
{
    struct log_entry *p, *q;
    struct flag_entry *flg;

    PRS(argc,argv);
    if (needroot && getuid() && geteuid()) {
	fprintf(stderr,"%s: must have root privs\n",progname);
	exit(1);
    }
    parse_file(&q, &flags);  /* NB: 'flags' is a global variable */
    
    for (p = q; p; p = p->next)
	do_entry(p);

    for (p = q; p; p = p->next)
	if (p->flags & CE_ACTIVE) {
	    if (verbose>1) printf("Trimming %s\n", p->log);
	    do_trim(p);
	}

    for (flg = flags; flg; flg = flg->next)
	if ((flg->type == PID) && flg->ref_count) {
	    if (verbose>1) printf("Restarting %s\n", flg->pidident);
	    restart_proc(flg->pidident, flg->signal);
	}

#ifdef ALLOW_PROCNAME
    for (flg = flags; flg; flg = flg->next)
	if ((flg->type == PNAME) && flg->ref_count) {
	    if (verbose>1) printf("Restarting %s\n", flg->pidident);
	    restart_procname(flg->pidident, flg->signal);
	}
#endif

    for (p = q; p; p = p->next) {
        if (p->flags & CE_ACTIVE) {
	    if (verbose>1) printf("Processing %s\n", p->log);
	    do_compress(p);
	}
    }

    exit(0);
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-label-offset: -3
 * End:
 */
