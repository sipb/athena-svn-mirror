/*
 * Copyright 1995 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 */

/*
 * mkfb.c
 *	Utility to create fallback definitions from a simple #ifdef'd .ad
 *	file
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFSZ	1024		/* input line buffer size */
#define ARRSZ	8192		/* output array size */

unsigned a_color[ARRSZ];	/* array of color indices */
unsigned n_color = 0;		/* number of color definitions */

unsigned a_mono[ARRSZ];		/* array of mono indices */
unsigned n_mono = 0;		/* number of mono definitions */

enum { BOTH, COLOR_ONLY, MONO_ONLY } mode = BOTH;

char *me;

void emit();

void
usage()
{
	fprintf(stderr, "usage: %s [infile]\n", me);
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	char buf[BUFSZ];
	int lno = 0;
	int cc = 0;
	int i;
	int continued = 0;
	char *filename = "standard input";

	/* Parse arguments. */
	if ((me = strrchr(argv[0], '/')) != (char *)NULL)
		me++;
	else
		me = argv[0];
	switch (argc) {
	    case 1:
		break;
	    case 2:
		if (freopen(argv[1], "r", stdin) == (FILE *)NULL) {
			perror(argv[1]);
			exit(1);
		}
		filename = argv[1];
		break;
	    default:
		usage();
	}

	/* Emit the initial boilerplate. */
	printf("/* This file was created automatically from %s by mkfb. */\n\n",
	    filename);
	printf("#if !defined(USE_APP_DEFAULTS) /*[*/\n\n");
	printf("#include <stdio.h>\n");
	printf("#include <X11/Intrinsic.h>\n\n");
	printf("static unsigned char fsd[] = {\n");

	/* Scan the file, emitting the fsd array and creating the indices. */
	while (fgets(buf, BUFSZ, stdin) != (char *)NULL) {
		char *s = buf;
		int sl;
		char c;
		int white;

		lno++;

		/* Skip leading white space. */
		while (isspace(*s))
			s++;

		/* Remove trailing white space. */
		while ((sl = strlen(s)) && isspace(s[sl-1]))
			s[sl-1] = '\0';

		if (continued)
			goto emit_text;

		/* Skip comments and empty lines. */
		if (!*s || *s == '!')
			continue;

		/* Check for simple if[n]defs. */
		if (*s == '#') {
			if (!strcmp(s, "#ifdef COLOR")) {
				if (mode != BOTH) {
					fprintf(stderr,
					    "%s, line %d: Nested #ifdef\n",
					    filename, lno);
					exit(1);
				}
				mode = COLOR_ONLY;
			} else if (!strcmp(s, "#ifndef COLOR")) {
				if (mode != BOTH) {
					fprintf(stderr,
					    "%s, line %d: Nested #ifndef\n",
					    filename, lno);
					exit(1);
				}
				mode = MONO_ONLY;
			} else if (!strcmp(s, "#else")) {
				switch (mode) {
				    case BOTH:
					fprintf(stderr,
					    "%s, line %d: Missing #if[n]def\n",
					    filename, lno);
					exit(1);
					break;
				    case COLOR_ONLY:
					mode = MONO_ONLY;
					break;
				    case MONO_ONLY:
					mode = COLOR_ONLY;
					break;
				}
			} else if (!strcmp(s, "#endif")) {
				if (mode == BOTH) {
					fprintf(stderr,
					    "%s, line %d: Missing #if[n]def\n",
					    filename, lno);
					exit(1);
				}
				mode = BOTH;
			} else {
				fprintf(stderr,
				    "%s, line %d: Unrecognized # directive\n",
				    filename, lno);
				exit(1);
			}
			continue;
		}

		/* Add array offsets. */
		switch (mode) {
		    case BOTH:
			if (n_color >= ARRSZ || n_mono >= ARRSZ) {
				fprintf(stderr,
				    "%s, line %d: Buffer overflow\n",
				    filename, lno);
				exit(1);
			}
			a_color[n_color++] = cc;
			a_mono[n_mono++] = cc;
			break;
		    case COLOR_ONLY:
			if (n_color >= ARRSZ) {
				fprintf(stderr,
				    "%s, line %d: Buffer overflow\n",
				    filename, lno);
				exit(1);
			}
			a_color[n_color++] = cc;
			break;
		    case MONO_ONLY:
			if (n_mono >= ARRSZ) {
				fprintf(stderr,
				    "%s, line %d: Buffer overflow\n",
				    filename, lno);
				exit(1);
			}
			a_mono[n_mono++] = cc;
			break;
		}

		/* Emit the text. */
	    emit_text:
		continued = 0;
		white = 0;
		while (c = *s++) {
			if (c == ' ' || c == '\t')
				white++;
			else if (white) {
				emit(' ');
				cc++;
				white = 0;
			}
			switch (c) {
			    case ' ':
			    case '\t':
				break;
			    case '#':
				emit('\\');
				emit('#');
				cc += 2;
				break;
			    case '\\':
				if (*s == '\0') {
					continued = 1;
					break;
				}
				/* else fall through */
			    default:
				emit(c);
				cc++;
				break;
			}
		}
		if (white) {
			emit(' ');
			cc++;
			white = 0;
		}
		if (!continued) {
			emit(0);
			cc++;
		}
	}
	printf("};\n\n");

	/* Emit the fallback arrays themselves. */
	printf("String color_fallbacks[%u] = {\n", n_color + 1);
	for (i = 0; i < n_color; i++)
		printf("\t(String)&fsd[%u],\n", a_color[i]);
	printf("\t(String)NULL\n};\n\n");
	printf("String mono_fallbacks[%u] = {\n", n_mono + 1);
	for (i = 0; i < n_mono; i++)
		printf("\t(String)&fsd[%u],\n", a_mono[i]);
	printf("\t(String)NULL\n};\n\n");

	/* Emit some test code. */
	printf("%s", "#if defined(DEBUG) /*[*/\n\
main()\n\
{\n\
	int i;\n\
\n\
	for (i = 0; color_fallbacks[i]; i++)\n\
		printf(\"color %d: %s\\n\", i, color_fallbacks[i]);\n\
	for (i = 0; mono_fallbacks[i]; i++)\n\
		printf(\"mono %d: %s\\n\", i, mono_fallbacks[i]);\n\
	exit(0);\n\
}\n");
	printf("#endif /*]*/\n\n");
	printf("#endif /*]*/\n");

	exit(0);
}

int n_out = 0;

void
emit(c)
char c;
{
	if (n_out >= 19) {
		printf("\n");
		n_out = 0;
	}
	printf("%3d,", (unsigned char)c);
	n_out++;
}
