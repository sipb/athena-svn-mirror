#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef XPG3
#include <stdlib.h>
#else    
#include <search.h>    
#endif
#include "transcript.h"
#include "afmdit.h"    

extern double atof();

FILE *afm, *dit, *aux, *map;

char *afmfile;
char *mapfile;
char *ditfile;
char *auxfile;

int ligs=TRUE;

char ditname[2];
char psname[256];

int scale = 5;
int fudge = 10;
int spacewidth;

int special = 0;
int isfixedpitch = FALSE;
int istext = TRUE;

long location;

double capheight;
double xheight;
double descender;
double ascender;


#define ARGS "a:m:o:x:n"

static void Usage() {
    fprintf(stderr,"Usage: afmdit -a afmfile -m mapfile -o outputfile -x auxiliary [-n]\n");
    exit(1);
}

static void AddToLigs(ligature)
    char *ligature;
{
    if (!strcmp(ligature,"ff"))
	whichligs[FF]++;
    else if (!strcmp(ligature,"fi"))
	whichligs[FI]++;
    else if (!strcmp(ligature,"fl"))
	whichligs[FL]++;
    else if (!strcmp(ligature,"ffi"))
	whichligs[FFI]++;
    else if (!strcmp(ligature,"ffl"))
	whichligs[FFL]++;
}
    


static int ParseMetrics(line)
    char *line;
{
    char *p, *q;
    char *tok;
    float wid, lx, ly, ux, uy;

    sscanf(line, "C %d ; WX %f ; N %s ; B %f %f %f %f ;",
      &characters[ncharacters].code, &wid, characters[ncharacters].name,
      &lx, &ly, &ux, &uy);
    if (characters[ncharacters].code < 0)
	return 1;
    q = line;
    if (!strcmp(characters[ncharacters].name, "space")) 
	spacewidth = (int) (0.5 + (wid / scale));
    characters[ncharacters].width = wid;
    characters[ncharacters].bbllx = lx;
    characters[ncharacters].bblly = ly;
    characters[ncharacters].bburx = ux;
    characters[ncharacters].bbury = uy;
    while ((p = strchr(q, 'L')) != NULL) {
	p--;
	p--;
	if (*p != ';')
	    break;
	p += 4;
	tok = p;
	p = strchr(tok, ' ');
	*p = '\0';
	p++;			/* skip over successor */
	tok = p;
	p = strchr(tok, ' ');
	*p = '\0';
	p++;
	AddToLigs(tok);
	q = p;
    }
    ncharacters++;
    if (ncharacters == MAXCHARS) {
	fprintf(stderr, "afmdit: too many characters in font!.\n");
	return 0;
    }
    return 1;
}

int ditcompar(a,b)
#ifdef __STDC__
    const void *a, *b;
#else
    void *a, *b;
#endif
{
    return strcmp(((struct ditmapentry *)a)->pname,
		  ((struct ditmapentry *)b)->pname);
}

int asccompar(a,b)
#ifdef __STDC__
    const void *a, *b;
#else
    void *a, *b;
#endif
{
    return strcmp(*(char **)a,*(char **)b);
}

/*
static int SearchDitMap(name)
    char *name;
{
    int i;
    int u,l;
    int comp;

    u = NDITMAP;
    l = 0;
    while (u >= l) {
	i = (l+u)/2;
	comp = strcmp(ditmap[i].pname,name);
	if (comp == 0)
	    return i;
	if (comp < 0)
	    l = i+1;
	else if (comp > 0)
	    u = i -1;
    }
    return -1;
}
*/

static int SearchMathOnly(name)
    char *name;
{
    int i;

    for (i=0; i< NMATHONLY; i++) {
	if (!strcmp(mathonly[i].shortname,name))
	    return i;
    }
    return -1;
}

static void OutputCharSet()
{
    int i;
    int scaledwidth;
    int ad;
    struct ditmapentry *ditptr;
    struct ditmapentry dummy;
    char *p, *q;
    char syn[10];
    int first;
    int em6, em12;
    char **asc;

    if (isfixedpitch == 0) {
	em6 = (int) (0.5 + (1000.0 / 6.0) / scale);
	em12 = (int) (0.5 + (1000.0 / 12.0) / scale);
    }
    else {
	em6 = em12 = spacewidth;
    }
    fprintf(dit, "spacewidth %d\n", spacewidth);
    fprintf(dit, "charset\n");
    fprintf(dit, "\\|\t%d 0 000\t1/6 em space\n", em6);
    fprintf(dit, "\\^\t%d 0 000\t1/12 em space\n", em12);
    fprintf(dit, "\\&\t00 0 000\tno space\n");

    for (i = 0; i < ncharacters; i++) {
	if (!strcmp(characters[i].name, "space") ||
	  !strcmp(characters[i].name, "asciitilde") ||
	  !strcmp(characters[i].name, "asciicircum") ||
	  (!strcmp(characters[i].name, "bar") && !special)) {
	    fprintf(aux, "%d %d 0\n", characters[i].code, (int) characters[i].width);
	    continue;
	}
	scaledwidth = (int) (0.5 + characters[i].width / scale);
	if (scaledwidth < 0)
	    scaledwidth = 0;
	if (scaledwidth > 256)
	    scaledwidth = 256;
	ad = 0;
	if ((characters[i].bblly - fudge) <= descender)
	    ad += 1;
	if ((characters[i].bbury + fudge) >= ascender)
	    ad += 2;
	dummy.pname = characters[i].name;
	p = characters[i].name;
	asc = (char **) bsearch((void *)&p, (void *)
			       asciitable, NASCII, sizeof(char *), asccompar);
	if (asc != NULL) {
	    fprintf(dit, "%c\t", characters[i].code);
	    fprintf(dit, "%d %d 0%o\t%s\n", scaledwidth, ad,
	      characters[i].code, characters[i].name);
	    ditptr = (struct ditmapentry *) bsearch((void *) &dummy, (void
		*) ditmap, NDITMAP, sizeof(struct ditmapentry), ditcompar);
	    if (ditptr != NULL) {
		p = ditptr->dnames;
		q = syn;
		while (*p != '\0') {
		    if (*p != ' ') {
			*q = *p;
			q++;
		    }
		    else {
			*q = '\0';
			/* got the whole synonym now */
			if (SearchMathOnly(syn) == -1 || special)
			    fprintf(dit, "%s\t\"\n", syn);
			q = syn;
		    }
		    p++;
		}
		*q = '\0';
		if (SearchMathOnly(syn) == -1 || special)
		    fprintf(dit, "%s\t\"\n", syn);
	    }
	}
	else {
	    ditptr = (struct ditmapentry *) bsearch((void *) &dummy, (void
		*) ditmap, NDITMAP, sizeof(struct ditmapentry), ditcompar);
	    if (ditptr != NULL) {
		p = ditptr->dnames;
		first = TRUE;
		while (*p != '\0') {
		    if (*p != ' ') {
			fprintf(dit, "%c", *p);
		    }
		    else {
			if (first) {
			    first = FALSE;
			    fprintf(dit, "\t%d %d 0%o\t%s\n", scaledwidth, ad,
			      characters[i].code, characters[i].name);
			}
			else
			    fprintf(dit, "\t\"\n");
		    }
		    p++;
		}
		if (first)
		    fprintf(dit, "\t%d %d 0%o\t%s\n", scaledwidth, ad,
		      characters[i].code, characters[i].name);
		else
		    fprintf(dit, "\t\"\n");
	    }
	}
	fprintf(aux, "%d %d 0\n", characters[i].code, (int) characters[i].width);
    }
}

static char *ligtostr(i)
    int i;
{
    switch (i) {
    case FF:
	return "ff";
	break;
    case FI:
	return "fi";
	break;
    case FL:
	return "fl";
	break;
    case FFI:
	return "ffi";
	break;
    case FFL:
	return "ffl";
	break;
    }
}

static void OutputLigatures()
{
    int i;
    int first = TRUE;

    for (i = 0; i<5; i++) {
	if (whichligs[i]) {
	    if (first) {
		first = FALSE;
		fprintf(dit,"ligatures ");
		fprintf(dit,"%s ", ligtostr(i));
	    }
	    else {
		fprintf(dit,"%s ", ligtostr(i));
	    }
	}
    }
    if (!first)
	fprintf(dit,"0\n");
}

static void OutputFudged()
{
    int i;
    int fwid;
    
    for (i = 0; i < NPROC; i++) {
	fwid = (int) (0.5 + (proc[i].width/scale));
	if (istext && !proc[i].special) {
	    fprintf(dit,"%s\t%d %d 0%o\tfudgedproc!\n", proc[i].name, fwid,
		    proc[i].kerncode, proc[i].ccode);
	    fprintf(aux,"%d %d 1\n",proc[i].ccode, (int) proc[i].width);
	}
	else if (special == 1 && proc[i].special) {
	    fprintf(dit,"%s\t%d %d 0%o\tfudgedproc!\n", proc[i].name, fwid,
		    proc[i].kerncode, proc[i].ccode);
	    fprintf(aux,"%d %d 1\n",proc[i].ccode, proc[i].width);
	}
    }
}
	    
	

main(argc,argv)
    int argc;
    char **argv;
{
    int c;
    char buf[BUFSIZ];
    char *p,*q;

    if (argc == 1)
	Usage();

    while ((c = getopt(argc, argv, ARGS)) != EOF) {
	switch (c) {
	    case 'a':
		afmfile = optarg;
		break;
	    case 'm':
		mapfile = optarg;
		break;
	    case 'o':
		ditfile = optarg;
		break;
	    case 'x':
		auxfile = optarg;
		break;
	    case 'n':
		ligs=FALSE;
		break;
	    default:
		Usage();
		break;
	}
    }
    if (!afmfile) {
	fprintf(stderr, "afmdit: must specify AFM file.\n");
	exit(1);
    }
    if (!mapfile) {
	fprintf(stderr, "afmdit: must specify map file.\n");
	exit(1);
    }
    if (!ditfile) {
	fprintf(stderr, "afmdit: must specify output file.\n");
	exit(1);
    }
    if (!auxfile) {
	fprintf(stderr, "afmdit: must specify auxiliary output file.\n");
	exit(1);
    }

    if ((afm = fopen(afmfile, "r")) == NULL) {
	fprintf(stderr, "afmdit: couldn't open AFM file %s.\n", afmfile);
	exit(1);
    }
    if ((map = fopen(mapfile, "r")) == NULL) {
	fprintf(stderr, "afmdit: couldn't open map file %s.\n", mapfile);
	exit(1);
    }
    if ((dit = fopen(ditfile, "w")) == NULL) {
	fprintf(stderr, "afmdit: couldn't open output file %s.\n", ditfile);
	exit(1);
    }
    if ((aux = fopen(auxfile, "w")) == NULL) {
	fprintf(stderr, "afmdit: couldn't open auxiliary output file %s.\n", auxfile);
	exit(1);
    }

    while (fgets(buf, BUFSIZ, map)) {
	p = strchr(buf,'\n');
	if (p) *p = '\0';
	if (!strcmp(buf,"special")) {
	    special = TRUE;
	    istext = FALSE;
	    break;
	}
	else {
	    strncpy(psname,buf,256);
	}
    }
    fclose(map);

    p = strrchr(mapfile,'/');
    if (p == NULL)
	p = mapfile;
    else
	p++;
    q = strchr(p,'.');
    if (q) *q = '\0';
    strncpy(ditname,p,2);

    if (!strcmp(ditname,"SS"))
	special = 2;

    fprintf(dit,"# %s\n",psname);
    fprintf(dit,"name %s\n\n",ditname);
    if (special)
	fprintf(dit,"special\n");

    /* handle global font info */
    
    while (fgets(buf,BUFSIZ,afm)) {
	p = strrchr(buf,'\n');
	if (p) *p = '\0';
	if (!strncmp("Comment Copyright",buf,17)) 
	    fprintf(dit,"# %s\n",buf);
	if (!strncmp("FontName",buf,8)) {
	    p = strchr(buf,' ');
	    p++;
	    fprintf(dit,"# PostScript %s from %s\n",p,afmfile);
	    fprintf(dit,"# PostScript is a trademark of Adobe Systems Incorporated\n");
	}
	if (!strncmp("IsFixedPitch",buf,12)) {
	    p = strchr(buf,' ');
	    p++;
	    if (!strcmp(p,"true"))
		isfixedpitch = TRUE;
	}
	if (!strncmp("CharWidth",buf,9))
	    isfixedpitch = TRUE;
	if (!strncmp("Notice",buf,6))
	    fprintf(dit,"# %s\n",buf);
	if (!strncmp("CapHeight",buf,9)) {
	    p = strchr(buf,' ');
	    p++;
	    capheight = atof(p);
	}
	if (!strncmp("XHeight",buf,7)) {
	    p = strchr(buf,' '); p++;
	    xheight = atof(p);
	}
	if (!strncmp("Descender",buf,9)) {
	    p = strchr(buf,' '); p++;
	    descender = atof(p);
	}
	if (!strncmp("Ascender",buf,8)) {
	    p = strchr(buf,' '); p++;
	    ascender = atof(p);
	}
	if (!strncmp("StartCharMetrics",buf,16)) {
	    break;
	}
    }
    if (capheight < ascender)
	ascender = capheight;
    while (fgets(buf,BUFSIZ,afm)) {
	if (!strncmp("EndCharMetrics",buf,14))
	    break;
	p = strrchr(buf,'\n');
	if (p) *p = '\0';
	if (!ParseMetrics(buf)) {
	    fprintf(stderr,"afmdit: error parsing AFM file.\n");
	    exit(1);
	}
    }
    if (ligs)
	OutputLigatures();
    OutputCharSet();
    OutputFudged();
    fclose(dit);
}
