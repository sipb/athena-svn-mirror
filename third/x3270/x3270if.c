/*
 * Copyright 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 * Script interface utility for x3270.
 *
 * Accesses an x3270 command stream on the file descriptors defined by the
 * environment variables X3270OUTPUT (output from x3270, input to script) and
 * X3270INPUT (input to x3270, output from script).
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define IBS	1024

extern int optind;
extern char *optarg;

char *me;
int verbose = 0;

void
usage()
{
	(void) fprintf(stderr, "usage: %s [-v] action[(param[,...])]\n", me);
	(void) fprintf(stderr, "       %s [-v] -s field\n", me);
	exit(2);
}

/* Get a file descriptor from the environment. */
int
fd_env(name)
char *name;
{
	char *fdname;
	int fd;

	fdname = getenv(name);
	if (fdname == (char *)NULL) {
		(void) fprintf(stderr, "%s: %s not set in the environment\n",
				me, name);
		exit(2);
	}
	fd = atoi(fdname);
	if (fd <= 0) {
		(void) fprintf(stderr, "%s: invalid value '%s' for %s\n", me,
		    fdname, name);
		exit(2);
	}
	return fd;
}

int
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int fn = -1;
	char *ptr;
	FILE *inf, *outf;
	char buf[IBS];
	static char status[IBS] = "";
	int xs = -1;

	/* Identify yourself. */
	if ((me = strrchr(argv[0], '/')) != (char *)NULL)
		me++;
	else
		me = argv[0];

	/* Parse options. */
	while ((c = getopt(argc, argv, "s:v")) != -1)
		switch (c) {
		    case 's':
			if (fn != -1)
				usage();
			fn = (int)strtol(optarg, &ptr, 0);
			if (ptr == optarg || *ptr != '\0' || fn < 0) {
				(void) fprintf(stderr,
				    "%s: Invalid field number: '%s'\n", me,
				    optarg);
				usage();
			}
			break;
		    case 'v':
			verbose++;
			break;
		    default:
			usage();
			break;
		}

	/* Validate positional arguments. */
	if (fn != -1) {
		if (optind != argc)
			usage();
	} else {
		if (optind != argc - 1)
			usage();
	}

	/* Verify the environment and open files. */
	inf = fdopen(fd_env("X3270OUTPUT"), "r");
	if (inf == (FILE *)NULL) {
		perror("x3270if: input: fdopen($X3270OUTPUT)");
		exit(2);
	}
	outf = fdopen(fd_env("X3270INPUT"), "w");
	if (outf == (FILE *)NULL) {
		perror("x3270if: output: fdopen($X3270INPUT)");
		exit(2);
	}

	/* Ignore broken pipes. */
	(void) signal(SIGPIPE, SIG_IGN);

	/* Speak to x3270. */
	if (fprintf(outf, "%s\n", (fn == -1) ? argv[optind] : "") < 0 ||
	    fflush(outf) < 0) {
		perror("x3270if: printf");
		exit(2);
	}
	if (verbose)
		(void) fprintf(stderr, "i+ out %s\n",
		    (fn == -1) ? argv[optind] : "");

	/* Get the answer. */
	while (fgets(buf, IBS, inf) != (char *)NULL) {
		int sl = strlen(buf);

		if (sl > 0 && buf[sl-1] == '\n')
			buf[--sl] = '\0';
		if (verbose)
			(void) fprintf(stderr, "i+ in %s\n", buf);
		if (!strcmp(buf, "ok")) {
			(void) fflush(stdout);
			xs = 0;
			break;
		} else if (!strcmp(buf, "error")) {
			(void) fflush(stdout);
			xs = 1;
			break;
		} else if (!strncmp(buf, "data: ", 6)) {
			if (printf("%s\n", buf+6) < 0) {
				perror("x3270if: printf");
				exit(2);
			}
		} else
			(void) strcpy(status, buf);
	}

	/* If fgets() failed, so should we. */
	if (xs == -1) {
		if (feof(inf))
			(void) fprintf(stderr,
				    "x3270if: input: unexpected EOF\n");
		else
			perror("x3270if: input");
		exit(2);
	}

	/* Print status, if that's what they want. */
	if (fn != -1) {
		char *sf = (char *)NULL;
		char *sb = status;

		do {
			if (!fn--)
				break;
			sf = strtok(sb, " \t");
			sb = (char *)NULL;
		} while (sf != (char *)NULL);
		if (printf("%s\n", (sf != (char *)NULL) ? sf : "") < 0) {
			perror("x3270if: printf");
			exit(2);
		}
	}

	if (fflush(stdout) < 0) {
		perror("x3270if: printf");
		exit(2);
	}

	return xs;
}
