/*
 * Copyright © 2002, 2003 Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sun Microsystems, Inc. nor the names of 
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind.
 *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES OR
 * LIABILITIES SUFFERED BY LICENSEE AS A RESULT OF OR RELATING TO USE,
 * MODIFICATION OR DISTRIBUTION OF THE SOFTWARE OR ITS DERIVATIVES.
 * IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE,
 * PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE
 * THEORY OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE
 * SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 */

/* $Id: sft.h,v 1.1.1.2 2005-03-10 22:11:35 ghudson Exp $ */
/* @(#)sft.h 1.17 03/01/08 SMI */

/*
 * @file sft.h
 * @brief Sun Font Tools
 * @author Alexander Gelfenbain <adg@sun.com>
 * @version 1.0
 */

/*
 *        If NO_MAPPERS is defined, MapChar() and MapString() and consequently GetTTSimpleCharMetrics()
 *        don't get compiled in. This is done to avoid including a large chunk of code (TranslateXY() from
 *        xlat.c in the projects that don't require it.
 *
 *        If NO_TYPE3 is defined CreateT3FromTTGlyphs() does not get compiled in.
 *        If NO_TYPE42 is defined Type42-related code is excluded
 *        If NO_TTCR is defined TrueType creation related code is excluded\
 *        If NO_LIST is defined list.h and piblic functions that use it don't get compiled
 *        If USE_GSUB is *not* defined Philipp's GSUB code does not get included
 *
 *        When STSF is defined several data types are defined elsewhere
 */

/*
 *        Generated fonts contain an XUID entry in the form of:
 *
 *                  103 0 T C1 N C2 C3
 *
 *        103 - Sun's Adobe assigned XUID number. Contact person: Alexander Gelfenbain <gelf@eng.sun.com>
 *
 *        T  - font type. 0: Type 3, 1: Type 42
 *        C1 - CRC-32 of the entire source TrueType font
 *        N  - number of glyphs in the subset
 *        C2 - CRC-32 of the array of glyph IDs used to generate the subset
 *        C3 - CRC-32 of the array of encoding numbers used to generate the subset
 *
 */
 

#ifndef __SUBFONT_H
#define __SUBFONT_H

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#ifndef NO_LIST
#include "list.h"
#endif

#ifdef STSF
#include <sttypes.h>
#endif
#include "config.h"
#include "../gnome-print-private.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0	/* Use glib's G_BYTE_ORDER == G_BIG_ENDIAN instead */
#ifdef __sparc
#ifndef G_BIG_ENDIAN
#define G_BIG_ENDIAN
#endif
#endif

#if defined(__powerpc__) || defined(POWERPC)
#ifndef G_BIG_ENDIAN
#define G_BIG_ENDIAN
#endif
#endif

#ifdef __i386
#ifndef G_LITTLE_ENDIAN
#define G_LITTLE_ENDIAN
#endif
#endif

#ifdef __mips
#ifndef G_BIG_ENDIAN
#define G_BIG_ENDIAN
#endif
#endif

#ifdef __BIG_ENDIAN__
#define G_BIG_ENDIAN
#endif

#ifdef __LITTLE_ENDIAN__
#define G_LITTLE_ENDIAN
#endif

#if !defined(G_BIG_ENDIAN) && !defined(G_LITTLE_ENDIAN)
#error "Either G_BIG_ENDIAN or G_LITTLE_ENDIAN should be defined."
#endif

#if defined(G_BIG_ENDIAN) && defined(G_LITTLE_ENDIAN)
#error "This is bizarre"
#endif
#endif

#if 0  /* These should be defined in the makefile */
#define DEBUG      /* Generate debugging output */
#define DEBUG2     /* More detailed debugging output */
#define DEBUG3     /* Dump of TrueType outlines */
#endif

    


/*@{*/
#define false 0               /**< standard false value */
#define true  1               /**< standard true value */
/*@}*/

#ifndef STSF
#if 1
/* glib already deals with different compilers */
#include <glibconfig.h>

typedef guchar     byte;
typedef guchar     uint8;
typedef gchar      int8;
typedef guint16    uint16;
typedef gint16     int16;

typedef guint32    uint32;
typedef gint32     int32;
typedef guint64    uint64;
typedef gint64     int64;
#  else
/*- XXX These should be dynamically configured */ /*FOLD00*/
    typedef unsigned char         byte;
    typedef unsigned char         uint8;
    typedef signed char           int8;
    typedef unsigned short int    uint16;
    typedef short int             int16;

    typedef unsigned int          uint32;
    typedef int                   int32;
    typedef unsigned long long    uint64;
    typedef long long             int64;
#  endif
/*@{*/
    typedef int16       F2Dot14;            /**< fixed: 2.14 */
    typedef int32       F16Dot16;           /**< fixed: 16.16 */
/*@}*/

    typedef struct {
        uint16 s;
        uint16 d;
    } uint16pair;
#endif

/** Return value of OpenTTFont() and CreateT3FromTTGlyphs() */
    enum SFErrCodes {
        SF_OK,                              /**< no error                                     */
        SF_BADFILE,                         /**< file not found                               */
        SF_FILEIO,                          /**< file I/O error                               */
        SF_MEMORY,                          /**< memory allocation error                      */
        SF_GLYPHNUM,                        /**< incorrect number of glyphs                   */
        SF_BADARG,                          /**< incorrect arguments                          */
        SF_TTFORMAT,                        /**< incorrect TrueType font format               */
        SF_TABLEFORMAT,                     /**< incorrect format of a TrueType table         */
        SF_FONTNO                           /**< incorrect logical font number of a TTC font  */
    };

#ifndef FW_THIN /* WIN32 compilation would conflict */
/** Value of the weight member of the TTGlobalFontInfo struct */
    enum WeightClass {
        FW_THIN = 100,                      /**< Thin                               */
        FW_EXTRALIGHT = 200,                /**< Extra-light (Ultra-light)          */
        FW_LIGHT = 300,                     /**< Light                              */
        FW_NORMAL = 400,                    /**< Normal (Regular)                   */
        FW_MEDIUM = 500,                    /**< Medium                             */
        FW_SEMIBOLD = 600,                  /**< Semi-bold (Demi-bold)              */
        FW_BOLD = 700,                      /**< Bold                               */
        FW_EXTRABOLD = 800,                 /**< Extra-bold (Ultra-bold)            */
        FW_BLACK = 900                      /**< Black (Heavy)                      */
    };

/** Value of the width member of the TTGlobalFontInfo struct */
    enum WidthClass {
        FWIDTH_ULTRA_CONDENSED = 1,         /**< 50% of normal                      */
        FWIDTH_EXTRA_CONDENSED = 2,         /**< 62.5% of normal                    */
        FWIDTH_CONDENSED = 3,               /**< 75% of normal                      */
        FWIDTH_SEMI_CONDENSED = 4,          /**< 87.5% of normal                    */
        FWIDTH_NORMAL = 5,                  /**< Medium, 100%                       */
        FWIDTH_SEMI_EXPANDED = 6,           /**< 112.5% of normal                   */
        FWIDTH_EXPANDED = 7,                /**< 125% of normal                     */
        FWIDTH_EXTRA_EXPANDED = 8,          /**< 150% of normal                     */
        FWIDTH_ULTRA_EXPANDED = 9           /**< 200% of normal                     */
    };
#endif /* FW_THIN */

/** Type of the 'kern' table, stored in _TrueTypeFont::kerntype */
    enum KernType {

        KT_NONE         = 0,                /**< no kern table                      */
        KT_APPLE_NEW    = 1,                /**< new Apple kern table               */
        KT_MICROSOFT    = 2                 /**< Microsoft table                    */
    };

/* Composite glyph flags definition */
    enum CompositeFlags {
        ARG_1_AND_2_ARE_WORDS     = 1,
        ARGS_ARE_XY_VALUES        = 1<<1,
        ROUND_XY_TO_GRID          = 1<<2,
        WE_HAVE_A_SCALE           = 1<<3,
        MORE_COMPONENTS           = 1<<5,
        WE_HAVE_AN_X_AND_Y_SCALE  = 1<<6,
        WE_HAVE_A_TWO_BY_TWO      = 1<<7,
        WE_HAVE_INSTRUCTIONS      = 1<<8,
        USE_MY_METRICS            = 1<<9,
        OVERLAP_COMPOUND          = 1<<10
    };

#ifndef NO_TTCR
/** Flags for TrueType generation */
    enum TTCreationFlags {
        TTCF_AutoName = 1,                  /**< Automatically generate a compact 'name' table.
                                               If this flag is not set, name table is generated
                                               either from an array of NameRecord structs passed as
                                               arguments or if the array is NULL, 'name' table
                                               of the generated TrueType file will be a copy
                                               of the name table of the original file.
                                               If this flag is set the array of NameRecord structs
                                               is ignored and a very compact 'name' table is automatically
                                               generated. */

        TTCF_IncludeOS2 = 2                 /** If this flag is set OS/2 table from the original font will be
                                                copied to the subset */
    };
#endif

    /** Structure used by GetTTGlyphMetrics() */
    /*- In horisontal writing mode right sidebearing is calculated using this formula
     *- rsb = aw - (lsb + xMax - xMin) -*/
     typedef struct {
         int16  xMin;
         int16  yMin;
         int16  xMax;
         int16  yMax;
         uint16 aw;                /*- Advance Width (horisontal writing mode)    */
         int16  lsb;               /*- Left sidebearing (horisontal writing mode) */
         uint16 ah;                /*- advance height (vertical writing mode)     */
         int16  tsb;               /*- top sidebearing (vertical writing mode)    */
     } TTGlyphMetrics;


    /** Structure used by GetTTSimpleGlyphMetrics() and GetTTSimpleCharMetrics() functions */
    typedef struct {
        uint16 adv;                         /**< advance width or height            */
        int16 sb;                           /**< left or top sidebearing            */
    } TTSimpleGlyphMetrics;

    /** Structure returned by ReadGlyphMetrics() */
    typedef struct {
        uint16 aw, ah;
        int16 lsb, tsb;
    } TTFullSimpleGlyphMetrics;


/** Structure used by the TrueType Creator and GetRawGlyphData() */

    typedef struct {
        uint32 glyphID;                     /**< glyph ID                           */
        uint16 nbytes;                      /**< number of bytes in glyph data      */
        byte  *ptr;                         /**< pointer to glyph data              */
        uint16 aw;                          /**< advance width                      */
        int16  lsb;                         /**< left sidebearing                   */
        uint16 compflag;                    /**< 0- if non-composite, 1- otherwise  */
        uint16 npoints;                     /**< number of points                   */
        uint16 ncontours;                   /**< number of contours                 */
        /* */
        uint32 newID;                       /**< used internally by the TTCR        */
    } GlyphData;


#ifndef STSF
    /* STSF defines NameRecord and FUnitBBox structures in its own include file sttypes.h */

    typedef struct {
        int16 xMin;
        int16 yMin;
        int16 xMax;
        int16 yMax;
    } FUnitBBox;

/** Structure used by the TrueType Creator and CreateTTFromTTGlyphs() */
    typedef struct {
        uint16 platformID;                  /**< Platform ID                                            */
        uint16 encodingID;                  /**< Platform-specific encoding ID                          */
        uint16 languageID;                  /**< Language ID                                            */
        uint16 nameID;                      /**< Name ID                                                */
        uint16 slen;                        /**< String length in bytes                                 */
        byte  *sptr;                        /**< Pointer to string data (not zero-terminated!)          */
    } NameRecord;
#endif



/** Return value of GetTTGlobalFontInfo() */

    typedef struct {
        char *family;             /**< family name                                             */
        uint16 *ufamily;		  /**< family name UCS2                                         */
        char *subfamily;          /**< subfamily name                                          */
        char *psname;             /**< PostScript name                                         */
        int   weight;             /**< value of WeightClass or 0 if can't be determined        */
        int   width;              /**< value of WidthClass or 0 if can't be determined         */
        int   pitch;              /**< 0: proportianal font, otherwise: monospaced             */
        int   italicAngle;        /**< in counter-clockwise degrees * 65536                    */
        uint16 fsSelection;       /**< fsSelection field of OS/2 table                         */
        int   xMin;               /**< global bounding box: xMin                               */
        int   yMin;               /**< global bounding box: yMin                               */
        int   xMax;               /**< global bounding box: xMax                               */
        int   yMax;               /**< global bounding box: yMax                               */
        int   ascender;           /**< typographic ascent.                                     */
        int   descender;          /**< typographic descent.                                    */
        int   linegap;            /**< typographic line gap.\ Negative values are treated as
                                     zero in Win 3.1, System 6 and System 7.                 */
        int   vascent;            /**< typographic ascent for vertical writing mode            */
        int   vdescent;           /**< typographic descent for vertical writing mode           */
        int   typoAscender;       /**< OS/2 portable typographic ascender                      */
        int   typoDescender;      /**< OS/2 portable typographic descender                     */
        int   typoLineGap;        /**< OS/2 portable typographc line gap                       */
        int   winAscent;          /**< ascender metric for Windows                             */
        int   winDescent;         /**< descender metric for Windows                            */
        int   symbolEncoded;      /**< 1: MS symbol encoded 0: not symbol encoded              */
        int   rangeFlag;          /**< if set to 1 Unicode Range flags are applicable          */
        uint32 ur1;               /**< bits 0 - 31 of Unicode Range flags                      */
        uint32 ur2;               /**< bits 32 - 63 of Unicode Range flags                     */
        uint32 ur3;               /**< bits 64 - 95 of Unicode Range flags                     */
        uint32 ur4;               /**< bits 96 - 127 of Unicode Range flags                    */
        byte   panose[10];        /**< PANOSE classification number                            */
        uint16 typeFlags;		  /**< type flags (copyright information)                      */
    } TTGlobalFontInfo;

/** Structure used by KernGlyphs()      */
    typedef struct {
        int x;                    /**< positive: right, negative: left                        */
        int y;                    /**< positive: up, negative: down                           */
    } KernData;


/** ControlPoint structure used by GetTTGlyphPoints() */
    typedef struct {
        uint32 flags;             /**< 00000000 00000000 e0000000 bbbbbbbb */
        /**< b - byte flags from the glyf array  */
        /**< e == 0 - regular point              */
        /**< e == 1 - end contour                */
        int16 x;                  /**< X coordinate in EmSquare units      */
        int16 y;                  /**< Y coordinate in EmSquare units      */
    } ControlPoint;

    typedef struct _TrueTypeFont TrueTypeFont;

/*
 * @defgroup sft Sun Font Tools Exported Functions
 */


/*
 * Get the number of fonts contained in a TrueType collection
 * @param  fname - file name
 * @return number of fonts or zero, if file is not a TTC file.
 * @ingroup sft
 */
    int CountTTCFonts(const char* fname);


/*
 * TrueTypeFont constructor. 
 * Reads the font file and allocates the memory for the structure.
 * @param  facenum - logical font number within a TTC file. This value is ignored
 *                   for TrueType fonts
 * @return value of SFErrCodes enum
 * @ingroup sft
 */
    int  OpenTTFont(const char *fname, uint32 facenum, TrueTypeFont**);

/*
 * TrueTypeFont destructor. Deallocates the memory.
 * @ingroup sft
 */
    void CloseTTFont(TrueTypeFont *);

/*
 * Extracts TrueType control points, and stores them in an allocated array pointed to
 * by *pointArray. This function returns the number of extracted points.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 * @param pointArray  Return value - address of the pointer to the first element of the array
 *                    of points allocated by the function
 * @return            Returns the number of points in *pointArray or -1 if glyphID is
 *                    invalid.
 * @ingroup sft
 *
 */
int GetTTGlyphPoints(TrueTypeFont *ttf, uint32 glyphID, ControlPoint **pointArray);

/*
 * Extracts bounding boxes in normalized FUnits (1000/em) for all glyphs in the
 * font and allocates an array of FUnitBBox structures. There are ttf->nglyphs
 * elements in the array.
 *
 * @param ttf     pointer to the TrueTypeFont structure
 *
 * @return        an array of FUnitBBox structures with values normalized to 1000 UPEm
 *
 * @ingroup sft
 */
FUnitBBox *GetTTGlyphBoundingBoxes(TrueTypeFont *ttf);

/*
 * Extracts raw glyph data from the 'glyf' table and returns it in an allocated
 * GlyphData structure.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 *
 * @return            pointer to an allocated GlyphData structure or NULL if
 *                    glyphID is not present in the font
 * @ingroup sft
 *
 */
    GlyphData *GetTTRawGlyphData(TrueTypeFont *ttf, uint32 glyphID);

#ifndef NO_LIST
/*
 * For a specified glyph adds all component glyphs IDs to the list and
 * return their number. If the glyph is a single glyph it has one component
 * glyph (which is added to the list) and the function returns 1.
 * For a composite glyphs it returns the number of component glyphs
 * and adds all of them to the list.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphID     Glyph ID
 * @param glyphlist   list of glyphs
 *
 * @return            number of component glyphs
 * @ingroup sft
 *
 */
    int GetTTGlyphComponents(TrueTypeFont *ttf, uint32 glyphID, list glyphlist);
#endif

/*
 * Extracts all Name Records from the font and stores them in an allocated
 * array of NameRecord structs
 *
 * @param ttf       pointer to the TrueTypeFont struct
 * @param nr        pointer to the array of NameRecord structs
 *
 * @return          number of NameRecord structs
 * @ingroup sft
 */

    int GetTTNameRecords(TrueTypeFont *ttf, NameRecord **nr);

/*
 * Deallocates previously allocated array of NameRecords.
 *
 * @param nr        array of NameRecord structs
 * @param n         number of elements in the array
 *
 * @ingroup sft
 */
    void DisposeNameRecords(NameRecord* nr, int n);


#ifndef NO_TYPE3
/*
 * Generates a new PostScript Type 3 font and dumps it to <b>outf</b> file.
 * This functions subsititues glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param outf        the resulting font is written to this stream
 * @param fname       font name for the new font. If it is NULL the PostScript name of the
 *                    original font will be used
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf
 * @param encoding    array of encoding values. encoding[i] specifies the position of the glyph
 *                    glyphArray[i] in the encoding vector of the resulting Type3 font
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param wmode       writing mode for the output file: 0 - horizontal, 1 - vertical
 * @return            return the value of SFErrCodes enum
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    int  CreateT3FromTTGlyphs(TrueTypeFont *ttf, FILE *outf, const char *fname, uint16 *glyphArray, byte *encoding, int nGlyphs, int wmode);
#endif

#ifndef NO_TTCR
/*
 * Generates a new TrueType font and dumps it to <b>outf</b> file.
 * This functions subsititues glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param fname       file name for the output TrueType font file
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf. The first
 *                    element of this array has to be glyph 0 (default glyph)
 * @param encoding    array of encoding values. encoding[i] specifies character code for
 *                    the glyphID glyphArray[i]. Character code 0 usually points to a default
 *                    glyph (glyphID 0)
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param nNameRecs   number of NameRecords for the font, if 0 the name table from the
 *                    original font will be used
 * @param nr          array of NameRecords
 * @param flags       or'ed TTCreationFlags
 * @return            return the value of SFErrCodes enum
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    int  CreateTTFromTTGlyphs(TrueTypeFont  *ttf,
                              const char    *fname,
                              uint16        *glyphArray,
                              byte          *encoding,
                              int            nGlyphs,
                              int            nNameRecs,
                              NameRecord    *nr,
                              uint32        flags);
#endif

#ifndef NO_TYPE42
/*
 * Generates a new PostScript Type42 font and dumps it to <b>outf</b> file.
 * This functions subsititues glyph 0 for all glyphIDs that are not found in the font.
 * @param ttf         pointer to the TrueTypeFont structure
 * @param outf        output stream for a resulting font
 * @param psname      PostScript name of the resulting font
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf. The first
 *                    element of this array has to be glyph 0 (default glyph)
 * @param encoding    array of encoding values. encoding[i] specifies character code for
 *                    the glyphID glyphArray[i]. Character code 0 usually points to a default
 *                    glyph (glyphID 0)
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @return            SF_OK - no errors
 *                    SF_GLYPHNUM - too many glyphs (> 255)
 *                    SF_TTFORMAT - corrupted TrueType fonts
 *
 * @see               SFErrCodes
 * @ingroup sft
 *
 */
    int  CreateT42FromTTGlyphs(TrueTypeFont  *ttf,
                               FILE          *outf,
                               const char    *psname,
                               uint16        *glyphArray,
                               byte          *encoding,
                               int            nGlyphs);
#endif

/*
 * Queries full glyph metrics for one glyph
 */
void GetTTGlyphMetrics(TrueTypeFont *ttf, uint32 glyphID, TTGlyphMetrics *metrics);


/*
 * Queries glyph metrics. Allocates an array of TTSimpleGlyphMetrics structs and returns it.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param glyphArray  pointer to an array of glyphs that are to be extracted from ttf
 * @param nGlyphs     number of glyph IDs in glyphArray and encoding values in encoding
 * @param mode        writing mode: 0 - horizontal, 1 - vertical
 * @ingroup sft
 *
 */
TTSimpleGlyphMetrics *GetTTSimpleGlyphMetrics(TrueTypeFont *ttf, uint16 *glyphArray, int nGlyphs, int mode);

#ifndef NO_MAPPERS
/*
 * Queries character metrics. Allocates an array of TTSimpleGlyphMetrics structs and returns it.
 * This function behaves just like GetTTSimpleGlyphMetrics() but it takes a range of Unicode
 * characters instead of an array of glyphs.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param firstChar   Unicode value of the first character in the range
 * @param nChars      number of Unicode characters in the range
 * @param mode        writing mode: 0 - horizontal, 1 - vertical
 *
 * @see GetTTSimpleGlyphMetrics
 * @ingroup sft
 *
 */
    TTSimpleGlyphMetrics *GetTTSimpleCharMetrics(TrueTypeFont *ttf, uint16 firstChar, int nChars, int mode);

/*
 * Maps a Unicode (UCS-2) string to a glyph array. Returns the number of glyphs in the array,
 * which for TrueType fonts is always the same as the number of input characters.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param str         pointer to a UCS-2 string
 * @param nchars      number of characters in <b>str</b>
 * @param glyphArray  pointer to the glyph array where glyph IDs are to be recorded.
 *
 * @return MapString() returns -1 if the TrueType font has no usable 'cmap' tables.
 *         Otherwise it returns the number of characters processed: <b>nChars</b>
 *
 * glyphIDs of TrueType fonts are 2 byte positive numbers. glyphID of 0 denotes a missing
 * glyph and traditionally defaults to an empty square.
 * glyphArray should be at least sizeof(uint16) * nchars bytes long. If glyphArray is NULL
 * MapString() replaces the UCS-2 characters in str with glyphIDs.
 * @ingroup sft
 */
#ifdef USE_GSUB
    int MapString(TrueTypeFont *ttf, uint16 *str, int nchars, uint16 *glyphArray, int bvertical);
#else
    int MapString(TrueTypeFont *ttf, uint16 *str, int nchars, uint16 *glyphArray);
#endif

/*
 * Maps a Unicode (UCS-2) character to a glyph ID and returns it. Missing glyph has
 * a glyphID of 0 so this function can be used to test if a character is encoded in the font.
 *
 * @param ttf         pointer to the TrueTypeFont structure
 * @param ch          Unicode (UCS-2) character
 * @return glyph ID, if the character is missing in the font, the return value is 0.
 * @ingroup sft
 */
#ifdef USE_GSUB
    uint16 MapChar(TrueTypeFont *ttf, uint16 ch, int bvertical);
#else
    uint16 MapChar(TrueTypeFont *ttf, uint16 ch);
#endif
#endif

/*
 * Returns global font information about the TrueType font.
 * @see TTGlobalFontInfo
 *
 * @param ttf         pointer to a TrueTypeFont structure
 * @param info        pointer to a TTGlobalFontInfo structure
 * @ingroup sft
 *
 */
    void GetTTGlobalFontInfo(TrueTypeFont *ttf, TTGlobalFontInfo *info);

/*
 * Returns kerning information for an array of glyphs.
 * Kerning is not cumulative.
 * kern[i] contains kerning information for a pair of glyphs at positions i and i+1
 *
 * @param ttf         pointer to a TrueTypeFont structure
 * @param glyphs      array of source glyphs
 * @param nglyphs     number of glyphs in the array
 * @param wmode       writing mode: 0 - horizontal, 1 - vertical
 * @param kern        array of KernData structures. It should contain nglyphs-1 elements
 * @see KernData
 * @ingroup sft
 *
 */
void KernGlyphs(TrueTypeFont *ttf, uint16 *glyphs, int nglyphs, int wmode, KernData *kern);

/*
 * Returns nonzero if font is a symbol encoded font
 */
int CheckSymbolEncoding(TrueTypeFont* ttf);

/*
 * Extracts a 'cmap' table from a font, allocates memory for it and returns a pointer to it.
 * DEPRECATED - use ExtractTable instead
 */
#if 0
byte *ExtractCmap(TrueTypeFont *ttf);
#endif

/*
 * Extracts a table from a font, allocates memort for it and returns a pointer to it.
 */
byte *ExtractTable(TrueTypeFont *ttf, uint32 tag);

/*
 * Returns a pointer to the table but does not allocate memory for it.
 */
const byte *GetTable(TrueTypeFont *ttf, uint32 tag);

/*
 * Functions that do not use TrueTypeFont structure
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/*
 * Reads full (vertical and horisontal) glyph metrics for an array of glyphs from hmtx and vmtx tables
 *
 * @param hmtx       TrueType hmtx table
 * @param vmtx       TrueType vmtx table
 * @param hcount     numberOfHMetrics value
 * @param vcount     numOfLongVerMetrics value
 * @param gcount     total number of glyphs in the font
 * @param UPEm       units per Em value
 * @param glyphArray array of source glyph IDs
 * @param nGlyphs    number of glyphs in the glyphArray array
 *
 * @return           array of TTFullSimpleGlyphMetrics data structures
 *
 */

TTFullSimpleGlyphMetrics *ReadGlyphMetrics(byte *hmtx, byte *vmtx, int hcount, int vcount, int gcount, int UPEm, uint16 *glyphArray, int nGlyphs);

void ReadSingleGlyphMetrics(byte *hmtx, byte *vmtx, int hcount, int vcount, int gcount, int UPEm, uint16 glyphID, TTFullSimpleGlyphMetrics *metrics);


/*
 * Returns the length of the 'kern' subtable
 *
 * @param kern       pointer to the 'kern' subtable
 *
 * @return           number of bytes in it
 */
uint32 GetKernSubtableLength(byte *kern);

/*
 *  Kerns a pair of glyphs.
 *
 * @param kerntype   type of the kern table
 * @param nkern      number of kern subtables
 * @param kern       array of pointers to kern subtables
 * @pram unitsPerEm  units per Em value
 * @param wmode      writing mode: 0 - horizontal, 1 - vertical
 * @param a          ID of the first glyoh
 * @param b          ID of the second glyoh
 * @param x          X-axis kerning value is returned here
 * @param y          Y-axis kerning value is returned here
 */
void KernGlyphPair(int kerntype, uint32 nkern, byte **kern, int unitsPerEm, int wmode, uint32 a, uint32 b, int *x, int *y);



/*- private definitions */ /*FOLD00*/

struct _TrueTypeFont {
    uint32 tag;

    char   *fname;
    off_t  fsize;
    byte   *ptr;

    char   *psname;
    char   *family;
    uint16  *ufamily;
    char   *subfamily;

    uint32 ntables;
    uint32 tdoffset;                              /* offset to the table directory (!= 0 for TrueType collections)      */
    uint32 *goffsets;
    int    nglyphs;
    int    unitsPerEm;
    int    numberOfHMetrics;
    int    numOfLongVerMetrics;                   /* if this number is not 0, font has vertical metrics information */
    byte   *cmap;
    int    cmapType;
    uint16 (*mapper)(const byte *, uint16);       /* character to glyphID translation function                          */
    void   **tables;                              /* array of pointers to tables                                        */
    uint32 *tlens;                                /* array of table lengths                                             */
    int    kerntype;                              /* Defined in the KernType enum                                       */
    uint32 nkern;                                 /* number of kern subtables                                           */
    byte   **kerntables;                          /* array of pointers to kern subtables                                */
#ifdef USE_GSUB
    void   *pGSubstitution;                       /* info provided by GSUB for UseGSUB()                                */
#endif
    GnomePrintBuffer gp_buf;
};

#ifdef __cplusplus
}
#endif

#endif /* __SUBFONT_H */
