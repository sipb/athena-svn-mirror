#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psnup.c,v 1.1.1.1 1996-10-07 20:25:51 ghudson Exp $";
#endif
/* psnup.c
 *
 *
 * Copyright (C) 1990,1991,1992 Adobe Systems Incorporated.  All rights
 *  reserved. 
 * GOVERNMENT END USERS: See Notice File in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * n x n printing
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.10  1994/02/16  00:31:00  snichols
 * support for Orientation comment.
 *
 * Revision 3.9  1993/12/21  23:00:14  snichols
 * correct %%Pages comment in output.
 *
 * Revision 3.8  1993/04/20  22:16:50  snichols
 * should close the prolog file when we're done, and should make sure
 * we only include the prolog once.
 *
 * Revision 3.7  1993/01/13  22:59:45  snichols
 * another uninitialized variable.
 *
 * Revision 3.6  1992/12/02  00:52:18  snichols
 * need to set ndex before using it.
 *
 * Revision 3.5  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.4  1992/07/14  22:38:39  snichols
 * Updated copyright.
 *
 * Revision 3.3  1992/07/02  23:07:35  snichols
 * xmax, ymax need to be floats.
 *
 * Revision 3.2  1992/06/02  18:26:31  snichols
 * support for -d option.
 *
 * Revision 3.1  1992/05/22  20:59:53  snichols
 * Now uses defaults if a printer name is supplied but there's no ppd
 * file.  Also added -w and -h switch, to allow page size to be
 * any size. (Page size, not paper size).
 *
 * Revision 3.0  1991/06/17  16:46:10  snichols
 * Release3.0
 *
 * Revision 1.9  1991/04/19  23:21:55  snichols
 * typo: should have %%IncludeFeature: *PageSize instead of
 * %%IncludeFeature: *PaperSize.
 *
 * Revision 1.8  1991/03/28  23:47:32  snichols
 * isolated code for finding PPD files to one routine, in psutil.c.
 *
 * Revision 1.7  1991/03/21  18:53:15  snichols
 * if printername is > 10 characters and can't find ppd, truncate to 10 and
 * check again.
 *
 * Revision 1.6  1991/02/20  16:53:48  snichols
 * mismatch in declaration: formal was int, actual was char *.
 *
 * Revision 1.5  91/02/19  16:42:09  snichols
 * added support for -n 1; this allows documents formatted for one size
 * paper to be printed on another size paper.  Cleaned up the paper size
 * DSC comment handling as well.
 * 
 * Revision 1.4  91/01/25  16:36:00  snichols
 * Oops, still had hard wired pathname for prolog.
 * 
 * Revision 1.3  90/12/12  10:28:11  snichols
 * new configuration stuff.
 * 
 * Revision 1.2  90/11/16  14:42:25  snichols
 * Support for other page sizes.  NxM printing.
 * 
 * Revision 1.1  90/10/29  14:05:26  snichols
 * Initial revision
 * 
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "transcript.h"
#include "config.h"

extern char *optarg;
extern int optind, opterr;

struct paper {
    float paperwidth;
    float paperlength;
    float imagexorig;
    float imageyorig;
    float imagewidth;
    float imagelength;
};

struct page {
    float width;
    float height;
} pagesize;

float width;
float length;
float xorig;
float yorig;

struct paper paperSizes[5] = {
    {612, 792, 18, 8, 582, 776},
    {612, 1008, 15, 8, 582, 992},
    {595, 842, 13, 10, 564, 822},
    {516, 729, 21, 10, 479, 705},
    {0,0,0,0,0,0}
};

#define LETTER 0
#define LEGAL 1
#define A4 2
#define B5 3
#define OTHER 4

int paperndx = LETTER;
char *papersize = "Letter";
int useDefault = FALSE;

char *printername;


/* scale factor */
float scale;

/* defaults - 4 up (2x2), not rotated */
int nrows = 2;
int ncolumns = 2;
int rotate = FALSE;
int urotate = FALSE;

int gaudy = FALSE;
int noscaleup = FALSE;


private void Usage()
{
    fprintf(stderr,"psnup [-n number[xnumber]] [-S size] [-s size] [-G] [-p outputfilename] filename\n");
    exit(0);
}

private int checkpower(i)
    int i;
{
    /* returns powers of 2 above 0 */
    int result;
    int rem;
    int count = 0;

    result = i;
    while (result != 0) {
	count++;
	rem = result % 2;
	result = result/2;
	if (result == 1 && (rem == 0) ) return count;
    }
    return -1;
}

private int strtondx(s)
    char *s;
{
    if (!strcmp(s, "Legal"))
	return LEGAL;
    if (!strcmp(s, "Letter"))
	return LETTER;
    if (!strcmp(s, "A4"))
	return A4;
    if (!strcmp(s,"B5"))
	return B5;
    /* if no match, return OTHER */
    return OTHER;
}

private char *ndxtostr(i)
    int i;
{
    switch (i) {
	case LETTER:
	    return "Letter";
	    break;
	case LEGAL:
	    return "Legal";
	    break;
	case A4:
	    return "A4";
	    break;
	case B5:
	    return "B5";
	    break;
    }
}

static int HandlePPD(printername, size, fp)
    char *printername;
    char *size;
    FILE **fp;
{
    int ndex;

    ndex = strtondx(size);
    if (printername == NULL) {
	if (ndex == OTHER) {
	    fprintf(stderr,
		    "psnup: must supply a printer name to use %s size\n",
		    size);
	    exit(1);
	}
	else return FALSE;
    }
    else {
	*fp = GetPPD(printername);
	if (*fp == NULL) {
	    fprintf(stderr,"psnup: couldn't open PPD file for printer %s,",
		    printername);
	    fprintf(stderr, " using defaults\n");
	    return FALSE;
	}
	return TRUE;
    }
}
	    
	

static void GetDimensions(fp, size, w, h)
    FILE *fp;
    char *size;
    float *w, *h;
{
    char *value;

    value = (char *) parseppd(fp, "*PaperDimension", size);
    if (value)
	sscanf(value, "%f %f", w, h);
    else {
	fprintf(stderr, "Couldn't find dimensions for size %s", size);
	exit(1);
    }
}

/* read page size and imageable area info from ppd file */
private void GetPageSizeInfo(size, def)
    char *size;
    int def;
{
    char *value;
    FILE *fptr;
    float xmax, ymax;
    int ndex;
    int useppd;

    ndex = strtondx(size);
    useppd = HandlePPD(printername, size, &fptr);
    if (useppd) {
	if (def) {
	    /* check default */
	    value = (char *) parseppd(fptr,"*DefaultPageSize", NULL);
	    if (value) {
		if (!strcmp(size, value))
		    useDefault = TRUE;
	    }
	    rewind(fptr);
	}
	GetDimensions(fptr, size, &paperSizes[ndex].paperwidth,
		      &paperSizes[ndex].paperlength); 
	rewind(fptr);
	value = (char *) parseppd(fptr, "*ImageableArea", size);
	if (value)
	    sscanf(value, "%f %f %f %f",
	      &paperSizes[ndex].imagexorig, &paperSizes[ndex].imageyorig,
		   &xmax, &ymax);
	paperSizes[ndex].imagewidth = xmax - paperSizes[ndex].imagexorig;
	paperSizes[ndex].imagelength = ymax - paperSizes[ndex].imageyorig;
	close(fptr);
    }
}

DoFixedProlog(output)
    FILE *output;
{
    FILE *pfp;
    char mybuf[BUFSIZ];
    char prolog[TSPATHSIZE];

    strncpy(prolog,PSLibDir,TSPATHSIZE);
    strncat(prolog,"psnup.pro",TSPATHSIZE);
    if ((pfp = fopen(prolog,"r")) == NULL) {
	fprintf(stderr,"psnup: couldn't open prolog file %s.\n",prolog);
	exit(1);
    }
    while (fgets(mybuf,255,pfp))
	fputs(mybuf,output);
    close(pfp);
}

static float ParseNum(s)
    char *s;
{
   char *p;

   if (p = strchr(s, 'i')) {
       /* inches, convert to points */
       *p = '\0';
       return atof(optarg) * 72.0;
   }
   else if (p = strchr(s, 'm')) {
       /* millimeters, convert to points */
       *p = '\0';
       return atof(optarg) * 72.0 / 254.0;
   }
   else
       return atof(optarg);
}

#define ARGS "P:d:p:n:S:s:w:h:rGf"

main(argc,argv)
    int argc;
    char **argv;
{

    FILE *ifp, *ofp;
    FILE *tfp;
    int row = 0;
    int column = 0;
    float x,y;
    float lscale,wscale;
    int i, c;
    char *outfile, *infile;
    char buf[255];
    int page = 0;
    int realpage = 0;
    int power;
    char *p,*q;
    int n;
    int prolog = FALSE;
    int sizecom = FALSE;
    char *tmpsize = NULL;
    int origpages;
    int newpages;

    if (argc == 1) {
	Usage();
	exit(1);
    }
    infile = outfile = NULL;
    pagesize.width = pagesize.height = 0;
    while ((c = getopt(argc,argv,ARGS)) != EOF) {
	switch (c) {
	case 'p':
	    outfile = optarg;
	    break;
	case 'r':
	    urotate = TRUE;
	    break;
	case 'n':
	    p = strchr(optarg,'x');
	    if (p) {
		*p = '\0';
		p++;
		ncolumns = atoi(p);
		nrows = atoi(optarg);
	    }
	    else {
		n = atoi(optarg);
		if (n == 1) {
		    /* special case */
		    rotate = FALSE;
		    nrows = ncolumns = 1;
		}
		else {
		    if ((power = checkpower(n)) == -1) {
			fprintf(stderr,"psnup: must supply either a power of 2 or rowsxcolumns.\n");
			exit(1);
		    }
		    if (power % 2) {
			rotate = TRUE;
			nrows = sqrt((double) n/2);
			ncolumns = sqrt((double) n*2);
		    }
		    else {
			rotate = FALSE;
			nrows = ncolumns = sqrt((double) n);
		    }
		}
	    }
	    break;
	case 'w':
	    pagesize.width = ParseNum(optarg);
	    break;
	case 'h':
	    pagesize.height = ParseNum(optarg);
	    break;
	case 'S':
	    if (islower(*optarg)) toupper(*optarg);
	    papersize = optarg;
	    paperndx = strtondx(optarg);
	    break;
	case 's':
	    tmpsize = optarg;
	    break;
	case 'd':
	case 'P':
	    printername = optarg;
	    break;
	case 'G':
	    gaudy = TRUE;
	    break;
	case 'f':
	    noscaleup = TRUE;
	default:
	    Usage();
	    break;
	}
    }
    if (urotate) rotate = urotate;
    GetPageSizeInfo(papersize, TRUE);
    if (pagesize.width == 0)
	pagesize.width = paperSizes[paperndx].paperwidth;
    if (pagesize.height == 0)
	pagesize.height = paperSizes[paperndx].paperlength;
    if (tmpsize) {
	if (HandlePPD(printername, tmpsize, &tfp)) {
	    GetDimensions(tfp, tmpsize, &pagesize.width, &pagesize.height);
	}
	else {
	    n = strtondx(tmpsize);
	    pagesize.width = paperSizes[n].paperwidth;
	    pagesize.height = paperSizes[n].paperlength;
	}
    }
    if (optind < argc)
	infile = argv[optind];

    if (infile) {
	if ((ifp = fopen(infile,"r")) == NULL) {
	    fprintf(stderr,"Couldn't open %s\n",infile);
	    exit(2);
	}
    }
    else ifp = stdin;
    if (outfile) {
	if ((ofp = fopen(outfile,"w")) == NULL) {
	    fprintf(stderr,"Couldn't open %s.\n",outfile);
	    exit(3);
	}
    }
    else ofp = stdout;

    if (rotate) {
	wscale = (float) ((paperSizes[paperndx].imagelength -
			   (ncolumns -1)*4.5)/ncolumns)/
			       pagesize.width; 
	lscale = (float) ((paperSizes[paperndx].imagewidth -
			   (nrows - 1)*4.5)/nrows)/
			       pagesize.height;
	if (wscale < lscale)
	    scale = wscale;
	else scale = lscale;
	width = nrows*scale*pagesize.height + (nrows -1)*4.5;
	length = ncolumns*scale*pagesize.width +
	    (ncolumns -1)*4.5;
	xorig = (paperSizes[paperndx].paperwidth - width)/2;
	yorig = (paperSizes[paperndx].paperlength - length)/2;
    }
    else {
	wscale = (float)((paperSizes[paperndx].imagewidth - (ncolumns -1)
			  *4.5)/ncolumns)/pagesize.width; 
	lscale = (float)((paperSizes[paperndx].imagelength - (nrows -1)
			  *4.5)/nrows)/pagesize.height;  
	if (wscale < lscale)
	    scale = wscale;
	else scale = lscale;
	length = nrows*scale*pagesize.height + (nrows -1)*4.5;
	width = ncolumns*scale*pagesize.width + (ncolumns -1)*4.5;
	yorig = (paperSizes[paperndx].paperlength - length)/2;
        xorig = (paperSizes[paperndx].paperwidth - width)/2;
    }


    row = nrows - 1;
    column = 0;
    if (!fgets(buf,255,ifp)) {
	fprintf(stderr,"psnup: empty input.\n");
	exit(1);
    }
    if (strncmp(buf,"%!PS-Adobe-",11)) {
	fprintf(stderr,"psnup: non-conforming input.\n");
	exit(1);
    }
    fputs(buf,ofp);
    /* redefine showpage */
    fprintf(ofp,"/PNshowpage /showpage load def\n /showpage { } def\n");

    while (fgets(buf,255,ifp)) {
	if (!strncmp(buf,"%%EndProlog",11)) {
	    if (!prolog) {
		prolog = TRUE;
		DoFixedProlog(ofp);
	    }
	}
	if (!strncmp(buf,"%%BeginFeature: *PageSize",25)) {
	    /* if it's not for the size we want, lose it! */
	    p = buf;
	    p += 26;
	    if (strncmp(p, papersize, strlen(papersize))) {
		while (fgets(buf, 255, ifp)) {
		    if (!strncmp(buf,"%%EndFeature", 12))
			break;
		}
		if (!useDefault) 
		    fprintf(ofp, "%%%%IncludeFeature: *PageSize %s\n", papersize);
		sizecom = TRUE;
		continue;
	    }
	    sizecom = TRUE;
	}
	if (!strncmp(buf,"%%IncludeFeature: *PageSize", 27)) {
	    /* again, if it's not for the size we want, get rid of it */
	    p = buf;
	    p += 28;
	    if (strncmp(p, papersize, strlen(papersize))) {
		if (!useDefault)
		    fprintf(ofp, "%%%%IncludeFeature: *PageSize %s\n", papersize);
		sizecom = TRUE;
		continue;
	    }
	    sizecom = TRUE;
	}
	if (!strncmp(buf, "%%Orientation:", 14)) {
	    /* if it doesn't match us, replace it */
	    p = buf + 15;
	    if (rotate) {
		if (strncmp(p, "Landscape", 9) != 0) {
		    fprintf(ofp, "%%%%Orientation: Landscape\n");
		}
	    }
	    else {
		if (strncmp(p, "Portrait", 9) != 0) {
		    fprintf(ofp, "%%%%Orientation: Portrait\n");
		}
	    }
	}
	if (!strncmp(buf, "%%EndSetup", 10) && !sizecom && !useDefault) {
	    /* if we reached here without a size comment and we're not */
	    /* the default page size, output a comment */
	    fprintf(ofp, "%%%%IncludeFeature: *PageSize %s\n", papersize);
	    sizecom = TRUE;
	}
	if (!strncmp(buf,"%%Page:",7)) {
	    if (!prolog) {
		prolog = TRUE;
		DoFixedProlog(ofp);
	    }
	    if (!sizecom && !useDefault) {
		/* still might not have seen or output a size comment! */
		fprintf(ofp, "%%%%IncludeFeature: *PageSize %s\n", papersize);
		sizecom = TRUE;
	    }
	    /* found a page boundary */
	    page++;
	    if ((page % (nrows*ncolumns)) == 1 || nrows*ncolumns == 1) {
		realpage++;
		if (page > 1) {
		    fprintf(ofp,"PNP restore\n"); 
		    fprintf(ofp,"PNEP\n");
		}
		fprintf(ofp,"%%%%Page: ? %d\n",realpage);
		if (rotate)
		    fprintf(ofp,"%7.4f %10.2f PNLS\n",scale,
			    -paperSizes[paperndx].paperwidth); 
		else
		    fprintf(ofp,"%7.4f PNSP\n",scale);
		row = nrows-1;
		column = 0;
	    }
	    if (rotate) {
		if (column == 0)
		    x = ((float) yorig )/scale;
		else
		    x = ((float) yorig + (float)column*length/ncolumns)/scale;
		if (row == 0)
		    y = ((float) xorig)/scale;
		else
		    y = ((float) xorig + (float)row*width/nrows)/scale;
	    }
	    else {
		if (column == 0)
		    x = ((float) xorig )/scale ;
		else
		    x = ((float) xorig  + (float)column*width/ncolumns)/scale;
		if (row == 0)
		    y = ((float) yorig )/scale ;
		else
		    y = ((float) yorig  + (float)row*length/nrows)/scale;
	    }
	    column++;
	    if (column == ncolumns) {
		column = 0;
		row--;
	    }
	    if ((page % (nrows*ncolumns)) != 1 && nrows*ncolumns != 1) fprintf(ofp,"PNP restore\n"); 
	    fprintf(ofp,"/PNP save def\n"); 
	    fprintf(ofp,"%7.4f %7.4f translate\n",x,y);
	    if (gaudy)
		fprintf(ofp,"%10.2f %10.2f PNBOX\n",pagesize.width,
			pagesize.height); 
	}
	else if (!strncmp(buf,"%%Trailer",9)) {
	    fprintf(ofp,"PNP restore\n"); 
	    fprintf(ofp,"PNEP\n");
	    fprintf(ofp,"%s",buf);
	}
	else if (!strncmp(buf, "%%Pages:", 8)) {
	    /* if it's not (atend), divide the number of pages by the
	       number of pages per sheet */
	    p = strchr(buf, '(');
	    if (p) {
		if (!strncmp(p, "(atend)", 7)) {
		    fprintf(ofp, "%s", buf);
		}
	    }
	    else {
		/* read the value */
		p = strchr(buf, ':');
		p++;
		while (*p == ' ') p++;
		q = strchr(p, '\n');
		if (q) *q = '\0';
		origpages = atoi(p);
		if (origpages % (nrows*ncolumns))
		    newpages = origpages/(nrows*ncolumns) + 1;
		else
		    newpages = origpages/(nrows*ncolumns);
		fprintf(ofp, "%%Pages: %d\n", newpages);
	    }
	}
	else fprintf(ofp,"%s",buf);
    }
    close(ofp);
}
	
