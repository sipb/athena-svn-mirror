/*========================================================================*\

Copyright (c) 1990-2001  Paul Vojta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
PAUL VOJTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

NOTE:
	xdvi is based on prior work, as noted in the modification history
	in xdvi.c.

\*========================================================================*/

#include "xdvi-config.h"
#include <stdlib.h>
#include "message-window.h"	/* for the message popups */
#include "kpathsea/c-ctype.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-stat.h"
#include "kpathsea/magstep.h"
#include "kpathsea/tex-file.h"
#include "kpathsea/c-vararg.h"
#include "kpathsea/expand.h"  /* for kpse_path_expand() */
#include "dvi.h"
#include "xdvi.h"
#include "string-utils.h"

#include <ctype.h>

#if HAVE_VFORK_H
# include <vfork.h>
#endif

#ifdef	DOPRNT	/* define this if vfprintf gives you trouble */
#define	vfprintf(stream, message, args)	_doprnt(message, args, stream)
#endif

#define BUF_SIZE 1024
/* hashing stuff for fontname lookup */
#define T1FONTS_INITIAL_HASHTABLE_SIZE 1031
static hash_table_type t1fonts_hash;
static hash_table_type tfminfo_hash;
static hash_table_type fontmaps_hash;

typedef enum { FAILURE_BLANK = -2, FAILURE_PK = -1, SUCCESS = 0 } t1font_load_status_t;

#ifdef T1LIB
#define PRIVATE static
#define PUBLIC

#if defined(HAVE_STRING_H)
#include <string.h>
#else
extern char *strtok(char *, _Xconst char *);
#endif

#include "dvi.h"
#include "kpathsea/tex-file.h"

#include "t1lib.h"

/* Datastructures:

   encodings[n]: Loaded from map file
   [0] = 8r,	<vector>
   [1] = 8c,	<vector>

   The vectors are demand-loaded in 'load_encoded_font'

*/

typedef struct encoding {
    char *enc;	/* This may be NULL.  From dvips maps we get 
		   encodings loaded by filename, and no name given */
    char *file;
    char **vector;
} encoding;

PRIVATE encoding *encodings;	/* Dynamic array = realloc when it gets full */
#define ENCSTART 16	/* Start size.  Normaly there will not
			   be even this many. */
#define ENCGROW 16	/* Grow by, when full */
PRIVATE int maxenc = ENCSTART - 1;	/* Current size of array */
PRIVATE int enclidx = -1;	/* Current last index in array */

/*

   t1fonts[n]: Built as xdvi loads t1 fonts
   idx	 file			short	t1id	loaded
   [0] = /path/cmr10.pfb,	cmr10	0	1
   [1] = /path/prmr.pfa,	prmr	1	0
   [2] = /path/pcrr8a.pfa,	pcrr8a	2	0

   This array enumerates the loaded t1 fonts by full path name.  The
   refered to t1id is the t1lib font id.  The font will not be
   extended/slanted/encoded.  The loaded field indicates if
   T1_LoadFont has been called for the font yet.  texfonts that need a
   modified version of the t1 font must use T1_CopyFont to copy the
   raw font and obtain the id of a new t1 font which can then be
   modified.  As described in the t1 docs a font must be loaded before
   it can be copied.

   Fonts that are copies of other fonts have NULL filenames.  If any
   font files are to be reused based on the file name the raw font
   must be reused, not a copy.

   The short field contains the font as it was identified in the
   fontmap.  This is used to save searching; if two fonts are
   identified the same way in the fontmap they are the same font.

*/

typedef struct t1font {
    char *file;
    char *shortname;
    int t1id;
    int loaded;
} t1font;

PRIVATE t1font *t1fonts;	/* Dynamic array */
#define T1START	2048	/* Japanese, Chineese, Korean and others
			   use a lot of fonts */
#define T1GROW  1024
PRIVATE int maxt1 = T1START - 1;
PRIVATE int t1lidx = -1;

/*
   fontmaps[n]: Loaded from map file, and extended with 'implied'
	fonts as needed.
   idx	 texname enc	exten.	slant	filena.	t1libid	pathn.	tfmidx
   [0] = pcrr8rn 0	750	0	pcrr8a	2	...	2
   [1] = putro8r 0	0	0	putro8r 3	...	3

   The first 5 fields of this table are loaded from the font map
   files, or filled out with default values when implied fonts are
   set-up.  The t1libid is -1 until the font is loaded or copied as
   described under t1fonts.  Once the dvi file reveals that the font
   is needed it is located on disk to ensure it exists, and the
   pathname field is filled out.  The t1 font is not loaded, or
   copied, before it's needed because set_t1_char has been called to
   draw a glyph from the font.  The late loading is necessiated by the
   high cost of loading fonts.  If the font loading is concentrated at
   startup of the program the startup-time becomes ecessively high due
   to the high number of fonts used by any normaly LaTeX-document.

   0 extension and slant means no extension/slant.  If the input
   values are decimal (less than 10) they're multiplied by 1000 and
   10000 respectively to obtain fixed-point integer values.  Integer
   values have the advandage of being testable for exact match (== 0)
   on all architectures (it's true!  Not all architectures can do
   (reliable) exact matching of floatingpoint values)

*/

typedef struct fontmap {
    char *texname;
    int enc;	/* Index in encoding array */
    int extension;	/* Fixed point, *1000 */
    int slant;	/* Fixed point, *10000, some font slantings have 4 
		   significant digits, all after the decimalpoint */
    char *filename;	/* Name of the t1 font file as given in map/dvi */

    int t1libid;	/* The t1lib id of this font, or -1 if not set up */
    char *pathname;	/* Full path of the font, once needed and located */
    int tfmidx;	/* Index in tfminfo array, or -1 */
    Boolean warned_about;	/* whether a warning message about this font has been displayed yet */
    /*
      if the font file was not readable for some reason (e.g. corrupt,
      or encrypted for proprietry distribution), load_font_now() will
      try to load a PK version via load_font() instead. If this is
      successful, it will set force_pk to true, which will tell
      get_t1_glyph() to return immediately, and just use the standard
      set_char() routine to set the PK glyph. If even that fails,
      get_t1_glyph() returns an error status in a special flag that's
      evaluated by set_t1_char().
    */
    Boolean force_pk;		
} fontmap;

PRIVATE fontmap *fontmaps;
#define FNTMAPSTART 2048
#define FNTMAPGROW 1024
PRIVATE int g_maxmap = FNTMAPSTART - 1;
PRIVATE int g_maplidx = 0; /* global index of fonts allocated in fontmap */

/* 
   widthinfo[n]: TFM widht info, demand loaded as the fonts are used.
   idx	 texname	widths
   [0] = cmr10		...
   [1] = ptmr8r		...
   [2] = pcrr8rn	...
   [3] = putro8r	...

   The widthinfo is loaded from the font tfm file since the type1
   width information isn't precise enough to avoid accumulating
   rounding errors that make things visibly unaligned.  This is
   esp. noticeable in tables; the vline column separators become
   unaligned.

   For other than "design" sizes the widths are scaled up or down.

*/

typedef struct tfminfos {
    char *texname;
    long designsize;
    long widths[256];
} tfminfos;

PRIVATE tfminfos *tfminfo;
#define TEXWSTART 2048
#define TEXWGROW 1024
PRIVATE int maxw = TEXWSTART - 1;
PRIVATE int wlidx = -1;

/* For file reading */
#define BUFFER_SIZE 1024

/* This is the conversion factor from TeXs pt to TeXs bp (big point).
   Maximum machine precision is needed to keep it accurate. */
/* UNUSED */
/* PRIVATE double bp_fac=72.0/72.27; */

/* Try to convert from PS charspace units to DVI units with minimal
   loss of significant digits */
#define t1_dvi_conv(x)	((((int) (x)) << 16)/1000)
/* Convert from DVI units to TeX pt */
#define dvi_pt_conv(x) (((long) ((x)*dimconv)) >> 19)

/* Convert from TFM units to DVI units */
#define tfm_dvi_conv(x) (((int) (x)) >> 1)

#define T1PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))
int bytepad;
int padMismatch=0; /* Does t1lib padding match xdvi required padding? */
int archpad;

PRIVATE struct glyph *get_t1_glyph(
#ifdef TEXXET
				   wide_ubyte cmd,
#endif
				   wide_ubyte ch, t1font_load_status_t *flag);

#ifdef WORDS_BIGENDIAN
unsigned char rbits[] = {
    0x00,   /* 00000000 -> 00000000 */
    0x80,   /* 00000001 -> 10000000 */
    0x40,   /* 00000010 -> 01000000 */
    0xC0,   /* 00000011 -> 11000000 */
    0x20,   /* 00000100 -> 00100000 */
    0xA0,   /* 00000101 -> 10100000 */
    0x60,   /* 00000110 -> 01100000 */
    0xE0,   /* 00000111 -> 11100000 */
    0x10,   /* 00001000 -> 00010000 */
    0x90,   /* 00001001 -> 10010000 */
    0x50,   /* 00001010 -> 01010000 */
    0xD0,   /* 00001011 -> 11010000 */
    0x30,   /* 00001100 -> 00110000 */
    0xB0,   /* 00001101 -> 10110000 */
    0x70,   /* 00001110 -> 01110000 */
    0xF0,   /* 00001111 -> 11110000 */
    0x08,   /* 00010000 -> 00001000 */
    0x88,   /* 00010001 -> 10001000 */
    0x48,   /* 00010010 -> 01001000 */
    0xC8,   /* 00010011 -> 11001000 */
    0x28,   /* 00010100 -> 00101000 */
    0xA8,   /* 00010101 -> 10101000 */
    0x68,   /* 00010110 -> 01101000 */
    0xE8,   /* 00010111 -> 11101000 */
    0x18,   /* 00011000 -> 00011000 */
    0x98,   /* 00011001 -> 10011000 */
    0x58,   /* 00011010 -> 01011000 */
    0xD8,   /* 00011011 -> 11011000 */
    0x38,   /* 00011100 -> 00111000 */
    0xB8,   /* 00011101 -> 10111000 */
    0x78,   /* 00011110 -> 01111000 */
    0xF8,   /* 00011111 -> 11111000 */
    0x04,   /* 00100000 -> 00000100 */
    0x84,   /* 00100001 -> 10000100 */
    0x44,   /* 00100010 -> 01000100 */
    0xC4,   /* 00100011 -> 11000100 */
    0x24,   /* 00100100 -> 00100100 */
    0xA4,   /* 00100101 -> 10100100 */
    0x64,   /* 00100110 -> 01100100 */
    0xE4,   /* 00100111 -> 11100100 */
    0x14,   /* 00101000 -> 00010100 */
    0x94,   /* 00101001 -> 10010100 */
    0x54,   /* 00101010 -> 01010100 */
    0xD4,   /* 00101011 -> 11010100 */
    0x34,   /* 00101100 -> 00110100 */
    0xB4,   /* 00101101 -> 10110100 */
    0x74,   /* 00101110 -> 01110100 */
    0xF4,   /* 00101111 -> 11110100 */
    0x0C,   /* 00110000 -> 00001100 */
    0x8C,   /* 00110001 -> 10001100 */
    0x4C,   /* 00110010 -> 01001100 */
    0xCC,   /* 00110011 -> 11001100 */
    0x2C,   /* 00110100 -> 00101100 */
    0xAC,   /* 00110101 -> 10101100 */
    0x6C,   /* 00110110 -> 01101100 */
    0xEC,   /* 00110111 -> 11101100 */
    0x1C,   /* 00111000 -> 00011100 */
    0x9C,   /* 00111001 -> 10011100 */
    0x5C,   /* 00111010 -> 01011100 */
    0xDC,   /* 00111011 -> 11011100 */
    0x3C,   /* 00111100 -> 00111100 */
    0xBC,   /* 00111101 -> 10111100 */
    0x7C,   /* 00111110 -> 01111100 */
    0xFC,   /* 00111111 -> 11111100 */
    0x02,   /* 01000000 -> 00000010 */
    0x82,   /* 01000001 -> 10000010 */
    0x42,   /* 01000010 -> 01000010 */
    0xC2,   /* 01000011 -> 11000010 */
    0x22,   /* 01000100 -> 00100010 */
    0xA2,   /* 01000101 -> 10100010 */
    0x62,   /* 01000110 -> 01100010 */
    0xE2,   /* 01000111 -> 11100010 */
    0x12,   /* 01001000 -> 00010010 */
    0x92,   /* 01001001 -> 10010010 */
    0x52,   /* 01001010 -> 01010010 */
    0xD2,   /* 01001011 -> 11010010 */
    0x32,   /* 01001100 -> 00110010 */
    0xB2,   /* 01001101 -> 10110010 */
    0x72,   /* 01001110 -> 01110010 */
    0xF2,   /* 01001111 -> 11110010 */
    0x0A,   /* 01010000 -> 00001010 */
    0x8A,   /* 01010001 -> 10001010 */
    0x4A,   /* 01010010 -> 01001010 */
    0xCA,   /* 01010011 -> 11001010 */
    0x2A,   /* 01010100 -> 00101010 */
    0xAA,   /* 01010101 -> 10101010 */
    0x6A,   /* 01010110 -> 01101010 */
    0xEA,   /* 01010111 -> 11101010 */
    0x1A,   /* 01011000 -> 00011010 */
    0x9A,   /* 01011001 -> 10011010 */
    0x5A,   /* 01011010 -> 01011010 */
    0xDA,   /* 01011011 -> 11011010 */
    0x3A,   /* 01011100 -> 00111010 */
    0xBA,   /* 01011101 -> 10111010 */
    0x7A,   /* 01011110 -> 01111010 */
    0xFA,   /* 01011111 -> 11111010 */
    0x06,   /* 01100000 -> 00000110 */
    0x86,   /* 01100001 -> 10000110 */
    0x46,   /* 01100010 -> 01000110 */
    0xC6,   /* 01100011 -> 11000110 */
    0x26,   /* 01100100 -> 00100110 */
    0xA6,   /* 01100101 -> 10100110 */
    0x66,   /* 01100110 -> 01100110 */
    0xE6,   /* 01100111 -> 11100110 */
    0x16,   /* 01101000 -> 00010110 */
    0x96,   /* 01101001 -> 10010110 */
    0x56,   /* 01101010 -> 01010110 */
    0xD6,   /* 01101011 -> 11010110 */
    0x36,   /* 01101100 -> 00110110 */
    0xB6,   /* 01101101 -> 10110110 */
    0x76,   /* 01101110 -> 01110110 */
    0xF6,   /* 01101111 -> 11110110 */
    0x0E,   /* 01110000 -> 00001110 */
    0x8E,   /* 01110001 -> 10001110 */
    0x4E,   /* 01110010 -> 01001110 */
    0xCE,   /* 01110011 -> 11001110 */
    0x2E,   /* 01110100 -> 00101110 */
    0xAE,   /* 01110101 -> 10101110 */
    0x6E,   /* 01110110 -> 01101110 */
    0xEE,   /* 01110111 -> 11101110 */
    0x1E,   /* 01111000 -> 00011110 */
    0x9E,   /* 01111001 -> 10011110 */
    0x5E,   /* 01111010 -> 01011110 */
    0xDE,   /* 01111011 -> 11011110 */
    0x3E,   /* 01111100 -> 00111110 */
    0xBE,   /* 01111101 -> 10111110 */
    0x7E,   /* 01111110 -> 01111110 */
    0xFE,   /* 01111111 -> 11111110 */
    0x01,   /* 10000000 -> 00000001 */
    0x81,   /* 10000001 -> 10000001 */
    0x41,   /* 10000010 -> 01000001 */
    0xC1,   /* 10000011 -> 11000001 */
    0x21,   /* 10000100 -> 00100001 */
    0xA1,   /* 10000101 -> 10100001 */
    0x61,   /* 10000110 -> 01100001 */
    0xE1,   /* 10000111 -> 11100001 */
    0x11,   /* 10001000 -> 00010001 */
    0x91,   /* 10001001 -> 10010001 */
    0x51,   /* 10001010 -> 01010001 */
    0xD1,   /* 10001011 -> 11010001 */
    0x31,   /* 10001100 -> 00110001 */
    0xB1,   /* 10001101 -> 10110001 */
    0x71,   /* 10001110 -> 01110001 */
    0xF1,   /* 10001111 -> 11110001 */
    0x09,   /* 10010000 -> 00001001 */
    0x89,   /* 10010001 -> 10001001 */
    0x49,   /* 10010010 -> 01001001 */
    0xC9,   /* 10010011 -> 11001001 */
    0x29,   /* 10010100 -> 00101001 */
    0xA9,   /* 10010101 -> 10101001 */
    0x69,   /* 10010110 -> 01101001 */
    0xE9,   /* 10010111 -> 11101001 */
    0x19,   /* 10011000 -> 00011001 */
    0x99,   /* 10011001 -> 10011001 */
    0x59,   /* 10011010 -> 01011001 */
    0xD9,   /* 10011011 -> 11011001 */
    0x39,   /* 10011100 -> 00111001 */
    0xB9,   /* 10011101 -> 10111001 */
    0x79,   /* 10011110 -> 01111001 */
    0xF9,   /* 10011111 -> 11111001 */
    0x05,   /* 10100000 -> 00000101 */
    0x85,   /* 10100001 -> 10000101 */
    0x45,   /* 10100010 -> 01000101 */
    0xC5,   /* 10100011 -> 11000101 */
    0x25,   /* 10100100 -> 00100101 */
    0xA5,   /* 10100101 -> 10100101 */
    0x65,   /* 10100110 -> 01100101 */
    0xE5,   /* 10100111 -> 11100101 */
    0x15,   /* 10101000 -> 00010101 */
    0x95,   /* 10101001 -> 10010101 */
    0x55,   /* 10101010 -> 01010101 */
    0xD5,   /* 10101011 -> 11010101 */
    0x35,   /* 10101100 -> 00110101 */
    0xB5,   /* 10101101 -> 10110101 */
    0x75,   /* 10101110 -> 01110101 */
    0xF5,   /* 10101111 -> 11110101 */
    0x0D,   /* 10110000 -> 00001101 */
    0x8D,   /* 10110001 -> 10001101 */
    0x4D,   /* 10110010 -> 01001101 */
    0xCD,   /* 10110011 -> 11001101 */
    0x2D,   /* 10110100 -> 00101101 */
    0xAD,   /* 10110101 -> 10101101 */
    0x6D,   /* 10110110 -> 01101101 */
    0xED,   /* 10110111 -> 11101101 */
    0x1D,   /* 10111000 -> 00011101 */
    0x9D,   /* 10111001 -> 10011101 */
    0x5D,   /* 10111010 -> 01011101 */
    0xDD,   /* 10111011 -> 11011101 */
    0x3D,   /* 10111100 -> 00111101 */
    0xBD,   /* 10111101 -> 10111101 */
    0x7D,   /* 10111110 -> 01111101 */
    0xFD,   /* 10111111 -> 11111101 */
    0x03,   /* 11000000 -> 00000011 */
    0x83,   /* 11000001 -> 10000011 */
    0x43,   /* 11000010 -> 01000011 */
    0xC3,   /* 11000011 -> 11000011 */
    0x23,   /* 11000100 -> 00100011 */
    0xA3,   /* 11000101 -> 10100011 */
    0x63,   /* 11000110 -> 01100011 */
    0xE3,   /* 11000111 -> 11100011 */
    0x13,   /* 11001000 -> 00010011 */
    0x93,   /* 11001001 -> 10010011 */
    0x53,   /* 11001010 -> 01010011 */
    0xD3,   /* 11001011 -> 11010011 */
    0x33,   /* 11001100 -> 00110011 */
    0xB3,   /* 11001101 -> 10110011 */
    0x73,   /* 11001110 -> 01110011 */
    0xF3,   /* 11001111 -> 11110011 */
    0x0B,   /* 11010000 -> 00001011 */
    0x8B,   /* 11010001 -> 10001011 */
    0x4B,   /* 11010010 -> 01001011 */
    0xCB,   /* 11010011 -> 11001011 */
    0x2B,   /* 11010100 -> 00101011 */
    0xAB,   /* 11010101 -> 10101011 */
    0x6B,   /* 11010110 -> 01101011 */
    0xEB,   /* 11010111 -> 11101011 */
    0x1B,   /* 11011000 -> 00011011 */
    0x9B,   /* 11011001 -> 10011011 */
    0x5B,   /* 11011010 -> 01011011 */
    0xDB,   /* 11011011 -> 11011011 */
    0x3B,   /* 11011100 -> 00111011 */
    0xBB,   /* 11011101 -> 10111011 */
    0x7B,   /* 11011110 -> 01111011 */
    0xFB,   /* 11011111 -> 11111011 */
    0x07,   /* 11100000 -> 00000111 */
    0x87,   /* 11100001 -> 10000111 */
    0x47,   /* 11100010 -> 01000111 */
    0xC7,   /* 11100011 -> 11000111 */
    0x27,   /* 11100100 -> 00100111 */
    0xA7,   /* 11100101 -> 10100111 */
    0x67,   /* 11100110 -> 01100111 */
    0xE7,   /* 11100111 -> 11100111 */
    0x17,   /* 11101000 -> 00010111 */
    0x97,   /* 11101001 -> 10010111 */
    0x57,   /* 11101010 -> 01010111 */
    0xD7,   /* 11101011 -> 11010111 */
    0x37,   /* 11101100 -> 00110111 */
    0xB7,   /* 11101101 -> 10110111 */
    0x77,   /* 11101110 -> 01110111 */
    0xF7,   /* 11101111 -> 11110111 */
    0x0F,   /* 11110000 -> 00001111 */
    0x8F,   /* 11110001 -> 10001111 */
    0x4F,   /* 11110010 -> 01001111 */
    0xCF,   /* 11110011 -> 11001111 */
    0x2F,   /* 11110100 -> 00101111 */
    0xAF,   /* 11110101 -> 10101111 */
    0x6F,   /* 11110110 -> 01101111 */
    0xEF,   /* 11110111 -> 11101111 */
    0x1F,   /* 11111000 -> 00011111 */
    0x9F,   /* 11111001 -> 10011111 */
    0x5F,   /* 11111010 -> 01011111 */
    0xDF,   /* 11111011 -> 11011111 */
    0x3F,   /* 11111100 -> 00111111 */
    0xBF,   /* 11111101 -> 10111111 */
    0x7F,   /* 11111110 -> 01111111 */
    0xFF    /* 11111111 -> 11111111 */
};
#endif /* WORDS_BIGENDIAN */

#endif /* T1LIB */

extern void home();  /* from events.c */
extern int fallbacktfm; /* from tfmload.c */

static struct frame frame0;	/* dummy head of list */
#ifdef	TEXXET
static struct frame *scan_frame;	/* head frame for scanning */
#endif

#ifndef	DVI_BUFFER_LEN
#define	DVI_BUFFER_LEN	512
#endif

static ubyte dvi_buffer[DVI_BUFFER_LEN];
static struct frame *current_frame;

#ifndef	TEXXET
#define	DIR	1
#else
#define	DIR	currinf.dir
#endif

/*
 *	Explanation of the following constant:
 *	offset_[xy]   << 16:	margin (defaults to one inch)
 *	currwin.shrinkfactor << 16:	one pixel page border
 *	currwin.shrinkfactor << 15:	rounding for pixel_conv
 */
#define OFFSET_X	(offset_x << 16) + (currwin.shrinkfactor * 3 << 15)
#define OFFSET_Y	(offset_y << 16) + (currwin.shrinkfactor * 3 << 15)

#if (BMBYTES == 1)
BMUNIT bit_masks[9] = {
    0x0, 0x1, 0x3, 0x7,
    0xf, 0x1f, 0x3f, 0x7f,
    0xff
};
#else
#if (BMBYTES == 2)
BMUNIT bit_masks[17] = {
    0x0, 0x1, 0x3, 0x7,
    0xf, 0x1f, 0x3f, 0x7f,
    0xff, 0x1ff, 0x3ff, 0x7ff,
    0xfff, 0x1fff, 0x3fff, 0x7fff,
    0xffff
};
#else /* BMBYTES == 4 */
BMUNIT bit_masks[33] = {
    0x0, 0x1, 0x3, 0x7,
    0xf, 0x1f, 0x3f, 0x7f,
    0xff, 0x1ff, 0x3ff, 0x7ff,
    0xfff, 0x1fff, 0x3fff, 0x7fff,
    0xffff, 0x1ffff, 0x3ffff, 0x7ffff,
    0xfffff, 0x1fffff, 0x3fffff, 0x7fffff,
    0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff,
    0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};
#endif
#endif

#ifdef	VMS
#define	off_t	int
#endif
extern off_t lseek();

#ifndef	SEEK_SET	/* if <unistd.h> is not provided (or for <X11R5) */
#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2
#endif

static void draw_part(struct frame *minframe, double current_dimconv);
static void source_fwd_draw_box(void);


#ifndef	TEXXET
static long set_no_char(wide_ubyte ch);
#else
static void set_no_char(wide_ubyte cmd, wide_ubyte ch);
#endif /* TEXXET */

/*
 *	X routines.
 */

/*
 *	Put a rectangle on the screen.
 */

/* HTEX needs this to be non-static -janl */
void
put_rule(Boolean highlight, int x, int y, unsigned int w, unsigned int h)
{
    if (x < max_x && x + (int)w >= min_x && y < max_y && y + (int)h >= min_y) {
	if (--event_counter == 0) {
	    read_events(False);
	}
	XFillRectangle(DISP, currwin.win,
#ifdef HTEX
		       highlight ? highGC : ruleGC,
#else
		       ruleGC,
#endif
		       x - currwin.base_x, y - currwin.base_y, w ? w : 1,
		       h ? h : 1);
    }
}

static void
put_bitmap(struct bitmap *bitmap, int x, int y)
{

#ifdef HTEX
    if (HTeXAnestlevel > 0) {
	htex_recordbits(x, y, bitmap->w, bitmap->h);
    }
#endif

    if (debug & DBG_BITMAP)
	Printf("X(%d,%d)\n", x - currwin.base_x, y - currwin.base_y);
    if (x < max_x && x + (int)bitmap->w >= min_x &&
	y < max_y && y + (int)bitmap->h >= min_y) {
	if (--event_counter == 0) {
	    read_events(False);
	}
	image->width = bitmap->w;
	image->height = bitmap->h;
	image->data = bitmap->bits;
	image->bytes_per_line = bitmap->bytes_wide;
	XPutImage(DISP, currwin.win,
		  foreGC,
		  image,
		  0, 0,
		  x - currwin.base_x, y - currwin.base_y, bitmap->w, bitmap->h);
	if (foreGC2) {
	    XPutImage(DISP, currwin.win, foreGC2, image,
		      0, 0,
		      x - currwin.base_x, y - currwin.base_y,
		      bitmap->w, bitmap->h);
	}
    }
}

#ifdef	GREY
static void
put_image(struct glyph *g, int x, int y)
{
    XImage *img = g->image2;

#ifdef HTEX
    if (HTeXAnestlevel > 0) {
	htex_recordbits(x, y, img->width, img->height);
    }
#endif
    if (x < max_x && x + img->width >= min_x &&
	y < max_y && y + img->height >= min_y) {
	if (--event_counter == 0) {
	    read_events(False);
	}
	XPutImage(DISP, currwin.win, foreGC, img,
		  0, 0,
		  x - currwin.base_x, y - currwin.base_y,
		  (unsigned int)img->width, (unsigned int)img->height);
	if (pixeltbl_t != NULL) {
	    img->data = g->pixmap2_t;
	    XPutImage(DISP, currwin.win, foreGC2, img,
		      0, 0,
		      x - currwin.base_x, y - currwin.base_y,
		      (unsigned int)img->width, (unsigned int)img->height);
	    img->data = g->pixmap2;
	}
    }
}
#endif /* GREY */

/*
 *	Draw the border of a rectangle on the screen.
 */

static void
put_border(int x, int y, unsigned int width, unsigned int height, GC ourGC)
{

    --width;
    --height;
    /* SU 2000/12/16: replaced 4 calls to XFillRectangle by the following: */
    XDrawRectangle(DISP, currwin.win, ourGC, x, y, width, height);
}

#ifdef GRID
/* drawing grid */
void
put_grid(int x, int y, unsigned int width, unsigned int height, unsigned int unit, GC gc)
{
    int i;	/* looping variable */
    float sep;	/* grid separation */
    int tmp;	/* temporary variable */

    --width;
    --height;


    /* drawing vertial grid */
#define DRAWGRID_VER(gc) for (i = 1; \
			      (tmp = x + (int)(i * sep)) < (int)(x + width); \
			      i++) \
			  XFillRectangle(DISP, currwin.win, (gc), \
				   tmp, y, 1, height)
    /* draw horizontal grid */
#define DRAWGRID_HOR(gc) for (i = 1; \
			      (tmp = y + (int)(i * sep)) < (int)(y + height); \
			      i++) \
		          XFillRectangle(DISP, currwin.win, (gc), \
				   x, tmp, width, 1)

    if (grid_mode > 2) {	/* third level grid */
	sep = (float)unit / 4.0;
	DRAWGRID_VER(gc);	/* vertical grid */
	DRAWGRID_HOR(gc);	/* horizontal grid */
    }

    if (grid_mode > 1) {	/* second */
	sep = (float)unit / 2.0;
	DRAWGRID_VER(gc);	/* vertical grid */
	DRAWGRID_HOR(gc);	/* horizontal grid */
    }

    if (grid_mode > 0) {	/* first level */
	sep = (float)unit;
	DRAWGRID_VER(gc);	/* vertical grid */
	DRAWGRID_HOR(gc);	/* horizontal grid */
    }
}

#endif /* GRID */

/*
 *	Byte reading routines for dvi file.
 */

#define	xtell(pos)	(lseek(fileno(dvi_file), 0L, SEEK_CUR) - \
			    (currinf.end - (pos)))

static ubyte
xxone(void)
{
    if (currinf.virtual) {
	++currinf.pos;
	return EOP;
    }
    currinf.end = dvi_buffer + read(fileno(dvi_file), (char *)(currinf.pos = dvi_buffer),
				    DVI_BUFFER_LEN);
    return currinf.end > dvi_buffer ? *(currinf.pos)++ : EOF;
}

#define	xone()  (currinf.pos < currinf.end ? *(currinf.pos)++ : xxone())

static unsigned long
xnum(ubyte size)
{
    long x = 0;

    while (size--)
	x = (x << 8) | xone();
    return x;
}

static long
xsnum(ubyte size)
{
    long x;

#if	__STDC__
    x = (signed char)xone();
#else
    x = xone();
    if (x & 0x80)
	x -= 0x100;
#endif
    while (--size)
	x = (x << 8) | xone();
    return x;
}

#define	xsfour()	xsnum(4)

static void
xskip(long offset)
{
    currinf.pos += offset;
    if (!currinf.virtual && currinf.pos > currinf.end)
	(void)lseek(fileno(dvi_file), (long)(currinf.pos - currinf.end),
		    SEEK_CUR);
}

static NORETURN void
tell_oops(_Xconst char * message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "%s: ", prog);
    (void)vfprintf(stderr, message, args);
    va_end(args);
    if (currinf.virtual)
	fprintf(stderr, " in virtual font %s\n", currinf.virtual->fontname);
    else
	fprintf(stderr, ", offset %ld\n", xtell(currinf.pos - 1));
    xdvi_exit(1);
}


/*
 *	Code for debugging options.
 */

static void
print_bitmap(struct bitmap *bitmap)
{
    BMUNIT *ptr = (BMUNIT *) bitmap->bits;
    int x, y, i;

    if (ptr == NULL)
	oops("print_bitmap called with null pointer.");
    Printf("w = %d, h = %d, bytes wide = %d\n",
	   bitmap->w, bitmap->h, bitmap->bytes_wide);
    for (y = 0; y < (int)bitmap->h; ++y) {
	for (x = bitmap->bytes_wide; x > 0; x -= BMBYTES) {
#ifndef	WORDS_BIGENDIAN
	    for (i = 0; i < BMBITS; ++i)
#else
	    for (i = BMBITS - 1; i >= 0; --i)
#endif
		Putchar((*ptr & (1 << i)) ? '@' : '.');
	    ++ptr;
	}
	Putchar('\n');
    }
}

static void
print_char(ubyte ch, struct glyph *g)
{
    Printf("char %d", ch);
    if (ISPRINT(ch))
	Printf(" (%c)", ch);
    Putchar('\n');
    Printf("x = %d, y = %d, dvi = %ld\n", g->x, g->y, g->dvi_adv);
    print_bitmap(&g->bitmap);
}

static _Xconst char *dvi_table1[] = {
#ifdef Omega
    "SET1", "SET2", NULL, NULL, "SETRULE", "PUT1", "PUT2", NULL,
#else
    "SET1", NULL, NULL, NULL, "SETRULE", "PUT1", NULL, NULL,
#endif
    NULL, "PUTRULE", "NOP", "BOP", "EOP", "PUSH", "POP", "RIGHT1",
    "RIGHT2", "RIGHT3", "RIGHT4", "W0", "W1", "W2", "W3", "W4",
    "X0", "X1", "X2", "X3", "X4", "DOWN1", "DOWN2", "DOWN3",
    "DOWN4", "Y0", "Y1", "Y2", "Y3", "Y4", "Z0", "Z1",
    "Z2", "Z3", "Z4"
};

static _Xconst char *dvi_table2[] = {
    "FNT1", "FNT2", "FNT3", "FNT4", "XXX1", "XXX2", "XXX3", "XXX4",
    "FNTDEF1", "FNTDEF2", "FNTDEF3", "FNTDEF4", "PRE", "POST", "POSTPOST",
    "SREFL", "EREFL", NULL, NULL, NULL, NULL
};

static void
print_dvi(ubyte ch)
{
    _Xconst char *s;

    Printf("%4d %4d ", PXL_H, PXL_V);
    if (ch <= (ubyte) (SETCHAR0 + 127)) {
	Printf("SETCHAR%-3d", ch - SETCHAR0);
	if (ISPRINT(ch))
	    Printf(" (%c)", ch);
	Putchar('\n');
	return;
    }
    else if (ch < FNTNUM0)
	s = dvi_table1[ch - 128];
    else if (ch <= (ubyte) (FNTNUM0 + 63)) {
	Printf("FNTNUM%d\n", ch - FNTNUM0);
	return;
    }
    else
	s = dvi_table2[ch - (FNTNUM0 + 64)];
    if (s)
	Puts(s);
    else
	tell_oops("print_dvi: unknown op-code %d", ch);
}


/*
 *	Count the number of set bits in a given region of the bitmap
 */

static char sample_count[] = { 0, 1, 1, 2, 1, 2, 2, 3,
    1, 2, 2, 3, 2, 3, 3, 4
};

static int
sample(BMUNIT *bits, int bytes_wide, int bit_skip, int w, int h)
{
    BMUNIT *ptr, *endp;
    BMUNIT *cp;
    int bits_left;
    int n, bit_shift, wid;

    ptr = bits + bit_skip / BMBITS;
    endp = ADD(bits, h * bytes_wide);
    bits_left = w;
#ifndef	WORDS_BIGENDIAN
    bit_shift = bit_skip % BMBITS;
#else
    bit_shift = BMBITS - bit_skip % BMBITS;
#endif
    n = 0;

    while (bits_left) {
#ifndef	WORDS_BIGENDIAN
	wid = BMBITS - bit_shift;
#else
	wid = bit_shift;
#endif
	if (wid > bits_left)
	    wid = bits_left;
	if (wid > 4)
	    wid = 4;
#ifdef	WORDS_BIGENDIAN
	bit_shift -= wid;
#endif
	for (cp = ptr; cp < endp; cp = ADD(cp, bytes_wide)) {
	  n += sample_count[ (*cp >> bit_shift) & bit_masks[wid] ];
	}
#ifndef	WORDS_BIGENDIAN
	bit_shift += wid;
	if (bit_shift == BMBITS) {
	    bit_shift = 0;
	    ++ptr;
	}
#else
	if (bit_shift == 0) {
	    bit_shift = BMBITS;
	    ++ptr;
	}
#endif
	bits_left -= wid;
    }
    return n;
}

static void
shrink_glyph(struct glyph *g)
{
    int shrunk_bytes_wide, shrunk_height;
    int rows_left, rows, init_cols;
    int cols_left;
    int cols;
    BMUNIT *old_ptr, *new_ptr;
    BMUNIT m, *cp;
    int min_sample = currwin.shrinkfactor * currwin.shrinkfactor * density / 100;
    int rtmp;

    /* These machinations ensure that the character is shrunk according to
       its hot point, rather than its upper left-hand corner. */
    g->x2 = g->x / currwin.shrinkfactor;
    init_cols = g->x - g->x2 * currwin.shrinkfactor;
    if (init_cols <= 0)
	init_cols += currwin.shrinkfactor;
    else
	++g->x2;
    g->bitmap2.w = g->x2 + ROUNDUP((int)g->bitmap.w - g->x, currwin.shrinkfactor);
    /* include row zero with the positively numbered rows */
    rtmp = g->y + 1;
    g->y2 = rtmp / currwin.shrinkfactor;
    rows = rtmp - g->y2 * currwin.shrinkfactor;
    if (rows <= 0) {
	rows += currwin.shrinkfactor;
	--g->y2;
    }
    g->bitmap2.h = shrunk_height = g->y2 +
	ROUNDUP((int)g->bitmap.h - rtmp, currwin.shrinkfactor) + 1;
    alloc_bitmap(&g->bitmap2);
    old_ptr = (BMUNIT *) g->bitmap.bits;
    new_ptr = (BMUNIT *) g->bitmap2.bits;
    shrunk_bytes_wide = g->bitmap2.bytes_wide;
    rows_left = g->bitmap.h;
    bzero((char *)new_ptr, shrunk_bytes_wide * shrunk_height);
    while (rows_left) {
	if (rows > rows_left)
	    rows = rows_left;
	cols_left = g->bitmap.w;
#ifndef	WORDS_BIGENDIAN
	m = (1 << 0);
#else
	m = ((BMUNIT) 1 << (BMBITS - 1));
#endif
	cp = new_ptr;
	cols = init_cols;
	while (cols_left) {
	    if (cols > cols_left)
		cols = cols_left;
	    if (sample(old_ptr, g->bitmap.bytes_wide,
		       (int)g->bitmap.w - cols_left, cols, rows)
		>= min_sample)
		*cp |= m;
#ifndef	WORDS_BIGENDIAN
	    if (m == ((BMUNIT) 1 << (BMBITS - 1))) {
		m = (1 << 0);
		++cp;
	    }
	    else
		m <<= 1;
#else
	    if (m == (1 << 0)) {
		m = ((BMUNIT) 1 << (BMBITS - 1));
		++cp;
	    }
	    else
		m >>= 1;
#endif
	    cols_left -= cols;
	    cols = currwin.shrinkfactor;
	}
	*((char **)&new_ptr) += shrunk_bytes_wide;
	*((char **)&old_ptr) += rows * g->bitmap.bytes_wide;
	rows_left -= rows;
	rows = currwin.shrinkfactor;
    }
    g->y2 = g->y / currwin.shrinkfactor;
    if (debug & DBG_BITMAP)
	print_bitmap(&g->bitmap2);
}

#ifdef	GREY
static void
shrink_glyph_grey(struct glyph *g)
{
    int rows_left, rows, init_cols;
    int cols_left;
    int cols;
    int x, y;
    long thesample;
    BMUNIT *old_ptr;
    unsigned int size;
    int rtmp;
    int c;

    /* These machinations ensure that the character is shrunk according to
       its hot point, rather than its upper left-hand corner. */
    g->x2 = g->x / currwin.shrinkfactor;
    init_cols = g->x - g->x2 * currwin.shrinkfactor;
    if (init_cols <= 0)
	init_cols += currwin.shrinkfactor;
    else
	++g->x2;
    g->bitmap2.w = g->x2 + ROUNDUP((int)g->bitmap.w - g->x, currwin.shrinkfactor);
    /* include row zero with the positively numbered rows */
    rtmp = g->y + 1;
    g->y2 = rtmp / currwin.shrinkfactor;
    rows = rtmp - g->y2 * currwin.shrinkfactor;
    if (rows <= 0) {
	rows += currwin.shrinkfactor;
	--g->y2;
    }
    g->bitmap2.h = g->y2 + ROUNDUP((int)g->bitmap.h - rtmp, currwin.shrinkfactor) + 1;

	g->image2 = XCreateImage(DISP, our_visual, our_depth, ZPixmap,
				 0, (char *)NULL, g->bitmap2.w, g->bitmap2.h,
				 BMBITS, 0);
	size = g->image2->bytes_per_line * g->bitmap2.h;
	g->pixmap2 = g->image2->data = xmalloc(size != 0 ? size : 1);
    if (pixeltbl_t != NULL)
	g->pixmap2_t = xmalloc(size != 0 ? size : 1);

    old_ptr = (BMUNIT *) g->bitmap.bits;
    rows_left = g->bitmap.h;
    y = 0;
    while (rows_left) {
	x = 0;
	if (rows > rows_left)
	    rows = rows_left;
	cols_left = g->bitmap.w;
	cols = init_cols;
	while (cols_left) {
	    if (cols > cols_left)
		cols = cols_left;

	    thesample = sample(old_ptr, g->bitmap.bytes_wide,
			       (int)g->bitmap.w - cols_left, cols, rows);
#ifdef XSERVER_INFO
	    if (debug & DBG_PK) {
		if (pixeltbl[thesample] > 65536)
		    c = pixeltbl[thesample] / 65536;
		else
		    c = pixeltbl[thesample] / 256;
		if (c == 0)
		    fprintf(stdout, ",..");
		else
		    fprintf(stdout, ",%.2x", c);
	    }
#endif
	    XPutPixel(g->image2, x, y, pixeltbl[thesample]);
	    if (pixeltbl_t != NULL) {
		g->image2->data = g->pixmap2_t;
		c = pixeltbl_t[thesample] / 256;
#ifdef XSERVER_INFO
		if (debug & DBG_PK)
		    fprintf(stdout, "|%.2x", c);
#endif
		XPutPixel(g->image2, x, y, pixeltbl_t[thesample]);
		g->image2->data = g->pixmap2;
	    }

	    cols_left -= cols;
	    cols = currwin.shrinkfactor;
	    x++;
	}
	*((char **)&old_ptr) += rows * g->bitmap.bytes_wide;
	rows_left -= rows;
	rows = currwin.shrinkfactor;
	y++;
#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    putchar('\n');
#endif
    }
#ifdef XSERVER_INFO
    if (debug & DBG_PK)
	putchar('\n');
#endif

    while (y < (int)g->bitmap2.h) {
	for (x = 0; x < (int)g->bitmap2.w; x++) {
#ifdef XSERVER_INFO
	    c = *pixeltbl / 256;
	    if (debug & DBG_PK)
		fprintf(stdout, ";%.2x", c);
#endif
	    XPutPixel(g->image2, x, y, *pixeltbl);
	    if (pixeltbl_t != NULL) {
#ifdef XSERVER_INFO
		c = *pixeltbl / 256;
		if (debug & DBG_PK)
		    fprintf(stdout, ":%.2x", c);
#endif
		g->image2->data = g->pixmap2_t;
		XPutPixel(g->image2, x, y, *pixeltbl_t);
		g->image2->data = g->pixmap2;
	    }
	}
#ifdef XSERVER_INFO
	if (debug & DBG_PK)
	    putchar('\n');
#endif
	y++;
    }

    g->y2 = g->y / currwin.shrinkfactor;
}
#endif /* GREY */

/*
 *	Find font #n.
 */

static void
change_font(unsigned long n)
{
    struct tn *tnp;

    if (n < currinf.tn_table_len)
	currinf.fontp = currinf.tn_table[n];
    else {
	currinf.fontp = NULL;
	for (tnp = currinf.tn_head; tnp != NULL; tnp = tnp->next)
	    if (tnp->TeXnumber == n) {
		currinf.fontp = tnp->fontp;
		break;
	    }
    }
    if (currinf.fontp == NULL)
	tell_oops("non-existent font #%lu", n);
    if (currinf.fontp->set_char_p == NULL)
	tell_oops("No procedure to set font #%d", n, currinf.fontp->fontname);
    maxchar = currinf.fontp->maxchar;
    currinf.set_char_p = currinf.fontp->set_char_p;
}


/*
 *	Open a font file.
 */

static void
open_font_file(struct font *fontp)
{
    if (fontp->file == NULL) {
	fontp->file = xfopen_local(fontp->filename, OPEN_MODE);
	if (fontp->file == NULL)
	    oops("Font file disappeared:  %s", fontp->filename);
    }
}

/*
 * Read and return special string; memory allocated for the string
 * will be overwritten by next call of this function.
 */

static char *
read_special(long nbytes)
{
    static char *spcl = NULL;
    static long spcl_len = -1;
    char *p;

    if (spcl_len < nbytes) {
	if (spcl != NULL)
	    free(spcl);
	spcl = xmalloc((unsigned)nbytes + 1);
	spcl_len = nbytes;
    }
    p = spcl;
    for (;;) {
	int i = currinf.end - currinf.pos;

	if (i > nbytes)
	    i = nbytes;
	bcopy((char *)currinf.pos, p, i);
	currinf.pos += i;
	p += i;
	nbytes -= i;
	if (nbytes == 0)
	    break;
	(void)xxone();
	--currinf.pos;
    }
    *p = '\0';
    return spcl;
}

/*
 *	Table used for scanning.  If >= 0, then skip that many bytes.
 *	M1 means end of page, M2 means special, M3 means FNTDEF,
 *	M4 means unrecognizable, and M5 means doesn't belong here.
 */

#define	M1	255
#define	M2	254
#define	M3	253
#define	M4	252
#define	M5	251
#define	MM	251

static ubyte scantable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* chars 0 - 127 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#ifdef Omega
    1, 2,	/* SET1,SET2 (128,129) */
    /* -,-,SETRULE,PUT1,PUT2,-,-,PUTRULE,NOP,BOP (130-139) */
    M4, M4, 8, 1, 2, M4, M4, 8, 0, 44,
#else

    1, M4,	/* SET1,- (128,129) */
    /* -,-,SETRULE,PUT1,-,-,-,PUTRULE,NOP,BOP (130-139) */
    M4, M4, 8, 1, M4, M4, M4, 8, 0, 44,
#endif
    M1, 0, 0, 1, 2, 3, 4, 0, 1, 2,	/* EOP,PUSH,POP,RIGHT1-4,W0M2 (140-149) */
    3, 4, 0, 1, 2, 3, 4, 1, 2, 3,	/* W3-4,X0-4,DOWN1-3 (150-159) */
    4, 0, 1, 2, 3, 4, 0, 1, 2, 3,	/* DOWN4,Y0-4,Z0-3 (160-169) */
    4,	/* Z4 (170) */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* change font 171 - 234 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 4, M2,	/* FNT1-4,XXX1 (235-239) */
    /* XXX2-4,FNTDEF1-4,PRE,POST,POSTPOST (240-249) */
    M2, M2, M2, M3, M3, M3, M3, M5, M5, M5,
    0, 0, M4, M4, M4, M4
};	/* SREFL,EREFL,-,-,-,- (250-255) */

/*
 *	This is the generic scanning routine.  It assumes that currinf, etc.
 *	are ready to go at the start of the page to be scanned.
 */

static void
spcl_scan(void (*spcl_proc)(char *))
{
    ubyte ch;
    ubyte n;
    long a;

    for (;;) {
	ch = xone();
	n = scantable[ch];
	
	if (n < MM)
	    while (n-- != 0)
		(void)xone();
	else if (n == M1)
	    break;	/* end of page */
	else {
	    switch (n) {
	    case M2:	/* special */
		a = xnum(ch - XXX1 + 1);
		if (a > 0)
		    spcl_proc(read_special(a));
		break;
	    case M3:	/* FNTDEF */
		xskip((long)(12 + ch - FNTDEF1 + 1));
		ch = xone();
		xskip((long)ch + (long)xone());
		break;
	    case M4:	/* unrecognizable */
		tell_oops("unknown op-code %d", ch);
		break;
	    case M5:	/* doesn't belong */
		tell_oops("spcl_scan: shouldn't happen: %s encountered",
			  dvi_table2[ch - (FNTNUM0 + 64)]);
		break;
	    }
	}
    }
}

#if	PS

/*
 *	Size of page interval for "Scanning pages xx-xx" message.
 */

#ifndef	REPORT_INCR
#define	REPORT_INCR	50
#endif

/*
 *	Prescanning routine for dvi file.  This looks for specials like
 *	`header=' and `!'. 
 */

static void
prescan(void)
{
    int nextreportpage;

    if (!resource._postscript) {
	scanned_page = total_pages;
	return;
    }
    nextreportpage = scanned_page;
    ++scanned_page;
    if (page_offset == NULL) {
/* 	dvi_time--; */
	return;
    }
    (void)lseek(fileno(dvi_file), page_offset[scanned_page], SEEK_SET);
    currinf.pos = currinf.end = dvi_buffer;
    for (;;) {
	if (debug & DBG_PS)
	    Printf("Scanning page %d\n", scanned_page);
	if (scanned_page > nextreportpage) {
	    nextreportpage += REPORT_INCR;
	    if (nextreportpage > current_page)
		nextreportpage = current_page;

	    /* SU: uncommented this, since it really isn't interesting for the user */
	    /*
	       print_statusline(1, "Scanning pages %d-%d",
	       scanned_page + pageno_correct, nextreportpage + pageno_correct);
	     */
	}

#if TOOLKIT
	ps_read_events(False, False);
#endif
	spcl_scan(scan_special);

	if (scanned_page >= current_page)
	    break;
	++scanned_page;
    }
    scanned_page_bak = scanned_page;
    XClearWindow(DISP, mane.win);
    psp.endheader();
}

#endif /* PS */

/*
 *	Routines to print characters.
 */

#ifndef	TEXXET
#define	ERRVAL	0L
#else
#define	ERRVAL
#endif

#ifndef	TEXXET
long
set_char(wide_ubyte ch)
#else
void
set_char(wide_ubyte cmd, wide_ubyte ch)
#endif
{
    struct glyph *g;
#ifdef	TEXXET
    long dvi_h_sav;
#endif

    if (ch > maxchar)
	realloc_font(currinf.fontp, WIDENINT ch);
    if ((g = &currinf.fontp->glyph[ch])->bitmap.bits == NULL) {
	if (g->addr == 0) {
	    if (!hush_chars)
		Fprintf(stderr, "Character %d not defined in font %s\n", ch,
			currinf.fontp->fontname);
	    g->addr = -1;
	    return ERRVAL;
	}
	if (g->addr == -1)
	    return ERRVAL;	/* previously flagged missing char */
	open_font_file(currinf.fontp);
	Fseek(currinf.fontp->file, g->addr, 0);
	(*currinf.fontp->read_char) (currinf.fontp, ch);
	if (debug & DBG_BITMAP)
	    print_char((ubyte) ch, g);
	currinf.fontp->timestamp = ++current_timestamp;
    }

#ifdef	TEXXET
    dvi_h_sav = DVI_H;
    if (currinf.dir < 0)
	DVI_H -= g->dvi_adv;

    if (scan_frame == NULL) {
#endif
	if (currwin.shrinkfactor == 1) {
	    put_bitmap(&g->bitmap, PXL_H - g->x, PXL_V - g->y);
	}
	else {
#ifdef	GREY
	    if (use_grey) {
		if (g->pixmap2 == NULL) {
		    shrink_glyph_grey(g);
		}
		put_image(g, PXL_H - g->x2, PXL_V - g->y2);
	    }
	    else {
		if (g->bitmap2.bits == NULL) {
		    shrink_glyph(g);
		}
		put_bitmap(&g->bitmap2, PXL_H - g->x2, PXL_V - g->y2);
	    }
#else
	    if (g->bitmap2.bits == NULL) {
		shrink_glyph(g);
	    }
	    put_bitmap(&g->bitmap2, PXL_H - g->x2, PXL_V - g->y2);
#endif
	}
#ifndef	TEXXET
	return g->dvi_adv;
#else
    }
#ifdef Omega
    if ((cmd == PUT1) || (cmd == PUT2))
#else
    if (cmd == PUT1)
#endif
	DVI_H = dvi_h_sav;
    else if (currinf.dir > 0)
	DVI_H += g->dvi_adv;
#endif
}


/* ARGSUSED */
#ifndef	TEXXET
static long
set_empty_char(wide_ubyte ch)
{
    return 0;
}
#else
static void
set_empty_char(wide_ubyte cmd, wide_ubyte ch)
{
    UNUSED(ch);
    UNUSED(cmd);
    return;
}
#endif

#ifndef	TEXXET
long
load_n_set_char(wide_ubyte ch)
#else
void
load_n_set_char(wide_ubyte cmd, wide_ubyte ch)
#endif
{
    if (!load_font(currinf.fontp)) {	/* if not found */
	/* FIXME: replace by GUI warning! */
	fprintf(stderr, "Character(s) will be left blank.\n");
	currinf.set_char_p = currinf.fontp->set_char_p = set_empty_char;
#if 0
	//TODO SU: following not used in xdvi
	// currinf.fontp->glyph = NULL;        /* init for read_postamble */
#endif
#ifndef	TEXXET
	return 0;
#else
	return;
#endif
    }
    maxchar = currinf.fontp->maxchar;
    currinf.set_char_p = currinf.fontp->set_char_p;
#ifndef	TEXXET
    return (*currinf.set_char_p) (ch);
#else
    (*currinf.set_char_p) (cmd, ch);
    return;
#endif
}


#ifndef	TEXXET
long
set_vf_char(wide_ubyte ch)
#else
void
set_vf_char(wide_ubyte cmd, wide_ubyte ch)
#endif
{
    struct macro *m;
    struct drawinf oldinfo;
#ifdef Omega
    wide_ubyte oldmaxchar;
#else
    ubyte oldmaxchar;
#endif
    static ubyte c;
#ifdef	TEXXET
    long dvi_h_sav;
#endif

    if (ch > maxchar)
	realloc_virtual_font(currinf.fontp, ch);
    if ((m = &currinf.fontp->macro[ch])->pos == NULL) {
	if (!hush_chars)
	    Fprintf(stderr, "Character %d not defined in font %s\n", ch,
		    currinf.fontp->fontname);
	m->pos = m->end = &c;
	return ERRVAL;
    }
#ifdef	TEXXET
    dvi_h_sav = DVI_H;
    if (currinf.dir < 0)
	DVI_H -= m->dvi_adv;
    if (scan_frame == NULL) {
#endif
	oldinfo = currinf;
	if (!currinf.virtual)
	    dvi_pointer_frame = &oldinfo;
	oldmaxchar = maxchar;
	WW = XX = YY = ZZ = 0;
	currinf.tn_table_len = VFTABLELEN;
	currinf.tn_table = currinf.fontp->vf_table;
	currinf.tn_head = currinf.fontp->vf_chain;
	currinf.pos = m->pos;
	currinf.end = m->end;
	currinf.virtual = currinf.fontp;

	draw_part(current_frame, currinf.fontp->dimconv);
	if (currinf.pos != currinf.end + 1)
	    tell_oops("virtual character macro does not end correctly");
	currinf = oldinfo;
	if (!currinf.virtual)
	    dvi_pointer_frame = &currinf;
	maxchar = oldmaxchar;
#ifndef	TEXXET
	return m->dvi_adv;
#else
    }
    if ((cmd == PUT1)
#ifdef Omega
	|| (cmd == PUT2)
#endif
	)
	DVI_H = dvi_h_sav;
    else if (currinf.dir > 0)
	DVI_H += m->dvi_adv;
#endif
}


#ifndef	TEXXET
static long
set_no_char(wide_ubyte ch)
#else
static void
set_no_char(wide_ubyte cmd, wide_ubyte ch)
#endif
{
    if (currinf.virtual) {
	currinf.fontp = currinf.virtual->first_font;
	if (currinf.fontp != NULL) {
	    maxchar = currinf.fontp->maxchar;
	    currinf.set_char_p = currinf.fontp->set_char_p;
#ifndef	TEXXET
	    return (*currinf.set_char_p) (ch);
#else
	    (*currinf.set_char_p) (cmd, ch);
	    return;
#endif
	}
    }
    tell_oops("set_no_char: attempt to set character of unknown font");
    /* NOTREACHED */
}


/*
 *	Set rule.  Arguments are coordinates of lower left corner.
 */

static void
set_rule(int h, int w)
{
#ifndef	TEXXET
    put_rule(False, PXL_H, PXL_V - h + 1, (unsigned int)w, (unsigned int)h);
#else
    put_rule(False, PXL_H - (currinf.dir < 0 ? w - 1 : 0), PXL_V - h + 1,
	     (unsigned int)w, (unsigned int)h);
#endif
}


/*
 *	Interpret a sequence of dvi bytes (either the page from the dvi file,
 *	or a character from a virtual font).
 */

#define	xspell_conv(n)	spell_conv0(n, current_dimconv)

static void
draw_part(struct frame *minframe, double current_dimconv)
{
    ubyte ch;
#ifdef	TEXXET
    struct drawinf oldinfo;
    ubyte oldmaxchar = 0;
    off_t file_pos = 0;
    int refl_count = 0;
#endif

    currinf.fontp = NULL;
    currinf.set_char_p = set_no_char;
#ifdef	TEXXET
    currinf.dir = 1;
    scan_frame = NULL;	/* indicates we're not scanning */
#endif
    for (;;) {
	ch = xone();

	if (debug & DBG_DVI)
	    print_dvi(ch);
	if (ch <= (ubyte) (SETCHAR0 + 127)) {
#ifndef	TEXXET
	    DVI_H += (*currinf.set_char_p) (ch);
#else
	    (*currinf.set_char_p) (ch, ch);
#endif
	}
	else if (FNTNUM0 <= ch && ch <= (ubyte) (FNTNUM0 + 63)) {
	    change_font((unsigned long)(ch - FNTNUM0));
	}
	else if (ch == 255) {
	    /* FIXME: we sometimes arrive here when doing a
	       forward search on a corrupted DVI file. */
	    fprintf(stderr, "shouldn't happen: op-code 255 (trying to recover ...)\n");
	    return;
	}
	else {
	    long a, b;

	    switch (ch) {
	    case SET1:
	    case PUT1:
#ifndef	TEXXET
		a = (*currinf.set_char_p) (xone());
		if (ch != PUT1)
		    DVI_H += a;
#else
		(*currinf.set_char_p) (ch, xone());
#endif
		break;

#ifdef Omega
	    case SET2:
	    case PUT2:
#ifndef       TEXXET
		a = (*currinf.set_char_p) (xnum(2));
		if (ch != PUT2)
		    DVI_H += a;
#else
		(*currinf.set_char_p) (ch, xnum(2));
#endif
		break;
#endif /* Omega */

	    case SETRULE:
		/* Be careful, dvicopy outputs rules with
		   height = 0x80000000.  We don't want any
		   SIGFPE here. */
		a = xsfour();
		b = xspell_conv(xsfour());
#ifndef	TEXXET
		if (a > 0 && b > 0)
#else
		if (a > 0 && b > 0 && scan_frame == NULL)
#endif
		    set_rule(pixel_round(xspell_conv(a)), pixel_round(b));
		DVI_H += DIR * b;
		break;

	    case PUTRULE:
		a = xspell_conv(xsfour());
		b = xspell_conv(xsfour());
#ifndef	TEXXET
		if (a > 0 && b > 0)
#else
		if (a > 0 && b > 0 && scan_frame == NULL)
#endif
		    set_rule(pixel_round(a), pixel_round(b));
		break;

	    case NOP:
		break;

	    case BOP:
		xskip((long)11 * 4);
		DVI_H = OFFSET_X;
		DVI_V = OFFSET_Y;
		PXL_V = pixel_conv(DVI_V);
		WW = XX = YY = ZZ = 0;
		break;

	    case EOP:
		if (current_frame != minframe)
		    tell_oops("draw_part: stack not empty at EOP");
		return;

	    case PUSH:
		if (current_frame->next == NULL) {
		    struct frame *newp = xmalloc(sizeof(struct frame));

		    current_frame->next = newp;
		    newp->prev = current_frame;
		    newp->next = NULL;
		}
		current_frame = current_frame->next;
		current_frame->data = currinf.data;
		break;

	    case POP:
		if (current_frame == minframe)
		    tell_oops("draw_part: more POPs than PUSHes");
		currinf.data = current_frame->data;
		current_frame = current_frame->prev;
		break;

#ifdef	TEXXET
	    case SREFL:
		if (scan_frame == NULL) {
		    /* we're not scanning:  save some info. */
		    oldinfo = currinf;
		    oldmaxchar = maxchar;
		    if (!currinf.virtual)
			file_pos = xtell(currinf.pos);
		    scan_frame = current_frame;	/* now we're scanning */
		    refl_count = 0;
		    break;
		}
		/* we are scanning */
		if (current_frame == scan_frame)
		    ++refl_count;
		break;

	    case EREFL:
		if (scan_frame != NULL) {	/* if we're scanning */
		    if (current_frame == scan_frame && --refl_count < 0) {
			/* we've hit the end of our scan */
			scan_frame = NULL;
			/* first:  push */
			if (current_frame->next == NULL) {
			    struct frame *newp = xmalloc(sizeof(struct frame));

			    current_frame->next = newp;
			    newp->prev = current_frame;
			    newp->next = NULL;
			}
			current_frame = current_frame->next;
			current_frame->data = currinf.data;
			/* next:  restore old file position, XX, etc. */
			if (!currinf.virtual) {
			    off_t bgn_pos = xtell(dvi_buffer);

			    if (file_pos >= bgn_pos) {
				oldinfo.pos = dvi_buffer + (file_pos - bgn_pos);
				oldinfo.end = currinf.end;
			    }
			    else {
				(void)lseek(fileno(dvi_file), file_pos,
					    SEEK_SET);
				oldinfo.pos = oldinfo.end;
			    }
			}
			currinf = oldinfo;
			maxchar = oldmaxchar;
			/* and then:  recover position info. */
			DVI_H = current_frame->data.dvi_h;
			DVI_V = current_frame->data.dvi_v;
			PXL_V = current_frame->data.pxl_v;
			/* and finally, reverse direction */
			currinf.dir = -currinf.dir;
		    }
		    break;
		}
		/* we're not scanning, */
		/* so just reverse direction and then pop */
		currinf.dir = -currinf.dir;
		currinf.data = current_frame->data;
		current_frame = current_frame->prev;
		break;
#endif /* TEXXET */

	    case RIGHT1:
	    case RIGHT2:
	    case RIGHT3:
	    case RIGHT4:
		DVI_H += DIR * xspell_conv(xsnum(ch - RIGHT1 + 1));
		break;

	    case W1:
	    case W2:
	    case W3:
	    case W4:
		WW = xspell_conv(xsnum(ch - W0));
	    case W0:
		DVI_H += DIR * WW;
		break;

	    case X1:
	    case X2:
	    case X3:
	    case X4:
		XX = xspell_conv(xsnum(ch - X0));
	    case X0:
		DVI_H += DIR * XX;
		break;

	    case DOWN1:
	    case DOWN2:
	    case DOWN3:
	    case DOWN4:
		DVI_V += xspell_conv(xsnum(ch - DOWN1 + 1));
		PXL_V = pixel_conv(DVI_V);
		break;

	    case Y1:
	    case Y2:
	    case Y3:
	    case Y4:
		YY = xspell_conv(xsnum(ch - Y0));
	    case Y0:
		DVI_V += YY;
		PXL_V = pixel_conv(DVI_V);
		break;

	    case Z1:
	    case Z2:
	    case Z3:
	    case Z4:
		ZZ = xspell_conv(xsnum(ch - Z0));
	    case Z0:
		DVI_V += ZZ;
		PXL_V = pixel_conv(DVI_V);
		break;

	    case FNT1:
	    case FNT2:
	    case FNT3:
	    case FNT4:
		change_font(xnum(ch - FNT1 + 1));
		break;

	    case XXX1:
	    case XXX2:
	    case XXX3:
	    case XXX4:
		a = xnum(ch - XXX1 + 1);
		if (a > 0) {
		    applicationDoSpecial(read_special(a));
		}
		break;

	    case FNTDEF1:
	    case FNTDEF2:
	    case FNTDEF3:
	    case FNTDEF4:
		xskip((long)(12 + ch - FNTDEF1 + 1));
		a = (long)xone();
		xskip(a + (long)xone());
		break;

#ifndef	TEXXET
	    case SREFL:
	    case EREFL:
#endif
	    case PRE:
	    case POST:
	    case POSTPOST:
		tell_oops("draw_part: shouldn't happen: %s encountered",
			  dvi_table2[ch - (FNTNUM0 + 64)]);
		break;

	    default:
		tell_oops("draw_part: unknown op-code %d", ch);
	    }	/* end switch */
	}	/* end else (ch not a SETCHAR or FNTNUM) */
    }	/* end for */
}

#undef	xspell_conv


void
draw_page(void)
{
    if (dvi_file_changed() > 0 || page_offset == NULL) {
/* 	dvi_time--; */
	return;
    }

#if	PS
    if (scanned_page < current_page)
	prescan();
#endif

    put_border(-currwin.base_x, -currwin.base_y,
	       ROUNDUP(unshrunk_paper_w, currwin.shrinkfactor) + 2,
	       ROUNDUP(unshrunk_paper_h, currwin.shrinkfactor) + 2, highGC);
#ifdef GRID
    if (grid_mode > 0)	/* grid is wanted */
	put_grid(-currwin.base_x, -currwin.base_y,
		 ROUNDUP(unshrunk_paper_w, currwin.shrinkfactor) + 2,
		 ROUNDUP(unshrunk_paper_h, currwin.shrinkfactor) + 2,
		 ROUNDUP(unshrunk_paper_unit, currwin.shrinkfactor),
		 rulerGC);
#endif /* GRID */
    (void)lseek(fileno(dvi_file), page_offset[current_page], SEEK_SET);

    bzero((char *)&currinf.data, sizeof currinf.data);
    currinf.tn_table_len = TNTABLELEN;
    currinf.tn_table = tn_table;
    currinf.tn_head = tn_head;
    currinf.pos = currinf.end = dvi_buffer;
    currinf.virtual = NULL;
    dvi_pointer_frame = &currinf;
#ifdef HTEX
    htex_initpage();	/* Start up the hypertext stuff for this page */
#ifdef PS_GS
    FIXME_ps_lock = FALSE;
#endif
#endif
    psfig_begun = False;
    draw_part(current_frame = &frame0, dimconv);
    dvi_pointer_frame = NULL;
#ifdef HTEX
    htex_donepage(current_page, 1);	/* Finish up the hypertext stuff for this page */
    check_for_anchor();	/* see if a new anchor was requested */
#endif
    if (currwin.win == mane.win) {
 	if (source_fwd_box_page >= 0) {
	    source_fwd_draw_box();	/* draw box showing found source line */
	}
 	if (debug & DBG_HYPER) {
 	    fprintf(stderr, "markers: %d, %d; page: %d (curr: %d)\n",
 		    global_x_marker, global_y_marker, global_marker_page, current_page);
 	}
 	if (global_x_marker != 0 && global_y_marker != 0 && current_page == global_marker_page) {
 	    htex_draw_anchormarker(global_x_marker, global_y_marker);
 	}
    }
#if	PS
    psp.endpage();
#endif
}

#ifdef HTEX
extern int *htex_parsedpages;	/* List of pages parsed */

/* Parse dvi pages not yet scanned by xdvi, searching for html anchors */
void
htex_parsepages(void)
{
    int i;
    if (debug & DBG_HYPER) 
	fprintf(stderr, "htex_parsepages\n");
    for (i = 0; i < total_pages; i++) {
	if (htex_parsedpages[i] != 0)
	    continue;
	(void)htex_parse_page(i);
    }
    /* Return to original position in dvi file? */
}

Boolean
htex_parse_page(int i)
{
    if (debug & DBG_HYPER)
	fprintf(stderr, "htex_parse_page called for %d\n", i);
    if (dvi_file_changed() > 0 || page_offset == NULL) {
/* 	dvi_time--; */
	return False;
    }

    /* Skip to beginning of page i */
    /* Stuff from draw_page in dvi_draw.c: */
    (void)lseek(fileno(dvi_file), page_offset[i], SEEK_SET);
    bzero((char *)&currinf.data, sizeof(currinf.data));
    currinf.tn_head = tn_head;
    currinf.pos = currinf.end = dvi_buffer;
    if (debug & DBG_HYPER) 
	fprintf(stderr, "setting currinf pos: %d\n", *(currinf.pos));
    currinf.virtual = NULL;
    htex_initpage();
    psfig_begun = False;
    if (debug & DBG_HYPER) 
	fprintf(stderr, "===calling htex_scanpage %d\n", i);
    htex_scanpage(i);
    dvi_pointer_frame = NULL;
    if (debug & DBG_HYPER) 
	fprintf(stderr, "===calling htex_donepage %d\n", i);
    htex_donepage(i, 0);
#if PS
    if (debug & DBG_HYPER) 
	fprintf(stderr, "===calling psp.endpage\n");
    psp.endpage();
#endif
    return True;
}


/* HTeX parse page without drawing it function: */
/* Basically the same as is done by draw_part, except we don't
   do anything other than record html anchors */
/* Assumes page has been set up properly as above */
void
htex_scanpage(int pageno)
{
    ubyte ch;
    long a;

    /* Read in page, searching for XXX1 to XXX4 (\special)
       sections and ignoring everything else */
    for (;;) {
	ch = xone();
/* 	fprintf(stderr, "ch: 0x%x - pos: %d, end: %d, diff: %d\n", ch, currinf.pos, currinf.end, currinf.end - currinf.pos); */
	if (ch == 0xff) {
	    return;
	}
	if (ch <= SETCHAR0 + 127) {	/* It's a character - do nothing */
	}
	else if (FNTNUM0 <= ch && ch <= FNTNUM0 + 63) {
	    /* It's a previously numbered font - again do nothing */
	}
	else {
	    switch (ch) {
	    case PUSH:	/* No args */
	    case POP:	/* No args */
	    case NOP:	/* Nothing! */
		break;
	    case SET1:
	    case PUT1:
		xskip((long)1);	/* Set or put 1 char - skip it */
		break;
#ifdef Omega
	    case SET2:
	    case PUT2:
		xskip((long)2);	/* Set or put 16-bit char - skip it */
		break;
#endif
	    case SETRULE:
	    case PUTRULE:
		xskip((long)8);	/* two integers a and b */
		break;
	    case BOP:	/* Beginning of page stuff */
		xskip((long)11 * 4);
		break;
	    case EOP:	/* End of the page! */
		return;
	    case SREFL:
	    case EREFL:
		break;
	    case RIGHT1:
	    case RIGHT2:
	    case RIGHT3:
	    case RIGHT4:
		xskip(ch - RIGHT1 + 1);
		break;
	    case W1:
	    case W2:
	    case W3:
	    case W4:
		xskip(ch - W0);
	    case W0:
		break;
	    case X1:
	    case X2:
	    case X3:
	    case X4:
		xskip(ch - X0);
	    case X0:
		break;
	    case DOWN1:
	    case DOWN2:
	    case DOWN3:
	    case DOWN4:
		xskip(ch - DOWN1 + 1);
		break;
	    case Y1:
	    case Y2:
	    case Y3:
	    case Y4:
		xskip(ch - Y0);
	    case Y0:
		break;
	    case Z1:
	    case Z2:
	    case Z3:
	    case Z4:
		xskip(ch - Z0);
	    case Z0:
		break;
	    case FNT1:
	    case FNT2:
	    case FNT3:
	    case FNT4:
		xskip(ch - FNT1 + 1);
		break;
	    case XXX1:
	    case XXX2:
	    case XXX3:
	    case XXX4:	/* Only thing we pay attention to! */
		a = xnum(ch - XXX1 + 1);
		if (a > 0)
		    htex_dospecial(a, pageno);
		break;
	    case FNTDEF1:
	    case FNTDEF2:
	    case FNTDEF3:
	    case FNTDEF4:
		xskip((long)(12 + ch - FNTDEF1 + 1));
		xskip((long)xone() + (long)xone());
		break;
	    case PRE:
	    case POST:
	    case POSTPOST:
	    default:
		break;
	    }
	}
    }
}

/* Do the same same stuff as special() in dvi_draw.c */
/* But only check HTeX specials */
void
htex_dospecial(long nbytes, int pageno)
{
    static char *cmd = NULL;
    static long cmdlen = -1;
    char *p;

    if (cmdlen < nbytes) {
	if (cmd)
	    free(cmd);
	cmd = xmalloc((unsigned)nbytes + 1);
	cmdlen = nbytes;
    }
    p = cmd;
    for (;;) {
	int i = currinf.end - currinf.pos;
	if (i > nbytes)
	    i = nbytes;
	bcopy((_Xconst char *)currinf.pos, p, i);
	currinf.pos += i;
	p += i;
	nbytes -= i;
	if (nbytes == 0)
	    break;
	(void)xxone();
	--(currinf.pos);
    }
    *p = '\0';
    p = cmd;
    while (isspace(*p))
	++p;
    checkHyperTeX(p, pageno);
}
#endif /* HTEX */


/*
 *	General dvi scanning routines.  These are used for:
 *	    o	source special lookups and
 *	    o	finding the dimensions of links (if compiling with support for
 *		hypertext specials).
 *	This routine can be a bit slower than draw_page()/draw_part(), since
 *	it is not run that often; that is why it is a separate routine in
 *	spite of much duplication.
 *
 *	Note that it does not use a separate copy of define_font().
 */

/*
 *	All calculations are done with shrink factor = 1, so we re-do some
 *	macros accordingly.  Many of these are also defined in special.c.
 */

#define	xspell_conv(n)	spell_conv0(n, current_dimconv)
#define	xpixel_conv(x)	((int) ((x) >> 16))
#define	xpixel_round(x)	((int) ROUNDUP(x, 1 << 16))

#define	G_PXL_H		xpixel_conv(currinf.data.dvi_h)
#define	G_OFFSET_X	(offset_x << 16) + (3 << 15)
#define	G_OFFSET_Y	(offset_y << 16) + (3 << 15)

#if TOOLKIT
# define mane_base_x	0
# define mane_base_y	0
#else
# define mane_base_x	mane.base_x
# define mane_base_y	mane.base_y
#endif

/*
 *	This set of routines can be called while draw_part() is active,
 *	so the global variables must be separate.
 */

static struct frame geom_frame0;	/* dummy head of list */
#ifdef TEXXET
static struct frame *geom_scan_frame;	/* head frame for scanning */
#endif
static struct frame *geom_current_frame;

static void geom_scan_part ARGS((struct geom_info *, struct frame *, double));

/*
 *	Handle a character in geometric scanning routine.
 */

static long
geom_do_char(struct geom_info *g_info, wide_ubyte ch)
{
    if (currinf.set_char_p == set_no_char) {
	if (currinf.virtual == NULL
	    || (currinf.fontp = currinf.virtual->first_font) == NULL)
	    return 0;	/* error; we'll catch it later */
	maxchar = currinf.fontp->maxchar;
	currinf.set_char_p = currinf.fontp->set_char_p;
    }

    if (currinf.set_char_p == set_empty_char)
	return 0;	/* error; we'll catch it later */

    if (currinf.set_char_p == load_n_set_char) {
	if (!load_font(currinf.fontp)) {	/* if not found */
	    /* FIXME: replace by GUI warning! */
	    fprintf(stderr, "Character(s) will be left blank.\n");
	    currinf.set_char_p = currinf.fontp->set_char_p = set_empty_char;
	    return 0;
	}
	maxchar = currinf.fontp->maxchar;
	currinf.set_char_p = currinf.fontp->set_char_p;
    }

    if (currinf.set_char_p == set_char) {
	struct glyph *g;
	long x, y;

	if (ch > maxchar)
	    return 0;	/* catch the error later */
	if ((g = &currinf.fontp->glyph[ch])->bitmap.bits == NULL) {
	    if (g->addr == 0)
		return 0;	/* catch the error later */
	    if (g->addr == -1)
		return 0;	/* previously flagged missing char */
	    open_font_file(currinf.fontp);
	    Fseek(currinf.fontp->file, g->addr, 0);
	    (*currinf.fontp->read_char) (currinf.fontp, ch);
	    if (debug & DBG_BITMAP)
		print_char((ubyte) ch, g);
	    currinf.fontp->timestamp = ++current_timestamp;
	}
#ifdef TEXXET
	if (geom_scan_frame == NULL) {
	    long dvi_h_sav = DVI_H;
	    if (currinf.dir < 0)
		DVI_H -= g->dvi_adv;
#endif
	    x = G_PXL_H - g->x;
	    y = PXL_V - g->y;
	    g_info->geom_box(g_info, x, y,
			     x + g->bitmap.w - 1, y + g->bitmap.h - 1);

#ifdef TEXXET
	    DVI_H = dvi_h_sav;
	}
#endif
	return DIR * g->dvi_adv;
    }
    else if (currinf.set_char_p == set_vf_char) {
	struct macro *m;
	struct drawinf oldinfo;
	ubyte oldmaxchar;
#ifdef TEXXET
	long dvi_h_sav;
#endif

	if (ch > maxchar)
	    return 0;	/* catch the error later */
	if ((m = &currinf.fontp->macro[ch])->pos == NULL)
	    return 0;	/* catch the error later */
#ifdef TEXXET
	dvi_h_sav = DVI_H;
	if (currinf.dir < 0)
	    DVI_H -= m->dvi_adv;
	if (geom_scan_frame == NULL) {
#endif
	    oldinfo = currinf;
	    oldmaxchar = maxchar;
	    WW = XX = YY = ZZ = 0;
	    currinf.tn_table_len = VFTABLELEN;
	    currinf.tn_table = currinf.fontp->vf_table;
	    currinf.tn_head = currinf.fontp->vf_chain;
	    currinf.pos = m->pos;
	    currinf.end = m->end;
	    currinf.virtual = currinf.fontp;
	    geom_scan_part(g_info, geom_current_frame, currinf.fontp->dimconv);
	    currinf = oldinfo;
	    maxchar = oldmaxchar;
#ifdef TEXXET
	    DVI_H = dvi_h_sav;
	}
#endif
	return DIR * m->dvi_adv;
    }
#ifdef T1LIB
    else if (currinf.set_char_p == set_t1_char) {
	struct glyph *g ;
	long x, y;
	t1font_load_status_t dummy;
	
#ifdef TEXXET
	g = get_t1_glyph(0, ch, &dummy);
	assert(g != NULL);
	if (geom_scan_frame == NULL) {
	    long dvi_h_sav = DVI_H;
	    if (currinf.dir < 0)
		DVI_H -= g->dvi_adv;
#else
	    g = get_t1_glyph(ch, &dummy);
	    assert(g != NULL);
#endif
	    x = G_PXL_H - g->x;
	    y = PXL_V - g->y;
	    g_info->geom_box(g_info, x, y,
			     x + g->bitmap.w - 1, y + g->bitmap.h - 1);
#ifdef TEXXET
	    DVI_H = dvi_h_sav;
	}
#endif
	return DIR * g->dvi_adv;
    }
#endif /* T1LIB */
    else {
	oops("internal error -- currinf.set_char_p = 0x%x", currinf.set_char_p);
    }
    /* NOTREACHED */
    return 0;
}

/*
 *	Do a rule in the geometry-scanning routine.
 */

static void
geom_do_rule(struct geom_info *g_info, long h, long w)
{
    long x, y;
#ifdef TEXXET
    long dvi_h_save = DVI_H;
#endif

#ifdef TEXXET
    if (currinf.dir < 0)
	DVI_H -= w - 1;
#endif
    x = G_PXL_H;
    y = PXL_V;
    g_info->geom_box(g_info, x, y - xpixel_round(h) + 1,
		     x + xpixel_round(w) - 1, y);
#ifdef TEXXET
    DVI_H = dvi_h_save;
#endif
}

/*
 *	Geometric dvi scanner work routine.  This does most of the work
 *	(a) reading from a page, and (b) executing vf macros.
 */

static void
geom_scan_part(struct geom_info *g_info, struct frame *minframe, double current_dimconv)
{
    ubyte ch;
#ifdef TEXXET
    struct drawinf oldinfo;
    ubyte oldmaxchar = 0;
    off_t file_pos = 0;
    int refl_count = 0;
#endif

    currinf.fontp = NULL;
    currinf.set_char_p = set_no_char;
#ifdef TEXXET
    currinf.dir = 1;
    geom_scan_frame = NULL;	/* indicates we're not scanning */
#endif
    for (;;) {
	ch = xone();
	if (ch <= (ubyte) (SETCHAR0 + 127))
	    DVI_H += geom_do_char(g_info, ch);
	else if (FNTNUM0 <= ch && ch <= (ubyte) (FNTNUM0 + 63)) {
	    change_font((unsigned long)(ch - FNTNUM0));
	}
	else {
	    long a, b;

	    switch (ch) {
	    case SET1:
	    case PUT1:
		a = geom_do_char(g_info, xone());
		if (ch != PUT1)
		    DVI_H += a;
		break;

	    case SETRULE:
		/* Be careful, dvicopy outputs rules with
		   height = 0x80000000.  We don't want any
		   SIGFPE here. */
		a = xsfour();
		b = xspell_conv(xsfour());
#ifdef TEXXET
		if (a >= 0 && b >= 0 && geom_scan_frame == NULL)
#else
		if (a >= 0 && b >= 0)
#endif
		    geom_do_rule(g_info, xspell_conv(a), b);
		DVI_H += DIR * b;
		break;

	    case PUTRULE:
		a = xspell_conv(xsfour());
		b = xspell_conv(xsfour());
#ifdef TEXXET
		if (a >= 0 && b >= 0 && geom_scan_frame == NULL)
#else
		if (a >= 0 && b >= 0)
#endif
		    geom_do_rule(g_info, a, b);
		break;

	    case NOP:
		break;

	    case BOP:
		xskip((long)11 * 4);
		DVI_H = G_OFFSET_X;
		DVI_V = G_OFFSET_Y;
		PXL_V = xpixel_conv(DVI_V);
		WW = XX = YY = ZZ = 0;
		break;

	    case PUSH:
		if (geom_current_frame->next == NULL) {
		    struct frame *newp = xmalloc(sizeof(struct frame));

		    geom_current_frame->next = newp;
		    newp->prev = geom_current_frame;
		    newp->next = NULL;
		}
		geom_current_frame = geom_current_frame->next;
		geom_current_frame->data = currinf.data;
		break;

	    case POP:
		if (geom_current_frame == minframe)
		    tell_oops("more POPs than PUSHes");
		currinf.data = geom_current_frame->data;
		geom_current_frame = geom_current_frame->prev;
		break;

#ifdef TEXXET
	    case SREFL:
		if (geom_scan_frame == NULL) {
		    /* we're not scanning:  save some info. */
		    oldinfo = currinf;
		    oldmaxchar = maxchar;
		    if (!currinf.virtual)
			file_pos = xtell(currinf.pos);
		    geom_scan_frame = geom_current_frame;	/* start scanning */
		    refl_count = 0;
		    break;
		}
		/* we are scanning */
		if (geom_current_frame == geom_scan_frame)
		    ++refl_count;
		break;

	    case EREFL:
		if (geom_scan_frame != NULL) {	/* if we're scanning */
		    if (geom_current_frame == geom_scan_frame
			&& --refl_count < 0) {
			/* we've hit the end of our scan */
			geom_scan_frame = NULL;
			/* first:  push */
			if (geom_current_frame->next == NULL) {
			    struct frame *newp = xmalloc(sizeof(struct frame));

			    geom_current_frame->next = newp;
			    newp->prev = geom_current_frame;
			    newp->next = NULL;
			}
			geom_current_frame = geom_current_frame->next;
			geom_current_frame->data = currinf.data;
			/* next:  restore old file position, XX, etc. */
			if (!currinf.virtual) {
			    off_t bgn_pos = xtell(dvi_buffer);

			    if (file_pos >= bgn_pos) {
				oldinfo.pos = dvi_buffer + (file_pos - bgn_pos);
				oldinfo.end = currinf.end;
			    }
			    else {
				(void)lseek(fileno(dvi_file), file_pos,
					    SEEK_SET);
				oldinfo.pos = oldinfo.end;
			    }
			}
			currinf = oldinfo;
			maxchar = oldmaxchar;
			/* and then:  recover position info. */
			DVI_H = geom_current_frame->data.dvi_h;
			DVI_V = geom_current_frame->data.dvi_v;
			PXL_V = geom_current_frame->data.pxl_v;
			/* and finally, reverse direction */
			currinf.dir = -currinf.dir;
		    }
		    break;
		}
		/* we're not scanning, */
		/* so just reverse direction and then pop */
		currinf.dir = -currinf.dir;
		currinf.data = geom_current_frame->data;
		geom_current_frame = geom_current_frame->prev;
		break;
#endif /* TEXXET */

	    case RIGHT1:
	    case RIGHT2:
	    case RIGHT3:
	    case RIGHT4:
		DVI_H += DIR * xspell_conv(xsnum(ch - RIGHT1 + 1));
		break;

	    case W1:
	    case W2:
	    case W3:
	    case W4:
		WW = xspell_conv(xsnum(ch - W0));
	    case W0:
		DVI_H += DIR * WW;
		break;

	    case X1:
	    case X2:
	    case X3:
	    case X4:
		XX = xspell_conv(xsnum(ch - X0));
	    case X0:
		DVI_H += DIR * XX;
		break;

	    case DOWN1:
	    case DOWN2:
	    case DOWN3:
	    case DOWN4:
		DVI_V += xspell_conv(xsnum(ch - DOWN1 + 1));
		PXL_V = xpixel_conv(DVI_V);
		break;

	    case Y1:
	    case Y2:
	    case Y3:
	    case Y4:
		YY = xspell_conv(xsnum(ch - Y0));
	    case Y0:
		DVI_V += YY;
		PXL_V = xpixel_conv(DVI_V);
		break;

	    case Z1:
	    case Z2:
	    case Z3:
	    case Z4:
		ZZ = xspell_conv(xsnum(ch - Z0));
	    case Z0:
		DVI_V += ZZ;
		PXL_V = xpixel_conv(DVI_V);
		break;

	    case FNT1:
	    case FNT2:
	    case FNT3:
	    case FNT4:
		change_font(xnum(ch - FNT1 + 1));
		break;

	    case XXX1:
	    case XXX2:
	    case XXX3:
	    case XXX4:
		a = xnum(ch - XXX1 + 1);
		if (a > 0) {
		    char *str = read_special(a);

		    /* process the bounding box */
		    geom_do_special(g_info, str, current_dimconv);

		    /* process the specials we're looking for */
		    g_info->geom_special(g_info, str);
		}
		break;

	    case FNTDEF1:
	    case FNTDEF2:
	    case FNTDEF3:
	    case FNTDEF4:
		xskip((long)(12 + ch - FNTDEF1 + 1));
		a = (long)xone();
		xskip(a + (long)xone());
		break;

#ifndef TEXXET
	    case SREFL:
	    case EREFL:
#endif
	    case PRE:
	    case POST:
	    case POSTPOST:
	    case EOP:
	    default:
		return;

	    }	/* end switch */
	}	/* end else (ch not a SETCHAR or FNTNUM) */
    }	/* end for */
}


/*
 *	Main scanning routine.
 */

static void
geom_scan(struct geom_info *g_info)
{
    volatile off_t pos_save = 0;
    struct drawinf currinf_save;
    ubyte maxchar_save;

    if (page_offset == NULL) {
/* 	dvi_time--; */
	return;
    }
#if PS
    if (scanned_page < current_page)
	return;	/* should not happen */
#endif

    if (dvi_pointer_frame != NULL)
	pos_save = lseek(fileno(dvi_file), 0L, SEEK_CUR)
	    - (dvi_pointer_frame->end - dvi_pointer_frame->pos);
    (void)lseek(fileno(dvi_file), page_offset[current_page], SEEK_SET);

    currinf_save = currinf;
    maxchar_save = maxchar;

    bzero((char *)&currinf.data, sizeof currinf.data);
    currinf.tn_table_len = TNTABLELEN;
    currinf.tn_table = tn_table;
    currinf.tn_head = tn_head;
    currinf.pos = currinf.end = dvi_buffer;
    currinf.virtual = NULL;

    if (!setjmp(g_info->done_env))
	geom_scan_part(g_info, geom_current_frame = &geom_frame0, dimconv);

    maxchar = maxchar_save;
    currinf = currinf_save;

    if (dvi_pointer_frame != NULL) {
	(void)lseek(fileno(dvi_file), pos_save, SEEK_SET);
	dvi_pointer_frame->pos = dvi_pointer_frame->end = dvi_buffer;
    }
}


/*
 *	Routines for source special lookup
 */

static void
src_spec_box ARGS((struct geom_info *, long, long, long, long));

static void
src_spec_special ARGS((struct geom_info *, _Xconst char *));

struct src_parsed_special {
    int line;
    int col;
    char *filename;
    size_t filename_len;
};

struct src_spec_data {
    long x, y;				/* coordinates we're looking for */
    unsigned long distance;		/* best distance so far */
    Boolean recent_in_best;		/* most recent string == XXX->best */
    struct src_parsed_special best;	/* best special so far */
    struct src_parsed_special recent;	/* most recent special */
};

static void
src_parse(_Xconst char *str, struct src_parsed_special *parsed)
{
    _Xconst char *p;

    p = str + 4;	/* skip "src:" */

    if (*p >= '0' && *p <= '9') {
	parsed->line = atoi(p);
	do
	    ++p;
	while (*p >= '0' && *p <= '9');
    }

    parsed->col = 0;
    if (*p == ':') {
	++p;
	parsed->col = atoi(p);
	while (*p >= '0' && *p <= '9')
	    ++p;
    }

    if (*p == ' ')
	++p;

    if (*p != '\0') {
	size_t len = strlen(p) + 1;

	if (len > parsed->filename_len) {
	    if (parsed->filename_len != 0)
		free(parsed->filename);
	    parsed->filename_len = (len & -8) + 64;
	    parsed->filename = xmalloc(parsed->filename_len);
	}
	memcpy(parsed->filename, p, len);
    }
}

static void
src_spec_box(struct geom_info *g_info, long ulx, long uly, long lrx, long lry)
{
    struct src_spec_data *data = g_info->geom_data;
    unsigned long distance;

    distance = 0;

    if (data->x < ulx)
	distance += (ulx - data->x) * (ulx - data->x);
    else if (data->x > lrx)
	distance += (data->x - lrx) * (data->x - lrx);

    if (data->y < uly)
	distance += (uly - data->y) * (uly - data->y);
    else if (data->y > lry)
	distance += (data->y - lry) * (data->y - lry);

    if (distance < data->distance) {
	data->distance = distance;

	/* Copy it over */
	if (!data->recent_in_best) {
	    data->best.line = data->recent.line;
	    data->best.col = data->recent.col;
	    if (data->recent.filename_len != 0) {
		if (data->best.filename_len < data->recent.filename_len) {
		    if (data->best.filename_len != 0)
			free(data->best.filename);
		    data->best.filename_len = data->recent.filename_len;
		    data->best.filename = xmalloc(data->best.filename_len);
		}
		memcpy(data->best.filename, data->recent.filename,
		       data->recent.filename_len);
	    }

	    data->recent_in_best = True;
	}

	/* Quit early if we've found our glyph.  */
	if (distance == 0 && data->best.filename_len != 0) {
	    longjmp(g_info->done_env, 1);
	}
    }
}

static Boolean src_fwd_active;

static void
src_spec_special(struct geom_info *g_info, _Xconst char *str)
{
    struct src_spec_data *data = g_info->geom_data;

    if (memcmp(str, "src:", 4) != 0)
	return;

    src_parse(str, &data->recent);

    /*
     * If this is the first special on the page, we may already have
     * spotted the nearest box.
     */

    if (data->best.filename_len == 0) {
	data->best.line = data->recent.line;
	data->best.col = data->recent.col;
	if (data->recent.filename_len != 0) {
	    if (data->best.filename_len < data->recent.filename_len) {
		if (data->best.filename_len != 0)
		    free(data->best.filename);
		data->best.filename_len = data->recent.filename_len;
		data->best.filename = xmalloc(data->best.filename_len);
	    }
	    memcpy(data->best.filename, data->recent.filename,
		   data->recent.filename_len);

	    data->recent_in_best = True;
	}

	if (data->distance == 0) { 
	    longjmp(g_info->done_env, 1);
	}
    }
    else
	data->recent_in_best = False;
}

/*
 *	Routines for reverse searches on other pages.
 */

static struct src_parsed_special found;
static jmp_buf scan_env;

static void
scan_first_src_spcl(char *str)
{
    if (memcmp(str, "src:", 4) != 0)
	return;

    src_parse(str, &found);

    longjmp(scan_env, 1);
}

static void
scan_last_src_spcl(char *str)
{
    if (memcmp(str, "src:", 4) != 0)
	return;

    src_parse(str, &found);
}

#if !HAVE_ULLTOSTR

static char *
ulltostr(unsigned int value, char *p)
{
    do
	*--p = value % 10 + '0';
    while ((value /= 10) != 0);

    return p;
}

#endif

/*------------------------------------------------------------
 *  src_find_file
 *
 *  Arguments:
 *	 filename - file name in src \special
 *   statbuf   - buffer to stat filename
 *	 
 *  Returns:
 *	 expanded filename
 *		 
 *  Purpose:
 *	Find a file name corresponding to <filename>, possibly
 *	expanding it to a full path name; checks if the file
 *	exists by stat()ing it; returns NULL if nothing has
 *	been found, else malloc()ed filename.
 *
 *  Side effects:
 *	malloc()s space for returned filename; the caller is
 *	responsible for free()ing it if it's no longer needed.
 *	 
 *------------------------------------------------------------*/

static char *
src_find_file(char *filename, struct stat *statbuf)
{
    char *tmp;

    TRACE_SRC((stderr, "checking filename \"%s\"", filename));

    /*
     * case 1:
     * try absolute filename
     */
    if (filename[0] == DIR_SEPARATOR) {
	if (stat(filename, statbuf) == 0) {
	    TRACE_SRC((stderr, "Found absolute filename \"%s\"", filename));
	    return xstrdup(filename);
	}
	else {
	    Fprintf(stderr, "Can't stat absolute filename \"%s\"\n", filename);
	    return NULL;
	}
    }

    /*
     * case 2:
     * prepend filename with path name from the `main' xdvi file (global_dvi_name)
     */
    assert(global_dvi_name != NULL);
    if ((tmp = strrchr(global_dvi_name, DIR_SEPARATOR)) != NULL) {	/* does it have a path component? */
	int len = tmp - global_dvi_name;
	char *pathname = xmalloc(len + strlen(filename) + 2); /* 1 for DIR_SEPARATOR */
	memcpy(pathname, global_dvi_name, len);
	pathname[len] = DIR_SEPARATOR;
	pathname[len + 1] = '\0';
	strcat(pathname, filename);

	if (stat(pathname, statbuf) == 0) {
	    TRACE_SRC((stderr, "Found relative file \"%s\"", pathname));
	    return pathname;
	}
	else {
	    free(pathname);
	}
    }

    /*
     * case 3:
     * try current directory
     */
    if (stat(filename, statbuf) == 0) {
	TRACE_SRC((stderr, "Found file \"%s\" in current dir", filename));
	return xstrdup(filename);
    }

    /*
     * case 4:
     * try a kpathsea search
     */
    TRACE_SRC((stderr,
	       "file \"%s\" not found in cwd, trying kpathsea expansion",
	       filename));
    tmp = kpse_find_tex(filename);

    if (stat(tmp, statbuf) == 0) {
	TRACE_SRC((stderr, "Found file: `%s'", tmp));
	return tmp;
    }
    else {
	free(tmp);
	return NULL;
    }
}

static void
src_spawn_editor(_Xconst struct src_parsed_special *parsed)
{
#if HAVE_ULLTOSTR
    char scr_str[5 * sizeof(unsigned long long) / 2];
#else
    char scr_str[5 * sizeof(unsigned int) / 2];
#endif
    char *expanded_filename;
    Boolean found_filename = False;
    Boolean found_lineno = False;
    static Boolean warned_about_format_err = False;
    size_t buffer_pos;
    int argc;
    char **argv;
    _Xconst char *p, *p1;
    char *q;
    int i;

    /* Used to store argv[] text.  */
    static char *buffer;
    static size_t buffer_len = 0;

    struct stat buf;
        
    /* first, determine the editor if necessary */
    if (resource.editor == NULL || *resource.editor == '\0') {
	p = getenv("XEDITOR");
	if (p != NULL)
	    resource.editor = xstrdup(p);
	else {

	    p = getenv("VISUAL");
	    if (p == NULL) {
		p = getenv("EDITOR");
		if (p == NULL) {
		    do_popup_message(MSG_WARN,
				     /* help text */
				     "Please use the \"-editor\" command-line opion, the X resource \
\"xdvi.editor\" or one of the following environment variables to select the editor for source specials: \
\"XEDITOR\", \"VISUAL\" or \"EDITOR\".\nSee the xdvi man page for more information on source specials \
and the editor options.",
				     /* message */
				     "No editor found - using built-in default (vi).");
		    p = "vi";
		}
	    }
	    q = xmalloc(strlen(p) + 10);
	    memcpy(q, "xterm -e ", 9);
	    strcpy(q + 9, p);
	    resource.editor = q;
	}
    }

    /* now try to open the file; src_find_file allocates space for expanded_filename */
    if ((expanded_filename = src_find_file(parsed->filename, &buf)) == NULL) {
      do_popup_message(MSG_ERR, NULL,
		       "File \"%s\" not found, couldn't jump to special\n\"%s:%d\"\n",
		       parsed->filename, parsed->filename, parsed->line);
    }
    else {
	if (buf.st_mtime > dvi_time) {
	    print_statusline(STATUS_FOREVER,
			     "Warning: TeX file is newer than dvi file - \
source special information might be wrong.");
	}
	
	if (buffer_len == 0)
	    buffer = xmalloc(buffer_len = 128);
	buffer_pos = 0;
	argc = 0;

	p = resource.editor;
	while (*p == ' ' || *p == '\t')
	    ++p;

	for (;;) {
	    size_t len;

	    if (*p == '%') {
		p1 = p;
		switch (p[1]) {
		case '\0':
		    --p;	/* partially undo p += 2 later */
		    /* control falls through */
		case '%':
		    len = 1;
		    break;
		case 'l':
		    p1 = ulltostr(parsed->line, scr_str + sizeof scr_str);
		    len = scr_str + sizeof scr_str - p1;
		    found_lineno = True;
		    break;
		case 'c':
		    if (parsed->col == 0) {
			p += 2;
			continue;
		    }
		    p1 = ulltostr(parsed->col, scr_str + sizeof scr_str);
		    len = scr_str + sizeof scr_str - p1;
		    break;
		case 'f':
		    p1 = expanded_filename;
		    len = strlen(expanded_filename);
		    found_filename = True;
		    break;
		default:
		    len = 2;
		}
		p += 2;
	    }
	    else if (*p == '\0' || *p == ' ' || *p == '\t') {
		buffer[buffer_pos++] = '\0';
		++argc;
		while (*p == ' ' || *p == '\t')
		    ++p;
		if (*p == '\0') {
		    if (found_filename && found_lineno) {
			break;	/* done */
		    }
		    else {
			if (!warned_about_format_err) {
			    XBell(DISP, 10);
			    do_popup_message(MSG_WARN,
					     /* helptext */
					     "The `editor' resource or command-line argument should contain \
at least the strings %%l (for the line number) and %%f (for the file name). For example:\n   \
xdvi.editor: gnuclient -q +%%l %%f\nThe missing strings have been appended for this instance, \
but you might want to fix this error in your xdvi call or X defaults file.",
					     /* popup */
					     "Warning: editor command contains no%s%s%s!",
					     !found_lineno
					     ? " %l (for the line number)" : "",
					     !found_lineno && !found_filename
					     ? " and no" : "",
					     !found_filename
					     ? " %f (for the file name)" : "");
			    warned_about_format_err = True;
			}
			if (!found_filename && !found_lineno) {
			    p = "+%l %f";
			}
			else if (!found_filename) {
			    p = "%f";
			}
			else {
			    p = "+%l";
			}
		    }
		}
		continue;	/* don't copy anything over */
	    }
	    else {
		p1 = p;
		len = strcspn(p, "% \t");
		p += len;
	    }

	    /* copy over len bytes starting at p1 into the buffer, */
	    /* leaving at least one byte free */
	    if (buffer_pos + len >= buffer_len) {
		buffer_len = ((buffer_pos + len) / 128 + 1) * 128;
		buffer = xrealloc(buffer, buffer_len);
	    }
	    memcpy(buffer + buffer_pos, p1, len);
	    buffer_pos += len;
	}

	argv = xmalloc((argc + 1) * sizeof(char *));
	q = buffer;

	for (i = 0; i < argc; ++i) {
	    argv[i] = q;
	    q += strlen(q) + 1;
	}
	  
	/* NULL-terminate argument list */
	argv[argc] = NULL;

	fork_process(argv[0], argv);
	free(expanded_filename);
	free(argv);
    }
}


/*
 *	The main routine for source specials (reverse search).
 */

void
source_reverse_search(int x, int y, wide_bool call_editor)
{
    struct geom_info g_info;
    struct src_spec_data data;
    struct src_parsed_special *foundp;

    if (dvi_file_changed() > 1 || page_offset == NULL) {
/* 	dvi_time--; */
	return;
    }

    g_info.geom_box = src_spec_box;
    g_info.geom_special = src_spec_special;
    g_info.geom_data = &data;

    data.x = x;
    data.y = y;
    data.distance = 0xffffffff;
    data.recent_in_best = True;
    data.best.filename_len = data.recent.filename_len = 0;
    foundp = &data.best;

    geom_scan(&g_info);

    if (data.best.filename_len == 0) {
	/*
	 * nothing found on current page;
	 * scan next and previous pages with increasing offset
	 */
	volatile int upper, lower;
	volatile off_t pos_save = 0;
	struct drawinf currinf_save;
	ubyte maxchar_save;

	/* Save file position */

	if (dvi_pointer_frame != NULL)
	    pos_save = lseek(fileno(dvi_file), 0L, SEEK_CUR)
		- (dvi_pointer_frame->end - dvi_pointer_frame->pos);

	currinf_save = currinf;
	maxchar_save = maxchar;

	upper = lower = current_page;
	found.filename_len = 0;	/* mark it as empty */
	for (;;) {

	    if (++upper < total_pages) {
		(void)lseek(fileno(dvi_file), page_offset[upper], SEEK_SET);
		bzero((char *)&currinf.data, sizeof currinf.data);
		currinf.tn_table_len = TNTABLELEN;
		currinf.tn_table = tn_table;
		currinf.tn_head = tn_head;
		currinf.pos = currinf.end = dvi_buffer;
		currinf.virtual = NULL;

		if (setjmp(scan_env) == 0) {
		    /* find first src special */
		    spcl_scan(scan_first_src_spcl);
		}
		else {	/* found it */
		    lower = upper;
		    break;
		}
	    }
	    else if (lower < 0)
		break;

	    if (--lower >= 0) {
		(void)lseek(fileno(dvi_file), page_offset[lower], SEEK_SET);
		bzero((char *)&currinf.data, sizeof currinf.data);
		currinf.tn_table_len = TNTABLELEN;
		currinf.tn_table = tn_table;
		currinf.tn_head = tn_head;
		currinf.pos = currinf.end = dvi_buffer;
		currinf.virtual = NULL;

		spcl_scan(scan_last_src_spcl);
		if (found.filename_len != 0)
		    break;
	    }
	}

	if (found.filename_len != 0)
	    print_statusline(STATUS_MEDIUM,
			     "No source specials on this page - nearest on page %d",
			     lower + pageno_correct);
	else {
	    /* nothing found at all; complain */
	    XBell(DISP, 10);
	    do_popup_message(MSG_ERR,
			     /* helptext */
			     "To enable forward search, the .tex file needs to be \
compiled with source special support. Please see the xdvi man page (section SOURCE SPECIALS) for details.",
			     /* popup */
			     "No source specials in this .dvi file - couldn't do reverse search");
	}

	/* Restore file status.  */

	maxchar = maxchar_save;
	currinf = currinf_save;

	if (dvi_pointer_frame != NULL) {
	    (void)lseek(fileno(dvi_file), pos_save, SEEK_SET);
	    dvi_pointer_frame->pos = dvi_pointer_frame->end = dvi_buffer;
	}

	foundp = &found;
    }

    if (data.recent.filename_len != 0)
	free(data.recent.filename);

    if (foundp->filename_len != 0) {
	if (call_editor) {
	    src_spawn_editor(foundp);
	}
	else {
	    print_statusline(STATUS_MEDIUM, "nearest special: \"%s:%d\"",
			     foundp->filename, foundp->line);
	}
	free(foundp->filename);
    }
}


/*
 *	Debug routines for source special display (highlight the first box
 *	after each source special, or highlight each box).
 */

static void
src_spec_show_box ARGS((struct geom_info *, long, long, long, long));

static void
src_spec_show_special ARGS((struct geom_info *, _Xconst char *));

struct src_spec_show_data {
    Boolean do_this_one;	/* flag set after source special */
    Boolean do_them_all;	/* flag to reset the above to */
};

static void
src_spec_show_box(struct geom_info *g_info, long ulx, long uly, long lrx, long lry)
{
    struct src_spec_show_data *data = g_info->geom_data;

    if (data->do_this_one) {
	long x = ulx / mane.shrinkfactor;
	long y = uly / mane.shrinkfactor;

	XDrawRectangle(DISP, mane.win, highGC,
		       x - mane_base_x, y - mane_base_y,
		       lrx / mane.shrinkfactor - x,
		       lry / mane.shrinkfactor - y);
	data->do_this_one = data->do_them_all;
    }
}

	/* ARGSUSED */
static void
src_spec_show_special(struct geom_info *g_info, _Xconst char *str)
{
    if (memcmp(str, "src:", 4) != 0)
	return;

    fprintf(stdout, "special: %s\n", str);
    ((struct src_spec_show_data *)g_info->geom_data)->do_this_one = True;
}

void
source_special_show(wide_bool do_them_all)
{
    struct geom_info g_info;
    struct src_spec_show_data data;

    g_info.geom_box = src_spec_show_box;
    g_info.geom_special = src_spec_show_special;
    g_info.geom_data = &data;

    data.do_this_one = data.do_them_all = do_them_all;

    geom_scan(&g_info);
}


/*
 *	Routines for forward search (look up a source line).
 *
 *	The general procedure is:
 *	   (1)	Use spcl_scan() to find the page containing the line (or at
 *		least the closest line).  This could be optimized further.
 *	   (2)	Use geom_scan_part() to find the exact location of the source
 *		special, and the box to highlight.
 */

/* These variables are referenced by src_scan_special().  */

static int src_this_line;
static size_t src_this_file_strcmp;
static int src_line;
static int src_col;
static _Xconst char *src_file;
static int src_page;
static jmp_buf src_env;
static Boolean found_src;
static unsigned long best_distance;
static unsigned long best_col_dist;
static int best_line;
static int best_page;
static off_t best_offset;
static off_t max_offset;

/* Some of the above, plus these below, are used by geom_scan_part().  */

static long src_fwd_min_x, src_fwd_max_x;
static long src_fwd_min_y, src_fwd_max_y;
static long src_fwd_min_x2, src_fwd_max_x2; /* hot point for spcl */
static long src_fwd_min_y2, src_fwd_max_y2;

static void
src_scan_special(char *str)
{
    char *p;
    int col = 0;
    unsigned long distance;
    unsigned long col_dist;
    
    if (memcmp(str, "src:", 4) != 0)
	return;

    found_src = True;

    p = str + 4;
    if (*p >= '0' && *p <= '9') {
	src_this_line = atoi(p);
	do
	    ++p;
	while (*p >= '0' && *p <= '9');
    }

    if (*p == ':') {
	++p;
	col = atoi(p);
	while (*p >= '0' && *p <= '9')
	    ++p;
    }

    if (*p == ' ')
	++p;

    if (*p != '\0')
	    src_this_file_strcmp = strcmp(p, src_file);
    
    if (src_this_file_strcmp != 0)
	return;

    distance = (src_line > src_this_line
		? src_line - src_this_line
		: 2 * (src_this_line - src_line));	/* favor earlier lines */

    if (distance < best_distance) {	/* found a better line */
	best_distance = distance;
	best_line = src_this_line;
	best_page = src_page;
	max_offset = best_offset = xtell(currinf.pos);
    }
    else if (distance == best_distance) { /* still on a good line, remember diff */
	max_offset = xtell(currinf.pos);
    }
    
    if (distance == 0 && best_distance == 0) {	/* found a better col */
	col_dist = (src_col > col ? src_col - col : 2 * (col - src_col));

	if (col_dist < best_col_dist) {
	    best_col_dist = col_dist;
	    best_page = src_page;
	    max_offset = best_offset = xtell(currinf.pos);
	}
	else if (col_dist == best_col_dist) {
	    max_offset = xtell(currinf.pos);
	}
    }
}

static void
src_spec_fwd_box ARGS((struct geom_info *, long, long, long, long));

static void
src_spec_fwd_special ARGS((struct geom_info *, _Xconst char *));

	/* ARGSUSED */
static void
src_spec_fwd_box(struct geom_info *g_info, long ulx, long uly, long lrx, long lry)
{
    UNUSED(g_info);
    if (src_fwd_active) {
	if (ulx < src_fwd_min_x)
	    src_fwd_min_x = ulx;
	if (uly < src_fwd_min_y)
	    src_fwd_min_y = uly;
	if (lrx > src_fwd_max_x)
	    src_fwd_max_x = lrx;
	if (lry > src_fwd_max_y)
	    src_fwd_max_y = lry;
   }
}

static void
src_spec_fwd_special(struct geom_info *g_info, _Xconst char *str)
{
    long pos;

    if (memcmp(str, "src:", 4) != 0)	/* if not "src:" */
	return;

    pos = xtell(currinf.pos);

    if (pos >= best_offset)
	src_fwd_active = True;

    if (src_fwd_active) {
	if (G_PXL_H < src_fwd_min_x2)
	    src_fwd_min_x2 = G_PXL_H;
	if (G_PXL_H > src_fwd_max_x2)
	    src_fwd_max_x2 = G_PXL_H;
	if (PXL_V < src_fwd_min_y2)
	    src_fwd_min_y2 = PXL_V;
	if (PXL_V > src_fwd_max_y2)
	    src_fwd_max_y2 = PXL_V;

	if (pos > max_offset) {
	    longjmp(g_info->done_env, 1);
	}
    }
}

/*
 *	Routine to actually draw the box.
 */

static void
source_fwd_draw_box(void)
{
    long x, y;

    if (source_fwd_box_page != current_page) {
	source_fwd_box_page = -1;	/* different page---clear it */
    }
    else {
	if (src_fwd_min_x == 0x7fffffff) {
	    /* If no glyphs or rules, use hot point of special instead.  */
	    src_fwd_min_x = src_fwd_min_x2;
	    src_fwd_min_y = src_fwd_min_y2;
	    src_fwd_max_x = src_fwd_max_x2;
	    src_fwd_max_y = src_fwd_max_y2;
	}

#define	PADDING	10	/* instead of PAD, to avoid conflict with t1lib.h */
	x = (src_fwd_min_x - PADDING) / mane.shrinkfactor;
	y = (src_fwd_min_y - PADDING) / mane.shrinkfactor;

	XDrawRectangle(DISP, mane.win, highGC,
		       x - mane_base_x, y - mane_base_y,
		       (src_fwd_max_x + 2 * PADDING) / mane.shrinkfactor - x,
		       (src_fwd_max_y + 2 * PADDING) / mane.shrinkfactor - y);
    }
}

void
source_forward_search(_Xconst char *str)
{
    volatile off_t pos_save = 0;
    struct drawinf currinf_save;
    ubyte maxchar_save;
    struct geom_info g_info;

    int test = dvi_file_changed();
    if (test > 1)
	return;

    TRACE_CLIENT((stderr, "Entering source_forward_search(%s)", str));

    max_offset = 0;
    src_file = str;
    while (*src_file == '0')
	++src_file;
    if (*src_file < '1' || *src_file > '9') {
	fprintf(stderr,	"Badly formatted source special; ignoring:  \"%s\"\n", str);
	return;
    }
    src_line = atoi(src_file);
    while (*src_file >= '0' && *src_file <= '9')
	++src_file;

    src_col = 0;
    if (*src_file == ':') {
	++src_file;
	src_col = atoi(src_file);
	while (*src_file >= '0' && *src_file <= '9')
	    ++src_file;
    }

    if (*src_file == ' ')
	++src_file;

    TRACE_CLIENT((stderr, "File = \"%s\", line = %d, col = %d", src_file, src_line, src_col));

    /* Save status of dvi_file reading (in case we hit an error and resume
       drawing).  */

    if (dvi_pointer_frame != NULL)
	pos_save = lseek(fileno(dvi_file), 0L, SEEK_CUR)
	    - (dvi_pointer_frame->end - dvi_pointer_frame->pos);
    if (page_offset == NULL) {
	dvi_time--;
	return;
    }
    (void)lseek(fileno(dvi_file), page_offset[0], SEEK_SET);
	
    currinf_save = currinf;
    maxchar_save = maxchar;

    bzero((char *)&currinf.data, sizeof currinf.data);
    currinf.tn_table_len = TNTABLELEN;
    currinf.tn_table = tn_table;
    currinf.tn_head = tn_head;
    currinf.pos = currinf.end = dvi_buffer;
    currinf.virtual = NULL;

    /* Start search over pages */

    found_src = False;
    best_distance = best_col_dist = 0xffffffff;
    src_this_line = 0;	/* initialize things that are kept as defaults */
    src_this_file_strcmp = 1;
    if (setjmp(src_env) == 0) {

	/* These two lines do the actual scanning (first pass).  */
	for (src_page = 0; src_page < total_pages; ++src_page)
	    spcl_scan(src_scan_special);

	if (best_distance == 0xffffffff) {
	    if (!found_src) {
		do_popup_message(MSG_WARN,
				 /* helptext */
				 "To enable reverse search, the .tex file has to be compiled with source specials.\
Please see the xdvi man page (section SOURCE SPECIALS) for details.",
				 /* popup */
				 "No source specials in this .dvi file - couldn't perform reverse search");
	    }
	    else {
		do_popup_message(MSG_WARN,
				 /* helptext */
				 "To enable reverse search, the .tex file has to be compiled with source specials.\
Please see the xdvi man page (section SOURCE SPECIALS) for details.",
				 /* popup */
				 "No references to source file \"%s\" in dvi file",
				 src_file);
	    }

	    /* Restore file position.  */
	    maxchar = maxchar_save;
	    currinf = currinf_save;

	    if (dvi_pointer_frame != NULL) {
		(void)lseek(fileno(dvi_file), pos_save, SEEK_SET);
		dvi_pointer_frame->pos = dvi_pointer_frame->end = dvi_buffer;
	    }

	    return;
	}
	TRACE_CLIENT((stderr, "Found close match:  line %d on page %d, offset %lu",
		      best_line, best_page + pageno_correct, best_offset));
    }
    else {
	TRACE_CLIENT((stderr, "Found exact match on page %d, offset %lu",
		   best_page + pageno_correct, best_offset));
    }

    /*
     * In this case we don't need to restore maxchar and currinf, since
     * we won't resume drawing -- we'll jump to a new page instead.
     */

    /* Move to that page.  */

    if (current_page != best_page) {
	current_page = best_page;
	warn_spec_now = warn_spec;
	if (!resource.keep_flag)
	    home(False);
    }
    
    canit = True;
    /* raise and eventually de-iconify window */
    XMapRaised(XtDisplay(top_level), XtWindow(top_level)); 

    /* Now search that particular page.  */

    g_info.geom_box = src_spec_fwd_box;
    g_info.geom_special = src_spec_fwd_special;
    g_info.geom_data = NULL;

    src_fwd_active = False;

    src_fwd_min_x = src_fwd_min_x2 = src_fwd_min_y = src_fwd_min_y2 = 0x7fffffff;
    src_fwd_max_x = src_fwd_max_x2 = src_fwd_max_y = src_fwd_max_y2 = -0x7fffffff;
    
    source_fwd_box_page = -1;	/* in case of error, suppress box */
    
    (void)lseek(fileno(dvi_file), page_offset[best_page], SEEK_SET);
    currinf.tn_table_len = TNTABLELEN;
    currinf.tn_table = tn_table;
    currinf.tn_head = tn_head;
    currinf.pos = currinf.end = dvi_buffer;
    currinf.virtual = NULL;

    if (!setjmp(g_info.done_env))
	geom_scan_part(&g_info, geom_current_frame = &geom_frame0, dimconv);

    if (!src_fwd_active) {
	fprintf(stderr, "%s:%d: geom_scan_part() failed to re-find the special!", __FILE__, __LINE__);
    }
    else {
	source_fwd_box_page = current_page;
    }
}


#ifdef T1LIB
/* ********************** FONT AND ENCODING LOADING ******************** */


static const char *t1_errlist[] = {
    /* -5 */	"Attempted to load a Multiple Master font (unsupported)",
    /* -4 */	"Couldn't open file for reading",
    /* -3 */	"Memory limit of MAXTRIAL exceeded",
    /* -2 */	"Scan error (malformed Type 1 file)",
    /* -1 */	"Encountered end-of-file during parsing",
    /* 0 */	"(No description available)",
    /* 1 */ 	"Path construction error (malformed or corrupted Type 1 file)",
    /* 2 */ 	"Parse error (inconsistent Type 1 font)",
    /* 3 */ 	"Rasterizer aborted (malformed Type 1 file)",
    /* 4 */ 	"(No description available)",
    /* 5 */ 	"(No description available)",
    /* 6 */ 	"(No description available)",
    /* 7 */ 	"(No description available)",
    /* 8 */ 	"(No description available)",
    /* 9 */ 	"(No description available)",
    /* 10 */ 	"Invalid font ID (ID out of range, or library incorrectly initialized)",
    /* 11 */ 	"Invalid parameter in t1lib function call",
    /* 12 */ 	"Operation not permitted",
    /* 13 */ 	"Out of memory",
    /* 14 */ 	"Couldn't open file for reading or writing",
    /* 15 */ 	"Unspecified error (maybe a too old t1lib version?)",
    /* 16 */ 	"Required AFM information not available",
    /* 17 */ 	"X11 library function error",
    /* 18 */ 	"Error with composite char",
};

#define ERR_OFFSET 5 /* T1_ERR* values start at -ERR_OFFSET */

static const char *
T1_strerror(int errnum)
{
    int err_idx = errnum + ERR_OFFSET;
    if (err_idx < 0 || err_idx >= (int)(sizeof(t1_errlist) / sizeof(t1_errlist[0])))
	return "(no details available)";
    return t1_errlist[err_idx];
}

PRIVATE char **
load_vector(char *enc)
{
    char *filename;

    /* First we try to see if there is a xdvi version of the .enc file. */
    filename = kpse_find_file(enc, kpse_program_text_format, 0);
    /* If that fails we try the standard place: postscript headers */
    if (filename == NULL)
      filename = kpse_find_file(enc, kpse_tex_ps_header_format, 1);

    if (filename != NULL) {
	return T1_LoadEncoding(filename);
    }
    return NULL;
}



PRIVATE int
add_tfm(char *texname)
{
    /* Load font metrics if not already loaded.  Return index into
       fontmetrics array.  Can't fail.  Fallback metrics must be
       loaded on faulure.  If that fails abort. */

    int i;

    if (tfminfo_hash.size == 0)
	tfminfo_hash = hash_create(T1FONTS_INITIAL_HASHTABLE_SIZE);

    if (find_str_int_hash(&tfminfo_hash, texname, &i)) {
	return i;
    }

    wlidx++;
    while (wlidx >= maxw) {
	maxw += TEXWGROW;
	tfminfo = xrealloc(tfminfo, sizeof(*tfminfo) * maxw);
    }

    tfminfo[wlidx].texname = texname;
    put_str_int_hash(&tfminfo_hash, tfminfo[wlidx].texname, wlidx);
    
    if (!tfmload(texname, &tfminfo[wlidx].designsize, tfminfo[wlidx].widths)) {
      fprintf(stderr,"Cannot find font metrics file %s.tfm, fallback cmr10.tfm is also missing.\nAborting\n",texname);
      exit(1);
    }

    if (fallbacktfm==1) {
      fallbacktfm = 2;
      do_popup_message(MSG_ERR,
"A font metrics file is missing and cmr10.tfm is used instead.  Either \
you don't have the file, or your TeX system cannot find it.  Try to \
invoke \"kpsewhich\" with the filename mentioned in the error \
message and the \"-debug 2\" option to find out where TeX is \
searching for your files. \n\
If you would like xdvi to automatically create the .tfm file for \
Metafont fonts, you should try to set the environment variable MKTEXTFM to 1 and \
re-start xdvi. See the file texmf.cnf for the default settings of MKTEXTFM and related \
variables.\
",
"Could not find/load font metrics file %s.tfm - using cmr10.tfm.  Expect ugly output.",
		       texname);
    }
    
    return wlidx;
}

PRIVATE int
add_t1font(char *fontname, char *filename)
{
    /* Add t1 font to list, or not if it's already there.  Return the
       t1lib font id. The filename argument is optional, but is assumed
       to be a full path if it is given */

    int id, i, free_it = 0;
    char *path;

    /* Already set up by that name? */
    if (t1fonts_hash.size == 0)
	t1fonts_hash = hash_create(T1FONTS_INITIAL_HASHTABLE_SIZE);
    
    if (find_str_int_hash(&t1fonts_hash, fontname, &i)) {
	return i;
    }

    /* Insert and set up new t1 font */

    t1lidx++;
    while (t1lidx >= maxt1) {
	maxt1 += T1GROW;
	t1fonts = xrealloc(t1fonts, sizeof(*t1fonts) * maxt1);
	TRACE_T1((stderr, "Enlarged t1 table from %d to %d entries",
		  t1lidx, maxt1));
    }

    if (filename == NULL) {
	/* We don't know the full path name yet, find it */
	path = kpse_find_file(fontname, kpse_type1_format, 0);
	if (path == NULL)
	    return -1;	/* xdvi will use substitution font */
	free_it = 1;
    }
    else {
	path = filename;
    }

    TRACE_T1((stderr,"Loaded font %i/%s from %s", t1lidx, fontname, path));

    t1fonts[t1lidx].file = xstrdup(path);
    t1fonts[t1lidx].t1id = id = T1_AddFont(path);
    t1fonts[t1lidx].shortname = xstrdup(fontname);
    t1fonts[t1lidx].loaded = 0;

    /* also save the info in the hashtable (note: no duplication of the string here!) */
    put_str_int_hash(&t1fonts_hash, t1fonts[t1lidx].shortname, t1lidx);
    
    if (free_it)
	free(path);

#if USE_AFM
    /* BZZT! AFM files are useless to us, their metric information is
       too inacurate for TeX use.  Never define USE_AFM */
    /* Set the afm filename.  Always before loading.  If no afm file
       found t1lib will use fallback code.  In fact the fallback code
       has been deactivated entierly, it's dog slow. */
    path = kpse_find_file(fontname, kpse_afm_format, 0);
    if (path != NULL) {
	TRACE_T1((stderr,"found afm file: %s", path));
	T1_SetAfmFileName(id, path);
	free(path);
    }
#endif

    return t1lidx;
}

PRIVATE int
find_texfont(const char *texname)
{
    int i;
    /* Find fontmap index of texfont */
    if (fontmaps_hash.size == 0)
	fontmaps_hash = hash_create(T1FONTS_INITIAL_HASHTABLE_SIZE);
    
    if (find_str_int_hash(&fontmaps_hash, texname, &i)) {
	return i;
    }
    return -1;
}


PRIVATE int
setup_encoded_T1_font(const char *mapfile,
		      int lineno,
		      const char *texname,
		      const char *alias,
		      const char *filename,
		      int enc,
		      int ext,
		      int sl)
{
    /* xdvi T1 Font loading is done in two steps:

       1. At xdvi startup two things happen:

       a. The fontmaps are read.

       b. the dvi file is (partialy) scanned, and fonts are set up.
       In the case of t1 fonts we only set up the data structures,
       they are not actually loaded until they are used.

       2. At the time a T1 font is used it is loaded, encoded, extended
       and slanted as prescribed.

       This procedure takes care of step 1a.  It locates the font and
       sets up the data-structures so it may be assumed, by xdvi, to be
       loaded.  Return the fontmaps array index.

       The 'texname' param should be the texname of the font, such as
       ptmr8r or cmr10.

       The 'alias' param should be the file name of the font if it is
       different from the texname.  This is often the case with fonts
       defined in fontmaps.

       If, for some reason, the full filename of the font has already
       been looked up before we get here it is passed in the filename
       param so we don't have to look it up again.

       Implied encodings are not handled here.

       REMEMBER: THIS PROC IS CALLED FOR EACH FONT IN THE FONTMAPS, WE
       CANNOT DO ANY EXPENSIVE OPERATIONS HERE!!!

     */

    int idx = -1;
    int curr_idx;
    
    /* Already setup by that name? */
    idx = find_texfont(texname);

    assert(idx == -1 || (idx > -1 && idx < g_maplidx));
    
    if (mapfile != NULL && idx != -1) {
	/* font is already set up, and we're scanning the map file: replace existing font with new one */
	curr_idx = idx;
	fprintf(stderr,
		"xdvi: Warning: %s: entry for font \"%s\" on line %d overrides previous entry.\n",
		mapfile, texname, lineno);
	
	free(fontmaps[curr_idx].filename);
	free(fontmaps[curr_idx].pathname);
    }
    else if (idx != -1) {
	/* font is already set up, but we're not scanning the map file - return font index */
	TRACE_T1_VERBOSE((stderr, "font |%s| already set up at index %d", texname, idx));
	return idx;
    }
    else { /* Not set up yet, do it. */
	curr_idx = g_maplidx++;		/* increment global index count */
	TRACE_T1_VERBOSE((stderr, "setting up font |%s| at index %d", texname, curr_idx));
	while (curr_idx >= g_maxmap) {
	    g_maxmap += FNTMAPGROW;
	    fontmaps = xrealloc(fontmaps, sizeof(*fontmaps) * g_maxmap);
	    
	    TRACE_T1((stderr, "Enlarged the fontmap from %d to %d entries",
		      curr_idx, g_maxmap));
	}
	fontmaps[curr_idx].texname = xstrdup(texname);
    }

    if (alias == NULL)
	alias = texname;


    if (idx == -1) {
        /* save into hash */
	if (fontmaps_hash.size == 0)
	    fontmaps_hash = hash_create(T1FONTS_INITIAL_HASHTABLE_SIZE);
	put_str_int_hash(&fontmaps_hash, fontmaps[curr_idx].texname, curr_idx);
    }
    fontmaps[curr_idx].enc = enc;
    fontmaps[curr_idx].extension = ext;
    fontmaps[curr_idx].slant = sl;

    fontmaps[curr_idx].filename = xstrdup(alias);
    fontmaps[curr_idx].t1libid = -1;

    if (filename != NULL) {
	fontmaps[curr_idx].pathname = xstrdup(filename);
    }
    else {
	fontmaps[curr_idx].pathname = NULL;
    }
    fontmaps[curr_idx].tfmidx = -1;
    fontmaps[curr_idx].warned_about = False;
    fontmaps[curr_idx].force_pk = False;

    return curr_idx;
}


static t1font_load_status_t
try_pk_fallback(int idx, struct font *fontp)
{
    Boolean save_t1lib_resource = resource.t1lib;
    Boolean success;

    /* force loading the non-t1 version by setting
       the T1 resource temporarily to false, which will be
       evaluated by load_font(): */
    resource.t1lib = False;
    success = load_font(fontp);
    fontmaps[idx].force_pk = True;
    TRACE_T1((stderr,
	      "setting force_pk for %d to true; success for PK fallback: %d\n",
	      idx, success));
    
    if (!success) {
	/* this is probably serious enough for a GUI warning */
	do_popup_message(MSG_ERR,
			 /* helptext */
			 "You don't seem to have a working installation/configuration of fonts "
			 "for previewing DVI files available. This error only occurs when xdvi has "
			 "tried all of the following possibilities in turn, and all of them have failed:\n\n"
			 "  (1) If the resource t1lib is set, try a Postscript Type1 version of a font.\n\n"
			 "  (2) Otherwise, or if the Type1 version hasn't been found, try to "
			 "locate, or generate via mktexpk, a TeX Pixel (PK) version of the font.\n\n"
			 "  (3) Use the fallback font defined via the \"altfont\" resource (cmr10 by default), "
			 "both as Type1 and as PK version, at various resolutions.\n\n"
			 "Since all of them have failed, xdvi had to leave all characters "
			 "coming from that font blank.",
			 /* errmsg */
			 "Error loading font %s: Neither a Type1 version nor "
			 "a pixel version could be found. The character(s) "
			 "will be left blank.",
			 fontmaps[idx].texname);
	currinf.set_char_p = currinf.fontp->set_char_p = set_empty_char;
	return FAILURE_BLANK;
    }
    /* free unneeded resources. We mustn't free
       fontmaps[idx].texname,
       else find_T1_font will try to load the font all over again.
    */
    free(fontmaps[idx].filename);
    free(fontmaps[idx].pathname);
    resource.t1lib = save_t1lib_resource;
    currinf.set_char_p = currinf.fontp->set_char_p = set_char;
    return FAILURE_PK;
}

PRIVATE t1font_load_status_t
load_font_now(int idx, struct font *fontp)
{
    /* At this point xdvi needs to draw a glyph from this font.  But
       first it must be loaded/copied and modified if needed. */

    int t1idx;	/* The fontmap entry number */
    int t1id;	/* The id of the unmodified font */
    int cid;	/* The id of the copied font */
    int enc, sl, ext;

/*      print_statusline(STATUS_SHORT, "Loading T1 font %s", fontmaps[idx].filename); */
    TRACE_T1((stderr, "adding %s %s",
	      fontmaps[idx].filename, fontmaps[idx].pathname));

    t1idx = add_t1font(fontmaps[idx].filename, fontmaps[idx].pathname);

    t1id = fontmaps[idx].t1libid = t1fonts[t1idx].t1id;

    if (!t1fonts[t1idx].loaded && T1_LoadFont(t1id) == -1) {
	/* this error is somehow better kept at stderr instead of
	   a popup (too annoying) or the statusline (might disappear
	   too fast) ... */
	fprintf(stderr,
		"%s: Could not load Type1 font %s from %s: %s (T1_errno = %d); "
		"will try pixel version instead.\n"
		"Please see the T1lib documentation for details about this.\n",
		prog, fontmaps[idx].texname, fontmaps[idx].pathname, T1_strerror(T1_errno), T1_errno);
	return try_pk_fallback(idx, fontp);
    }

    fontmaps[idx].tfmidx = add_tfm(fontmaps[idx].texname);

    t1fonts[t1idx].loaded = 1;

    /* If there is nothing further to do, just return */
    enc = fontmaps[idx].enc;
    ext = fontmaps[idx].extension;
    sl = fontmaps[idx].slant;

    if (enc == -1 && ext == 0 && sl == 0)
	return SUCCESS;

    /* The fontmap entry speaks of a modified font.  Copy it first. */
    cid = T1_CopyFont(t1id);

    fontmaps[idx].t1libid = cid;

    if (enc != -1) {
	/* Demand load vector */
	if (encodings[enc].vector == NULL)
	    encodings[enc].vector = load_vector(encodings[enc].file);

	if (encodings[enc].vector == NULL) {
	    /* this error is somehow better kept at stderr instead of
	       a popup (too annoying) or the statusline (might disappear
	       too fast) ... */
	    fprintf(stderr,
		    "%s: Could not load load encoding file %s for vector %s (font %s): %s (T1_errno = %d); "
		    "will try pixel version instead.\n"
		    "Please see the T1lib documentation for details about this.\n",
		    prog, encodings[enc].file, encodings[enc].enc, fontmaps[idx].texname,
		    T1_strerror(T1_errno), T1_errno);
	    return try_pk_fallback(idx, fontp);
	}
	else {
	    if (T1_ReencodeFont(cid, encodings[enc].vector) != 0) {
		fprintf(stderr,
			"%s: Re-encoding of %s failed: %s (T1_errno = %d); "
			"will try pixel version instead.\n"
			"Please see the T1lib documentation for details about this.\n",
			prog, fontmaps[idx].texname,
			T1_strerror(T1_errno), T1_errno);
		return try_pk_fallback(idx, fontp);
	    }
	}
    }

    if (ext)
	T1_ExtendFont(cid, ext / 1000.0);

    if (sl)
	T1_SlantFont(cid, sl / 10000.0);

    return SUCCESS;
}


PUBLIC int
find_T1_font(const char *texname)
{
    /* Step 1b in the scenario above.  xdvi knows that this font is
       needed.  Check if it is available.  But do not load it yet.
       Return the fontmap index at which the font was found */

    int idx;	/* Iterator, t1 font id */
    char *filename;
    int fl, el;	/* Fontname length, encoding name length */
    char *mname;	/* Modified font name */

    /* First: Check the maps */
    idx = find_texfont(texname);

    if (idx != -1) {
	if (fontmaps[idx].force_pk) {
	    return idx;
	}
	if (fontmaps[idx].pathname == NULL) {
	    filename = kpse_find_file(fontmaps[idx].filename, kpse_type1_format, 0);
	    /* It should have been on disk according to the map, but never
	       fear, xdvi will try to find it other ways. */
	    if (filename == NULL) {
		if (!fontmaps[idx].warned_about) {
		    fontmaps[idx].warned_about = True;
		    fprintf(stderr,
			    "xdvi: Warning: Font map calls for %s, but it was not found (will try PK version instead).\n",
			    fontmaps[idx].filename);
		}
		return -1;
	    }
	    fontmaps[idx].pathname = filename;
	}
	TRACE_T1((stderr,"find_T1_font map: %s, entered as #%d",texname,idx));
	return idx;
    }

    /* Second: the bare name */
    filename = kpse_find_file(texname, kpse_type1_format, 0);

    if (filename != NULL) {
	idx = setup_encoded_T1_font(NULL, 0, texname, NULL, filename, -1, 0, 0);
	TRACE_T1((stderr, "find_T1_font bare: %s, entered as #%d\n", texname, idx));
	return idx;
    }

    /* Third: Implied encoding? */
    fl = strlen(texname);

    for (idx = 0; idx <= enclidx; idx++) {

	if (encodings[idx].enc == NULL)
	    continue;

	el = strlen(encodings[idx].enc);

	if (strcmp(texname + (fl - el), encodings[idx].enc) == 0) {
	    /* Found a matching encoding */
	    TRACE_T1((stderr, "Encoding match: %s - %s", texname,
		      encodings[idx].enc));

	    mname = malloc(fl + 2);
	    strcpy(mname, texname);
	    mname[fl - el] = '\0';
	    /* If we had 'ptmr8r' the we now look for 'ptmr' */
	    filename = kpse_find_file(mname, kpse_type1_format, 0);
	    if (filename == NULL) {
		/* No? Look for ptmr8a. 8a postfix is oft used on raw fonts */
		strcat(mname, "8a");
		filename = kpse_find_file(mname, kpse_type1_format, 0);
	    }
	    if (filename != NULL) {
		idx = setup_encoded_T1_font(NULL, 0, texname, mname, filename, idx, 0, 0);
		TRACE_T1((stderr, "find_T1_font ienc: %s, is #%d\n", texname, idx));
		return idx;
	    }

	    free(mname);
	    /* Now we have tried everything.  Time to give up. */
	    return -1;

	}	/* If (strcmp...) */
    }	/* for */

    return -1;
}

/* ************************* CONFIG FILE READING ************************ */


PRIVATE int
new_encoding(char *enc, char *file)
{
    /* (Possebly) new encoding entered from .cfg file, or from dvips
       map.  When entered from dvips map the enc is null/anonymous */

    int i;

    /* Shirley!  You're jesting! */
    if (file == NULL)
	return -1;

    if (enc == NULL) {
	/* Enter by file name.  First check if the file has been entered
	   already. */
	for (i = 0; i <= enclidx; i++) {
	    if (strcmp(encodings[i].file, file) == 0)
		return i;
	}
    }
    else {
	/* Enter by encoding name.  Check if already loaded first. */
	for (i = 0; i <= enclidx; i++) {
	    if (encodings[i].enc != NULL && strcmp(encodings[i].enc, enc) == 0)
		return i;
	}
    }

    /* Bonafide new encoding */
    enclidx++;
    while (enclidx >= maxenc) {
	maxenc += ENCGROW;
	encodings = xrealloc(encodings, sizeof(*encodings) * maxenc);
	TRACE_T1((stderr, "Enlarged encoding map from %d to %d entries\n",
		  enclidx, maxenc));
    }

    TRACE_T1((stderr, "New encoding #%d: '%s' -> '%s'", enclidx, enc ? enc : "<none>", file));

    /* The encoding name is optional */
    encodings[enclidx].enc = NULL;
    if (enc != NULL)
	encodings[enclidx].enc = xstrdup(enc);

    /* The file name is required */
    encodings[enclidx].file = xstrdup(file);
    encodings[enclidx].vector = NULL;	/* Demand load later */

    return enclidx;
}


PUBLIC void
add_T1_mapentry(int lineno,
		char *mapfile,
		const char *name,
		char *file,
		char *vec,
		char *spec)
{
    /* This is called from getpsinfo, once for each fontmap line `lineno'.  We
       will dutifully enter the information into our fontmap table */

    static char delim[] = "\t ";
    char *last, *current;
    float number;
    int extend = 0;
    int slant = 0;

    TRACE_T1_VERBOSE((stderr, "%s:%d: %s -> %s Enc: %s Spec: %s",
		      mapfile, lineno, name, file,
		      vec == NULL ? "<none>" : vec,
		      spec == NULL ? "<none>" : spec));

    if (spec != NULL) {
	/* Try to analyze the postscript string.  We recognize two things:
	   "n ExtendFont" and "m SlantFont".  n can be a decimal number in
	   which case it's an extension factor, or a integer, in which
	   case it's a charspace unit? In any case 850 = .85 */

	last = strtok(spec, delim);
	current = strtok(NULL, delim);
	while (current != NULL) {
	    if (strcmp(current, "ExtendFont") == 0) {
		sscanf(last, "%f", &number);
		if (number < 10.0)
		    extend = number * 1000.0;
		else
		    extend = number;

		TRACE_T1((stderr, "EXTEND: %d", extend));
	    }
	    else if (strcmp(current, "SlantFont") == 0) {
		sscanf(last, "%f", &number);
		slant = number * 10000.0;
		TRACE_T1((stderr, "SLANT: %d", slant));
	    }
	    last = current;
	    current = strtok(NULL, delim);
	}
    }

    setup_encoded_T1_font(mapfile, lineno,
			  name, file, NULL, new_encoding(NULL, vec), extend,
			  slant);
}


PRIVATE void
read_cfg_file(char *file)
{
    char *filename;
    FILE *mapfile;
    int len;
    char *keyword;
    char *f;
    char *buffer;
    char *enc;
    char *name;
    int i;
    static _Xconst char delim[] = "\t \n\r";

    if (file == NULL)
	file = "xdvi.cfg";

    filename = kpse_find_file(file, kpse_program_text_format, 1);
    if (filename == NULL) {
	char *path = kpse_path_expand("$XDVIINPUTS");
	do_popup_message(MSG_ERR,
			 "Direct Type 1 (PostScript) font rendering has been disabled. "
			 "You should try to fix this error, since direct "
			 "Type 1 font rendering gives you a lot of benefits, such as:\n"
			 " - quicker startup time, since no bitmap fonts need to be generated;\n"
			 " - saving disk space for storing the bitmap fonts.\n"
			 "To fix this error, check that xdvi.cfg is located somewhere "
			 "in your XDVIINPUTS path. Have a look at the xdvi wrapper shell script "
			 "(type \"which xdvi\" to locate that shell script) for the current setting "
			 "of XDVIINPUTS.",
			 "Could not find config file %s in path \"%s\" - disabling T1lib.",
			 file, path);
	free(path);
	resource.t1lib = False;
	return;
    }

    mapfile = fopen(filename, "r");

    if (mapfile == NULL) {
	fprintf(stderr, "xdvi: Error: Unable to open %s for reading: ",
		filename);
	perror("");
	return;
    }

    TRACE_T1((stderr, "Reading cfg file %s", filename));

    buffer = xmalloc(BUFFER_SIZE);

    while (fgets(buffer, BUFFER_SIZE, mapfile) != NULL) {
	len = strlen(buffer);
	if (buffer[len - 1] != '\n') {
	    int c;
	    /* this really shouldn't happen ... */
	    fprintf(stderr, "%s: Error in config file %s: Skipping line longer than %d characters:\n%s ...\n",
		    prog, filename, BUFFER_SIZE, buffer);
	    /* read until end of this line */
	    while((c = fgetc(mapfile)) != '\n' && c != EOF) { ; }
	    continue;
	}

	keyword = buffer;

	/* Skip leading whitespace */
	while (keyword[0] != '\0' && (keyword[0] == ' ' || keyword[0] == '\t'))
	    keyword++;

	/* % in first column is a correct comment */
	if (keyword[0] == '%' || keyword[0] == '\0' || keyword[0] == '\n')
	    continue;

	keyword = strtok(keyword, delim);

	if (strcmp(keyword, "dvipsmap") == 0) {
	    f = strtok(NULL, delim);
	    
	    TRACE_T1((stderr,"DVIPSMAP: '%s'", f));

	    if (!getpsinfo(f)) {
		/* at least, nag them with a popup so that they'll do something about this ... */
		do_popup_message(MSG_ERR,
				 "Direct Type 1 (PostScript) font rendering has been disabled. "
				 "You should try to fix this error, since direct "
				 "Type 1 font rendering gives you a lot of benefits, such as:\n"
				 " - quicker startup time, since no bitmap fonts need to be generated;\n"
				 " - saving disk space for storing the bitmap fonts.\n"
				 "To fix this error, check that the dvips map file is located somewhere "
				 "in your XDVIINPUTS path. Have a look at the xdvi wrapper shell script "
				 "(type \"which xdvi\" to locate that shell script) for the current setting "
				 "of XDVIINPUTS.",
				 "Could not find dvips map %s - disabling T1lib.",
				 f);
		resource.t1lib = False;
		return;
	    }
	}
	else if (strcmp(keyword, "encmap") == 0) {
	    do_popup_message(MSG_ERR,
			     "Your xdvi.cfg file is for a previous version of xdvik. Please replace "
			     "it by the xdvi.cfg file in the current xdvik distribution.",
			     "Keyword \"encmap\" in xdvi.cfg is no longer supported, "
			     "please update the config file %s.",
			     filename);
	    continue;
	}
	else if (strcmp(keyword, "enc") == 0) {
	    enc = strtok(NULL, delim);
	    name = strtok(NULL, delim);
	    f = strtok(NULL, delim);
	    i = new_encoding(enc, f);
	    TRACE_T1((stderr, "Encoding[%d]: '%s' = '%s' -> '%s'\n", i, enc, name, f));
	} else {
	    /* again, nag them with a popup so that they'll do something about this ... */
	    do_popup_message(MSG_ERR,
			     "Please check the syntax of your config file. "
			     "Valid keywords are: \"enc\" and \"dvipsmap\".",
			     "Skipping unknown keyword \"%s\" in config file %s.",
			     keyword, filename);
	    continue;
	}
    }

    fclose(mapfile);

    free(buffer);
    free(filename);

    return;
}


/* **************************** GLYPH DRAWING *************************** */


/* Set character# ch */

PRIVATE struct glyph *
get_t1_glyph(
#ifdef TEXXET
	     wide_ubyte cmd,
#endif
	     wide_ubyte ch, t1font_load_status_t *flag)
{
    /* 
   A problem is that there are some gaps in math modes tall braces
   '{', (see for example the amstex users guide).  These gaps are
   seen less if a small size factor is applied, 1.03 seems nice.
   This does not seem like the Right Thing though.  Also gaps do
   not show up in the fullsize window.

   All in all the factor has been dropped.  Despite the beauty flaw.
*/

    float size = dvi_pt_conv(currinf.fontp->scale);
      
    int id = currinf.fontp->t1id;
    int t1libid = fontmaps[id].t1libid;

    GLYPH *G;		/* t1lib glyph */
    struct glyph *g;	/* xdvi glyph */

#ifdef TEXXET
    UNUSED(cmd);
#endif
    *flag = SUCCESS;
    
    /* return immediately if we need to use the fallback PK */
    if (fontmaps[id].force_pk)
	return NULL;
    
    TRACE_T1((stderr, "scale: %ld, ppi %d, sf: %d, size: %f",
	      currinf.fontp->scale, pixels_per_inch,
	      currwin.shrinkfactor, size));

    if (t1libid == -1) {
	TRACE_T1((stderr, "trying to load font %d", id));
	*flag = load_font_now(id, currinf.fontp);
	if (*flag < 0) {
	    TRACE_T1((stderr, "load_font_now failed for T1 font %d", id));
	    return NULL;
	}
	t1libid = fontmaps[id].t1libid;
    }

    TRACE_T1((stderr, "Setting '0x%x' of %d, at %ld(%.2fpt), shrinkage is %d",
	      ch, t1libid, currinf.fontp->scale,
	      size, currwin.shrinkfactor));

    /* Check if the glyph already has been rendered */
    if ((g = &currinf.fontp->glyph[ch])->bitmap.bits == NULL) {
	int bitmapbytes;
	int h;
	unsigned char *f;
	unsigned char *t;
	/* Not rendered: Generate xdvi glyph from t1font */
  
	g->addr=1; /* Dummy, should not provoke errors other places in xdvi */
  
	/* Use the widht from the tfm file */
	g->dvi_adv = tfminfo[fontmaps[id].tfmidx].widths[ch] * currinf.fontp->dimconv;
	if (debug & DBG_T1) {
	    fprintf(stderr, "0x%x, dvi_adv = %ld; dimconv: %f; size: %f\n",
		    ch,
		    tfminfo[fontmaps[id].tfmidx].widths[ch],
		    currinf.fontp->dimconv, currinf.fontp->fsize);
	}

	/* Render the glyph.  Size here should be postscript bigpoints */
	G = T1_SetChar(t1libid, ch, size, NULL);
	if (G==NULL) {
	    fprintf(stderr,"T1lib error %d, no glyph rendered\n",T1_errno);
	    assert(G!=NULL);
	}
	if (G->bits==NULL) {
	    /* FIXME: Blank glyph != .notdef. Drop message until I find out how
	       .notdef can be detected */

	    TRACE_T1((stderr, "Character %d not defined in t1-font %s",ch,
		      currinf.fontp->fontname));
	    g->addr=-1;
	    return NULL;
	}

	g->bitmap.w = abs(G->metrics.rightSideBearing - G->metrics.leftSideBearing);
	g->bitmap.h = abs(G->metrics.descent - G->metrics.ascent);
	g->bitmap.bytes_wide = T1PAD(g->bitmap.w,archpad)/8;
	alloc_bitmap(&g->bitmap);

	bitmapbytes = g->bitmap.bytes_wide * g->bitmap.h;

	if (padMismatch) {
	    /* Fix alignment mismatch */
	    int from_bytes = T1PAD(g->bitmap.w,8)/8;

	    f = (unsigned char *)G->bits;
	    t = (unsigned char *)g->bitmap.bits;

	    memset(t, 0, bitmapbytes); /* OR ELSE! you get garbage */

	    for (h = 0 ; h < g->bitmap.h; h++) {
		memcpy(t, f, from_bytes);
		f += from_bytes;
		t += g->bitmap.bytes_wide;
	    }


	} else {
	    /* t1lib and arch alignment matches */
	    memcpy(g->bitmap.bits,G->bits,bitmapbytes);
	}

#ifdef WORDS_BIGENDIAN
	/* xdvi expects the bits to be reversed according to
	   endianness.  t1lib expects no such thing.  This loop reverses
	   the bit order in all the bytes. -janl 18/5/2001 */
	t=g->bitmap.bits;

	for (h = 0; h < bitmapbytes; h++) {
	    *t = rbits[*t];
	    t++;
	}
#endif

	g->x = -G->metrics.leftSideBearing;  /* Oposed x-axis */
	g->y =  G->metrics.ascent;
    }
    return g;
}


#ifdef TEXXET
PUBLIC void
set_t1_char(wide_ubyte cmd, wide_ubyte ch)
#else
PUBLIC long
set_t1_char(wide_ubyte ch)
#endif
{
    t1font_load_status_t flag;
    /* Process events even deepest in the drawing loop */
    if (--event_counter == 0) {
	read_events(False);
    }
    
    /*
      get_t1_glyph() will itself set currinf.fontp->set_char_p
      to set_empty_char() or set_char() if it failed to load the
      T1 version, but for the *current* call of set_t1_char,
      we still need to know whether (1) or (2) holds:
      (1) Type1 font hasn't been found but PK is working,
          use set_char() to set the PK version
      (2) neither Type1 nor PK have been found (this is the
          FAILURE_BLANK case), use set_empty_char().
      So we need the flag to pass this additional information.
     */
#ifdef TEXXET
    (void)get_t1_glyph(cmd, ch, &flag);
    if (flag == FAILURE_BLANK)
	set_empty_char(cmd, ch);
    else
	set_char(cmd, ch);
#else
    (void)get_t1_glyph(ch, &flag);
    if (flag == FAILURE_BLANK)
	return set_empty_char(cmd, ch);
    else
	return set_char(ch);
#endif
}


/* ARGSUSED */
PUBLIC void
read_T1_char(struct font *fontp, wide_ubyte ch)
{
    UNUSED(fontp);
    /* Should never be called */
    fprintf(stderr, "BUG: asked to load %c\n", ch);
}


PUBLIC void
init_t1(void)
{
    int i;
    void *success;

    /* Attempt to set needed padding.

       FIXME: This is not really the required alignment/padding.  On
       ix86 the requirement for int * is 8 bits, on sparc on the other
       hand it's 32 bits.  Even so, some operations will be faster if
       the bitmaps lines are alligned "better".  But on the other hand
       this requires a more complex glyph copying operation... 

       - janl 16/5/2001
    */
    archpad=BMBYTES*8; 
    i = T1_SetBitmapPad(archpad);
    if (i == -1) {
      /* Failed, revert to 8 bits and compilicated bitmap copying */
      padMismatch=1;
      bytepad=8;
      T1_SetBitmapPad(bytepad);
    } else {
      padMismatch=0;
      bytepad=archpad;
    }

    /* Initialize t1lib, use LOGFILE to get logging, NO_LOGFILE otherwise */
    /* Under NO circumstances remove T1_NO_AFM, it slows us down a LOT */
    if (debug & DBG_T1) {
	fprintf(stderr, "Note: additional t1lib messages may be found in \"t1lib.log\".\n");
	success = T1_InitLib(LOGFILE | T1_NO_AFM | IGNORE_CONFIGFILE | IGNORE_FONTDATABASE);
    }
    else
	success = T1_InitLib(NO_LOGFILE | T1_NO_AFM | IGNORE_CONFIGFILE | IGNORE_FONTDATABASE);
    if (success == NULL) {
	fprintf(stderr, "xdvi: Error: Initialization of t1lib failed\n");
	exit(1);
    }
    T1_SetLogLevel(T1LOG_DEBUG);

    
    T1_SetDeviceResolutions(BDPI, BDPI);

    encodings = xmalloc(sizeof(*encodings) * ENCSTART);
    fontmaps = xmalloc(sizeof(*fontmaps) * FNTMAPSTART);
    t1fonts = xmalloc(sizeof(*t1fonts) * T1START);
    tfminfo = xmalloc(sizeof(*tfminfo) * TEXWSTART);

    /* Malloc mem before reading config files! */
    read_cfg_file(NULL);
}

#endif /* T1LIB */
