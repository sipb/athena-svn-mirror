#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1992  Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psdraft.c,v 1.1.1.1 1996-10-07 20:25:51 ghudson Exp $";
#endif
/* psdraft.c
 *
 * Copyright (C) 1992 Adobe Systems Incorporated. All
 * rights reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notic
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  1993/05/21  21:37:39  snichols
 * Added extern dec'l for strtod.
 *
 * Revision 1.5  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 1.4  1992/07/14  22:02:34  snichols
 * added copyright.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "transcript.h"
#include "config.h"

extern double strtod();

char *draftstring = "DRAFT";
float x = 575.0;
float y = 300.0;
float angle = 90.0;
float gray = 0.0;
int outline = FALSE;
float paperwidth = 612.0;

char fontname[100] = "Times-Roman";
float fontsize = 30;
char *printer = NULL;
char *outfile = NULL;
char *infile = NULL;

extern char *optarg;
extern int optind;

#define ARGS "s:x:y:r:f:g:op:P:d:"
static FILE *infp;
static FILE *outfp;

static void GetPaperWidth(str)
    char *str;
{
    char *p, *q;
    FILE *ppd;
    char *value;
    

    p = strchr(str, ' ');
    while (*p == ' ') p++;
    q = strchr(p, '\n');
    if (q) *q = '\0';

    ppd = GetPPD(printer);
    if (ppd != NULL) {
	value = parseppd(ppd, "*PaperDimension", p);
	if (value)
	    sscanf(value, "%f", &paperwidth);
    }
}

static void DecodeFont(name)
    char *name;
{
    char *d, *p;
    char *s;

    p = name;
    d = fontname;
    fontsize = 0;
    while (isascii(*p) && (isalpha(*p) || (*p == '-'))) {
	*d++ = *p++;
    }
    *d++ = '\0';
    fontsize = strtod(p, &s);
    if (*s || !fontsize || !fontname[0]) {
	fprintf(stderr, "psdraft: poorly formed font name & size: \"%s\"\n",
	  name);
	exit(1);
    }
}

static float ParseNum(s)
    char *s;
{
   char *p, *e;
   float x;
   float mult;

   if (p = strchr(s, 'i')) {
       /* inches, convert to points */
       *p = '\0';
       mult = 72.0;
   }
   else if (p = strchr(s, 'm')) {
       *p = '\0';
       mult = 72.0/254.0;
   }
   else {
       mult = 1.0;
   }

   x =  strtod(s, &e);
   if (*e) {
       fprintf(stderr, "psdraft: ill-formed number: %s.\n", e);
       exit(1);
   }
   return x * mult;
}




static void ProcessFile()
{
    char buf[1024];
    FILE *pro;
    char *tmp, *p;

    /* check type of file */
    fgets(buf, 1024, infp);
    if (strncmp(buf, "%!", 2) != 0) {
	fprintf(stderr, "psdraft: input must be a PostScript file!\n");
	return;
    }
    if (strncmp(buf, "%!PS-Adobe-", 11) != 0) {
	fprintf(stderr,
		"psdraft: input must be a conforming PostScript file.\n");
	return;
    }
    fputs(buf, outfp);

    /* insert our prolog */
    tmp = getenv("PSLIBDIR");
    if (tmp)
	strcpy(buf, tmp);
    else
	strcpy(buf, PSLibDir);
    strcat(buf, "/psdraft.pro");
    if ((pro = fopen(buf, "r")) == NULL) {
	fprintf(stderr, "psdraft: couldn't open prolog file %s...quitting.\n",
		buf);
	exit(1);
    }
    while (fgets(buf, 1024, pro))
	fputs(buf, outfp);

    /* now back to file */
    while (fgets(buf, 1024, infp)) {
	if ((strncmp(buf, "IncludeFeature:", 15) == 0) ||
	    strncmp(buf, "Feature:", 7) == 0) {
	    /* might give page size */
	    tmp = strchr(buf, ':');
	    tmp++;
	    while (*tmp == ' ') tmp++;
	    if (strncmp(tmp, "*PageSize", 8) == 0) {
		/* found page size */
		GetPaperWidth(tmp);
	    }
	}
	if (strncmp(buf, "%%Page:", 7) == 0) {
	    fputs(buf, outfp);
	    /* insert call to proc */
	    fprintf(outfp,
		    "(%s) %s %4.2f %8.3f %5.1f %8.3f %8.3f %5.1f /%s DoDraft\n",
		    draftstring, outline ? "true" : "false", gray,
		    -paperwidth, angle, x, y, fontsize, fontname);
	    continue;
	}
	fputs(buf, outfp);
    }
}


main(argc, argv)
    int argc;
    char **argv;
{
    char buf[1024];
    int c;


    while ((c = getopt(argc, argv, ARGS)) != EOF) {
	switch (c) {
	case 's':
	    draftstring = optarg;
	    break;
	case 'x':
	    x = ParseNum(optarg);
	    break;
	case 'y':
	    y = ParseNum(optarg);
	    break;
	case 'r':
	    angle = ParseNum(optarg);
	    break;
	case 'f':
	    DecodeFont(optarg);
	    break;
	case 'g':
	    gray = ParseNum(optarg);
	    break;
	case 'o':
	    outline = TRUE;
	    break;
	case 'p':
	    outfile = optarg;
	    break;
	case 'P':
	case 'd':
	    printer = optarg;
	    break;
	}
    }

    if (outfile) {
	if (strcmp(outfile, "-") == 0) {
	    outfp = stdout;
	}
	else {
	    if ((outfp = fopen(outfile, "w")) == NULL) {
		fprintf(stderr, "psdraft: couldn't open %s.\n", outfile);
		exit(1);
	    }
	}
    }
    else
	outfp = stdout;

    if (argc > optind) {
	for (;optind < argc; optind++) {
	    infile = argv[optind];
	    if ((infp = fopen(infile, "r")) == NULL) {
		fprintf(stderr, "psdraft: couldn't open %s.\n", infile);
		continue;
	    }
	    ProcessFile();
	}
    }
    else {
	infp = stdin;
	ProcessFile();
    }
}
	    
    
    

    
