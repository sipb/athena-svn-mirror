#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987,1990,1991,1992  Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/enscript.c,v 1.2 1996-10-14 04:57:22 ghudson Exp $";
#endif
/* enscript.c
 *
 * Copyright (C) 1985,1987,1990,1991,1992 Adobe Systems Incorporated. All
 * rights reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * inspired by Gosling's cz
 * there have been major overhauls, but the input language is the same:
 * 	new widths format
 *	generate PostScript (much easier than Press)
 *	new and renamed switches (to match 4.2bsd lpr spooler)
 *	obeys PostScript comment conventions
 *	doesn't worry so much about fonts (everything is scalable and
 *	rotatable, use PS font names, no face properties)
 *
 * enscript generates POSTSCRIPT print files of a special kind
 * the coordinate system is in 20ths of a point. (1440 per inch)
 *
 * RCSLOG:
 * $/Log: enscript.c,v/$
 * Revision 3.24  1994/03/22  22:13:45  snichols
 * typo in variable name.
 *
 * Revision 3.23  1994/02/16  00:31:14  snichols
 * support for encoding vectors, courtesy of Antony Simmins, and
 * support for Orientation comment.
 *
 * Revision 3.22  1994/01/04  23:21:41  snichols
 * if we're not going to treat spaces specially, then we don't need this
 * conditional at all, and it was mucking up parentheses.
 *
 * Revision 3.21  1993/12/01  21:04:53  snichols
 * don't separate out command line args by system type; accept 'em all.
 *
 * Revision 3.20  1993/09/15  21:02:41  snichols
 * keep blanks at beginning of line in string; let PostScript place them.
 *
 * Revision 3.19  1993/09/09  16:16:59  snichols
 * string should be null-terminated.
 *
 * Revision 3.18  1993/04/28  17:28:57  snichols
 * make compiler happy.
 *
 * Revision 3.17  1992/11/25  20:27:22  snichols
 * forgot to remove debugging statement.
 *
 * Revision 3.16  1992/11/25  20:16:29  snichols
 * Should use DefaultPageSize as specified in the PPD file.
 *
 * Revision 3.15  1992/11/23  23:16:40  snichols
 * imageable area numbers may be floats.
 *
 * Revision 3.14  1992/11/23  21:33:53  snichols
 * fixed problem with first column being smaller than subsequent columns.
 *
 * Revision 3.13  1992/11/10  17:13:06  snichols
 * stop complaining about multiple AFM files.
 *
 * Revision 3.12  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.11  1992/07/27  20:01:02  snichols
 * If we're spooling, then use PPD file for printer specified by PRINTER
 * environment variable, if printer isn't specified on command line.  Still
 * don't use that printer's PPD file if we're not spooling.
 *
 * Revision 3.10  1992/07/14  22:57:54  snichols
 * Updated copyright.
 *
 * Revision 3.9  1992/07/14  21:58:22  snichols
 * Updated copyright.
 *
 * Revision 3.8  1992/07/02  20:49:43  snichols
 * missing a rewind before a call to parseppd, and the default string
 * wasn't getting reinitialized each time through the loop.
 *
 * Revision 3.7  1992/06/04  18:31:07  snichols
 * fixed comments after endif, got rid of unnecessary dec'l for
 * getpwuid.
 *
 * Revision 3.6  1992/05/18  20:42:25  snichols
 * Allow escapes in user-supplied header for inserting file name, date, and
 * page number.
 *
 * Revision 3.5  1992/03/24  20:52:51  snichols
 * Properly scale lower left coordinates of imageable area.
 *
 * Revision 3.4  1992/01/22  18:04:05  snichols
 * use ppd file for imageable area whenever one is supplied, and fix maxX bug.
 *
 * Revision 3.3  1992/01/16  22:38:51  snichols
 * modified enscript to try and open more than one AFM file, if more than
 * one name is available, and the first one can't be opened.
 *
 * Revision 3.2  1991/10/29  00:47:35  snichols
 * long-standing bug with ^M and ^I; ^M should reset col count.
 *
 * Revision 3.1  1991/10/02  19:21:33  snichols
 * fixed bug where truncation and limiting output PS lines to 255
 * were interacting poorly.
 *
 * Revision 3.0  1991/06/17  16:33:58  snichols
 * release 3.0
 *
 * Revision 2.26  1991/06/13  22:34:54  snichols
 * wrong file name on error message concerning inability to open
 * output file.
 *
 * Revision 2.25  1991/06/12  00:10:29  snichols
 * should include time.h, not sys/time.h.
 *
 * Revision 2.24  1991/06/06  21:32:58  snichols
 * use lpr rather than pslpr to spool; otherwise, page reversal doesn't
 * happen in spooler.
 * Also, stopped overwriting characters with tabs and variable width
 * fonts; now, if overwrite would occur, move over a space, and notify
 * the user, suggesting the use of -T to adjust tab width.
 *
 * Revision 2.23  1991/05/22  22:21:51  snichols
 * error message reporting wrong file name.
 *
 * Revision 2.22  1991/05/14  19:19:55  snichols
 * break up lines into multiple shows if line will exceed 255 characters,
 * since 255 is the max line length for conforming document.
 *
 * Revision 2.21  1991/05/01  22:06:01  snichols
 * Sigh.  Use -v number for columns, rather than -number.  Keep
 * backward compatibility with -2, -1.  Can't handle general -number
 * case, because with getopt, can't distinguish between -2 -2 and -22.
 *
 * Revision 2.20  1991/04/29  22:51:12  snichols
 * fall back to showchar code if we encounter esc-F.
 *
 * Revision 2.19  1991/03/28  23:47:32  snichols
 * isolated code for finding PPD files to one routine, in psutil.c.
 *
 * Revision 2.18  1991/03/27  00:16:30  snichols
 * got rid of -X switch; decided to always proceed even without
 * PPD file.
 *
 * Revision 2.17  1991/03/21  18:53:15  snichols
 * if printername is > 10 characters and can't find ppd, truncate to 10 and
 * check again.
 *
 * Revision 2.16  1991/03/20  22:28:51  snichols
 * added -X switch to usage.
 *
 * Revision 2.15  1991/03/01  14:03:29  snichols
 * Use default resource path in ListPSResourceFiles.
 * Allow continued execution despite some errors with -X switch.
 *
 * Revision 2.14  91/02/19  16:44:41  snichols
 * added new resource-handling package for locating AFM files.
 * 
 * Revision 2.13  91/02/04  11:15:14  snichols
 * check NULL return from mapname rather than checking for NULL parameter,
 * since mapname doesn't set the parameter to NULL, just the first byte.
 * 
 * Revision 2.12  91/01/23  15:12:36  snichols
 * Print error if mapname fails to find font name.
 * 
 * Revision 2.11  91/01/18  15:21:17  snichols
 * now reports total number of pages with %%Pages comment.
 * 
 * Revision 2.10  90/12/17  13:46:19  snichols
 * Uses bindir defined in config.h to find pslpr.
 * 
 * Revision 2.9  90/12/17  13:31:44  snichols
 * Fixed LinesLeft bug; sometimes LinesLeft was not getting reset properly
 * in InitPage, resulting in spurious pagebreaks.
 * 
 * Revision 2.8  90/12/12  09:58:46  snichols
 * New configuration stuff.
 * 
 * Revision 2.7  90/12/03  16:50:32  snichols
 * performance enhancements
 * 
 * Revision 2.6  90/11/16  14:49:08  snichols
 * Optimized normal case for printing characters; 30% speedup.  Changes
 * for XPG3 compliance.
 * 
 * Revision 2.4  90/10/04  15:11:31  snichols
 * Added support for all printer features in a general way, and did 
 * some cleanup.
 * 
 * Revision 2.2  87/11/17  16:49:37  byron
 * Release 2.1
 * 
 * Revision 2.1.1.11  87/11/12  13:39:58  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.10  87/09/16  11:23:23  byron
 * Changed right margin in first column of two-column output.
 * 
 * Revision 2.1.1.9  87/06/19  18:47:09  byron
 * Added back line truncation (from 2.1.1.4) with -c option.  Also added
 * report of number of lines wrapped.
 * 
 * Revision 2.1.1.8  87/06/04  11:25:15  byron
 * Type problems caught by customer compiler (courtesy Brian Reid).
 * 
 * Revision 2.1.1.7  87/04/23  10:25:09  byron
 * Copyright notice.
 * 
 * Revision 2.1.1.6  87/04/22  10:03:26  byron
 * Normal exit now returns 0 explicitly.
 * 
 * Revision 2.1.1.5  87/03/19  09:12:08  byron
 * Added Ivor Durham's line-wrap mods, which replace the line truncation
 * that happens now.
 * 
 * Revision 2.1.1.4  86/06/02  10:06:40  shore
 * fixed behavior on formfeeds and some newlines
 * 
 * Revision 2.1.1.3  86/05/14  18:27:23  shore
 * fixed column computation on backspace
 * 
 * Revision 2.1.1.2  86/04/10  14:43:27  shore
 * fixed parsing of ENSCRIPT environment variable
 * fixed handling of -p and -q (BeQuiet and OutOnly)
 * fixed handling of unreadable/non-existent input files
 * 
 * Revision 2.1.1.1  86/03/20  08:55:30  shore
 * fix to parse newer AFM file format
 * 
 * Revision 2.1  85/11/24  11:48:55  shore
 * Product Release 2.0
 * 
 * Revision 1.3  85/11/20  00:10:01  shore
 * Added System V support (input options and spooling)
 * margins/linecount reworked (Dataproducts)
 * incompatible options changes, getopt!
 * Guy Riddle's Gaudy mode and other changes
 * output spooling messages, pages, copies
 * 
 * Revision 1.2  85/05/14  11:22:14  shore
 * *** empty log message ***
 * 
 *
 */
#define POSTSCRIPTPRINTER "PostScript"

#define BODYROMAN "Courier"
#define HEADFONT "Courier-Bold"

#ifdef DEBUG
#define debugp(x) {fprintf x ; VOIDC fflush(stderr);}
#else
#define debugp(x)
#endif

#define UperInch (1440L)
#define PtsPerInch 72
#define UperPt 20

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "transcript.h"
#include "psparse.h"
#include "config.h"
#include "PSres.h"

#ifdef ASE_BV
#include <search.h>
#endif

#define MAXBAD 20		/* number of bad chars to pass before
				   complaint */


extern double strtod();
/* forwards */
private FlushShow();
private VOID InitPage();

private struct stat S;

extern char *optarg;		/* getopt current opt char */
extern int optind;		/* getopt argv index */


private VOID int1();

#define FSIZEMAX 256		/* number of chars per font */

/* the layout of a font information block */
struct font {
    char name[100];		/* PostScript font name */
    double dsize;		/* size */
    int Xwid[FSIZEMAX];		/* X widths for each character */
};

private struct font fonts[16];	/* 16 possible fonts at one time */
private int nf = 0;		/* number of fonts known about */

private int goodchar[16][FSIZEMAX];

struct page {
    long pagewidth;
    long pagelength;
    long imagexorig;
    long imageyorig;
    long imagewidth;
    long imagelength;
};

#define LETTER 0
#define LEGAL 1
#define A4 2
#define B5 3
#define OTHER 4

private int defaultSize = LETTER;
private char *defaultPaperSize = "Letter";

private struct page pageSizes[5] = {
    {UperPt*612, UperPt*792, UperPt*18, UperPt*8, UperPt*593, UperPt*784},
    {UperPt*612, UperPt*1008, UperPt*15, UperPt*8, UperPt*597, UperPt*1000},
    {UperPt*595, UperPt*842, UperPt*13, UperPt*10, UperPt*577, UperPt*832},
    {UperPt*516, UperPt*729, UperPt*21, UperPt*10, UperPt*500, UperPt*715},
    {0,0,0,0,0,0}
}; 

private int psize = LETTER;
private int useDefault = TRUE;

/* TabWidth = tabMultiple * width of '0' character */
private int tabMultiple = 8;
private int TabWidth;		/* width of a tab, in points */
private int BSWidth;		/* width of a backspace */

private long UperLine = UperInch / 7;
private long UperHLine = UperInch / 7;

private char *prog;		/* program name argv[0] */
private char *libdir;		/* place for library files */
private char *tempdir;		/* place for temp file */
private char *tempName;		/* name of temporary PostScript file */
private char lprcmd[1024];
private char OutName[256] = "";	/* filename for disk output */
private int PipeOut = FALSE;	/* output to stdout (-p -) */
private int ListOmitted = FALSE;/* list omitted chars on the tty */
private int BeQuiet = FALSE;	/* suppress stderr info messages */
private int Gaudy = FALSE;	/* pretty bars along the top */
private int head = FALSE;       /* are we printing the header? */
private int LPTsimulate = FALSE;/* an lpt should be simulated */
private int Lines = 0;		/* max lines per page */
private int LinesLeft = 66;	/* lines left on page when in LPT mode */
private int col;		/* column number on current line; reset if
				   broken into two PS lines */
private int maxchars = 5000;	/* max width; after this, trunc or wrap */
private int nchars;		/* num. of chars on this line  */
private int SeenText = TRUE;	/* true if seen some text on this page */
private int OutOnly = FALSE;	/* PS file only wanted */
private int Rotated = FALSE;	/* pages to be rotated landscape */
private int PreFeed = FALSE;	/* prefeed should be enabled */
private int columns = 1;	/* multi-column mode */
private int PrintCol = 1;	/* printing column  */
private int NoTitle = FALSE;	/* title line is suppressed */
private int Cvted = FALSE;	/* converted a file to PS format */
private int linewrap = TRUE;	/* wrap-around lines. this is default. */
private int PSpecified = FALSE;	/* printer specified on command line? */

private int IgnoreGarbage = FALSE;	/* garbage should be ignored */
private int SeenFile = FALSE;	/* a file has been processed */
private int SeenFont = FALSE;	/* we've seen a font request */
private int ScannedFonts = FALSE;	/* we've scanned the font file */
private char *FileName = 0;	/* name of file currently being PSed */
private char *FileDate = 0;	/* last mod date of file being PSed */
private char DateStr[27];	/* thanks, but no thanks ctime! */
private int spoolNoBurst = FALSE;	/* no break page flag for spooler
					   */

private struct comfeatures features[20];
private int nfeatures = 0;

private char *spoolTitle = NULL;
private char *spoolJobClass = NULL;
private char *spoolJobName = NULL;
private char *PrinterName = NULL;
private int spoolNotify = 0;
private char *spoolCopies = "1";

private char tempstr[256];	/* various path names */

private int CurFont;		/* current Font */
private int fontindex[26];	/* table of fonts, indexed by font
				   designator ('a' to 'z') */
/* indexes for default fonts */

#define Roman fontindex['r'-'a']
#define HeaderFont fontindex['h'-'a']

private long cX, cY;		/* current page positions */
private long dX, dY;		/* desired page positions */
private long lX, lY;		/* page positions of the start of the line */
private long crX, crY;		/* X and Y increments to apply to CR's */
private long leftMargin;
private long minX;
private long maxX;		/* maximum x coord on line */
private long minY;		/* minimum y coord on page */
private long maxY;		/* maximum y coord on page */

#define None	0
#define RelX	1
#define	RelY	2
#define RelXY	3
#define AbsX	4
#define AbsY	8
#define AbsXY	12

private int movepending;	/* moveto pending coords on stack */
private int showpending;	/* number of characters waiting to be
				   shown */
private int pagepending;	/* start on next page when have something
				   to print */
private int newpage;		/* multiple formfeed flag */
private char *UsersHeader = NULL;	/* user specified heading */
private char *Header = NULL;	/* generated header (usually FileName) */
private int Page = 0;		/* current page number */
private int TruncChars = 0;	/* number of characters truncated */
private int TotalPages = 0;	/* total number of pages printed */
private int UndefChars = 0;	/* number of characters skipped because
				   they weren't defined in some font */
private int BadChars = 0;	/* number of bad characters seen so far */
private int LinesWrapped = 0;	/* number of lines that were
				   wrapped-around */
private int overwrites = 0;	/* number of places where overwriting would
				   have occurred (can happen with tabs and
				   variable width fonts */
private FILE *OutFile = NULL;	/* output ps file */
private FILE *infile = NULL;

private char *bigargv[100];
private int bigargc = 0;

#ifdef ASE_BV
/**
 ** Constants, definitions and global variables
 ** (What? Global variables? You let your kids use your revolver too, right?)
 **
 ** Antony Simmins, Adobe BV SPD TSG, 16/6/93
 **
 **/
#define ENCODINGSUFFIX	".enc"
#define CHARNAME_LENGTH	32
#define HASH_TABLE_SIZE	FSIZEMAX
private int EncodingFile = FALSE;	/* user specified encoding vector */
private char EncodingFileName[256];
private char EncodingDir[256];
struct encvec { char charname[CHARNAME_LENGTH]; int charpos; };
#endif

/* decode a fontname string - e.g. Courier10 Helvetica-Bold12 */
private decodefont(name, f)
    register char *name;
    register struct font *f;
{
    register char *d, *p;
    char *s;

    SeenFont = TRUE;
    p = name;
    d = f->name;
    f->dsize = 0;
    while (isascii(*p) && (isalpha(*p) || (*p == '-'))) {
	*d++ = *p++;
    }
    *d++ = '\0';
    f->dsize = strtod(p, &s);
    if (*s || !f->dsize || !f->name[0]) {
	fprintf(stderr, "%s: poorly formed font name & size: \"%s\"\n",
	  prog, name);
	exit(1);
    }
}


#define NOTDEF 0x8000
#define ForAllFonts(p) for(p = &fonts[nf-1]; p >= &fonts[0]; p--)


/* Scan the font metrics directory looking for entries that match the
   entries in ``fonts''.  For entries that are found the data in the font
   description is filled in, if any are missing, it dies horribly. */
private VOID ScanFont()
{
    register struct font *f;
    register FILE *FontData;	/* afm file */
    char *c;
    int ccode, cwidth, inChars;
    char **names;
    char **files;
    int count;
    char FontFile[512];		/* afm file name */
    char afmbuf[BUFSIZ];
    int i;
#ifdef SYSV
    char shortname[15];
#endif
#ifdef ASE_BV
/**
 ** Local variables
 **
 ** Antony Simmins, Adobe BV SPD TSG, 16/6/93
 **
 **/
    FILE *encvecfp;
    int charcount, retval;
    char charstring[CHARNAME_LENGTH];
    struct encvec EncodingVector[HASH_TABLE_SIZE];
    ENTRY item, *found_item, *hsearch();
    char cname[CHARNAME_LENGTH];
#endif

    if (!SeenFont) {
	/* if (Lines == 0) Lines = 64; */
	if (Rotated && columns > 1)
	    fonts[Roman].dsize = 7;
    }

#ifdef ASE_BV
/**
 ** Read in font encoding if required
 **
 ** Antony Simmins, Adobe BV SPD TSG, 16/6/93
 **
 **/

    if (EncodingFile == TRUE)
	{ char tmpstr[256];
	  char *encdir;

	  if ((encdir = envget("ENCODINGDIR")) == NULL) {
	      strncpy(tmpstr, libdir, 256);
	      strncat(tmpstr, "/enc/", 256);
	      strncat(tmpstr, EncodingFileName, 256);
	  }
	  else {
	      strncpy(tmpstr, encdir, 256);
	      strncat(tmpstr, "/", 256);
	      strncat(tmpstr, EncodingFileName, 256);
	  }
	      
fprintf(stderr, "Debug: enc file is %s\n", tmpstr);
	  if ((encvecfp = fopen(tmpstr, "r")) == NULL)
	    { fprintf(stderr, "%s: cannot open encoding file %s\n", prog, EncodingFileName);
	      exit(1);
	    }

	  while (fgetc(encvecfp) != '[') 
	    { if (feof(encvecfp) != 0)
		{ fprintf(stderr, "s: unexpected EOF before [ in encoding vector file %s\n", prog, EncodingFileName);
		  exit(1);
		}
	    }

	  if (hcreate(256) == 0)
	    { fprintf(stderr, "%s: hcreate() cannot allocate sufficient space\n", prog);
	      exit(1);
	    }

	  charcount = 0;

	  while ((charcount < 256) && (feof(encvecfp) == 0))
	    { if ((retval = fscanf(encvecfp, " /%s ", charstring)) != 1)
		{ switch (retval)
		    { case EOF : fprintf(stderr, "%s: unexpected EOF in encoding vector file %s\n", prog, EncodingFileName);
				 exit(1);
				 break;

		      case 0 : fprintf(stderr, "%s: failed to match item in encoding vector file %s\n", prog, EncodingFileName);
			       exit(1);
			       break;
		    }
		}

	      if (strcpy(EncodingVector[charcount].charname, charstring) != EncodingVector[charcount].charname)
		{ fprintf(stderr, "%s: error in strcpy()\n", prog);
		  exit(1);
		}

	      EncodingVector[charcount].charpos = charcount;

	      item.key = EncodingVector[charcount].charname;
	      item.data = (char *)(EncodingVector[charcount].charpos);

	      if (hsearch(item, ENTER) == NULL)
		{ fprintf(stderr, "%s: hsearch() cannot enter item in table\n", prog);
		  exit(1);
		}

	      charcount++;
	    }

	  while (fgetc(encvecfp) != ']') 
	    { if (feof(encvecfp) != 0)
		{ fprintf(stderr, "s: unexpected EOF before ] in encoding vector file %s\n", prog, EncodingFileName);
		  exit(1);
		}
	    }
	}
#endif
    /* loop through fonts, find and read metric entry in dir */
    ForAllFonts(f) {
	count = ListPSResourceFiles(NULL, ResourceDir, PSResFontAFM, f->name,
				     &names, &files);
	if (count == 0) {
	    fprintf(stderr,"%s: could not locate AFM file for %s\n", prog, f->name);
	    exit(1);
	}
	for (i = 0; i < count; i++) {
	    if ((FontData = fopen(files[i], "r")) != NULL)
		break;
	}
	if (i == count) {
	    fprintf(stderr, "%s: couldn't open font metrics file.\n", prog);
	    exit(1);
	}
	/* read the .afm file to get the widths */
	for (ccode = 0; ccode < FSIZEMAX; ccode++)
	    f->Xwid[ccode] = NOTDEF;

	inChars = 0;
	while (fgets(afmbuf, sizeof afmbuf, FontData) != NULL) {
	    /* strip off newline */
	    if ((c = strchr(afmbuf, '\n')) == 0) {
		fprintf(stderr, "%s: AFM file %s line too long %s\n",
		  prog, FontFile, afmbuf);
		exit(1);
	    }
	    *c = '\0';
	    if (*afmbuf == '\0')
		continue;
	    if (strncmp(afmbuf, "StartCharMetrics", 16) == 0) {
		inChars++;
		continue;
	    }
	    if (strcmp(afmbuf, "EndCharMetrics") == 0)
		break;
	    if (inChars == 1) {
#ifdef ASE_BV
/**
 ** This is where it all happens
 **
 ** Antony Simmins, Adobe BV SPD TSG, 22/6/93
 **
 **/
		if (EncodingFile == TRUE)
		  { if (sscanf(afmbuf, "C %d ; WX %d ; N %s ;", &ccode, &cwidth, cname) != 3) {
		      fprintf(stderr, "%s: trouble with AFM file %s\n",
		        prog, FontFile);
		      exit(1);
		   }

		  if (ccode > 255)
		      continue;

		  item.key = cname;

		  if ((found_item = hsearch(item, FIND)) != NULL)
		    { f->Xwid[(int)found_item->data] =
		        (short) (((long) cwidth * (long) f->dsize * (long) UperPt) / 1000L);
		    }
		  }
		else
		  { if (sscanf(afmbuf, "C %d ; WX %d ;", &ccode, &cwidth) != 2) {
		      fprintf(stderr, "%s: trouble with AFM file %s\n",
		        prog, FontFile);
		      exit(1);
	    	    }
		    /* get out once we see an unencoded char */
		    if (ccode == -1)
		        break;
		    if (ccode > 255)
		        continue;
		    f->Xwid[ccode] =
		      (short) (((long) cwidth * (long) f->dsize * (long) UperPt)
		      / 1000L);
		  }
#else
		if (sscanf(afmbuf, "C %d ; WX %d ;", &ccode, &cwidth) != 2) {
		    fprintf(stderr, "%s: trouble with AFM file %s\n",
		      prog, FontFile);
		    exit(1);
		}
		/* get out once we see an unencoded char */
		if (ccode == -1)
		    break;
		if (ccode > 255)
		    continue;
		f->Xwid[ccode] =
		  (short) (((long) cwidth * (long) f->dsize * (long) UperPt)
		  / 1000L);
#endif		
		continue;
	    }
	}
#ifdef ASE_BV
/**
 ** Some encodings have duplicate characters but our hash table only knows
 ** about the first in the encoding, so here we fill in widths for the others.
 **
 ** Note: those entries in the encoding whose character name is .notdef will
 **	  still have a width of NOTDEF. However, PLRM 5.3 penultimate paragraph
 **	  notes that the width should be that of space for an Adobe Type 1 font.
 **
 ** Antony Simmins, Adobe BV SPD TSG, 14/7/93
 **
 **/
	if (EncodingFile == TRUE)
	  { int i;

	    for (i=0; i<FSIZEMAX; i++)
	      { if (f->Xwid[i] == NOTDEF)
		  { item.key = EncodingVector[i].charname;
		    if ((found_item = hsearch(item, FIND)) != NULL)
		      f->Xwid[i] = f->Xwid[(int)found_item->data];
		  }
	      }
	  }
#endif
	
	VOIDC fclose(FontData);
    }

    TabWidth = fonts[Roman].Xwid['0'] * tabMultiple;
    BSWidth = fonts[Roman].Xwid[' '];	/* space width */

    UperLine = (fonts[Roman].dsize + 1) * UperPt;

    if (LPTsimulate) {
	UperHLine = UperLine;
	Lines = 66;
    }
    else {
	UperHLine = (fonts[HeaderFont].dsize + 1) * UperPt;
    }

    crX = 0;
    crY = -UperLine;

}


/* Return a font number for the font with the indicated name and size.
   Adds info to the font list for the eventual search. */
private int DefineFont(name, size)
    char *name;
{
    register struct font *p;
    p = &fonts[nf];
    VOIDC strcpy(p->name, name);
    p->dsize = size;
    return (nf++);
}


/* dump the fonts to the PS file for setup */
private VOID DumpFonts()
{
    register struct font *f;

    ForAllFonts(f) {
	fprintf(OutFile,"%%%%IncludeResource: font %s\n",f->name);
	fprintf(OutFile, "%d %10.3f /%s\n", f - &fonts[0], f->dsize * UperPt, f->name);
    }
    fprintf(OutFile, "%d SetUpFonts\n", nf);
}


/* add a shown character to the PS file */
private VOID OUTputc(c)
    register int c;
{
    if (!showpending) {
	putc('(', OutFile);
	showpending = TRUE;
    }
#ifdef ASE_BV
/**
 ** See comment at bof.
 **
 ** Antony Simmins, Adobe BV SPD TSG, 24/3/93
 **
 **/
    if (EncodingFile == TRUE)
      { if (c == '(' || c == ')')
	  putc('\\', OutFile);
      }
    else
      { if (c == '\\' || c == '(' || c == ')')
	  putc('\\', OutFile);
      }
#else
    if (c == '\\' || c == '(' || c == ')')
	putc('\\', OutFile);
#endif    
    if ((c > 0176) || (c < 040)) {
	putc('\\', OutFile);
	putc((c >> 6) + '0', OutFile);
	putc(((c >> 3) & 07) + '0', OutFile);
	putc((c & 07) + '0', OutFile);
    }
    else
	putc(c, OutFile);
}

/* Set the current font */
private VOID SetFont(f)
    int f;
{
    FlushShow();
    CurFont = f;
    fprintf(OutFile, "%d F\n", f);
}

/* put a character onto the page at the desired X and Y positions. If the
   current position doesn't agree with the desired position, put out
   movement directives.  Leave the current position updated to account for
   the character. */
private VOID ShowChar(c)
    register int c;
{
    register struct font *f;
    register long nX, nY;
    static level = 0;

    level++;
    f = &fonts[CurFont];

    if (f->Xwid[c] == NOTDEF) {
	UndefChars++;
	if (ListOmitted)
	    fprintf(stderr, "%s: \\%03o not found in font %s\n",
	      prog, c, f->name);
	if (level <= 1) {
	    ShowChar('\\');
	    ShowChar((c >> 6) + '0');
	    ShowChar(((c >> 3) & 07) + '0');
	    ShowChar((c & 07) + '0');
	    col += 3;
	    nchars += 3;
	}
	level--;
	return;
    }
    nX = dX + f->Xwid[c];	/* resulting position after showing this
				   char */
    nY = dY;

    if (nX > maxX && linewrap && !head) {	/* Wrap line */
	Emit_Newline();
	LinesWrapped++;
	if (pagepending)
	    InitPage();
	nX = dX + f->Xwid[c];
	nY = dY;
    }
    
    if (nX <= maxX || head) {
	if (cX != dX) {
	    if (cY != dY) {
		FlushShow();
		/* absolute x, relative y */
		fprintf(OutFile, "%ld %ld", dX, dY);
		movepending = AbsXY;
	    }
	    else {
		FlushShow();
		fprintf(OutFile, "%ld", dX - cX);	/* relative x */
		movepending = RelX;
	    }
	}
	else if (cY != dY) {
	    FlushShow();
	    fprintf(OutFile, "%ld", dY - cY);	/* relative y */
	    movepending = RelY;
	}
	OUTputc(c);
	cX = nX;
	cY = nY;
    }
    else if (!linewrap) {
	TruncChars++;
	maxchars = nchars;
    }
    dX = nX;
    dY = nY;
    
    level--;
}

/* put out a shown string to the PS file */
private VOID ShowStr(s)
    register char *s;
{
    while (*s) {
	if (*s >= 040)
	    ShowChar(*s);
	s++;
    }
}

/* flush pending show */
private FlushShow()
{
    if (showpending) {
	putc(')', OutFile);
	switch (movepending) {
	    case RelX:
		putc('X', OutFile);
		break;
	    case RelY:
		putc('Y', OutFile);
		break;
	    case AbsXY:
		putc('B', OutFile);
		break;
	    case None:
		putc('S', OutFile);
		break;
	}
	putc('\n', OutFile);
	movepending = None;
	showpending = FALSE;
    }
}

/* put out a page heading to the PS file */
private VOID InitPage()
{
    char header[200];
    register int OldFont = CurFont;
    long hX, hY;
    long barlength;
    char *p, *q;;

    TotalPages++;
    fprintf(OutFile, "%%%%Page: ? %d\n", TotalPages);
    fprintf(OutFile, "StartPage\n");

    SeenText = FALSE;
    cX = cY = -1;
    showpending = pagepending = newpage = FALSE;
    PrintCol = 1;

    if (Rotated) {
	fprintf(OutFile, "%ld Landscape\n",-pageSizes[psize].pagewidth);
	/* minimum margin */
	minX = 360;  /* 1/4 inch */
	/* if orig of imageable area is smaller than minimum margin, use
	   minimum margin for beginning */
	if (pageSizes[psize].imageyorig < 360)
	    lX = dX = minX;
	else
	    lX = dX = pageSizes[psize].imageyorig;

	leftMargin = lX;

	/* back off from upper limit of imageable area by 1.5 times the
	   inter-line spacing */
	lY = dY = pageSizes[psize].imagewidth - (UperHLine * 3)/2;

	if (columns > 1) {
	    /* end column just slightly before the dividing point */
	    maxX = pageSizes[psize].pagelength/columns - UperInch*0.06;
	}
	else {
	    /* maxX is the limit of the imageable area, unless that exceeds
	       the minimum margin */
	    maxX = pageSizes[psize].imagelength;
	    if (maxX > (pageSizes[psize].pagelength - UperInch * 0.3))
		maxX = pageSizes[psize].pagelength - UperInch * 0.3;
	}
	if ((minY = pageSizes[psize].imagexorig) < UperInch / 4)
	    minY = UperInch / 4;
	maxY = pageSizes[psize].pagewidth - minY - ((UperHLine * 3) / 2);
    }
    else {
	lX = dX = pageSizes[psize].imagexorig;
	minX = (columns > 1) ? (UperInch * 0.3) : ((UperInch * 5) / 8);
	if (dX < minX)
	    lX = dX = minX;
	leftMargin = lX;
	lY = dY = pageSizes[psize].imagelength - UperHLine;
	if (columns > 1) {
	    maxX = pageSizes[psize].pagewidth / columns - UperInch * 0.06;
	}
	else {
	    maxX = pageSizes[psize].imagewidth;
	    if (maxX  > (pageSizes[psize].pagewidth - UperInch * 0.3))
		maxX =
		  pageSizes[psize].pagewidth - UperInch * 0.3;
	}
	if ((minY = pageSizes[psize].imageyorig) < UperInch / 4)
	    minY = UperInch / 4;
	maxY = pageSizes[psize].pagelength - minY - UperHLine;
    }
    if (dY > maxY)
	dY = lY = maxY;

    
    movepending = None;
    cX = dX;
    cY = dY;

    if (!NoTitle) {
	if (Gaudy) {
	    if (Rotated) {
		if (pageSizes[psize].imageyorig < 360)
		    hX = 360;	/* quarter-inch */
		else
		    hX = pageSizes[psize].imageyorig;
		if ((pageSizes[psize].pagewidth-
		    pageSizes[psize].imagewidth) < 360)
		    hY = pageSizes[psize].pagewidth - 360 - 720;
		else
		    hY = pageSizes[psize].imagewidth - 720;
		if ((pageSizes[psize].pagelength - pageSizes[psize].imagelength)
		  < 360)
		    barlength = pageSizes[psize].pagelength - 360 - hX;
		else
		    barlength = pageSizes[psize].imagelength - hX;
	    }
	    else {
		if (pageSizes[psize].imagexorig < 360)
		    hX = 360;	/* quarter-inch */
		else
		    hX = pageSizes[psize].imagexorig;
		if ((pageSizes[psize].pagelength -
		    pageSizes[psize].imagelength) < 360)
		    hY = pageSizes[psize].pagelength - 360 - 720;
		else
		    hY = pageSizes[psize].imagelength - 720;
		if ((pageSizes[psize].pagewidth - pageSizes[psize].imagewidth)
		  < 360)
		    barlength = pageSizes[psize].pagewidth - 360 - hX;
		else
		    barlength = pageSizes[psize].imagewidth - hX;
	    }
	    fprintf(OutFile, "(%s)(%s)[%s](%d) %ld %ld %ld ",
	      UsersHeader, Header, FileDate, ++Page, hX, hY, barlength);
	    if (Rotated)
		fprintf(OutFile,"%ld ",pageSizes[psize].pagelength);
	    else fprintf(OutFile,"%ld ",pageSizes[psize].pagewidth);
	    fprintf(OutFile,"Gaudy\n");
	    cX = cY = 0;	/* force moveto here */
	}
	else {
	    SetFont(HeaderFont);
	    fprintf(OutFile, "%ld %ld ", cX, cY);
	    movepending = AbsXY;
	    if (UsersHeader) {
		if (*UsersHeader == 0) {
		    fprintf(OutFile, "()B\n");
		    movepending = None;
		    showpending = FALSE;
		}
		else {
		    /* insert requested info */
		    q = header;
		    for (p = UsersHeader; *p != '\0'; p++) {
			if (*p == '%') {
			    p++;
			    switch (*p) {
			    case 'f':
				sprintf(q, "%s", Header ? Header: "");
				q += strlen(q);
				break;
			    case 'd':
				sprintf(q, "%s", FileDate);
				q += strlen(q);
				break;
			    case 'n':
				sprintf(q, "%d", ++Page);
				q += strlen(q);
				break;
			    case '%':
				*q = '%';
				q++;
				break;
			    default:
				sprintf(q, "%%%c", *p);
				q += 2;
			    }
			    continue;
			}
			*q = *p;
			q++;
			*q = 0;
		    }
		    head = TRUE;
		    ShowStr(header);
		    head = FALSE;
		}
	    }
	    else {
		VOIDC sprintf(header, "%s        %s        %d",
		        Header ? Header : "              ", FileDate, ++Page);
		head = TRUE;
		ShowStr(header);
		head = FALSE;
	    }
	    FlushShow();
	}
	dX = lX = lX + crX * 2;
	if (Gaudy)
	    dY = lY = lY + crY * 4;
	else
	    dY = lY = lY + crY * 2;
    }
    else {
	/* fake it to force a moveto */
	cX = cY = 0;
    }
    if (Lines <= 0)
	Lines = LinesLeft = (lY - minY) / (-crY);
    else
	LinesLeft = Lines;
    SetFont(OldFont);
}

private VOID ClosePage()
{
    FlushShow();
    if (!pagepending)
	fprintf(OutFile, "EndPage\n");
    pagepending = TRUE;
}

/* skip to a new page */
private VOID PageEject()
{
    if (columns > 1 && PrintCol != columns) {
	if (Rotated) {
	    lY = dY = pageSizes[psize].imagewidth - (UperHLine * 3) / 2;
	    minX = lX = dX = PrintCol * pageSizes[psize].pagelength /
		columns + leftMargin;
	    if (++PrintCol != columns)
		maxX = PrintCol * pageSizes[psize].pagelength / columns
		    -UperInch * 0.06;
	    else 
		if ((maxX = pageSizes[psize].imagelength) >
		    pageSizes[psize].pagelength - UperInch * 0.3)
		    maxX = PrintCol*pageSizes[psize].pagelength/columns - UperInch*0.3;
	}
	else {
	    lY = dY = pageSizes[psize].imagelength - UperHLine;
	    minX = lX = dX = PrintCol * pageSizes[psize].pagewidth /
		columns + leftMargin;
	    if (++PrintCol != columns)
		maxX = PrintCol * pageSizes[psize].pagelength/columns;
	    else
		if ((maxX = pageSizes[psize].imagewidth) >
		    pageSizes[psize].pagewidth - UperInch * 0.3)
		    maxX = pageSizes[psize].pagelength - UperInch*0.3;
	}
	if (dY > maxY)
	    lY = dY = maxY;
	if (!NoTitle) {
	    dX = lX = lX + crX * 2;
	    if (Gaudy)
		dY = lY = lY + crY * 4;
	    else
		dY = lY = lY + crY * 2;
	}
    }
    else
	ClosePage();
    LinesLeft = Lines;
    SeenText = FALSE;
}

private VOID CommentHeader()
{
    long clock;
    struct passwd *pswd;
    char hostname[40];
    /* copy the file, prepending a new comment header */
    fprintf(OutFile, "%%!%s\n", COMMENTVERSION);
    fprintf(OutFile, "%%%%Creator: enscript\n");
    pswd = getpwuid((int) getuid());
    /* ++++ X/Open doesn't specify how to do this; check POSIX */
    VOIDC gethostname(hostname, (int) sizeof hostname);
    if (pswd)
	fprintf(OutFile, "%%%%For: %s:%s (%s)\n", hostname, pswd->pw_name,
		pswd->pw_gecos); 
    fprintf(OutFile, "%%%%Title: %s\n", (FileName ? FileName : "stdin"));
    fprintf(OutFile, "%%%%CreationDate: %s", (VOIDC time(&clock), ctime(&clock)));
}

/* Handle newline input character or line wrap */
Emit_Newline()
{
    if (pagepending)
	InitPage();

    SeenText = TRUE;

    dY = lY = lY + crY;
    dX = lX = lX + crX;

    if ((dY < minY) || (--LinesLeft <= 0))
	PageEject();

    col = 1;
    nchars = 1;
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

/* read page size and imageable area info from ppd file */
private VOID GetPageSizeInfo(fptr,size)
    FILE *fptr;
    char *size;
{
    char *value;
    float xsize, ysize, xmin, ymin, xmax, ymax;
    int ndex;

    ndex = strtondx(size);
    rewind(fptr);
    value = (char *) parseppd(fptr, "*PaperDimension", size);
    if (value) {
	sscanf(value, "%f %f", &xsize, &ysize);
	pageSizes[ndex].pagewidth = xsize * UperPt;
	pageSizes[ndex].pagelength = ysize * UperPt;
    }
    rewind(fptr);
    value = (char *) parseppd(fptr, "*ImageableArea", size);
    if (value) {
	sscanf(value, "%f %f %f %f", &xmin, &ymin, &xmax, &ymax);
	pageSizes[ndex].imagewidth = xmax * UperPt;
	pageSizes[ndex].imagelength = ymax * UperPt;
	pageSizes[ndex].imagexorig = xmin * UperPt;
	pageSizes[ndex].imageyorig = ymin * UperPt;
    }
}

static void DefaultPageSize(fp)
    FILE *fp;
{
    char *defstr = "*DefaultPageSize";
    char *value;
    
    rewind(fp);
    value = parseppd(fp, defstr, NULL);
    if (value) {
	if (strcmp(value, "Unknown"))
	    psize = strtondx(value);
    }
}    
    
static void InitGoodChar()
{
    int i, j;

    for (i = 0; i < nf; i++)
	for (j = 0; j < FSIZEMAX; j++) {
	    if (j < 040) {
		goodchar[i][j] = 0;
		continue;
	    }
	    if (fonts[i].Xwid[j] == NOTDEF) {
		goodchar[i][j] = 0;
		continue;
	    }
	    switch (j) {
		case 0134:
		    /* backslash */
		case 050:
		    /* open paren */
		case 051:
		    /* close paren */
		case 0177:
		    /* DEL */
		    goodchar[i][j] = 0;
		    break;
		default:
#ifdef ASE_BV
/**
 ** This is required for our version so that 8-bit
 ** characters are always output as an ocyal number
 ** preceded by a / character. However I feel it
 ** should also be in the normal version because
 ** it means that "garbage" files are processed to
 ** give ASCII output and not non-portable 8-bit
 ** characters. Of course, postprocessors may be
 ** trapping such characters and doing their own
 ** evil thing with them ...
 **
 ** If this was the case, the test would be
 ** if (IgnoreGarbage == TRUE)
 ** and since (EncodingFile == TRUE) implies
 ** IgnoreGarbage our version would be covered.
 **
 ** Antony Simmins, Adobe BV SPD TSG, 16/6/93
 **
 **/
		    if (EncodingFile == TRUE)
		      goodchar[i][j] = 0;
		    else
		      goodchar[i][j] = 1;
#else
		    goodchar[i][j] = 1;
#endif		    
		    break;
	    }
	}
}

static FILE *OpenPPD()
{
    FILE *ppd;

    if (PrinterName == NULL)
	return NULL;
    if (OutOnly && !PSpecified)
	return NULL;
    ppd = GetPPD(PrinterName);
    if (ppd != NULL)
	return ppd;
#ifdef ATHENA_REALLY_WANTED_TO_INSTALL_PPD_FILES_FOR_ALL_PRINTERS
    if (!BeQuiet) {
	fprintf(stderr, "%s: warning: couldn't open ppd file for printer %s.\n",
		prog, PrinterName);
	fprintf(stderr, "Using built-in defaults.\n");
    }
#endif
    return NULL;
}
	    

/* Copy the standard input file to the PS file */
private VOID CopyFile()
{
    register int c;
    register FILE *input;
    register int whichfont;
    char *pagesizecode = NULL;
    FILE *fp;
    char *printerDefault;
    char *p;
    char defstring[100];
    int i;
    int barlength;
    int first = TRUE;
    long nX;
    int escfont = FALSE;
    int setsize = FALSE;


    col = 1;
    nchars = 1;
    input = infile;
    whichfont = CurFont;
    if (OutFile == 0) {
	if (OutOnly) {
	    OutFile = PipeOut ? stdout : fopen(OutName, "w");
	}
	else {
	    tempName = (char *)tempnam(tempdir, "ES");
	    if (!tempName) {
		fprintf(stderr, "enscript: couldn't create temp file name.\n");
		perror("");
		exit(1);
	    }
	    VOIDC strcpy(OutName, tempName);
#ifdef SYSV
	    VOIDC umask(0);
#else
	    VOIDC umask(077);
#endif
	    OutFile = fopen(tempName, "w");
	}
    }
    if (OutFile == NULL) {
	fprintf(stderr, "%s: can't create PS file %s\n", prog, OutName);
	exit(1);
    }
    if (!ScannedFonts) {
	ScannedFonts = TRUE;
	ScanFont();
	InitGoodChar();
    }
    if (!Cvted) {
	CommentHeader();
	if (nf) {
	    register struct font *f;
	    fprintf(OutFile, "%%%%DocumentNeededResources: font");
	    ForAllFonts(f)
	      fprintf(OutFile, " %s", f->name);
	    if (Gaudy)
		fprintf(OutFile, "\n%%+ Times-Roman Times-Bold Helvetica-Bold");
	    fprintf(OutFile, "\n");
	}
	if (Rotated) {
	    fprintf(OutFile, "%%%%Orientation:  Landscape\n");
	}
	else {
	    fprintf(OutFile, "%%%%Orientation: Portrait\n");
	}
	fprintf(OutFile, "%%%%Pages: (atend)\n");
	fprintf(OutFile, "%%%%BeginProlog\n");
	/* copy in fixed prolog */
#ifdef ASE_BV
/**
 ** Put out definition of reef - used in enscript.pro
 **
 ** Antony Simmins, Adobe BV SPD TSG, 22/6/93
 **
 **/
	if (EncodingFile == TRUE)
	  { fprintf(OutFile, "/reef	%% dict reef dict\n");
	    fprintf(OutFile, "{ dup length dict begin { exch dup /FID eq {pop pop} {exch def} ifelse } forall\n");
	    fprintf(OutFile, "/Encoding\n");
	    if (copyfile(mstrcat(tempstr, libdir, EncodingFileName, sizeof tempstr),
	        OutFile)) {
	        fprintf(stderr, "%s: trouble copying encoding file\n", prog);
	        exit(1);
	    }  
	    fprintf(OutFile, "def\n");
	    fprintf(OutFile, "currentdict /newfontname exch definefont end } def\n");
	  }
	else
	  fprintf(OutFile, "/reef {} def\n");
#else
	fprintf(OutFile, "/reef {} def\n");
#endif
	if (copyfile(mstrcat(tempstr, libdir, ENSCRIPTPRO, sizeof tempstr),
	    OutFile)) {
	    fprintf(stderr, "%s: trouble copying prolog file\n", prog);
	    exit(1);
	}
	fprintf(OutFile, "%%%%EndProlog\n");
	fprintf(OutFile, "%%%%BeginSetup\n");
	DumpFonts();
	if (Gaudy)
	    fprintf(OutFile, "%d InitGaudy\n", columns);
	if (PreFeed)
	    fprintf(OutFile, "true DoPreFeed\n");
	fp = OpenPPD();
	if (fp) {
	    if (useDefault) {
		DefaultPageSize(fp);
	    }
	    for (i = 0; i < nfeatures; i++) {
		rewind(fp);
		strncpy(defstring, "*Default", 100);
		/* don't copy the leading * */
		p = features[i].keyword;
		p++;
		strncat(defstring, p, 100);
		printerDefault = parseppd(fp, defstring, NULL);
		if (printerDefault) {
		    if (!strcmp(printerDefault, features[i].option)) {
			/* we're asking for default behavior */
			fprintf(OutFile, "%%%%IncludeFeature: %s %s\n",
			  features[i].keyword,
			  features[i].option);
		    }
		    else {
			/* we're asking for non-default behavior */
			rewind(fp);
			features[i].value =
			  parseppd(fp, features[i].keyword, features[i].option);
			if (features[i].value) {
			    fprintf(OutFile, "%%%%BeginFeature: %s %s\n",
			      features[i].keyword, features[i].option);
			    fprintf(OutFile, "%s\n", features[i].value);
			    fprintf(OutFile, "%%%%EndFeature\n");
			}
			else {
			    fprintf(stderr, "enscript: printer %s does not support feature %s: %s.\n", PrinterName, features[i].keyword, features[i].option);
			    exit(1);
			}
		    }
		}
		else {
		    /* no default for this value */
		    rewind(fp);
		    features[i].value =
		      parseppd(fp, features[i].keyword, features[i].option);
		    if (features[i].value) {
			fprintf(OutFile, "%%%%BeginFeature: %s %s\n",
			  features[i].keyword, features[i].option);
			fprintf(OutFile, "%s\n", features[i].value);
			fprintf(OutFile, "%%%%EndFeature\n");
		    }
		    else {
			fprintf(stderr, "enscript: printer %s does not support feature %s: %s.\n", PrinterName, features[i].keyword, features[i].option);
			exit(1);
		    }
		}
		/* if it's page size, get the info */
		if (!strncmp("*PageSize", features[i].keyword, 9)) {
		    psize = strtondx(features[i].option);
		    GetPageSizeInfo(fp, features[i].option);
		    setsize = TRUE;
		}
	    }
	    if (!setsize) {
		/* we've got a ppd file, use it even though there wasn't an
		   explicit page size request */
		GetPageSizeInfo(fp, ndxtostr(psize));
	    }
	}
	else {
	    for (i = 0; i < nfeatures; i++) {
		fprintf(OutFile, "%%%%IncludeFeature: %s %s\n",
		  features[i].keyword, features[i].option);
	    }
	}
	fprintf(OutFile, "%%%%EndSetup\n");
    }

    Cvted = TRUE;

    Page = 0;
    BadChars = 0;		/* give each file a clean slate */
    pagepending = newpage = TRUE;
    while ((c = getc(input)) != EOF) {
	first = FALSE;
	if (goodchar[whichfont][c] && (cX == dX) && (cY == dY) &&
	    !pagepending && !escfont && col < 230 && nchars < maxchars) {
	    nX = dX + fonts[whichfont].Xwid[c];
	    if (nX <= maxX) {
		putc(c, OutFile);
		cX = dX = nX;
		col++;
		nchars++;
		continue;
	    }
	}

	if ((c > 0177 || c < 0) && (!IgnoreGarbage)) {
	    if (BadChars++ > MAXBAD) {	/* allow some kruft but not much */
		fprintf(stderr, "%s: \"%s\" not a text file? Try -g.\n",
		  prog, FileName ? FileName : "(stdin)");
		if (!PipeOut)
		    VOIDC unlink(OutName);
		exit(1);
	    }
	}
	else if (c >= ' ') {
	    if (pagepending)
		InitPage();
	    if (col >= 230 && nchars < maxchars) {
		FlushShow();
		fprintf(OutFile, "%ld %ld", dX, dY);
		movepending = AbsXY;
		col = 1;
	    }
	    ShowChar(c);
	    col++;
	    nchars++;
	}
	else
	    switch (c) {
		case 010:	/* backspace */
		    dX -= BSWidth;
		    col--;
		    nchars--;
		    break;
		case 015:	/* carriage return ^M */
		    dY = lY;
		    dX = lX;
		    col = 1;
		    break;
		case 012:	/* linefeed ^J */
		    Emit_Newline();
		    break;
		case 033:	/* escape */
		    switch (c = getc(input)) {
			case '7':	/* backup one line */
			    dY = lY = lY - crY;
			    dX = lX = lX - crX;
			    break;
			case '8':	/* backup 1/2 line */
			    dY -= crY / 2;
			    dX -= crX / 2;
			    break;
			case '9':	/* forward 1/2 linefeed */
			    dY += crY / 2;
			    dX += crX / 2;
			    break;
			case 'F':	/* font setting */
			    escfont = TRUE;
			    c = getc(input);
			    if ('a' <= c && c <= 'z')
				if (fontindex[c - 'a'] >= 0)
				    SetFont(fontindex[c - 'a']);
				else {
				    fprintf(stderr, "%s: font '%c' not defined\n",
				      prog, c);
				    exit(1);
				}
			    else {
				fprintf(stderr, "%s: bad font code in file: '%c'\n",
				  prog, c);
				exit(1);
			    }
			    break;
			case 'D':	/* date string */
			    VOIDC fgets(DateStr, 27, input);
			    FileDate = DateStr;
			    break;
			case 'U':	/* new "user's" heading */
			    {
				static char header[100];
				VOIDC fgets(header, 100, input);
				UsersHeader = header;
				break;
			    }
			case 'H':	/* new heading */
			    {
				static char header[100];

				VOIDC fgets(header, 100, input);
				ClosePage();
				Header = header;
				Page = 0;
				break;
			    }
		    }
		    break;
		case '%':	/* included PostScript line */
		    {
			char psline[200];
			VOIDC fgets(psline, 200, input);
			fprintf(OutFile, "%s\n", psline);
			break;
		    }
		case 014:	/* form feed ^L */
		    if (pagepending && newpage)
			InitPage();
		    PageEject();
		    newpage = TRUE;
		    col = 1;
		    nchars = 1;
		    break;
		case 011:	/* tab ^I */
		    if (pagepending)
			InitPage();
		    /* assume that in the original document, a tab was 8
		       characters, and therefore, the decision of which tab
		       stop to go to should be mod 8 */
		    col = (col - 1) / 8 * 8 + 9;
		    nchars = (nchars - 1) / 8 * 8 + 9;
		    nX = lX + (col - 1) / 8 * TabWidth;
		    if (nX < dX) {
			/* this would result in overprinting, so move over
			   to avoid it */
			overwrites++;
			dX = dX + fonts[whichfont].Xwid['0'];
		    }
		    else
			dX = nX;
		    break;
		default:	/* other control character, take your
				   chances */
		    if (pagepending)
			InitPage();
		    ShowChar(c);
		    col++;
		    nchars++;
	    }
    }
    if (first) {
	/* there was no input! */
	fprintf(stderr, "enscript: null input.\n");
	unlink(tempName);
	exit(1);
    }
    ClosePage();
    fflush(OutFile);
}


/*
 * close the PS file
 */
private VOID ClosePS()
{
    
    fprintf(OutFile,"%%%%Trailer\n");
    if (PreFeed) {
	fprintf(OutFile,"false DoPreFeed\n");
    }
    fprintf(OutFile,"end\n");
    fprintf(OutFile,"%%%%Pages: %d\n", TotalPages);
}


private VOID SetTime(tval)
    long tval;
{
    struct tm *tp;

    if (Gaudy) {
	tp = localtime(&tval);
	VOIDC sprintf(DateStr, "(%02d/%02d/%02d)(%02d:%02d:%02d)",
		tp->tm_year, tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min, tp->tm_sec);
    }
    else {
	VOIDC strcpy(DateStr, ctime(&tval));
	DateStr[24] = '\0'; /* get rid of newline */
    }

    FileDate = DateStr;
}


#ifdef ASE_BV
#define ARGS "12v:gGBlL:coqrRkKmf:F:b:e:p:t:d:n:w:hT:S:s:J:C:P:#:"
#else
#define ARGS "12v:gGBlL:coqrRkKmf:F:b:p:t:d:n:w:hT:S:s:J:C:P:#:"
#endif

private VOID Usage()
{
    fprintf(stderr,"Usage: enscript ");
#ifdef SYSV
    fprintf(stderr,"[-dprinter][-ttitle][-m][-w][-ncopies] ");
#else
    fprintf(stderr,"[-Pprinter][-Cclass][-Jjob][-m][-#copies] ");
#endif
#ifdef ASE_BV
    fprintf(stderr, "[-eencodingfile]");
#endif    
    fprintf(stderr,"[-c][-G][-g][-B][-l][-Llines][-o][-q][-r][-R][-X]");
    fprintf(stderr,"[-ffont][-Fheaderfont][-bheader][-pfile][-ssize]");
    fprintf(stderr,"[-Sfeature=value][-Ttabsize][-h][-1][-2][-vcols]\n");
    exit(0);
}

private VOID ParseArgs(ac, av)
    int ac;
    char **av;
{
    int argp;
    char *p, *q;

    while ((argp = getopt(ac, av, ARGS)) != EOF) {
	debugp((stderr,"option: %c\n",argp));
	switch (argp) {
	    case '1':
	        columns = 1;
		break;
	    case '2':
		columns = 2;
		break;
	    case 'v':
		columns = atoi(optarg);
		break;
	    case 'c': linewrap = FALSE; break;
	    case 'G':
		Gaudy = TRUE;
		if (UsersHeader == NULL) UsersHeader = "";
		if (Header == NULL) Header = "";
		break;
#ifdef ASE_BV
/**
 ** Argument -e filename to specify use of alternative encoding vector
 ** Implies -g
 **
 ** Antony Simmins, Adobe BV SPD TSG, 16/6/93
 **
 **/
	    case 'e':
		EncodingFile = TRUE;
		if (strcpy(EncodingFileName, optarg) != EncodingFileName)
		  { fprintf(stderr, "%s: error in strcpy()\n", prog);
		    exit(1);
		  }
		if (strcat(EncodingFileName, ENCODINGSUFFIX) != EncodingFileName)
		  { fprintf(stderr, "%s: error in strcat()\n", prog);
		    exit(1);
		  }
#endif
	    case 'g': IgnoreGarbage = TRUE; break;
	    case 'B': NoTitle = TRUE; break;
	    case 'l': LPTsimulate = TRUE; NoTitle = TRUE; Lines = 66; break;
	    case 'L': Lines = atoi(optarg); break;
	    case 'o': ListOmitted = TRUE; break;
	    case 'q': BeQuiet = TRUE; break;
	    case 'r': Rotated = TRUE; break;
	    case 'R': Rotated = FALSE; break;
	    case 'k': PreFeed = TRUE; break;
	    case 'K': PreFeed = FALSE; break;
	    case 'f': {
		register char font = 'r';
		int *whichfont;
	        if (*optarg == '-') {
		    font = *++optarg;
		    optarg++;
		}
		if ((font < 'a') || ('z' < font)) {
		    fprintf(stderr,
			"%s: '%c' isn't a valid font designator.\n",
			prog, font);
		    exit(1);
		}
		whichfont = &fontindex[font - 'a'];
		if (*whichfont < 0)
		    *whichfont = nf++;
		decodefont (optarg, &fonts[*whichfont]);
	        }
		break;
	    case 'F':
		decodefont (optarg, &fonts[HeaderFont]);
		break;
	    case 'b':
	        UsersHeader = optarg;
		break;
	    case 'p':
		OutOnly = TRUE;
		VOIDC strcpy(OutName,optarg);
		if (strcmp(OutName,"-") == 0) PipeOut = TRUE;
		break;
	    case 's':
		strncpy(features[nfeatures].keyword,"*PageSize",50);
		if (islower(*optarg)) toupper(*optarg);
		strncpy(features[nfeatures].option,optarg,50);
		features[nfeatures].value = NULL;
		psize = strtondx(features[nfeatures].option);
		nfeatures++;
		break;
	    case 'S':
		p = optarg;
		q = strchr(optarg,'=');
		if (q) {
		    *q = '\0'; q++;
		    strncpy(features[nfeatures].option,q,50);
		}
		else strncpy(features[nfeatures].option,"True",50);
		features[nfeatures].keyword[0] = '*';
		strncat(features[nfeatures].keyword,p,50);
		features[nfeatures].value = NULL;
		nfeatures++;
		useDefault = FALSE;
		break;
	    case 'T':
		tabMultiple = atoi(optarg);
		break;
	    case 'h': spoolNoBurst = TRUE; break;
	/* SYS V lp and BSD lpr options processing */
	    case 'm': case 'w':
	    	spoolNotify = argp; break;
	    case 'n': case '#': spoolCopies = optarg; break;
	    case 'd': case 'P':
		PrinterName = optarg;
		PSpecified = TRUE;
		break;
	    case 't': spoolTitle = optarg; break;
	    case 'C': spoolJobClass = optarg; break;
	    case 'J': spoolJobName = optarg; break;
	    case '?':
	    default:
		Usage();
	    	break;
	}
    }
}


/* addarg is used to construct an argv for the spooler */
private VOID addarg(argv, argstr, argc)
    char **argv;
    char *argstr;
    register int *argc;
{
    register char *p = (char *) malloc((unsigned) (strlen(argstr) + 1));
    VOIDC strcpy(p, argstr);
    argv[(*argc)++] = p;
    argv[*argc] = NULL;
}

    
private VOID SpoolIt()
{
    char temparg[200];
    char *argstr[200];
    int nargs = 0;
#ifdef SYSV
    int cpid, wpid;
#endif

    addarg(argstr, lprcmd, &nargs);
#ifdef SYSV
    addarg(argstr, "-c", &nargs);

    if ((PrinterName == NULL) && ((PrinterName = envget("LPDEST")) == NULL)) {
	PrinterName = POSTSCRIPTPRINTER;
    }
    VOIDC sprintf(temparg,"-d%s",PrinterName);
    addarg(argstr, temparg, &nargs);
    if (!BeQuiet) fprintf(stderr,"spooled to %s\n",PrinterName);

    if (spoolNotify) {
	VOIDC sprintf(temparg,"-%c",spoolNotify);
	addarg(argstr, temparg, &nargs);
    }
    if (atoi(spoolCopies) > 1) {
	VOIDC sprintf(temparg,"-n%s",spoolCopies);
	addarg(argstr, temparg, &nargs);
    }
    if (BeQuiet) {
	addarg(argstr, "-s", &nargs);
    }
    if (spoolTitle) {
	VOIDC sprintf(temparg,"-t%s",spoolTitle);
    }
    else {
	VOIDC sprintf(temparg, "-t%s", (FileName == NULL) ? "stdin" : FileName);
    }
    if (spoolNoBurst) {
	addarg(argstr,"-o-h",&nargs);
    }
    addarg(argstr, temparg, &nargs);

#else
    /* BSD spooler */
    if (atoi(spoolCopies) > 1) {
	VOIDC sprintf(temparg,"-#%s",spoolCopies);
	addarg(argstr, temparg, &nargs);
    }
    if ((PrinterName == NULL) && ((PrinterName = envget("PRINTER")) == NULL)){
	PrinterName = POSTSCRIPTPRINTER;
    }
    VOIDC sprintf(temparg,"-P%s",PrinterName);
    addarg(argstr, temparg, &nargs);
    if (!BeQuiet) fprintf(stderr,"spooled to %s\n",PrinterName);

    if (spoolJobClass) {
	addarg(argstr, "-C", &nargs);
	addarg(argstr, spoolJobClass, &nargs);
    }
    addarg(argstr, "-J", &nargs);
    if (spoolJobName) {
	addarg(argstr, spoolJobName, &nargs);
    }
    else {
	if (!FileName) addarg(argstr, "stdin", &nargs);
	else addarg(argstr, FileName, &nargs);
    }
    if (spoolNotify) {
	addarg(argstr, "-m", &nargs);
    }
    if (spoolNoBurst) {
	addarg(argstr, "-h", &nargs);
    }

    /* remove the temporary file after spooling */
    addarg(argstr, "-r", &nargs); /* should we use a symbolic link too? */
#endif
    addarg(argstr, tempName, &nargs);

#ifdef DEBUG
    { int i;
    fprintf(stderr,"called spooler with: ");
    for (i = 0; i < nargs ; i++) fprintf(stderr,"(%s)",argstr[i]);
    fprintf(stderr,"\n");
    }
#endif

#ifdef SYSV
    if ((cpid = fork()) < 0) {pexit2(prog,"can't fork spooler",1);}
    else if (cpid) {
	while (wpid = wait((int *) 0) > 0) {if (wpid == cpid) break;}
	VOIDC unlink(tempName);
    }
    else {
	execvp(lprcmd, argstr);
	pexit2(prog,"can't exec spooler",1);
    }
#else
    execvp(lprcmd, argstr);
    pexit2(prog,"can't exec spooler",1);
#endif
}



main(argc, argv)
    int argc;
    char  **argv;
{
    register char *p;		/* pointer to "ENSCRIPT" in env */
    int i;

    prog = *argv;		/* argv[0] is program name */

#ifdef DEBUG
    fprintf(stderr,"enscript args: ");
    for (i=0;i<argc;i++)
	fprintf(stderr,"%s ",argv[i]);
    fprintf(stderr,"\n");
#endif /* DEBUG */

    debugp((stderr,"PL %ld PW %ld TPL %ld TPW %ld\n",pageSizes[psize].imagelength,pageSizes[psize].imagewidth,pageSizes[psize].pagelength,pageSizes[psize].pagewidth));

    if (signal(SIGINT, int1) == SIG_IGN) {
	VOIDC signal(SIGINT, SIG_IGN);
	VOIDC signal(SIGQUIT, SIG_IGN);
	VOIDC signal(SIGHUP, SIG_IGN);
	VOIDC signal(SIGTERM, SIG_IGN);
    }
    else {
	VOIDC signal(SIGQUIT, int1);
	VOIDC signal(SIGHUP, int1);
	VOIDC signal(SIGTERM, int1);
    }

    {	register int    i;
	for (i = 0; i < 26; i++)
	    fontindex[i] = -1;
    }

    strncpy(lprcmd,lp,1024);

    if ((libdir = envget("PSLIBDIR")) == NULL) libdir = PSLibDir;
    if ((tempdir = envget("PSTEMPDIR")) == NULL) tempdir = TempDir;
#ifdef SYSV
    PrinterName = envget("LPDEST");
#endif /* SYSV */
#ifdef BSD
    PrinterName = envget("PRINTER");
#endif

    Roman = CurFont = DefineFont (BODYROMAN, 10);
    HeaderFont = DefineFont (HEADFONT, 10);

    /* build our big arg vector, with environment and command line combined */
    /* we're doing this, rather than calling ParseArgs twice, because */
    /* the Sun man page for 4.0 says that calling getopt with different */
    /* argv's may result in unexpected behavior. */
    
    /* first, let's put in the program name */
    bigargv[bigargc++] = argv[0];
    /* process args in environment variable ENSCRIPT */
    /* throw them into our bigargv, we'll tack on command line later */
    if (p = envget("ENSCRIPT")) {
	while (*p != '\0') {
	    register char quote = ' ';
	    while (*p == ' ') p++;
	    if ((*p == '\"') || (*p == '\'')) quote = *p++;
	    if (*p != '\0') bigargv[bigargc++] = p;
	    while ((*p != quote) && (*p != '\0')) p++;
	    if (*p == '\0') break;
	    *p++ = '\0';
	}
    }

    /* tack on command line arguments */
    for (i = 1; i < argc; i++)
	bigargv[bigargc++] = argv[i];

    ParseArgs(bigargc, bigargv);

    /* process non-option args */
    for (; optind < bigargc ; optind++) {
	FileName = Header = bigargv[optind];
	SeenFile = TRUE;
	if (FileName) {
	    if ((infile = fopen(FileName,"r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", prog, FileName);
		continue;
	    }
	}
	else infile = stdin;
	VOIDC fstat(fileno(infile), &S);
	SetTime(S.st_mtime);
	CopyFile();
	VOIDC fclose(infile);
    }
    if (!SeenFile) {
	FileName = Header = Gaudy ? "" : 0;
	infile = stdin;
	VOIDC fstat(fileno(infile), &S);

	if ((S.st_mode & S_IFMT) == S_IFREG)
	    SetTime(S.st_mtime);
	else
	    SetTime(time((long *)0));
	CopyFile();
    }

    if (Cvted) {
	ClosePS();
	if (fclose(OutFile) != 0) {
	    fprintf(stderr,"enscript: couldn't close output ");
	    perror("");
	}
	OutFile = 0;
    }
    if (LinesWrapped && !BeQuiet)
	fprintf(stderr,"%s: %d lines were wrapped because of length.\n",
		prog, LinesWrapped);
    if (TruncChars && !BeQuiet)
	fprintf(stderr,"%s: %d characters omitted because of long lines.\n",
		prog, TruncChars);
    if (UndefChars && !BeQuiet)
	fprintf(stderr,"%s: %d characters omitted because of incomplete fonts.\n",
		prog, UndefChars);
    if (overwrites && !BeQuiet) {
	fprintf(stderr,"%s: %d tabs could overlap previous characters.\n",
		prog, overwrites); 
	fprintf(stderr,"Use '-T' to adjust the width of tabs.\n");
    }
    if (!BeQuiet && (TotalPages > 0)) {
	fprintf(stderr,"[ %d page%s * %s cop%s ] ",
		TotalPages, TotalPages > 1 ? "s" : "",
		spoolCopies, atoi(spoolCopies) > 1 ? "ies" : "y" );
    }
    if (Cvted) {
	if (OutOnly) {
	    if (!BeQuiet) {
		fprintf(stderr,"left in %s\n", OutName);
	    }
	}
	else {
	    SpoolIt(); /* does an exec */
	}
    }
    exit(0);
}


/* signal catcher */
private VOID int1()
{
    if ((!PipeOut) && (*OutName != '\0')) {
	VOIDC unlink(OutName);
    }
    exit(1);
}
