/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)install.c	5.12 (Berkeley) 7/6/88";
#endif /* not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#ifdef SYSV
#include <fcntl.h> 
#endif
#include <sys/time.h>
#ifndef SOLARIS
#include <a.out.h>
#endif 
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>

#define	NO	0			/* no/false */
#define	YES	1			/* yes/true */

#define	PERROR(head, msg) { \
	fputs(head, stderr); \
	perror(msg); \
}

#ifdef SOLARIS
#define STRIP "/usr/ccs/bin/strip"
#else
#define STRIP "/usr/bin/strip"
#endif

#define MAXARGS 1024

#ifndef MAXBSIZE
#define MAXBSIZE 10240
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

static uid_t uid;
static gid_t gid;

static int	docopy = NO,
		dostrip = NO,
		domove = NO,
		dotime = NO,
		multiple = NO,
		mode = 0755;

static char	*group, *owner,
		pathbuf[MAXPATHLEN];

extern char *getenv();

static install(), strip(), copy(), isnumber(), atoo(), bad(), usage();

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct stat from_sb, to_sb;
	int ch, no_target;
	char *to_name;
	struct passwd *pp;
	struct group *gp;

	int argc_extend = 1, argc_extra;
	char *argv_extend[MAXARGS];
	char *inst_env;

	if ((inst_env = getenv("INSTOPT")) == NULL)
		inst_env = "";
	else
		inst_env = strdup(inst_env);

	while (*inst_env) {
		argv_extend[argc_extend++] = inst_env;
		while (*++inst_env && *inst_env != ' ' && *inst_env != '\t');
		if (*inst_env)
			*inst_env++ = '\0';
	}

	if (argc_extend + argc > MAXARGS) {
		fprintf(stderr, "install: too many command-line arguments.\n");
		exit(1);
	}

	
	argc_extra = argc_extend;
	argv_extend[0] = *argv++;
	while (--argc)
		argv_extend[argc_extend++] = *argv++;

	argc = argc_extend;
	argv = argv_extend;
	
	while ((ch = getopt(argc, argv, "cdstg:m:o:")) != EOF)
		switch((char)ch) {
		case 'c':
                        if (domove == YES)
                                multiple = YES;
			docopy = YES;
			break;
                case 'd':
                        if (docopy == YES)
                                multiple = YES;
                        domove = YES;
                        break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			mode = atoo(optarg);
			break;
		case 'o':
			owner = optarg;
			break;
		case 's':
			dostrip = YES;
			break;
		case 't':
			dotime = YES;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

        /* Check for multiple specifications of copy and move. */
        if (multiple == YES) {
                fprintf(stderr, "install: multiple specifications of -c/-d\n");
                exit(1);
        }
	else if (domove == NO)
	    docopy = YES;	/* do copy by default, rather than move */

	/* get group and owner id's */
	if (owner) {
	    if (to_name = index(owner, '.')) {
		*to_name++ = '\0';
		if (!group) {
		    group = to_name;
		} else {
		    fprintf(stderr, "install: multiple specification of the group\n");
		    exit(1);
		}
	    }
	    if (!isnumber(owner)) {
		if (!(pp = getpwnam(owner))) {
		    fprintf(stderr, "install: unknown user %s.\n", owner);
		    exit(1);
		}
		else
		    uid = pp->pw_uid;
	    }
	    else
		uid = atoi(owner);
	} else 
	    uid = -1;
	
	if (group)
	    if (!isnumber(group)) {
		if (!(gp = getgrnam(group))) {
		    fprintf(stderr, "install: unknown group %s.\n", group);
		    exit(1);
		}
		else
		    gid = gp->gr_gid;
	    }
	    else 
		gid = atoi(group);
	else 
	    gid = -1;
	
	no_target = stat(to_name = argv[argc - 1], &to_sb);
	if (!no_target && (to_sb.st_mode & S_IFMT) == S_IFDIR) {
		for (; *argv != to_name; ++argv)
			install(*argv, to_name, YES);
		exit(0);
	}

	/* can't do file1 file2 directory/file */
	if (argc != 2)
		usage();

	if (!no_target) {
		if (stat(*argv, &from_sb)) {
			fprintf(stderr, "install: can't find %s.\n", *argv);
			exit(1);
		}
		if ((to_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "install: %s isn't a regular file.\n", to_name);
			exit(1);
		}
		if (to_sb.st_dev == from_sb.st_dev && to_sb.st_ino == from_sb.st_ino) {
			fprintf(stderr, "install: %s and %s are the same file.\n", *argv, to_name);
			exit(1);
		}
		/* unlink now... avoid ETXTBSY errors later */
		(void)unlink(to_name);
	}
	install(*argv, to_name, NO);
	exit(0);
}

/*
 * install --
 *	build a path name and install the file
 */
static
install(from_name, to_name, isdir)
	char *from_name, *to_name;
	int isdir;
{
	struct stat from_sb;
	struct timeval timep[2];
	int devnull, from_fd, to_fd;
	char *C;

	/* if try to install "/dev/null" to a directory, fails */
	if (isdir || strcmp(from_name, "/dev/null")) {
		if (stat(from_name, &from_sb)) {
			fprintf(stderr, "install: can't find %s.\n", from_name);
			exit(1);
		}
		if ((from_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "install: %s isn't a regular file.\n", from_name);
			exit(1);
		}
		/* build the target path */
		if (isdir) {
			(void)sprintf(pathbuf, "%s/%s", to_name, (C = rindex(from_name, '/')) ? ++C : from_name);
			to_name = pathbuf;
		}
		devnull = NO;
	}
	else
		devnull = YES;

	/* unlink now... avoid ETXTBSY errors later */
	(void)unlink(to_name);

	/* create target */
	if ((to_fd = open(to_name, O_CREAT|O_WRONLY|O_TRUNC, 0700)) < 0) {
		PERROR("install: ", to_name);
		exit(1);
	}
	if (!devnull) {
		if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
			(void)unlink(to_name);
			PERROR("install: open: ", from_name);
			exit(1);
		}
		copy(from_fd, from_name, to_fd, to_name);
		(void)close(from_fd);
		(void)close(to_fd);
		if (dostrip) {
		      char stripname[PATH_MAX + 50];

		      sprintf(stripname, "%s %s", STRIP, to_name);
		      system(stripname);
		}
		if (!docopy)
			(void)unlink(from_name);
	}
	if (dotime) {
		timep[0].tv_sec = from_sb.st_atime;
		timep[1].tv_sec = from_sb.st_mtime;
		timep[0].tv_usec = timep[1].tv_usec = 0;
		if (utimes(to_name, timep)) {
			PERROR("install: utimes: ", to_name);
			bad();
		}
	}
	/* set owner, group, mode. and time for target */
	if (chmod(to_name, mode)) {
		PERROR("install: fchmod: ", to_name);
		bad();
	}
	if ((uid != -1 || gid != -1)
	    && chown(to_name,
		     (uid != -1) ? uid : from_sb.st_uid,
		     (gid != -1) ? gid : from_sb.st_gid)){
	    PERROR("install: chown: ", to_name);
	    bad();
	}
}

/*
 * copy --
 *	copy from one file to another
 */
static
copy(from_fd, from_name, to_fd, to_name)
	register int from_fd, to_fd;
	char *from_name, *to_name;
{
	register int n;
	char buf[MAXBSIZE];

	while ((n = read(from_fd, buf, sizeof(buf))) > 0)
		if (write(to_fd, buf, n) != n) {
			PERROR("install: write: ", to_name);
			bad();
		}
	if (n == -1) {
		PERROR("install: read: ", from_name);
		bad();
	}
}

/*
 * isnumber --
 *      determine whether string is a number
 */
static
isnumber(string)
  char *string;
{
    char *s = string;

    while(*s)
	if (!isdigit(*s))
	    return(0);
	else
	    s++;
    return(1);
}

/*
 * atoo --
 *	octal string to int
 */
static
atoo(str)
	register char *str;
{
	register int val;

	for (val = 0; isdigit(*str); ++str)
		val = val * 8 + *str - '0';
	return(val);
}

/*
 * bad --
 *	remove created target and die
 */
static
bad()
{
	(void)unlink(pathbuf);
	exit(1);
}

/*
 * usage --
 *	print a usage message and die
 */
static
usage()
{
	fputs("usage: install [-cds] [-g group] [-m mode] [-o owner] file1 file2;\n\tor file1 ... fileN directory\n", stderr);
	exit(1);
}
