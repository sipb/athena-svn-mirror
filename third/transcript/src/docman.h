/* docman.h
 *
 * Copyright (C) 1990 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/docman.h,v 1.1.1.1 1996-10-07 20:25:48 ghudson Exp $
 *
 * global defines for the psparse package.
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.3  1994/05/03  23:01:32  snichols
 * Handle null in name of resource, and handle files with neither prolog
 * nor setup comments.
 *
 * Revision 3.2  1994/02/16  00:32:04  snichols
 * support for Orientation comment
 *
 * Revision 3.1  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.0  1991/06/19  20:05:42  snichols
 * Release 3.0
 *
 * Revision 1.5  1991/06/19  20:05:10  snichols
 * increased limits on MAXRESOURCES, added MAXREM.
 *
 * Revision 1.4  1991/02/20  16:34:11  snichols
 * support for new, as yet undefined, resource types.
 *
 * Revision 1.3  91/01/23  16:32:32  snichols
 * Added support for landscape, cleaned up some defaults, and
 * handled *PageRegion features better.
 * 
 * Revision 1.2  90/12/12  09:56:23  snichols
 * added ranges to resource data structure, and added feature data
 * structure.
 * 
 *
 */



#define MAXRESOURCES 1000
#define MAXREM 10000

#define VERS1 10
#define VERS2 20
#define VERS21 21
#define VERS3 30

#define FONT 0
#define RFILE 1
#define PROCSET 2
#define UNKNOWN -1
#define MAXRESTYPES 20

char resmap[MAXRESTYPES][30] = {
    "font", "file", "procset", "pattern", "form", "encoding", "", "", "",
    "", "", "", "", "", "", "", "", "", "", ""};


#define HEADCOM 10


enum comtypes {
    docfont, docneedfont, docsupfont, docneedres, docsupres,
    docneedfile, docsupfile, docproc, docneedproc, docsupproc,
    pagefont, pageres, incfont, incres, begfont, endfont,
    begres, endres, pagefile, incfile, begfile, endfile, 
    incproc, begproc, endproc, page, begdoc, enddoc, trailer,
    incfeature, begfeature, endfeature, begbin, endbin,
    begdata, enddata, begsetup, endsetup, begprolog, endprolog, endcomment,
    pageorder, langlevel, begdef, enddef, orient, pages, invalid
};

enum resstates {
    inprinter, download, verify, indocument, future, nowhere
};

enum rangestates {
    header, defaults, pageinrange, undefined
};

struct resource {
    int type;			/* what type of resource */
    char name[255];		/* name of resource */
    char path[255];		/* where to find it */
    float version;		/* only used for procsets */
    int rev;			/* only used for procsets */
    enum resstates state;	/* from resstates, above */
    enum comtypes comment;	/* what kind of comment did we find it in */
    enum rangestates rangeinfo;	/* from rangestates, above */
    int  inrange;		/* is it in the range we're interested in? */
    long minvm;			/* min vm consumed (fonts) */
    long maxvm;			/* max vm consumed (fonts)  */
};

struct dtable {
    long location;
    int type;
    int index;
    enum comtypes comment;
};

struct feature {
    long location;
    char *code;
    char name[255];
    char option[255];
};

char *standardfonts[35] = {
    "AvantGarde-Book", "AvantGarde-BookOblique", "AvantGarde-Demi", 
    "AvantGarde-DemiOblique", "Bookman-Demi", "Bookman-DemiItalic",
    "Bookman-Light", "Bookman-LightItalic", "Courier", "Courier-Bold",
    "Courier-BoldOblique", "Courier-Oblique", "Helvetica", "Helvetica-Bold",
    "Helvetica-BoldOblique", "Helvetica-Narrow", "Helvetica-Narrow-Bold",
    "Helvetica-Narrow-BoldOblique", "Helvetica-Narrow-Oblique", "Helvetica-Oblique",
    "NewCenturySchlbk-Bold", "NewCenturySchlbk-BoldItalic",
    "NewCenturySchlbk-Italic", "NewCenturySchlbk-Roman", "Palatino-Bold",
    "Palatino-BoldItalic", "Palatino-Italic", "Palatino-Roman", "Symbol",
    "Times-Bold", "Times-BoldItalic", "Times-Italic", "Times-Roman",
    "ZapfChancery-MediumItalic", "ZapfDingbats"
};
