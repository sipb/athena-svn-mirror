/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1995 Patrick Powell, San Diego State University
 *     papowell@sdsu.edu
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: textps.c
 * PURPOSE: convert text to postscript file
 **************************************************************************/

static const char *const _id =
"textps.c,v 1.1 1997/01/05 04:58:19 papowell Exp";

/***************************************************************************

From the Original LPRPS Distribution
Mon Aug 21 07:15:20 PDT 1995 Patrick Powell

This is lprps version 2.6plp.

lprps is a collection of programs for using lpr with a PostScript
printer connected by a bidirectional serial channel.  It has been
tested mainly with Suns running various versions of SunOS (4.0.3, 4.1
and 4.1.1).

It contains the following programs:

textps  simple text to PostScript filter

* Text to PostScript filter supports ISO Latin-1.
There is no copyright on lprps.

James Clark
jjc@jclark.com

 Id: README,v 1.14 1993/02/22 12:43:37 jjc Exp


The orignal code has been hacked, slashed, bombproofed, reformatted, debugged,
and just generally abused beyond all reasonable bounds.  Typical work.

Patrick Powell Mon Aug 21 08:48:10 PDT 1995
<papowell@sdsu.edu>

 ***************************************************************************/

/* textps.c */

#include "ifhp.h"

#ifndef TAB_WIDTH
# define TAB_WIDTH 8
#endif /* not TAB_WIDTH */

#ifndef LINES_PER_PAGE
# define LINES_PER_PAGE 66
#endif /* not LINES_PER_PAGE */

#ifndef PAGE_WIDTH
# ifdef A4
#   define PAGE_WIDTH (8.25*72) /* not exact, but close */
#  else /* this funny format these US folks want to make us use... */
#   define PAGE_WIDTH (8.5*72)
# endif
#endif /* not PAGE_WIDTH */

#ifndef CPI
# define CPI (12.0)
#endif /* not CPI */

/* page length in points, 72/inch */
#ifndef PAGE_LENGTH
# ifdef A4
#   define PAGE_LENGTH 842.0
# else /* 11 inches */
#   define PAGE_LENGTH (11.0*72)
# endif
#endif /* PAGE_LENGTH */

#ifndef BASELINE_OFFSET
# define BASELINE_OFFSET 72.0
#endif /* not BASELINE_OFFSET */

#ifndef BOTTOM_OFFSET
# define BOTTOM_OFFSET 36.0
#endif /* not BOTTOM_OFFSET */

#ifndef VERTICAL_SPACING
# define VERTICAL_SPACING 8.0
#endif /* not VERTICAL_SPACING */

#ifndef LEFT_MARGIN
# define LEFT_MARGIN 54.0
#endif /* not LEFT_MARGIN */

#ifndef RIGHT_MARGIN
# define RIGHT_MARGIN 54.0
#endif /* not RIGHT_MARGIN */

#ifndef FONT
# define FONT "Courier"
#endif /* not FONT */

#ifndef BOLD_FONT
# define BOLD_FONT "Courier-Bold"
#endif /* not BOLD_FONT */

extern char *optarg;
extern int optind, getopt();

double header_height = 0.50*72;
/* you may want to have more margins */
#ifndef TOPDROP
#define TOPDROP .125
#endif
double topdrop = TOPDROP*72;
double header_width = 1.25*72;
double bar_height = 0.25*72;

typedef struct bar {
	double llx, lly;
	double dx, dy;
	double pts;
	double gray;
	char *font;
	char *str;
} bar;

bar Lhdr, Rhdr, Center;

typedef struct output_char {
  int c;
  int is_bold;
} output_char;

output_char *output_char_list;
int max_line_len = 256;
int max_h_pos;

int page_started;
double page_width = PAGE_WIDTH;	/* in points */
double column_width;				/* in points */
double font_size;
int line_width;				/* max chars in line */

int tab_width = TAB_WIDTH;
double cpi = CPI;                    /* characters per inch */
int lines_per_page = LINES_PER_PAGE;
double page_length = PAGE_LENGTH; /* in points */
     /* distance in points from top of page to first baseline */
double baseline_offset = BASELINE_OFFSET;
double bottom_offset = BOTTOM_OFFSET;
double vertical_spacing = VERTICAL_SPACING; /* in points */
double left_margin = LEFT_MARGIN; /* in points */
double right_margin = RIGHT_MARGIN; /* in points */
char *font = FONT;
char *bold_font = BOLD_FONT;
char *times_font = "Times-Roman";
char *times_bold = "Times-Bold";
double times_size = 12.0;
double times_bold_size = 14.0;
int rotated;		/* rotated */
int double_cols;	/* two columns */
int line_count;		/* number on line */
int gaudy;			/* do header */
char *datestr;
char *filename;

int column;
int vpos;
int hpos;

double char_width;

void PUTSTR( char *s );

int pageno = 0;
int filepage = 0;
int debug;
int nscript;
char *outputfile;
FILE *outfp;
char *printer;

enum { NONE, ROMAN, BOLD } current_font;

char outbuffer[256];

void do_file();
void prologue();
void trailer();
char *prog;

char *use_m[] = {
	"usage: %s [-T=opts[,opts]]* [files ...]\n",
	"r     - rotated or landscape mode\n",
	"d     - double columns\n",
	"c=n   - set cpi (characters per inch)\n",
	"l=n   - set lines per page\n",
	"m=n   - set left margin (points)\n",
	"t=n   - space from top of page to first line (points)\n",
	"v=n   - vertical spacing (points)\n",
	"files  - files to print\n",
	0
};
char *use_n[] = {
	"usage: %s [-2Gr]* [-Ln] [-pout] [-Pprinter] [files ...]\n",
	"r     - rotated or landscape mode\n",
	"2     - double columns\n",
	"G     - gaudy output with headers"
	"Ln    - set lines per page\n",
	"pout  - output to file\n",
	"Pprinter  - lpr to printer\n",
    0
};

void usage()
{
	int i;
	char **m = use_m;
	if( nscript ) m = use_n;
	for( i = 0; m[i]; ++i ){
		fprintf(stderr, m[i], prog );
	}
	exit(JABORT);
}


int main(int argc, char **argv)
{
  int bad_files = 0;
  int opt;
  char *arg, *s, *end = 0;
  double t;
  int c;

  prog = argv[0];

  if( (s = strrchr( prog, '/' )) ){
	++s;
  } else {
    s = prog;
  }
  if( !strcmp(s, "nscript" ) ){
	nscript = 1;
  }

  
  output_char_list = malloc( sizeof(output_char_list[0])*max_line_len);

  if( !nscript ) for( optind = 1; optind < argc; ++optind ){
	arg = argv[optind];
	if( arg[0] != '-' || (opt = arg[1]) == '-' ) break;
	optarg = &arg[2];
	switch (opt) {
	case 'D': debug = atoi(optarg); break;
	case 'T':
      for( s = optarg; s ; s = end ){
        if( debug ) fprintf(stderr, "arg '%s'\n", s );
        while( isspace( cval(s) ) ) ++s;
        if( (end = strchr( s, ','))  ){
          *end++ = 0;
        }
        if( debug ) fprintf(stderr, "arg '%s'\n", s );
        optarg = s+2;
        switch( s[0] ){
          case 'g': gaudy = 1; break;
          case 'r': rotated = 1; break;
          case 'd': double_cols = 1; break;
          case 'c': if (sscanf(optarg, "%lf", &cpi) != 1) usage();
            break;
          case 'l': if (sscanf(optarg, "%d", &lines_per_page) != 1) usage();
            break;
          case 'm': if (sscanf(optarg, "%lf", &left_margin) != 1) usage();
			right_margin = left_margin;
            break;
          case 't': if (sscanf(optarg, "%lf", &baseline_offset) != 1) usage();
            break;
          case 'v': if (sscanf(optarg, "%lf", &vertical_spacing) != 1) usage();
            break;
          default: usage();
            break;
        }
      }
      break;
	}
  } else {
	while( (c = getopt( argc, argv, "12GrL:p:P:t:" )) != EOF ){
		switch( c ){
          case 'G': gaudy = 1; break;
          case 'r': rotated = 1; break;
          case '2': double_cols = 1; break;
          case 'L': lines_per_page = atoi( optarg ); break;
          case 'p': outputfile = optarg; break;
          case 'P': printer = optarg; break;
          case 't': tab_width = atoi(optarg);
				if( tab_width <= 0 ) tab_width = TAB_WIDTH;
				break;
          default: usage();
        }
    }
  }

	if( outputfile ){
		outfp = fopen( outputfile, "w" );
		if( outfp == NULL ){
			c = errno;
			sprintf( outbuffer, "cannot open '%s'", outputfile );
			errno = c;
			perror( outbuffer );
			exit( JABORT );
		}
		if( fileno(outfp) != 1 ){
			dup2( fileno(outfp), 1 );
			fclose( outfp );
		}
	}
  if( nscript && outputfile == 0 ){
	char cmd[128];
	strcpy( cmd, "lpr " );
    if( printer ){
		if( strlen(printer) > (sizeof(cmd) - strlen(cmd) - 10) ){
			fprintf( stderr, "printer name too long\n" );
			usage();
		}
		sprintf( cmd+strlen(cmd), " -P%s", printer );
	}
    outfp = popen( cmd, "w" );
	if( outfp == NULL ){
		c = errno;
		sprintf( outbuffer, "popen '%s' failed", cmd );
		errno = c;
		perror( outbuffer );
		exit( JABORT );
	}
	if( fileno(outfp) != 1 ){
		dup2( fileno(outfp), 1 );
	}
  }
  if( rotated ){
    t = page_width; page_width = page_length; page_length = t;
  }
  column_width = page_width/(1+double_cols);
  if( double_cols ){
	/* courier 10 has 12 CPI, 12 point spacing */
	/* courier 7 has   cpi */
    left_margin = left_margin*0.5;
    right_margin = right_margin*0.5;
    font_size = 7;
    char_width = .6*font_size; /* from 10 points, proportional */
    cpi = 72/char_width;
    vertical_spacing = font_size+1;
	bottom_offset = .25*72;
  } else {
	/* courier 10 has 12 CPI, 12 point line spacing */
    char_width = 72.0/cpi;
    font_size = char_width/.6;
    vertical_spacing = font_size*1.0;
  }
  topdrop = header_height + topdrop;
  if( gaudy ){
    baseline_offset = topdrop + 2*vertical_spacing;
  }
  line_width = (column_width-left_margin-right_margin)/char_width;
  Lhdr.str = "date";
  Rhdr.str = "page";
  Center.str = "title";
  Lhdr.gray = Rhdr.gray = 0.80;
  Center.gray = 0.95;
  Lhdr.pts = 12.0;
  Lhdr.font = "TR";
  Rhdr.pts = 14.0;
  Rhdr.font = "TB";
  Center.font = "TR";
  Center.pts = 12;
  Rhdr.dx = Lhdr.dx = header_width;
  Rhdr.dy = Lhdr.dy = header_height;
  Center.dy = bar_height;
  Lhdr.lly = Rhdr.lly = Center.lly = page_length - topdrop;
  Lhdr.llx = left_margin;
  Center.llx = Lhdr.llx + Lhdr.dx;
  Rhdr.llx = page_width - left_margin - right_margin - Rhdr.dx;
  Center.dx = Rhdr.llx - Center.llx;
    line_count = (int)((column_width-left_margin)/char_width)-1;
	t = (int)(page_length - baseline_offset-bottom_offset )/vertical_spacing;
    if( t < lines_per_page ) lines_per_page = t;

  prologue();
  if (optind >= argc){
	filename = "STDIN";
	do_file();
  } else {
	int i;
	for (i = optind; i < argc; i++)
	  if (strcmp(argv[i], "-") != 0
		  && freopen((filename = argv[i]), "r", stdin) == NULL) {
		perror(argv[i]);
		bad_files++;
	  } else {
		do_file();
	  }
  }
  trailer();
  exit(JSUCC);
}



void add_char( int c )
{
	output_char *tem;
	if( hpos >= max_line_len ){
		max_line_len = hpos + 100;
		output_char_list = realloc( output_char_list, 
			sizeof(output_char_list[0])*max_line_len );
		if( output_char_list == 0 ){
			perror("realloc failed");
			exit( JFAIL );
		}
	}
	while( max_h_pos <= hpos  ){
		tem = &output_char_list[max_h_pos];
		memset( tem, 0, sizeof(tem[0]));
		tem->c = ' ';
		++max_h_pos;
	}
	tem = &output_char_list[hpos];
	if( tem->c == c && c != ' ' ) tem->is_bold = BOLD;
	tem->c = c;
	++hpos;
}

void pschar( int c)
{
  int i = 0;
  c = c & 0377;
  if (!isascii(c) || iscntrl(c)){
	sprintf( outbuffer, "\\%03o", c );
  } else if (c == '(' || c == ')' || c == '\\') {
	outbuffer[i++] = '\\';
	outbuffer[i++] = c;
	outbuffer[i++] = 0;
  } else {
	outbuffer[i++] = c;
	outbuffer[i++] = 0;
  }
  PUTSTR( outbuffer );
}

/* print an integer approximation to floating number */
void psnum(double f)
{
  char *p;
  sprintf(outbuffer, "%f", f);
  if( (p = strchr( outbuffer, '.' )) ) *p = 0;
  if( strlen(outbuffer) == 0 ) strcpy(outbuffer, "0" );
  PUTSTR( outbuffer );
}

/* put out the line */

void end_line( int start_pos, int vpos )
{
	PUTSTR(")");
	psnum(column_width*column + left_margin + start_pos*char_width);
	PUTSTR(" ");
	psnum(page_length - baseline_offset - vpos*vertical_spacing);
	PUTSTR(" L\n");
	/*
    printf( "%% page_length %d, baseline_offset %d, vpos %d, vspace %d\n",
		(int)page_length, (int)baseline_offset, (int)vpos,
		(int)vertical_spacing );
	*/
	fflush(stdout);
}

void page_start()
{
  sprintf( outbuffer, "%%%%Page: ? %d\n%%%%BeginPageSetup\nPS\n%%%%EndPageSetup\n",
		 ++pageno);
  ++filepage;
  PUTSTR( outbuffer );
  if( rotated ){
	sprintf( outbuffer, "%d 0 translate 90 rotate\n", (int)page_length );
	PUTSTR( outbuffer );
  }
  if( gaudy ){
    if( datestr == 0 ){
		static char timestr[64];
		time_t t = time((void*)0);
		struct tm *ltime;

		ltime = localtime(&t);
		sprintf( timestr,"(%02d/%02d/%02d) (%02d:%02d:%02d)",
			ltime->tm_year, ltime->tm_mon+1, ltime->tm_mday,
			ltime->tm_hour, ltime->tm_min, ltime->tm_sec );
		datestr = timestr;
	}
    sprintf( outbuffer, "/page [(%d)] def\n", filepage);
	PUTSTR( outbuffer );
    sprintf( outbuffer, "/date [%s] def\n", datestr );
	PUTSTR( outbuffer );
    sprintf( outbuffer, "/title [(%s)] def\n", filename );
	PUTSTR( outbuffer );
    sprintf( outbuffer, "HDR\n" );
	PUTSTR( outbuffer );
  }
  current_font = NONE;
  page_started = 1;
  column = 0;
}

void page_end( int endjob )
{
  if( !endjob && column < double_cols ){
    ++column;
  } else {
    if( page_started ) PUTSTR("PE\n");
    page_started = 0;
    column = 0;
  }
  vpos = 0;
}

/*static int ln;*/

void print_line(int advance)
{
  int i, c;

  int font = 0;
  output_char *tem;
  int start_pos = 0;

  /* fprintf( stderr, "ln[%d] max_h_pos '%d'\n", ++ln, max_h_pos ); */
  /*
  for( i = 0; i < max_h_pos; ++i ){
	fprintf( stderr, "[%d] '%c'\n", i, output_char_list[i].c );
  }
  */
  while( max_h_pos > line_width ){
    i = max_h_pos;
    max_h_pos = line_width;
    print_line( 1 );
    for( c = 0; c + line_width < i; ++c ){
		output_char_list[c] = output_char_list[c+line_width];
	}
    max_h_pos = i - line_width;
  }

  if (!page_started) page_start();

  for( i = 0; i < max_h_pos; ++i ){
    tem = &output_char_list[i];
	c = tem->c;
	if( c == ' ' && current_font != NONE ){
		font = current_font;
	} else if( tem->is_bold ){
        font = BOLD;
    } else {
        font = ROMAN;
	}
	if( current_font != font ){
	  if( start_pos != i ){
	    end_line(start_pos,vpos);
	  }
      start_pos = i;
      if( font == BOLD ){
	    PUTSTR( "B" );
      } else {
        PUTSTR( "R" );
      }
      current_font = font;
      start_pos = i;
    }
	if( start_pos == i ){
		PUTSTR("(");
	};
	pschar(c);
  }

  if( start_pos != i ){
	end_line( start_pos, vpos );
  }
  if( advance ) ++vpos;
  if (vpos >= lines_per_page) {
    page_end(0);
  }
  hpos = 0;
  max_h_pos = 0;
}

char *defs[] = {
  /* set up a dictionary with 10 entries*/
  "/textps 30 dict def textps begin",
  /* line output */
  "/L { moveto show } bind def",
  /* page start */
  "/PS { /level0 save def } bind def",
  /* page end */
  "/PE { level0 restore showpage } bind def",
  /* box at (llx,lly) (len=dx,ht=dy,color=boxcolor), text size */
  "/BX{ % llx lly dx dy boxcolor pts textcolor [label]",
  "  /label exch def",
  "  /textcolor exch def",
  "  /pts exch def",
  "  /boxcolor exch def",
  "  /dy exch def",
  "  /dx exch def",
  "  /lly exch def",
  "  /llx exch def",
  "  /yh label length pts mul pts 0.3 mul add def",
  "  /ypos lly dy yh sub 2 div add yh add def",
  "  gsave",
  "    boxcolor setgray",
  "    llx lly moveto",
  "      dx 0 rlineto 0 dy rlineto dx neg 0 rlineto",
  "      closepath fill",
  "    textcolor setgray",
  "    label {",
  "        dup",
  "          /ypos ypos pts sub def",
  "          stringwidth pop /strxh exch def",
  "            /xpos llx dx strxh sub 2 div add def",
  "          xpos ypos moveto show",
  "    } forall",
  "  grestore",
  "}def",
  "",
  /* font change establishment */
  "/RE {",
  "\tfindfont",
  "\tdup maxlength dict begin",
  "\t{",
  "\t\t1 index /FID ne { def } { pop pop } ifelse",
  "\t} forall",
  "\t/Encoding exch def",
  "\tdup /FontName exch def",
  "\tcurrentdict end definefont pop",
  "} bind def",
  (char *)0
};

char *latin1[] = {
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  "space",
  "exclam",
  "quotedbl",
  "numbersign",
  "dollar",
  "percent",
  "ampersand",
  "quoteright",
  "parenleft",
  "parenright",
  "asterisk",
  "plus",
  "comma",
  "hyphen",  /* this should be `minus', but not all PS printers have it */
  "period",
  "slash",
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "colon",
  "semicolon",
  "less",
  "equal",
  "greater",
  "question",
  "at",
  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",
  "P",
  "Q",
  "R",
  "S",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z",
  "bracketleft",
  "backslash",
  "bracketright",
  "asciicircum",
  "underscore",
  "quoteleft",
  "a",
  "b",
  "c",
  "d",
  "e",
  "f",
  "g",
  "h",
  "i",
  "j",
  "k",
  "l",
  "m",
  "n",
  "o",
  "p",
  "q",
  "r",
  "s",
  "t",
  "u",
  "v",
  "w",
  "x",
  "y",
  "z",
  "braceleft",
  "bar",
  "braceright",
  "asciitilde",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  ".notdef",
  "dotlessi",
  "grave",
  "acute",
  "circumflex",
  "tilde",
  "macron",
  "breve",
  "dotaccent",
  "dieresis",
  ".notdef",
  "ring",
  "cedilla",
  ".notdef",
  "hungarumlaut",
  "ogonek",
  "caron",
  ".notdef",
  "exclamdown",
  "cent",
  "sterling",
  "currency",
  "yen",
  "brokenbar",
  "section",
  "dieresis",
  "copyright",
  "ordfeminine",
  "guilsinglleft",
  "logicalnot",
  "hyphen",
  "registered",
  "macron",
  "degree",
  "plusminus",
  "twosuperior",
  "threesuperior",
  "acute",
  "mu",
  "paragraph",
  "periodcentered",
  "cedilla",
  "onesuperior",
  "ordmasculine",
  "guilsinglright",
  "onequarter",
  "onehalf",
  "threequarters",
  "questiondown",
  "Agrave",
  "Aacute",
  "Acircumflex",
  "Atilde",
  "Adieresis",
  "Aring",
  "AE",
  "Ccedilla",
  "Egrave",
  "Eacute",
  "Ecircumflex",
  "Edieresis",
  "Igrave",
  "Iacute",
  "Icircumflex",
  "Idieresis",
  "Eth",
  "Ntilde",
  "Ograve",
  "Oacute",
  "Ocircumflex",
  "Otilde",
  "Odieresis",
  "multiply",
  "Oslash",
  "Ugrave",
  "Uacute",
  "Ucircumflex",
  "Udieresis",
  "Yacute",
  "Thorn",
  "germandbls",
  "agrave",
  "aacute",
  "acircumflex",
  "atilde",
  "adieresis",
  "aring",
  "ae",
  "ccedilla",
  "egrave",
  "eacute",
  "ecircumflex",
  "edieresis",
  "igrave",
  "iacute",
  "icircumflex",
  "idieresis",
  "eth",
  "ntilde",
  "ograve",
  "oacute",
  "ocircumflex",
  "otilde",
  "odieresis",
  "divide",
  "oslash",
  "ugrave",
  "uacute",
  "ucircumflex",
  "udieresis",
  "yacute",
  "thorn",
  "ydieresis",
};

/* llx lly dx dy boxcolor pts textcolor label BX */
void do_bar( bar *p )
{
	sprintf( outbuffer,
	"%s %f %f %f %f %f %f 0 %s BX\n",
	p->font, p->llx, p->lly, p->dx, p->dy, p->gray, p->pts, p->str );
    PUTSTR( outbuffer );
}
  
void prologue()
{
  int col, i;

  PUTSTR( "%!PS-Adobe-3.0\n");

  sprintf( outbuffer, "%%%%DocumentNeededResources: font %s\n", font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%+ font %s\n", bold_font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%+ font %s\n", times_font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%+ font %s\n", times_bold);
  PUTSTR( outbuffer );

  PUTSTR( "%%Pages: (atend)\n");
  PUTSTR( "%%EndComments\n");
  PUTSTR( "%%BeginProlog\n");
  for (i = 0; defs[i]; i++){
    PUTSTR( defs[i] );
    PUTSTR( "\n" );
  }
  PUTSTR( "/date [()] def\n" );
  PUTSTR( "/page [()] def\n" );
  PUTSTR( "/title [()] def\n" );
  PUTSTR( "/HDR { gsave\n" );
  do_bar( &Rhdr );
  do_bar( &Lhdr );
  do_bar( &Center );
  sprintf( outbuffer, "0 setlinewidth %f %f moveto %f %f lineto stroke\n",
	 column_width, page_length - topdrop, column_width, bottom_offset );
  PUTSTR( outbuffer );
  PUTSTR( "grestore } def\n" );
  PUTSTR( "/ISOLatin1Encoding where{pop}{/ISOLatin1Encoding[\n");

  col = 0;
  for (i = 0; i < 256; i++) {
	int len = strlen(latin1[i]) + 2;
	col += len;
	if (col > 79) {
	  PUTSTR("\n");
	  col = len;
	}
	sprintf( outbuffer, "/%s ", latin1[i]);
    PUTSTR( outbuffer );
  }
  PUTSTR( "\n] def}ifelse\nend\n");
  PUTSTR( "%%BeginSetup\n");

  sprintf( outbuffer, "%%%%IncludeResource: font %s\n", font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%IncludeResource: font %s\n", bold_font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%IncludeResource: font %s\n", times_font);
  PUTSTR( outbuffer );
  sprintf( outbuffer, "%%%%IncludeResource: font %s\n", times_bold);
  PUTSTR( outbuffer );

  PUTSTR( "textps begin\n");

  sprintf( outbuffer, "/__%s ISOLatin1Encoding /%s RE\n", font, font);
  PUTSTR( outbuffer );

  sprintf( outbuffer,
	"/R { /__%s findfont %f scalefont setfont } def\n",
		 font, font_size);
  PUTSTR( outbuffer );

  sprintf( outbuffer, "/__%s ISOLatin1Encoding /%s RE\n", bold_font, bold_font);
  PUTSTR( outbuffer );

  sprintf( outbuffer,
	"/B { /__%s findfont %f scalefont setfont } def\n", 
		 bold_font, font_size);
  PUTSTR( outbuffer );

  sprintf( outbuffer,
	"/TR { /%s findfont %f scalefont setfont } def\n", 
		 times_font, times_size);
  PUTSTR( outbuffer );
  sprintf( outbuffer,
	"/TB { /%s findfont %f scalefont setfont } def\n", 
		 times_bold, times_bold_size);
  PUTSTR( outbuffer );

  PUTSTR( "%%EndSetup\n");
  PUTSTR( "%%EndProlog\n");
}

void trailer()
{
  sprintf( outbuffer, "%%%%Trailer\nend\n%%%%Pages: %d\n", pageno);
  PUTSTR( outbuffer );
}

void do_file()
{
  int c;
  int esced = 0;
  filepage = 0;
  while ((c = getchar()) != EOF){
	if (esced) switch(c) {
	  case '7':
		if (hpos > 0) {
			if (!page_started) page_start();
			print_line(1);
		}
		esced = 0;
		break;
	  default:
		/* silently ignore */
		esced = 0;
		break;
	} else {
	  switch (c) {
	  case '\033':
		esced = 1;
		break;
	  case '\b':
		if (hpos > 0) hpos--;
		break;
	  case '\f':
		if (!page_started) page_start();
		print_line(1);
		if(vpos<=0) page_start();
		page_end(0);
		break;
	  case '\r':
		if (!page_started) page_start();
		print_line(0);
		break;
	  case '\n':
		if (!page_started) page_start();
		print_line(1);
		break;
	  case '\t':
		hpos = ((hpos + tab_width)/tab_width)*tab_width;
		break;
	  default:
		if (!(isascii(c) && iscntrl(c))) {
		  add_char(c);
		}
		break;
	  }
    }
  }
  if (hpos > 0 ){
	if (!page_started) page_start();
	print_line(1);
  }
  if (page_started) page_end(1);
}

void PUTSTR( char *msg )
{
	int len, i;

	i = len = strlen(msg);
	while( len > 0 && (i = write( 1, msg, len ) ) > 0 ){
		len -= i, msg += i;
	}
	if( i < 0 ) exit( JFAIL );
}
