
/*  A Bison parser, made from sieve.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NUMBER	257
#define	STRING	258
#define	IF	259
#define	ELSIF	260
#define	ELSE	261
#define	REJCT	262
#define	FILEINTO	263
#define	REDIRECT	264
#define	KEEP	265
#define	STOP	266
#define	DISCARD	267
#define	VACATION	268
#define	REQUIRE	269
#define	SETFLAG	270
#define	ADDFLAG	271
#define	REMOVEFLAG	272
#define	MARK	273
#define	UNMARK	274
#define	NOTIFY	275
#define	DENOTIFY	276
#define	ANYOF	277
#define	ALLOF	278
#define	EXISTS	279
#define	SFALSE	280
#define	STRUE	281
#define	HEADER	282
#define	NOT	283
#define	SIZE	284
#define	ADDRESS	285
#define	ENVELOPE	286
#define	COMPARATOR	287
#define	IS	288
#define	CONTAINS	289
#define	MATCHES	290
#define	REGEX	291
#define	COUNT	292
#define	VALUE	293
#define	OVER	294
#define	UNDER	295
#define	ALL	296
#define	LOCALPART	297
#define	DOMAIN	298
#define	USER	299
#define	DETAIL	300
#define	DAYS	301
#define	ADDRESSES	302
#define	SUBJECT	303
#define	MIME	304
#define	METHOD	305
#define	ID	306
#define	OPTIONS	307
#define	LOW	308
#define	NORMAL	309
#define	HIGH	310
#define	MESSAGE	311

#line 1 "sieve.y"

/* sieve.y -- sieve parser
 * Larry Greenfield
 * $Id: sieve.c,v 1.1.1.1 2002-10-13 18:03:39 ghudson Exp $
 */
/***********************************************************
        Copyright 1999 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Carnegie Mellon
University not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "xmalloc.h"
#include "comparator.h"
#include "interp.h"
#include "script.h"
#include "tree.h"

#include "../lib/util.h"
#include "../lib/imparse.h"

    /* definitions */
    extern int addrparse(void);

struct vtags {
    int days;
    stringlist_t *addresses;
    char *subject;
    int mime;
};

struct htags {
    char *comparator;
    int comptag;
    char *relation;
};

struct aetags {
    int addrtag;
    char *comparator;
    int comptag;
    char *relation;
};

struct ntags {
    char *method;
    char *id;
    stringlist_t *options;
    const char *priority;
    char *message;
};

struct dtags {
    int comptag;
    char *relation;
    void *pattern;
    char *priority;
};

static commandlist_t *ret;
static sieve_script_t *parse_script;
static int check_reqs(stringlist_t *sl);
static test_t *build_address(int t, struct aetags *ae,
			     stringlist_t *sl, patternlist_t *pl);
static test_t *build_header(int t, struct htags *h,
			    stringlist_t *sl, patternlist_t *pl);
static commandlist_t *build_vacation(int t, struct vtags *h, char *s);
static commandlist_t *build_notify(int t, struct ntags *n);
static commandlist_t *build_denotify(int t, struct dtags *n);
static struct aetags *new_aetags(void);
static struct aetags *canon_aetags(struct aetags *ae);
static void free_aetags(struct aetags *ae);
static struct htags *new_htags(void);
static struct htags *canon_htags(struct htags *h);
static void free_htags(struct htags *h);
static struct vtags *new_vtags(void);
static struct vtags *canon_vtags(struct vtags *v);
static void free_vtags(struct vtags *v);
static struct ntags *new_ntags(void);
static struct ntags *canon_ntags(struct ntags *n);
static void free_ntags(struct ntags *n);
static struct dtags *new_dtags(void);
static void free_dtags(struct dtags *d);

static int verify_stringlist(stringlist_t *sl, int (*verify)(char *));
static int verify_mailbox(char *s);
static int verify_address(char *s);
static int verify_header(char *s);
static int verify_flag(char *s);
#ifdef ENABLE_REGEX
static regex_t *verify_regex(char *s, int cflags);
static patternlist_t *verify_regexs(stringlist_t *sl, char *comp);
#endif
static int ok_header(char *s);

int yyerror(char *msg);
extern int yylex(void);

#define YYERROR_VERBOSE /* i want better error messages! */

#line 126 "sieve.y"
typedef union {
    int nval;
    char *sval;
    stringlist_t *sl;
    test_t *test;
    testlist_t *testl;
    commandlist_t *cl;
    struct vtags *vtag;
    struct aetags *aetag;
    struct htags *htag;
    struct ntags *ntag;
    struct dtags *dtag;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		139
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 311 ? yytranslate[x] : 90)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    64,
    65,     2,     2,    61,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    58,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    59,     2,    60,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    62,     2,    63,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     5,     8,    12,    14,    17,    20,    25,
    28,    29,    34,    37,    40,    43,    46,    48,    50,    52,
    56,    59,    62,    65,    67,    69,    72,    75,    76,    80,
    84,    88,    91,    95,    96,    99,   103,   107,   109,   111,
   113,   114,   118,   122,   126,   129,   133,   135,   137,   141,
   145,   148,   151,   154,   157,   159,   161,   166,   171,   174,
   178,   180,   182,   184,   185,   188,   191,   195,   199,   200,
   203,   207,   211,   213,   215,   217,   219,   221,   223,   225,
   227,   229,   231,   233,   235,   237,   241,   243
};

static const short yyrhs[] = {    -1,
    67,    69,     0,     0,    68,    67,     0,    15,    77,    58,
     0,    70,     0,    70,    69,     0,    72,    58,     0,     5,
    80,    79,    71,     0,     1,    58,     0,     0,     6,    80,
    79,    71,     0,     7,    79,     0,     8,     4,     0,     9,
     4,     0,    10,     4,     0,    11,     0,    12,     0,    13,
     0,    14,    76,     4,     0,    16,    77,     0,    17,    77,
     0,    18,    77,     0,    19,     0,    20,     0,    21,    73,
     0,    22,    74,     0,     0,    73,    52,     4,     0,    73,
    51,     4,     0,    73,    53,    77,     0,    73,    75,     0,
    73,    57,     4,     0,     0,    74,    75,     0,    74,    85,
     4,     0,    74,    86,     4,     0,    54,     0,    55,     0,
    56,     0,     0,    76,    47,     3,     0,    76,    48,    77,
     0,    76,    49,     4,     0,    76,    50,     0,    59,    78,
    60,     0,     4,     0,     4,     0,     4,    61,    78,     0,
    62,    69,    63,     0,    62,    63,     0,    23,    88,     0,
    24,    88,     0,    25,    77,     0,    26,     0,    27,     0,
    28,    83,    77,    77,     0,    81,    82,    77,    77,     0,
    29,    80,     0,    30,    87,     3,     0,     1,     0,    31,
     0,    32,     0,     0,    82,    84,     0,    82,    85,     0,
    82,    86,     4,     0,    82,    33,     4,     0,     0,    83,
    85,     0,    83,    86,     4,     0,    83,    33,     4,     0,
    42,     0,    43,     0,    44,     0,    45,     0,    46,     0,
    34,     0,    35,     0,    36,     0,    37,     0,    38,     0,
    39,     0,    40,     0,    41,     0,    64,    89,    65,     0,
    80,     0,    80,    61,    89,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   166,   167,   170,   171,   174,   180,   181,   184,   185,   186,
   189,   190,   191,   194,   199,   208,   213,   214,   215,   216,
   224,   233,   242,   251,   256,   262,   270,   281,   282,   285,
   288,   291,   294,   299,   300,   303,   319,   327,   328,   329,
   332,   333,   336,   344,   350,   356,   357,   360,   361,   364,
   365,   368,   369,   370,   371,   372,   373,   393,   413,   414,
   416,   419,   420,   423,   424,   429,   433,   439,   449,   450,
   454,   460,   471,   472,   473,   474,   479,   486,   487,   488,
   489,   496,   501,   508,   509,   512,   515,   516
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NUMBER",
"STRING","IF","ELSIF","ELSE","REJCT","FILEINTO","REDIRECT","KEEP","STOP","DISCARD",
"VACATION","REQUIRE","SETFLAG","ADDFLAG","REMOVEFLAG","MARK","UNMARK","NOTIFY",
"DENOTIFY","ANYOF","ALLOF","EXISTS","SFALSE","STRUE","HEADER","NOT","SIZE","ADDRESS",
"ENVELOPE","COMPARATOR","IS","CONTAINS","MATCHES","REGEX","COUNT","VALUE","OVER",
"UNDER","ALL","LOCALPART","DOMAIN","USER","DETAIL","DAYS","ADDRESSES","SUBJECT",
"MIME","METHOD","ID","OPTIONS","LOW","NORMAL","HIGH","MESSAGE","';'","'['","']'",
"','","'{'","'}'","'('","')'","start","reqs","require","commands","command",
"elsif","action","ntags","dtags","priority","vtags","stringlist","strings","block",
"test","addrorenv","aetags","htags","addrparttag","comptag","relcomp","sizetag",
"testlist","tests", NULL
};
#endif

static const short yyr1[] = {     0,
    66,    66,    67,    67,    68,    69,    69,    70,    70,    70,
    71,    71,    71,    72,    72,    72,    72,    72,    72,    72,
    72,    72,    72,    72,    72,    72,    72,    73,    73,    73,
    73,    73,    73,    74,    74,    74,    74,    75,    75,    75,
    76,    76,    76,    76,    76,    77,    77,    78,    78,    79,
    79,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    80,    81,    81,    82,    82,    82,    82,    82,    83,    83,
    83,    83,    84,    84,    84,    84,    84,    85,    85,    85,
    85,    86,    86,    87,    87,    88,    89,    89
};

static const short yyr2[] = {     0,
     0,     2,     0,     2,     3,     1,     2,     2,     4,     2,
     0,     4,     2,     2,     2,     2,     1,     1,     1,     3,
     2,     2,     2,     1,     1,     2,     2,     0,     3,     3,
     3,     2,     3,     0,     2,     3,     3,     1,     1,     1,
     0,     3,     3,     3,     2,     3,     1,     1,     3,     3,
     2,     2,     2,     2,     1,     1,     4,     4,     2,     3,
     1,     1,     1,     0,     2,     2,     3,     3,     0,     2,
     3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     1,     3
};

static const short yydefact[] = {     3,
     0,     0,     3,    47,     0,     0,     0,     0,     0,     0,
     0,    17,    18,    19,    41,     0,     0,     0,    24,    25,
    28,    34,     2,     0,     0,     4,    48,     0,     5,    10,
    61,     0,     0,     0,    55,    56,    69,     0,     0,    62,
    63,     0,    64,    14,    15,    16,     0,    21,    22,    23,
    26,    27,     7,     8,     0,    46,     0,    52,    53,    54,
     0,    59,    84,    85,     0,     0,    11,     0,    20,     0,
     0,     0,    45,     0,     0,     0,    38,    39,    40,     0,
    32,    78,    79,    80,    81,    82,    83,    35,     0,     0,
    49,    87,     0,     0,     0,    70,     0,    60,    51,     0,
     0,     0,     9,     0,    73,    74,    75,    76,    77,     0,
    65,    66,     0,    42,    43,    44,    30,    29,    31,    33,
    36,    37,     0,    86,    72,    57,    71,    50,     0,    13,
    68,    58,    67,    88,    11,    12,     0,     0,     0
};

static const short yydefgoto[] = {   137,
     2,     3,    23,    24,   103,    25,    51,    52,    81,    47,
     6,    28,    67,    92,    43,    68,    61,   111,    89,    90,
    65,    58,    93
};

static const short yypact[] = {     6,
     0,   135,    -2,-32768,    16,   -36,   -34,   103,    21,    38,
    39,-32768,-32768,-32768,-32768,     0,     0,     0,-32768,-32768,
-32768,-32768,-32768,    53,   -14,-32768,   -15,   -13,-32768,-32768,
-32768,    12,    12,     0,-32768,-32768,-32768,   103,   -33,-32768,
-32768,    15,-32768,-32768,-32768,-32768,     1,-32768,-32768,-32768,
   107,    83,-32768,-32768,    16,-32768,   103,-32768,-32768,-32768,
    52,-32768,-32768,-32768,    75,    18,     5,    64,-32768,    77,
     0,    78,-32768,    79,    80,     0,-32768,-32768,-32768,    88,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    89,    91,
-32768,    44,    47,   109,     0,-32768,   110,-32768,-32768,    61,
   103,    15,-32768,   111,-32768,-32768,-32768,-32768,-32768,     0,
-32768,-32768,   121,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   103,-32768,-32768,-32768,-32768,-32768,    15,-32768,
-32768,-32768,-32768,-32768,     5,-32768,   141,   142,-32768
};

static const short yypgoto[] = {-32768,
   147,-32768,    -9,-32768,    30,-32768,-32768,-32768,   114,-32768,
   -16,   112,   -88,    -5,-32768,-32768,-32768,-32768,   -52,   -51,
-32768,   136,    45
};


#define	YYLAST		169


static const short yytable[] = {    48,
    49,    50,    42,     4,    69,    -1,    63,    64,    96,    97,
   101,   102,     1,   130,    53,   112,   113,    60,     7,    27,
     1,    29,     8,    30,    44,     9,    10,    11,    12,    13,
    14,    15,    62,    16,    17,    18,    19,    20,    21,    22,
   135,    45,    46,    54,    95,    55,    56,    70,    71,    72,
    73,   110,    -6,     7,   115,     4,   100,     8,     5,   119,
     9,    10,    11,    12,    13,    14,    15,     4,    16,    17,
    18,    19,    20,    21,    22,    57,    66,    98,   126,   114,
    99,   116,   117,   118,    94,    82,    83,    84,    85,    86,
    87,   120,   121,   132,   122,   129,   104,    82,    83,    84,
    85,    86,    87,    31,   123,   105,   106,   107,   108,   109,
     5,   124,   125,   127,   131,    -6,    82,    83,    84,    85,
    86,    87,     5,   128,   133,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,     7,    77,    78,    79,     8,
   138,   139,     9,    10,    11,    12,    13,    14,    15,    26,
    16,    17,    18,    19,    20,    21,    22,    74,    75,    76,
    77,    78,    79,    80,   136,    88,    91,   134,    59
};

static const short yycheck[] = {    16,
    17,    18,     8,     4,     4,     0,    40,    41,    61,    61,
     6,     7,    15,   102,    24,    68,    68,    34,     1,     4,
    15,    58,     5,    58,     4,     8,     9,    10,    11,    12,
    13,    14,    38,    16,    17,    18,    19,    20,    21,    22,
   129,     4,     4,    58,    61,    61,    60,    47,    48,    49,
    50,    68,     0,     1,    71,     4,    66,     5,    59,    76,
     8,     9,    10,    11,    12,    13,    14,     4,    16,    17,
    18,    19,    20,    21,    22,    64,    62,     3,    95,     3,
    63,     4,     4,     4,    33,    34,    35,    36,    37,    38,
    39,     4,     4,   110,     4,   101,    33,    34,    35,    36,
    37,    38,    39,     1,    61,    42,    43,    44,    45,    46,
    59,    65,     4,     4,     4,    63,    34,    35,    36,    37,
    38,    39,    59,    63,     4,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,     1,    54,    55,    56,     5,
     0,     0,     8,     9,    10,    11,    12,    13,    14,     3,
    16,    17,    18,    19,    20,    21,    22,    51,    52,    53,
    54,    55,    56,    57,   135,    52,    55,   123,    33
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/local/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 166 "sieve.y"
{ ret = NULL; ;
    break;}
case 2:
#line 167 "sieve.y"
{ ret = yyvsp[0].cl; ;
    break;}
case 5:
#line 174 "sieve.y"
{ if (!check_reqs(yyvsp[-1].sl)) {
                                    yyerror("unsupported feature");
				    YYERROR; 
                                  } ;
    break;}
case 6:
#line 180 "sieve.y"
{ yyval.cl = yyvsp[0].cl; ;
    break;}
case 7:
#line 181 "sieve.y"
{ yyvsp[-1].cl->next = yyvsp[0].cl; yyval.cl = yyvsp[-1].cl; ;
    break;}
case 8:
#line 184 "sieve.y"
{ yyval.cl = yyvsp[-1].cl; ;
    break;}
case 9:
#line 185 "sieve.y"
{ yyval.cl = new_if(yyvsp[-2].test, yyvsp[-1].cl, yyvsp[0].cl); ;
    break;}
case 10:
#line 186 "sieve.y"
{ yyval.cl = new_command(STOP); ;
    break;}
case 11:
#line 189 "sieve.y"
{ yyval.cl = NULL; ;
    break;}
case 12:
#line 190 "sieve.y"
{ yyval.cl = new_if(yyvsp[-2].test, yyvsp[-1].cl, yyvsp[0].cl); ;
    break;}
case 13:
#line 191 "sieve.y"
{ yyval.cl = yyvsp[0].cl; ;
    break;}
case 14:
#line 194 "sieve.y"
{ if (!parse_script->support.reject) {
				     yyerror("reject not required");
				     YYERROR;
				   }
				   yyval.cl = new_command(REJCT); yyval.cl->u.str = yyvsp[0].sval; ;
    break;}
case 15:
#line 199 "sieve.y"
{ if (!parse_script->support.fileinto) {
				     yyerror("fileinto not required");
	                             YYERROR;
                                   }
				   if (!verify_mailbox(yyvsp[0].sval)) {
				     YYERROR; /* vm should call yyerror() */
				   }
	                           yyval.cl = new_command(FILEINTO);
				   yyval.cl->u.str = yyvsp[0].sval; ;
    break;}
case 16:
#line 208 "sieve.y"
{ yyval.cl = new_command(REDIRECT);
				   if (!verify_address(yyvsp[0].sval)) {
				     YYERROR; /* va should call yyerror() */
				   }
				   yyval.cl->u.str = yyvsp[0].sval; ;
    break;}
case 17:
#line 213 "sieve.y"
{ yyval.cl = new_command(KEEP); ;
    break;}
case 18:
#line 214 "sieve.y"
{ yyval.cl = new_command(STOP); ;
    break;}
case 19:
#line 215 "sieve.y"
{ yyval.cl = new_command(DISCARD); ;
    break;}
case 20:
#line 216 "sieve.y"
{ if (!parse_script->support.vacation) {
				     yyerror("vacation not required");
				     yyval.cl = new_command(VACATION);
				     YYERROR;
				   } else {
  				     yyval.cl = build_vacation(VACATION,
					    canon_vtags(yyvsp[-1].vtag), yyvsp[0].sval);
				   } ;
    break;}
case 21:
#line 224 "sieve.y"
{ if (!parse_script->support.imapflags) {
                                    yyerror("imapflags not required");
                                    YYERROR;
                                   }
                                  if (!verify_stringlist(yyvsp[0].sl, verify_flag)) {
                                    YYERROR; /* vf should call yyerror() */
                                  }
                                  yyval.cl = new_command(SETFLAG);
                                  yyval.cl->u.sl = yyvsp[0].sl; ;
    break;}
case 22:
#line 233 "sieve.y"
{ if (!parse_script->support.imapflags) {
                                    yyerror("imapflags not required");
                                    YYERROR;
                                    }
                                  if (!verify_stringlist(yyvsp[0].sl, verify_flag)) {
                                    YYERROR; /* vf should call yyerror() */
                                  }
                                  yyval.cl = new_command(ADDFLAG);
                                  yyval.cl->u.sl = yyvsp[0].sl; ;
    break;}
case 23:
#line 242 "sieve.y"
{ if (!parse_script->support.imapflags) {
                                    yyerror("imapflags not required");
                                    YYERROR;
                                    }
                                  if (!verify_stringlist(yyvsp[0].sl, verify_flag)) {
                                    YYERROR; /* vf should call yyerror() */
                                  }
                                  yyval.cl = new_command(REMOVEFLAG);
                                  yyval.cl->u.sl = yyvsp[0].sl; ;
    break;}
case 24:
#line 251 "sieve.y"
{ if (!parse_script->support.imapflags) {
                                    yyerror("imapflags not required");
                                    YYERROR;
                                    }
                                  yyval.cl = new_command(MARK); ;
    break;}
case 25:
#line 256 "sieve.y"
{ if (!parse_script->support.imapflags) {
                                    yyerror("imapflags not required");
                                    YYERROR;
                                    }
                                  yyval.cl = new_command(UNMARK); ;
    break;}
case 26:
#line 262 "sieve.y"
{ if (!parse_script->support.notify) {
				       yyerror("notify not required");
				       yyval.cl = new_command(NOTIFY); 
				       YYERROR;
	 			    } else {
				      yyval.cl = build_notify(NOTIFY,
				             canon_ntags(yyvsp[0].ntag));
				    } ;
    break;}
case 27:
#line 270 "sieve.y"
{ if (!parse_script->support.notify) {
                                       yyerror("notify not required");
				       yyval.cl = new_command(DENOTIFY);
				       YYERROR;
				    } else {
					yyval.cl = build_denotify(DENOTIFY, yyvsp[0].dtag);
					if (yyval.cl == NULL) { 
			yyerror("unable to find a compatible comparator");
			YYERROR; } } ;
    break;}
case 28:
#line 281 "sieve.y"
{ yyval.ntag = new_ntags(); ;
    break;}
case 29:
#line 282 "sieve.y"
{ if (yyval.ntag->id != NULL) { 
					yyerror("duplicate :method"); YYERROR; }
				   else { yyval.ntag->id = yyvsp[0].sval; } ;
    break;}
case 30:
#line 285 "sieve.y"
{ if (yyval.ntag->method != NULL) { 
					yyerror("duplicate :method"); YYERROR; }
				   else { yyval.ntag->method = yyvsp[0].sval; } ;
    break;}
case 31:
#line 288 "sieve.y"
{ if (yyval.ntag->options != NULL) { 
					yyerror("duplicate :options"); YYERROR; }
				     else { yyval.ntag->options = yyvsp[0].sl; } ;
    break;}
case 32:
#line 291 "sieve.y"
{ if (yyval.ntag->priority != NULL) { 
					yyerror("duplicate :priority"); YYERROR; }
				   else { yyval.ntag->priority = yyvsp[0].sval; } ;
    break;}
case 33:
#line 294 "sieve.y"
{ if (yyval.ntag->message != NULL) { 
					yyerror("duplicate :message"); YYERROR; }
				   else { yyval.ntag->message = yyvsp[0].sval; } ;
    break;}
case 34:
#line 299 "sieve.y"
{ yyval.dtag = new_dtags(); ;
    break;}
case 35:
#line 300 "sieve.y"
{ if (yyval.dtag->priority != NULL) { 
				yyerror("duplicate priority level"); YYERROR; }
				   else { yyval.dtag->priority = yyvsp[0].sval; } ;
    break;}
case 36:
#line 303 "sieve.y"
{ if (yyval.dtag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR;
				   } else {
				       yyval.dtag->comptag = yyvsp[-1].nval;
#ifdef ENABLE_REGEX
				       if (yyval.dtag->comptag == REGEX) {
					   int cflags = REG_EXTENDED |
					       REG_NOSUB | REG_ICASE;
					   yyval.dtag->pattern =
					       (void*) verify_regex(yyvsp[0].sval, cflags);
					   if (!yyval.dtag->pattern) { YYERROR; }
				       }
				       else
#endif
					   yyval.dtag->pattern = yyvsp[0].sval;
				   } ;
    break;}
case 37:
#line 319 "sieve.y"
{ yyval.dtag = yyvsp[-2].dtag;
				   if (yyval.dtag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR; }
				   else { yyval.dtag->comptag = yyvsp[-1].nval;
				     yyval.dtag->relation = yyvsp[0].sval;
				   } ;
    break;}
case 38:
#line 327 "sieve.y"
{ yyval.sval = "low"; ;
    break;}
case 39:
#line 328 "sieve.y"
{ yyval.sval = "normal"; ;
    break;}
case 40:
#line 329 "sieve.y"
{ yyval.sval = "high"; ;
    break;}
case 41:
#line 332 "sieve.y"
{ yyval.vtag = new_vtags(); ;
    break;}
case 42:
#line 333 "sieve.y"
{ if (yyval.vtag->days != -1) { 
					yyerror("duplicate :days"); YYERROR; }
				   else { yyval.vtag->days = yyvsp[0].nval; } ;
    break;}
case 43:
#line 336 "sieve.y"
{ if (yyval.vtag->addresses != NULL) { 
					yyerror("duplicate :addresses"); 
					YYERROR;
				       } else if (!verify_stringlist(yyvsp[0].sl,
							verify_address)) {
					  YYERROR;
				       } else {
					 yyval.vtag->addresses = yyvsp[0].sl; } ;
    break;}
case 44:
#line 344 "sieve.y"
{ if (yyval.vtag->subject != NULL) { 
					yyerror("duplicate :subject"); 
					YYERROR;
				   } else if (!ok_header(yyvsp[0].sval)) {
					YYERROR;
				   } else { yyval.vtag->subject = yyvsp[0].sval; } ;
    break;}
case 45:
#line 350 "sieve.y"
{ if (yyval.vtag->mime != -1) { 
					yyerror("duplicate :mime"); 
					YYERROR; }
				   else { yyval.vtag->mime = MIME; } ;
    break;}
case 46:
#line 356 "sieve.y"
{ yyval.sl = yyvsp[-1].sl; ;
    break;}
case 47:
#line 357 "sieve.y"
{ yyval.sl = new_sl(yyvsp[0].sval, NULL); ;
    break;}
case 48:
#line 360 "sieve.y"
{ yyval.sl = new_sl(yyvsp[0].sval, NULL); ;
    break;}
case 49:
#line 361 "sieve.y"
{ yyval.sl = new_sl(yyvsp[-2].sval, yyvsp[0].sl); ;
    break;}
case 50:
#line 364 "sieve.y"
{ yyval.cl = yyvsp[-1].cl; ;
    break;}
case 51:
#line 365 "sieve.y"
{ yyval.cl = NULL; ;
    break;}
case 52:
#line 368 "sieve.y"
{ yyval.test = new_test(ANYOF); yyval.test->u.tl = yyvsp[0].testl; ;
    break;}
case 53:
#line 369 "sieve.y"
{ yyval.test = new_test(ALLOF); yyval.test->u.tl = yyvsp[0].testl; ;
    break;}
case 54:
#line 370 "sieve.y"
{ yyval.test = new_test(EXISTS); yyval.test->u.sl = yyvsp[0].sl; ;
    break;}
case 55:
#line 371 "sieve.y"
{ yyval.test = new_test(SFALSE); ;
    break;}
case 56:
#line 372 "sieve.y"
{ yyval.test = new_test(STRUE); ;
    break;}
case 57:
#line 374 "sieve.y"
{ patternlist_t *pl;
                                   if (!verify_stringlist(yyvsp[-1].sl, verify_header)) {
                                     YYERROR; /* vh should call yyerror() */
                                   }

				   yyvsp[-2].htag = canon_htags(yyvsp[-2].htag);
#ifdef ENABLE_REGEX
				   if (yyvsp[-2].htag->comptag == REGEX) {
				     pl = verify_regexs(yyvsp[0].sl, yyvsp[-2].htag->comparator);
				     if (!pl) { YYERROR; }
				   }
				   else
#endif
				     pl = (patternlist_t *) yyvsp[0].sl;
				       
				   yyval.test = build_header(HEADER, yyvsp[-2].htag, yyvsp[-1].sl, pl);
				   if (yyval.test == NULL) { 
			yyerror("unable to find a compatible comparator");
			YYERROR; } ;
    break;}
case 58:
#line 394 "sieve.y"
{ patternlist_t *pl;
                                   if (!verify_stringlist(yyvsp[-1].sl, verify_header)) {
                                     YYERROR; /* vh should call yyerror() */
                                   }

				   yyvsp[-2].aetag = canon_aetags(yyvsp[-2].aetag);
#ifdef ENABLE_REGEX
				   if (yyvsp[-2].aetag->comptag == REGEX) {
				     pl = verify_regexs(yyvsp[0].sl, yyvsp[-2].aetag->comparator);
				     if (!pl) { YYERROR; }
				   }
				   else
#endif
				     pl = (patternlist_t *) yyvsp[0].sl;
				       
				   yyval.test = build_address(yyvsp[-3].nval, yyvsp[-2].aetag, yyvsp[-1].sl, pl);
				   if (yyval.test == NULL) { 
			yyerror("unable to find a compatible comparator");
			YYERROR; } ;
    break;}
case 59:
#line 413 "sieve.y"
{ yyval.test = new_test(NOT); yyval.test->u.t = yyvsp[0].test; ;
    break;}
case 60:
#line 414 "sieve.y"
{ yyval.test = new_test(SIZE); yyval.test->u.sz.t = yyvsp[-1].nval;
		                   yyval.test->u.sz.n = yyvsp[0].nval; ;
    break;}
case 61:
#line 416 "sieve.y"
{ yyval.test = NULL; ;
    break;}
case 62:
#line 419 "sieve.y"
{ yyval.nval = ADDRESS; ;
    break;}
case 63:
#line 420 "sieve.y"
{ yyval.nval = ENVELOPE; ;
    break;}
case 64:
#line 423 "sieve.y"
{ yyval.aetag = new_aetags(); ;
    break;}
case 65:
#line 424 "sieve.y"
{ yyval.aetag = yyvsp[-1].aetag;
				   if (yyval.aetag->addrtag != -1) { 
			yyerror("duplicate or conflicting address part tag");
			YYERROR; }
				   else { yyval.aetag->addrtag = yyvsp[0].nval; } ;
    break;}
case 66:
#line 429 "sieve.y"
{ yyval.aetag = yyvsp[-1].aetag;
				   if (yyval.aetag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR; }
				   else { yyval.aetag->comptag = yyvsp[0].nval; } ;
    break;}
case 67:
#line 433 "sieve.y"
{ yyval.aetag = yyvsp[-2].aetag;
				   if (yyval.aetag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR; }
				   else { yyval.aetag->comptag = yyvsp[-1].nval;
				     yyval.aetag->relation = yyvsp[0].sval;
				   } ;
    break;}
case 68:
#line 439 "sieve.y"
{ yyval.aetag = yyvsp[-2].aetag;
				   if (yyval.aetag->comparator != NULL) { 
			yyerror("duplicate comparator tag"); YYERROR; }
				   else if (!strcmp(yyvsp[0].sval, "i;ascii-numeric") &&
					    !parse_script->support.i_ascii_numeric) {
			yyerror("comparator-i;ascii-numeric not required");
			YYERROR; }
				   else { yyval.aetag->comparator = yyvsp[0].sval; } ;
    break;}
case 69:
#line 449 "sieve.y"
{ yyval.htag = new_htags(); ;
    break;}
case 70:
#line 450 "sieve.y"
{ yyval.htag = yyvsp[-1].htag;
				   if (yyval.htag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR; }
				   else { yyval.htag->comptag = yyvsp[0].nval; } ;
    break;}
case 71:
#line 454 "sieve.y"
{ yyval.htag = yyvsp[-2].htag;
				   if (yyval.htag->comptag != -1) { 
			yyerror("duplicate comparator type tag"); YYERROR; }
				   else { yyval.htag->comptag = yyvsp[-1].nval;
				     yyval.htag->relation = yyvsp[0].sval;
				   } ;
    break;}
case 72:
#line 460 "sieve.y"
{ yyval.htag = yyvsp[-2].htag;
				   if (yyval.htag->comparator != NULL) { 
			yyerror("duplicate comparator tag");
					YYERROR; }
				   else if (!strcmp(yyvsp[0].sval, "i;ascii-numeric") &&
					    !parse_script->support.i_ascii_numeric) { 
			yyerror("comparator-i;ascii-numeric not required");
			YYERROR; }
				   else { yyval.htag->comparator = yyvsp[0].sval; } ;
    break;}
case 73:
#line 471 "sieve.y"
{ yyval.nval = ALL; ;
    break;}
case 74:
#line 472 "sieve.y"
{ yyval.nval = LOCALPART; ;
    break;}
case 75:
#line 473 "sieve.y"
{ yyval.nval = DOMAIN; ;
    break;}
case 76:
#line 474 "sieve.y"
{ if (!parse_script->support.subaddress) {
				     yyerror("subaddress not required");
				     YYERROR;
				   }
				   yyval.nval = USER; ;
    break;}
case 77:
#line 479 "sieve.y"
{ if (!parse_script->support.subaddress) {
				     yyerror("subaddress not required");
				     YYERROR;
				   }
				   yyval.nval = DETAIL; ;
    break;}
case 78:
#line 486 "sieve.y"
{ yyval.nval = IS; ;
    break;}
case 79:
#line 487 "sieve.y"
{ yyval.nval = CONTAINS; ;
    break;}
case 80:
#line 488 "sieve.y"
{ yyval.nval = MATCHES; ;
    break;}
case 81:
#line 489 "sieve.y"
{ if (!parse_script->support.regex) {
				     yyerror("regex not required");
				     YYERROR;
				   }
				   yyval.nval = REGEX; ;
    break;}
case 82:
#line 496 "sieve.y"
{ if (!parse_script->support.relational) {
				     yyerror("relational not required");
				     YYERROR;
				   }
				   yyval.nval = COUNT; ;
    break;}
case 83:
#line 501 "sieve.y"
{ if (!parse_script->support.relational) {
				     yyerror("relational not required");
				     YYERROR;
				   }
				   yyval.nval = VALUE; ;
    break;}
case 84:
#line 508 "sieve.y"
{ yyval.nval = OVER; ;
    break;}
case 85:
#line 509 "sieve.y"
{ yyval.nval = UNDER; ;
    break;}
case 86:
#line 512 "sieve.y"
{ yyval.testl = yyvsp[-1].testl; ;
    break;}
case 87:
#line 515 "sieve.y"
{ yyval.testl = new_testlist(yyvsp[0].test, NULL); ;
    break;}
case 88:
#line 516 "sieve.y"
{ yyval.testl = new_testlist(yyvsp[-2].test, yyvsp[0].testl); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/local/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 519 "sieve.y"

commandlist_t *sieve_parse(sieve_script_t *script, FILE *f)
{
    commandlist_t *t;
    extern FILE *yyin;

    yyin = f;
    parse_script = script;
    if (yyparse()) {
	t = NULL;
    } else {
	t = ret;
    }
    ret = NULL;
    return t;
}

int yyerror(char *msg)
{
    extern int yylineno;
    int ret;

    parse_script->err++;
    if (parse_script->interp.err) {
	ret = parse_script->interp.err(yylineno, msg, 
				       parse_script->interp.interp_context,
				       parse_script->script_context);
    }

    return 0;
}

static int check_reqs(stringlist_t *sl)
{
    int i = 1;
    stringlist_t *s;
    
    while (sl != NULL) {
	s = sl;
	sl = sl->next;

	i &= script_require(parse_script, s->s);

	if (s->s) free(s->s);
	free(s);
    }
    return i;
}

static test_t *build_address(int t, struct aetags *ae,
			     stringlist_t *sl, patternlist_t *pl)
{
    test_t *ret = new_test(t);	/* can be either ADDRESS or ENVELOPE */

    assert((t == ADDRESS) || (t == ENVELOPE));

    if (ret) {
	ret->u.ae.comptag = ae->comptag;
	ret->u.ae.comp = lookup_comp(ae->comparator, ae->comptag,
				     ae->relation, &ret->u.ae.comprock);
	ret->u.ae.sl = sl;
	ret->u.ae.pl = pl;
	ret->u.ae.addrpart = ae->addrtag;
	free_aetags(ae);
	if (ret->u.ae.comp == NULL) {
	    free_test(ret);
	    ret = NULL;
	}
    }
    return ret;
}

static test_t *build_header(int t, struct htags *h,
			    stringlist_t *sl, patternlist_t *pl)
{
    test_t *ret = new_test(t);	/* can be HEADER */

    assert(t == HEADER);

    if (ret) {
	ret->u.h.comptag = h->comptag;
	ret->u.h.comp = lookup_comp(h->comparator, h->comptag,
				    h->relation, &ret->u.h.comprock);
	ret->u.h.sl = sl;
	ret->u.h.pl = pl;
	free_htags(h);
	if (ret->u.h.comp == NULL) {
	    free_test(ret);
	    ret = NULL;
	}
    }
    return ret;
}

static commandlist_t *build_vacation(int t, struct vtags *v, char *reason)
{
    commandlist_t *ret = new_command(t);

    assert(t == VACATION);

    if (ret) {
	ret->u.v.subject = v->subject; v->subject = NULL;
	ret->u.v.days = v->days;
	ret->u.v.mime = v->mime;
	ret->u.v.addresses = v->addresses; v->addresses = NULL;
	free_vtags(v);
	ret->u.v.message = reason;
    }
    return ret;
}

static commandlist_t *build_notify(int t, struct ntags *n)
{
    commandlist_t *ret = new_command(t);

    assert(t == NOTIFY);

    if (ret) {
	ret->u.n.method = n->method; n->method = NULL;
	ret->u.n.id = n->id; n->id = NULL;
	ret->u.n.options = n->options; n->options = NULL;
	ret->u.n.priority = n->priority;
	ret->u.n.message = n->message; n->message = NULL;
	free_ntags(n);
    }
    return ret;
}

static commandlist_t *build_denotify(int t, struct dtags *d)
{
    commandlist_t *ret = new_command(t);

    assert(t == DENOTIFY);

    if (ret) {
	ret->u.d.comptag = d->comptag;
	ret->u.d.comp = lookup_comp("i;ascii-casemap", d->comptag,
				    d->relation, &ret->u.d.comprock);
	ret->u.d.pattern = d->pattern; d->pattern = NULL;
	ret->u.d.priority = d->priority;
	free_dtags(d);
	if (ret->u.d.comp == NULL) {
	    free_tree(ret);
	    ret = NULL;
	}
    }
    return ret;
}

static struct aetags *new_aetags(void)
{
    struct aetags *r = (struct aetags *) xmalloc(sizeof(struct aetags));

    r->addrtag = r->comptag = -1;
    r->comparator = r->relation = NULL;

    return r;
}

static struct aetags *canon_aetags(struct aetags *ae)
{
    if (ae->addrtag == -1) { ae->addrtag = ALL; }
    if (ae->comparator == NULL) { ae->comparator = strdup("i;ascii-casemap"); }
    if (ae->comptag == -1) { ae->comptag = IS; }
    return ae;
}

static void free_aetags(struct aetags *ae)
{
    free(ae->comparator);
    if (ae->relation) free(ae->relation);
    free(ae);
}

static struct htags *new_htags(void)
{
    struct htags *r = (struct htags *) xmalloc(sizeof(struct htags));

    r->comptag = -1;
    r->comparator = r->relation = NULL;

    return r;
}

static struct htags *canon_htags(struct htags *h)
{
    if (h->comparator == NULL) { h->comparator = strdup("i;ascii-casemap"); }
    if (h->comptag == -1) { h->comptag = IS; }
    return h;
}

static void free_htags(struct htags *h)
{
    free(h->comparator);
    if (h->relation) free(h->relation);
    free(h);
}

static struct vtags *new_vtags(void)
{
    struct vtags *r = (struct vtags *) xmalloc(sizeof(struct vtags));

    r->days = -1;
    r->addresses = NULL;
    r->subject = NULL;
    r->mime = -1;

    return r;
}

static struct vtags *canon_vtags(struct vtags *v)
{
    assert(parse_script->interp.vacation != NULL);

    if (v->days == -1) { v->days = 7; }
    if (v->days < parse_script->interp.vacation->min_response) 
       { v->days = parse_script->interp.vacation->min_response; }
    if (v->days > parse_script->interp.vacation->max_response)
       { v->days = parse_script->interp.vacation->max_response; }
    if (v->mime == -1) { v->mime = 0; }

    return v;
}

static void free_vtags(struct vtags *v)
{
    if (v->addresses) { free_sl(v->addresses); }
    if (v->subject) { free(v->subject); }
    free(v);
}

static struct ntags *new_ntags(void)
{
    struct ntags *r = (struct ntags *) xmalloc(sizeof(struct ntags));

    r->method = NULL;
    r->id = NULL;
    r->options = NULL;
    r->priority = NULL;
    r->message = NULL;

    return r;
}

static struct ntags *canon_ntags(struct ntags *n)
{
    if (n->priority == NULL) { n->priority = "normal"; }
    if (n->message == NULL) { n->message = strdup("$from$: $subject$"); }

    return n;
}

static void free_ntags(struct ntags *n)
{
    if (n->method) { free(n->method); }
    if (n->id) { free(n->id); }
    if (n->options) { free_sl(n->options); }
    if (n->message) { free(n->message); }
    free(n);
}

static struct dtags *new_dtags(void)
{
    struct dtags *r = (struct dtags *) xmalloc(sizeof(struct dtags));

    r->comptag = -1;
    r->relation = r->pattern = r->priority = NULL;

    return r;
}

static void free_dtags(struct dtags *d)
{
    if (d->relation) free(d->relation);
    if (d->pattern) free(d->pattern);
    free(d);
}

static int verify_stringlist(stringlist_t *sl, int (*verify)(char *))
{
    for (; sl != NULL && verify(sl->s); sl = sl->next) ;
    return (sl == NULL);
}

char *addrptr;		/* pointer to address string for address lexer */
char addrerr[500];	/* buffer for address parser error messages */

static int verify_address(char *s)
{
    char errbuf[500];

    addrptr = s;
    addrerr[0] = '\0';	/* paranoia */
    if (addrparse()) {
	sprintf(errbuf, "address '%s': %s", s, addrerr);
	yyerror(errbuf);
	return 0;
    }
    return 1;
}

static int verify_mailbox(char *s __attribute__((unused)))
{
    /* xxx if not a mailbox, call yyerror */
    return 1;
}

static int verify_header(char *hdr)
{
    char *h = hdr;
    char errbuf[100];

    while (*h) {
	/* field-name      =       1*ftext
	   ftext           =       %d33-57 / %d59-126         
	   ; Any character except
	   ;  controls, SP, and
	   ;  ":". */
	if (!((*h >= 33 && *h <= 57) || (*h >= 59 && *h <= 126))) {
	    sprintf(errbuf, "header '%s': not a valid header", hdr);
	    yyerror(errbuf);
	    return 0;
	}
	h++;
    }
    return 1;
}
 
static int verify_flag(char *f)
{
    char errbuf[100];
 
    if (f[0] == '\\') {
	lcase(f);
	if (strcmp(f, "\\seen") && strcmp(f, "\\answered") &&
	    strcmp(f, "\\flagged") && strcmp(f, "\\draft") &&
	    strcmp(f, "\\deleted")) {
	    sprintf(errbuf, "flag '%s': not a system flag", f);
	    yyerror(errbuf);
	    return 0;
	}
	return 1;
    }
    if (!imparse_isatom(f)) {
	sprintf(errbuf, "flag '%s': not a valid keyword", f);
	yyerror(errbuf);
	return 0;
    }
    return 1;
}
 
#ifdef ENABLE_REGEX
static regex_t *verify_regex(char *s, int cflags)
{
    int ret;
    char errbuf[100];
    regex_t *reg = (regex_t *) xmalloc(sizeof(regex_t));

    if ((ret = regcomp(reg, s, cflags)) != 0) {
	(void) regerror(ret, reg, errbuf, sizeof(errbuf));
	yyerror(errbuf);
	free(reg);
	return NULL;
    }
    return reg;
}

static patternlist_t *verify_regexs(stringlist_t *sl, char *comp)
{
    stringlist_t *sl2;
    patternlist_t *pl = NULL;
    int cflags = REG_EXTENDED | REG_NOSUB;
    regex_t *reg;

    if (!strcmp(comp, "i;ascii-casemap")) {
	cflags |= REG_ICASE;
    }

    for (sl2 = sl; sl2 != NULL; sl2 = sl2->next) {
	if ((reg = verify_regex(sl2->s, cflags)) == NULL) {
	    free_pl(pl, REGEX);
	    break;
	}
	pl = (patternlist_t *) new_pl(reg, pl);
    }
    if (sl2 == NULL) {
	free_sl(sl);
	return pl;
    }
    return NULL;
}
#endif

/* xxx is it ok to put this in an RFC822 header body? */
static int ok_header(char *s __attribute__((unused)))
{
    return 1;
}
