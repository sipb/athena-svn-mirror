/*
 * Quick C preprocessor substitute, for converting X3270.ad to X3270.ad.
 *
 * Copyright (c) 1997 Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *optarg;
extern int optind;

char *me;
int color = 0;

void
usage()
{
	fprintf(stderr, "usage: %s [-DCOLOR] [-UCOLOR] file\n", me);
	exit(1);
}

void
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	char buf[1024];
	FILE *f;
	int pass = 1;
	int nested = 0;
	int ln = 0;

	if ((me = strrchr(argv[0], '/')) != (char *)NULL)
		me++;
	else
		me = argv[0];

	while ((c = getopt(argc, argv, "D:U:")) != -1) {
		switch (c) {
		    case 'D':
			if (strcmp(optarg, "COLOR")) {
				fprintf(stderr, "only -DCOLOR is supported\n");
				exit(1);
			}
			color = 1;
			break;
		    case 'U':
			if (strcmp(optarg, "COLOR")) {
				fprintf(stderr, "only -UCOLOR is supported\n");
				exit(1);
			}
			color = 0;
			break;
		    default:
			usage();
			break;
		}
	}
	switch (argc - optind) {
	    case 0:
		f = stdin;
		break;
	    case 1:
		f = fopen(argv[optind], "r");
		if (f == (FILE *)NULL) {
			perror(argv[optind]);
			exit(1);
		}
		break;
	    default:
		usage();
		break;
	}

	while (fgets(buf, sizeof(buf), f) != (char *)NULL) {
		ln++;
		if (buf[0] == '!')
			continue;
		if (buf[0] != '#') {
			if (pass)
				printf("%s", buf);
			continue;
		}
		if (!strcmp(buf, "#ifdef COLOR\n")) {
			if (nested) {
				fprintf(stderr, "line %d: no nested #ifs\n",
				    ln);
				exit(1);
			}
			nested = 1;
			pass = color;
		} else if (!strcmp(buf, "#ifndef COLOR\n")) {
			if (nested) {
				fprintf(stderr, "line %d: no nested #ifs\n",
				    ln);
				exit(1);
			}
			nested = 1;
			pass = !color;
		} else if (!strcmp(buf, "#else\n")) {
			if (nested != 1) {
				fprintf(stderr, "line %d: #else without #if\n",
				    ln);
				exit(1);
			}
			pass = !pass;
			nested = 2;
		} else if (!strcmp(buf, "#endif\n")) {
			if (!nested) {
				fprintf(stderr, "line %d: #endif without #if\n",
				    ln);
				exit(1);
			}
			pass = 1;
			nested = 0;
		} else {
			fprintf(stderr, "line %d: unknown directive\n", ln);
			exit(1);
		}
	}

	if (f != stdin)
		fclose(f);
	exit(0);
}
