/*
 * 	newsyslog - roll over selected logs at the appropriate time,
 * 		keeping the a specified number of backup files around.
 *
 * 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/newsyslog.c,v $
 * 	$Author: bert $
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/newsyslog.c,v 1.2 1995-10-15 06:34:16 bert Exp $";
#endif  /* lint */

#define CONF "/etc/athena/newsyslog.conf" /* Configuration file */
#ifdef SOLARIS
#define COMPRESS "/usr/bin/compress" /* File compression program */
#else
#ifdef sgi
#define COMPRESS "/usr/bsd/compress" /* File compression program */
#else
#define COMPRESS "/usr/ucb/compress" /* File compression program */
#endif /* sgi */
#endif /* SOLARIS */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if defined(ultrix) || defined(VAX)
#include <strings.h>  /* check! */
#else
#include <string.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

#define kbytes(size)  (((size) + 1023) >> 10)
#ifdef _IBMR2
/* Calculates (db * DEV_BSIZE) */
#define dbtob(db)  ((unsigned)(db) << UBSHIFT) 
#endif

#define CE_COMPACT 1		/* Compact achived log files using compress */

#define CE_BINARY 2		/* Logfile is in binary, don't add */
				/* status messages */
#define CE_DATED 4		/* Mark the logfile with date, not number */

#define CE_USER_MIN 256		/* User-defined post-processing methods set */
#define CE_USER_MAX 32768	/* bits between these two values (inclusive) */

#define CE_USER_ANY (2*CE_USER_MAX-CE_USER_MIN)
				/* Mask for _any_ user-defined bits */
#define NONE -1
	
struct conf_entry {
	char	*log;		/* Name of the log */
	int     uid;            /* Owner of log */
	int     gid;            /* Group of log */
	int	numlogs;	/* Number of logs to keep */
	int	size;		/* Size cutoff to trigger trimming the log */
	int	hours;		/* Hours between log trimming */
	int	permissions;	/* File permissions on the log */
	int	flags;		/* Flags (CE_COMPACT & CE_BINARY)  */
	struct conf_entry *next; /* Linked list pointer */
};

struct exec_entry {
	char	option;		/* Option character used for this program */
	char	*extension;	/* Extension added by this program */
	char	**args;		/* Program name and command-line arguments */
	int	nargs;		/* Number of arguments, including args[0] */
	struct exec_entry *next; /* Linked list pointer */
};

extern int	optind;
extern char	*optarg;
extern uid_t getuid(),geteuid();
extern time_t time();

char	*progname;		/* contains argv[0] */
int	verbose = 0;		/* Print out what's going on */
int	needroot = 1;		/* Root privs are necessary */
int	noaction = 0;		/* Don't do anything, just show it */
char	*conf = CONF;		/* Configuration file to use */
time_t	timenow;
int	syslog_pid;		/* read in from /etc/syslog.pid */
char	hostname[64];		/* hostname */
char	*daytime;		/* timenow in human readable form */


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
	    if (*string < '0' || *string > '9') return(0);
	    string++;
	}
	return(1);
}

#ifndef SOLARIS
/* Duplicate a string using malloc */
char *strdup (register char *strp)
{
	register char *cp;

	if ((cp = malloc((unsigned) strlen(strp) + 1)) == NULL)
		abort();
	return(strcpy (cp, strp));
}
#endif

/* Fing the file whose name matches one given, with any extension */
char *matching_file(char* name)
{
  DIR *parent;
  struct dirent *dent;
  char *dirname, *namefrag, *fp;
  int fraglen, have_path;

  /* extract the directory path and the filename fragment. */
  fp = dirname = strdup(name);
  if (have_path = !(!(namefrag = strrchr(dirname, '/')))) {
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

  /* find matching directory entry */
  while ((dent=readdir(parent)) && strncmp(dent->d_name, namefrag, fraglen)) ;

  free(fp);

  if (dent && dent->d_name) {
    char *glued = (char*)malloc(strlen(dirname) + strlen(dent->d_name) + 3);
    if (!glued)
      abort();

    /* reconstruct pathname+filename */
    if (have_path) {
      strcpy(glued, dirname);
      strcat(glued, "/");
      strcat(glued, dent->d_name);
    } else
      strcpy(glued, dent->d_name);

    closedir(parent);
    return glued;
  } else {
    closedir(parent);
    return (char*)NULL;
  }
}

/*** more support functions ***/

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
	char tmp[MAXPATHLEN];

	(void) strcpy(tmp,file);
	if (stat(strcat(tmp,".0"),&sb) < 0) {
	  char* file = matching_file(strcat(tmp,"."));
	  if (!file || (stat(file, &sb) < 0))
	    return(-1);
	}
	return( (int) (timenow - sb.st_mtime + 1800) / 3600);
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


/* Fork of /usr/ucb/compress to compress the old log file */
void compress_log(char *log)
{
	int	pid;
	char	tmp[MAXPATHLEN];
	
	pid = fork();
	(void) sprintf(tmp,"%s.0",log);
	if (pid < 0) {
		fprintf(stderr,"%s: ",progname);
		perror("fork");
		exit(1);
	} else if (!pid) {
		(void) execl(COMPRESS,"compress","-f",tmp,0);
		fprintf(stderr,"%s: ",progname);
		perror(COMPRESS);
		exit(1);
	}
}

void dotrim(struct conf_entry *ent)
{
	char	file1[MAXPATHLEN], file2[MAXPATHLEN];
	char    *zfile1, zfile2[MAXPATHLEN];
	char	dfile[MAXPATHLEN], *freep = NULL;
	int	fd;
	struct	stat st;
	int	numdays = ent->numlogs;
	int	owner_uid = ent->uid;

#ifdef _IBMR2
/* AIX 3.1 has a broken fchown- if the owner_uid is -1, it will actually */
/* change it to be owned by uid -1, instead of leaving it as is, as it is */
/* supposed to. */
	if (owner_uid == -1)
	  owner_uid = geteuid();
#endif

	/* Remove oldest log */
	(void) sprintf(file1,"%s.%d",ent->log,numdays);
	if (!stat(file1, &st))
	  zfile1 = file1;
	else {
	  (void) strcpy(dfile, file1);
	  (void) strcat(dfile, ".");
	  zfile1 = freep = matching_file(dfile);
	}

	if (noaction) {
	  if (zfile1)
	    printf("  rm -f %s\n", zfile1);
	} else {
	  if (zfile1)
	    (void) unlink(zfile1);
	}

	/* Move down log files */
	while (numdays--) {
		(void) strcpy(file2,file1);
		(void) sprintf(file1,"%s.%d",ent->log,numdays);

		if (!stat(file1, &st))
		  zfile1 = file1;
		else {
		  (void) strcpy(dfile, file1);
		  (void) strcat(dfile, ".");
		  if (freep) { free(freep);  freep = NULL; }
		  zfile1 = freep = matching_file(dfile);
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

	if (!noaction && !(ent->flags & CE_BINARY))
		(void) log_trim(ent->log); /* Report trimming to the old log */

	if (noaction) 
		printf("  mv %s %s\n",ent->log, file1);
	else
		(void) rename(ent->log, file1);
	if (noaction) 
		printf("Start new log...");
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
		printf(" chmod %o %s...",ent->permissions,ent->log);
	else
		(void) chmod(ent->log,ent->permissions);
	if (noaction)
		printf(" kill -HUP %d (syslogd)\n",syslog_pid);
	else
	  if (kill(syslog_pid,SIGHUP)) {
			fprintf(stderr,"%s: ",progname);
			perror("warning - could not restart syslogd");
		}
	if (ent->flags & CE_COMPACT) {
		if (noaction)
			printf("  compress %s.0\n",ent->log);
		else
			compress_log(ent->log);
	}
}


void do_entry(ent)
	struct conf_entry	*ent;
	
{
	int	size, modtime;
	
	if (verbose) {
		if (ent->flags & CE_COMPACT)
			printf("%s <%dZ>: ",ent->log,ent->numlogs);
		else
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
				if (ent->flags & CE_COMPACT)
					printf("%s <%dZ>: trimming\n",
					       ent->log,ent->numlogs);
				else
					printf("%s <%d>: trimming\n",
					       ent->log,ent->numlogs);
			}
			dotrim(ent);
		} else {
			if (verbose)
				printf("--> skipping\n");
		}
	}
}

void usage(void)
{
	fprintf(stderr,
		"Usage: %s <-nrv> <-f config-file>\n", progname);
	exit(1);
}

void PRS(argc,argv)
	int argc;
	char **argv;
{
	int	c;
	FILE	*f;
	char	line[BUFSIZ];

	progname = argv[0];
	timenow = time((time_t *) 0);
	daytime = ctime(&timenow);
	daytime[strlen(daytime)-1] = '\0';

	/* Let's find the pid of syslogd */
	syslog_pid = 0;
	f = fopen("/etc/syslog.pid","r");
	if (f && fgets(line,BUFSIZ,f))
		syslog_pid = atoi(line);

	/* Let's get our hostname */
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
		default:
			usage();
		}
	}

/* Complain if a variable is NULL, return it otherwise. */
char *missing_field(p,errline)
	char	*p,*errline;
{
	if (!p || !*p) {
		fprintf(stderr,"Missing field in config file:\n");
		fputs(errline,stderr);
		exit(1);
	}
	return(p);
}

/* Parse a configuration file and return a linked list of all the logs
 * to process
 */
void parse_file(struct conf_entry **files)
{
	FILE	*f;
	char	line[BUFSIZ], *parse, *q;
	char	*errline, *group = NULL;
	struct conf_entry *first = NULL;
	struct conf_entry *working;
	struct passwd *pass;
	struct group *grp;

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
		errline = strdup(line);
		if (!first) {
			working = (struct conf_entry *) malloc(sizeof(struct conf_entry));
			first = working;
		} else {
			working->next = (struct conf_entry *) malloc(sizeof(struct conf_entry));
			working = working->next;
		}

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
		else 
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
		while (q && *q && !isspace(*q)) {
			if ((*q == 'Z') || (*q == 'z'))
				working->flags |= CE_COMPACT;
			else if ((*q == 'B') || (*q == 'b'))
				working->flags |= CE_BINARY;
			else {
				fprintf(stderr,
					"Illegal flag in config file -- %c\n",
					*q);
				exit(1);
			}
			q++;
		}
		
		free(errline);
	}
	if (working)
		working->next = (struct conf_entry *) NULL;
	(void) fclose(f);
	(*files) = first;
}

int main(int argc, char **argv)
{
	struct conf_entry *p, *q;
	
	PRS(argc,argv);
	if (needroot && getuid() && geteuid()) {
		fprintf(stderr,"%s: must have root privs\n",progname);
		exit(1);
	}
	parse_file(&p);
	q=p;
	while (p) {
		do_entry(p);
		p=p->next;
		free((char *) q);
		q=p;
	}
	exit(0);
}
