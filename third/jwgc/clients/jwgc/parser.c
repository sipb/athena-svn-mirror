
/*  A Bison parser, made from parser.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	ERROR	257
#define	VARNAME	258
#define	VARREF	259
#define	STRING	260
#define	SHOW	261
#define	APPENDPORT	262
#define	BUFFER	263
#define	BREAK	264
#define	CLOSEINPUT	265
#define	CLOSEOUTPUT	266
#define	CLOSEPORT	267
#define	CASE	268
#define	CLEARBUF	269
#define	DEFAULT	270
#define	DISPLAY	271
#define	DO	272
#define	DOWNCASE	273
#define	ELSE	274
#define	ELSEIF	275
#define	ENDCASE	276
#define	ENDIF	277
#define	ENDWHILE	278
#define	EXEC	279
#define	EXECPORT	280
#define	EXIT	281
#define	FIELDS	282
#define	GET	283
#define	GETENV	284
#define	IF	285
#define	INPUTPORT	286
#define	LANY	287
#define	LBREAK	288
#define	LSPAN	289
#define	MATCH	290
#define	NOOP	291
#define	NOT	292
#define	OUTPUTPORT	293
#define	PRINT	294
#define	PROTECT	295
#define	VERBATIM	296
#define	PUT	297
#define	RANY	298
#define	RBREAK	299
#define	RSPAN	300
#define	SET	301
#define	SUBSTITUTE	302
#define	THEN	303
#define	UPCASE	304
#define	WHILE	305
#define	JVAR	306
#define	PARAGRAPH	307
#define	EQ	308
#define	NEQ	309
#define	REGEQ	310
#define	REGNEQ	311

#line 1 "parser.y"

#include <sysdep.h>

/* Saber-C suppressions because yacc loses */

/*SUPPRESS 288*/
/*SUPPRESS 287*/

#include <stdio.h>
#include "lexer.h"
#include "node.h"
#include "main.h"

static void yyerror();
int yyparse();

/*
 * the_program - local variable used to communicate the program's node
 *               representation from the program action to the parse_file
 *               function.
 */

static Node *the_program;

#line 26 "parser.y"
typedef union{
    char *text;
    struct _Node *node;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		176
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 311 ? yytranslate[x] : 79)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    61,     2,     2,     2,     2,    55,     2,    62,
    63,     2,    60,    64,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    65,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    54,     2,     2,     2,     2,     2,     2,
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
    47,    48,    49,    50,    51,    52,    53,    56,    57,    58,
    59
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,    10,    12,    14,    17,    21,    25,
    29,    33,    37,    41,    45,    49,    54,    59,    64,    69,
    74,    79,    84,    89,    96,   103,   110,   117,   124,   131,
   138,   140,   145,   148,   151,   153,   155,   159,   164,   168,
   172,   176,   178,   181,   184,   187,   191,   198,   203,   209,
   211,   213,   215,   219,   220,   226,   230,   233,   234,   237,
   239,   243,   244,   247,   248,   251,   252
};

static const short yyrhs[] = {    78,
     0,     4,     0,     6,     0,    62,    69,    63,     0,    68,
     0,     5,     0,    61,    69,     0,    69,    60,    69,     0,
    69,    54,    69,     0,    69,    55,    69,     0,    69,    56,
    69,     0,    69,    57,    69,     0,    69,    58,    69,     0,
    69,    59,    69,     0,     9,    62,    63,     0,    48,    62,
    69,    63,     0,    41,    62,    69,    63,     0,    42,    62,
    69,    63,     0,    30,    62,    69,    63,     0,    50,    62,
    69,    63,     0,    19,    62,    69,    63,     0,    52,    62,
    69,    63,     0,    29,    62,    69,    63,     0,    33,    62,
    69,    64,    69,    63,     0,    44,    62,    69,    64,    69,
    63,     0,    34,    62,    69,    64,    69,    63,     0,    45,
    62,    69,    64,    69,    63,     0,    35,    62,    69,    64,
    69,    63,     0,    46,    62,    69,    64,    69,    63,     0,
    53,    62,    69,    64,    69,    63,     0,    37,     0,    47,
    67,    65,    69,     0,    28,    76,     0,    40,    74,     0,
     7,     0,    15,     0,     8,    69,    69,     0,    26,    69,
    69,    74,     0,    32,    69,    69,     0,    39,    69,    69,
     0,    43,    69,    74,     0,    43,     0,    11,    69,     0,
    12,    69,     0,    13,    69,     0,    25,    69,    74,     0,
    31,    69,    49,    78,    71,    23,     0,    14,    69,    77,
    22,     0,    51,    69,    18,    78,    24,     0,    10,     0,
    27,     0,    72,     0,    72,    20,    78,     0,     0,    72,
    21,    69,    49,    78,     0,    36,    75,    78,     0,    16,
    78,     0,     0,    74,    69,     0,    69,     0,    75,    64,
    69,     0,     0,    76,    67,     0,     0,    77,    73,     0,
     0,    78,    70,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    59,    64,    68,    72,    75,    77,    80,    83,    85,    87,
    89,    91,    93,    95,    98,   101,   103,   105,   107,   109,
   111,   113,   115,   118,   120,   122,   124,   126,   128,   130,
   134,   136,   138,   145,   148,   153,   159,   161,   164,   166,
   168,   171,   173,   175,   177,   183,   190,   195,   198,   201,
   203,   207,   209,   217,   219,   225,   229,   244,   246,   251,
   253,   258,   260,   265,   267,   272,   274
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","ERROR",
"VARNAME","VARREF","STRING","SHOW","APPENDPORT","BUFFER","BREAK","CLOSEINPUT",
"CLOSEOUTPUT","CLOSEPORT","CASE","CLEARBUF","DEFAULT","DISPLAY","DO","DOWNCASE",
"ELSE","ELSEIF","ENDCASE","ENDIF","ENDWHILE","EXEC","EXECPORT","EXIT","FIELDS",
"GET","GETENV","IF","INPUTPORT","LANY","LBREAK","LSPAN","MATCH","NOOP","NOT",
"OUTPUTPORT","PRINT","PROTECT","VERBATIM","PUT","RANY","RBREAK","RSPAN","SET",
"SUBSTITUTE","THEN","UPCASE","WHILE","JVAR","PARAGRAPH","'|'","'&'","EQ","NEQ",
"REGEQ","REGNEQ","'+'","'!'","'('","')'","','","'='","program","varname","string",
"expr","statement","elseparts","elseifparts","match","exprlist","comma_exprlist",
"varnamelist","matchlist","statements", NULL
};
#endif

static const short yyr1[] = {     0,
    66,    67,    68,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
    70,    71,    71,    72,    72,    73,    73,    74,    74,    75,
    75,    76,    76,    77,    77,    78,    78
};

static const short yyr2[] = {     0,
     1,     1,     1,     3,     1,     1,     2,     3,     3,     3,
     3,     3,     3,     3,     3,     4,     4,     4,     4,     4,
     4,     4,     4,     6,     6,     6,     6,     6,     6,     6,
     1,     4,     2,     2,     1,     1,     3,     4,     3,     3,
     3,     1,     2,     2,     2,     3,     6,     4,     5,     1,
     1,     1,     3,     0,     5,     3,     2,     0,     2,     1,
     3,     0,     2,     0,     2,     0,     2
};

static const short yydefact[] = {    66,
     1,    35,     0,    50,     0,     0,     0,     0,    36,     0,
     0,    51,    62,     0,     0,    31,     0,    58,    42,     0,
     0,    67,     6,     3,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     5,     0,    43,    44,    45,    64,    58,     0,
    33,     0,     0,     0,    34,    58,     2,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     7,     0,     0,     0,     0,
     0,     0,     0,     0,    37,     0,    46,    58,    63,    66,
    39,    40,    59,    41,     0,    66,    15,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     4,     9,    10,    11,    12,    13,    14,     8,
    66,    48,     0,    65,    38,    54,    32,     0,    21,    23,
    19,     0,     0,     0,    17,    18,     0,     0,     0,    16,
    20,    22,     0,    57,    60,    66,     0,    52,    49,     0,
     0,     0,     0,     0,     0,     0,     0,    56,    47,    66,
     0,    24,    26,    28,    25,    27,    29,    30,    61,    53,
     0,    66,    55,     0,     0,     0
};

static const short yydefgoto[] = {   174,
    58,    43,    93,    22,   147,   148,   124,    55,   146,    51,
    86,     1
};

static const short yypact[] = {-32768,
   274,-32768,   176,-32768,   176,   176,   176,   176,-32768,   176,
   176,-32768,-32768,   176,   176,-32768,   176,-32768,   176,     6,
   176,-32768,-32768,-32768,   -61,   -41,   -40,   -38,   -36,   -35,
   -34,   -25,   -22,   -20,   -19,   -18,   -16,   -13,    -5,    20,
   176,   176,-32768,   118,   194,   194,   194,   194,   194,   118,
     6,   -24,   118,   118,   176,   194,-32768,    18,    56,    21,
   176,   176,   176,   176,   176,   176,   176,   176,   176,   176,
   176,   176,   176,   176,   176,-32768,   327,   176,   176,   176,
   176,   176,   176,   176,   194,    -7,   176,   194,-32768,-32768,
   194,   194,   194,   176,   176,-32768,-32768,   337,   347,   357,
   132,   143,   272,   367,   377,   283,   294,   305,   387,   397,
   407,   316,-32768,    86,    -4,    25,    25,    25,    25,-32768,
-32768,-32768,   176,-32768,   176,   274,   194,   232,-32768,-32768,
-32768,   176,   176,   176,-32768,-32768,   176,   176,   176,-32768,
-32768,-32768,   176,   274,   194,    22,    65,    -1,-32768,   417,
   427,   437,   447,   457,   467,   477,   176,   274,-32768,-32768,
   176,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   194,   274,
    46,-32768,   274,    89,    90,-32768
};

static const short yypgoto[] = {-32768,
    40,-32768,    -3,-32768,-32768,-32768,-32768,   -43,-32768,-32768,
-32768,   -73
};


#define	YYLAST		540


static const short yytable[] = {    44,
    60,    45,    46,    47,    48,    87,    49,    50,   121,    57,
    52,    53,    94,    54,   122,    56,   126,    59,   160,   161,
    61,    62,   128,    63,    90,    64,    65,    66,   123,    78,
    79,    80,    81,    82,    83,    84,    67,    76,    77,    68,
    85,    69,    70,    71,   125,    72,    88,   144,    73,    91,
    92,    80,    81,    82,    83,    84,    74,    98,    99,   100,
   101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
   111,   112,   158,    96,   114,   115,   116,   117,   118,   119,
   120,    75,    95,    97,    84,   157,   170,   159,   175,   176,
    89,   127,     0,     0,   172,     0,     0,     0,   173,    78,
    79,    80,    81,    82,    83,    84,     0,     0,     0,    78,
    79,    80,    81,    82,    83,    84,     0,     0,     0,   145,
     0,     0,    23,    24,     0,     0,    25,     0,   150,   151,
   152,     0,     0,   153,   154,   155,    26,     0,     0,   156,
    79,    80,    81,    82,    83,    84,    27,    28,     0,     0,
    29,    30,    31,   169,     0,     0,     0,   171,    32,    33,
     0,    34,    35,    36,     0,    37,     0,    38,     0,    39,
    40,    78,    79,    80,    81,    82,    83,    84,    41,    42,
    23,    24,     0,     0,    25,    78,    79,    80,    81,    82,
    83,    84,     0,     0,    26,   132,    78,    79,    80,    81,
    82,    83,    84,     0,    27,    28,   133,     0,    29,    30,
    31,     0,     0,     0,     0,     0,    32,    33,     0,    34,
    35,    36,     0,    37,     0,    38,     0,    39,    40,     0,
     0,     0,     0,     0,     0,     0,    41,    42,     2,     3,
     0,     4,     5,     6,     7,     8,     9,    78,    79,    80,
    81,    82,    83,    84,     0,   149,    10,    11,    12,    13,
     0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
    17,    18,     0,     0,    19,     0,     0,     0,    20,     0,
     2,     3,    21,     4,     5,     6,     7,     8,     9,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    10,    11,
    12,    13,     0,     0,    14,    15,     0,     0,     0,     0,
    16,     0,    17,    18,     0,     0,    19,     0,     0,     0,
    20,     0,     0,     0,    21,    78,    79,    80,    81,    82,
    83,    84,     0,     0,     0,   134,    78,    79,    80,    81,
    82,    83,    84,     0,     0,     0,   137,    78,    79,    80,
    81,    82,    83,    84,     0,     0,     0,   138,    78,    79,
    80,    81,    82,    83,    84,     0,     0,     0,   139,    78,
    79,    80,    81,    82,    83,    84,     0,     0,     0,   143,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   113,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   129,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   130,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   131,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   135,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   136,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   140,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   141,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   142,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   162,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   163,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   164,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   165,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   166,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   167,
    78,    79,    80,    81,    82,    83,    84,     0,     0,   168
};

static const short yycheck[] = {     3,
    62,     5,     6,     7,     8,    49,    10,    11,    16,     4,
    14,    15,    56,    17,    22,    19,    90,    21,    20,    21,
    62,    62,    96,    62,    49,    62,    62,    62,    36,    54,
    55,    56,    57,    58,    59,    60,    62,    41,    42,    62,
    44,    62,    62,    62,    88,    62,    50,   121,    62,    53,
    54,    56,    57,    58,    59,    60,    62,    61,    62,    63,
    64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
    74,    75,   146,    18,    78,    79,    80,    81,    82,    83,
    84,    62,    65,    63,    60,    64,   160,    23,     0,     0,
    51,    95,    -1,    -1,    49,    -1,    -1,    -1,   172,    54,
    55,    56,    57,    58,    59,    60,    -1,    -1,    -1,    54,
    55,    56,    57,    58,    59,    60,    -1,    -1,    -1,   123,
    -1,    -1,     5,     6,    -1,    -1,     9,    -1,   132,   133,
   134,    -1,    -1,   137,   138,   139,    19,    -1,    -1,   143,
    55,    56,    57,    58,    59,    60,    29,    30,    -1,    -1,
    33,    34,    35,   157,    -1,    -1,    -1,   161,    41,    42,
    -1,    44,    45,    46,    -1,    48,    -1,    50,    -1,    52,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
     5,     6,    -1,    -1,     9,    54,    55,    56,    57,    58,
    59,    60,    -1,    -1,    19,    64,    54,    55,    56,    57,
    58,    59,    60,    -1,    29,    30,    64,    -1,    33,    34,
    35,    -1,    -1,    -1,    -1,    -1,    41,    42,    -1,    44,
    45,    46,    -1,    48,    -1,    50,    -1,    52,    53,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    61,    62,     7,     8,
    -1,    10,    11,    12,    13,    14,    15,    54,    55,    56,
    57,    58,    59,    60,    -1,    24,    25,    26,    27,    28,
    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,    37,    -1,
    39,    40,    -1,    -1,    43,    -1,    -1,    -1,    47,    -1,
     7,     8,    51,    10,    11,    12,    13,    14,    15,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
    27,    28,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
    37,    -1,    39,    40,    -1,    -1,    43,    -1,    -1,    -1,
    47,    -1,    -1,    -1,    51,    54,    55,    56,    57,    58,
    59,    60,    -1,    -1,    -1,    64,    54,    55,    56,    57,
    58,    59,    60,    -1,    -1,    -1,    64,    54,    55,    56,
    57,    58,    59,    60,    -1,    -1,    -1,    64,    54,    55,
    56,    57,    58,    59,    60,    -1,    -1,    -1,    64,    54,
    55,    56,    57,    58,    59,    60,    -1,    -1,    -1,    64,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
    54,    55,    56,    57,    58,    59,    60,    -1,    -1,    63
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/afs/bp.ncsu.edu/contrib/gnu/share/bison.simple"
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

#line 217 "/afs/bp.ncsu.edu/contrib/gnu/share/bison.simple"

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
#line 60 "parser.y"
{ the_program = reverse_list_of_nodes(yyvsp[0].node);
	  yyval.node = the_program; ;
    break;}
case 2:
#line 65 "parser.y"
{ yyval.node = node_create_string_constant(VARNAME_OPCODE, yyvsp[0].text); ;
    break;}
case 3:
#line 69 "parser.y"
{ yyval.node = node_create_string_constant(STRING_CONSTANT_OPCODE, yyvsp[0].text); ;
    break;}
case 4:
#line 73 "parser.y"
{ yyval.node = yyvsp[-1].node; ;
    break;}
case 5:
#line 76 "parser.y"
{ yyval.node = yyvsp[0].node; ;
    break;}
case 6:
#line 78 "parser.y"
{ yyval.node = node_create_string_constant(VARREF_OPCODE, yyvsp[0].text); ;
    break;}
case 7:
#line 81 "parser.y"
{ yyval.node = node_create_unary(NOT_OPCODE, yyvsp[0].node); ;
    break;}
case 8:
#line 84 "parser.y"
{ yyval.node = node_create_binary(PLUS_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 9:
#line 86 "parser.y"
{ yyval.node = node_create_binary(OR_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 10:
#line 88 "parser.y"
{ yyval.node = node_create_binary(AND_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 11:
#line 90 "parser.y"
{ yyval.node = node_create_binary(EQ_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 12:
#line 92 "parser.y"
{ yyval.node = node_create_binary(NEQ_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 13:
#line 94 "parser.y"
{ yyval.node = node_create_binary(REGEQ_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 14:
#line 96 "parser.y"
{ yyval.node = node_create_binary(REGNEQ_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 15:
#line 99 "parser.y"
{ yyval.node = node_create_noary(BUFFER_OPCODE); ;
    break;}
case 16:
#line 102 "parser.y"
{ yyval.node = node_create_unary(SUBSTITUTE_OPCODE, yyvsp[-1].node); ;
    break;}
case 17:
#line 104 "parser.y"
{ yyval.node = node_create_unary(PROTECT_OPCODE, yyvsp[-1].node); ;
    break;}
case 18:
#line 106 "parser.y"
{ yyval.node = node_create_unary(VERBATIM_OPCODE, yyvsp[-1].node); ;
    break;}
case 19:
#line 108 "parser.y"
{ yyval.node = node_create_unary(GETENV_OPCODE, yyvsp[-1].node); ;
    break;}
case 20:
#line 110 "parser.y"
{ yyval.node = node_create_unary(UPCASE_OPCODE, yyvsp[-1].node); ;
    break;}
case 21:
#line 112 "parser.y"
{ yyval.node = node_create_unary(DOWNCASE_OPCODE, yyvsp[-1].node); ;
    break;}
case 22:
#line 114 "parser.y"
{ yyval.node = node_create_unary(JVAR_OPCODE, yyvsp[-1].node); ;
    break;}
case 23:
#line 116 "parser.y"
{ yyval.node = node_create_unary(GET_OPCODE, yyvsp[-1].node); ;
    break;}
case 24:
#line 119 "parser.y"
{ yyval.node = node_create_binary(LANY_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 25:
#line 121 "parser.y"
{ yyval.node = node_create_binary(RANY_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 26:
#line 123 "parser.y"
{ yyval.node = node_create_binary(LBREAK_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 27:
#line 125 "parser.y"
{ yyval.node = node_create_binary(RBREAK_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 28:
#line 127 "parser.y"
{ yyval.node = node_create_binary(LSPAN_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 29:
#line 129 "parser.y"
{ yyval.node = node_create_binary(RSPAN_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 30:
#line 131 "parser.y"
{ yyval.node = node_create_binary(PARAGRAPH_OPCODE, yyvsp[-3].node, yyvsp[-1].node ); ;
    break;}
case 31:
#line 135 "parser.y"
{ yyval.node = node_create_noary(NOOP_OPCODE); ;
    break;}
case 32:
#line 137 "parser.y"
{ yyval.node = node_create_binary(SET_OPCODE, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 33:
#line 139 "parser.y"
{ yyval.node = node_create_unary(FIELDS_OPCODE,
				       reverse_list_of_nodes(yyvsp[0].node)); ;
    break;}
case 34:
#line 146 "parser.y"
{ yyval.node = node_create_unary(PRINT_OPCODE,
				       reverse_list_of_nodes(yyvsp[0].node)); ;
    break;}
case 35:
#line 149 "parser.y"
{ yyval.node = node_create_unary(PRINT_OPCODE,
		       node_create_unary(SUBSTITUTE_OPCODE,
			 node_create_string_constant(STRING_CONSTANT_OPCODE,
						     yyvsp[0].text))); ;
    break;}
case 36:
#line 154 "parser.y"
{ yyval.node = node_create_noary(CLEARBUF_OPCODE); ;
    break;}
case 37:
#line 160 "parser.y"
{ yyval.node = node_create_binary(APPENDPORT_OPCODE, yyvsp[-1].node, yyvsp[0].node); ;
    break;}
case 38:
#line 162 "parser.y"
{ yyvsp[-1].node->next = reverse_list_of_nodes(yyvsp[0].node);
		yyval.node = node_create_binary(EXECPORT_OPCODE, yyvsp[-2].node, yyvsp[-1].node); ;
    break;}
case 39:
#line 165 "parser.y"
{ yyval.node = node_create_binary(INPUTPORT_OPCODE, yyvsp[-1].node, yyvsp[0].node); ;
    break;}
case 40:
#line 167 "parser.y"
{ yyval.node = node_create_binary(OUTPUTPORT_OPCODE, yyvsp[-1].node, yyvsp[0].node); ;
    break;}
case 41:
#line 169 "parser.y"
{ yyval.node = node_create_binary(PUT_OPCODE, yyvsp[-1].node,
					reverse_list_of_nodes(yyvsp[0].node)); ;
    break;}
case 42:
#line 172 "parser.y"
{ yyval.node = node_create_binary(PUT_OPCODE, 0, 0); ;
    break;}
case 43:
#line 174 "parser.y"
{ yyval.node = node_create_unary(CLOSEINPUT_OPCODE, yyvsp[0].node); ;
    break;}
case 44:
#line 176 "parser.y"
{ yyval.node = node_create_unary(CLOSEOUTPUT_OPCODE, yyvsp[0].node); ;
    break;}
case 45:
#line 178 "parser.y"
{ yyval.node = node_create_unary(CLOSEPORT_OPCODE, yyvsp[0].node); ;
    break;}
case 46:
#line 184 "parser.y"
{ yyvsp[-1].node->next = reverse_list_of_nodes(yyvsp[0].node);
		yyval.node = node_create_unary(EXEC_OPCODE, yyvsp[-1].node); ;
    break;}
case 47:
#line 191 "parser.y"
{ Node *n = node_create_binary(IF_OPCODE, yyvsp[-4].node,
					     reverse_list_of_nodes(yyvsp[-2].node));
		n->next = yyvsp[-1].node;
		yyval.node = node_create_unary(IF_STMT_OPCODE, n); ;
    break;}
case 48:
#line 196 "parser.y"
{ yyval.node = node_create_binary(CASE_OPCODE, yyvsp[-2].node,
					reverse_list_of_nodes(yyvsp[-1].node)); ;
    break;}
case 49:
#line 199 "parser.y"
{ yyval.node = node_create_binary(WHILE_OPCODE, yyvsp[-3].node,
					reverse_list_of_nodes(yyvsp[-1].node)); ;
    break;}
case 50:
#line 202 "parser.y"
{ yyval.node = node_create_noary(BREAK_OPCODE); ;
    break;}
case 51:
#line 204 "parser.y"
{ yyval.node = node_create_noary(EXIT_OPCODE); ;
    break;}
case 52:
#line 208 "parser.y"
{ yyval.node = reverse_list_of_nodes(yyvsp[0].node); ;
    break;}
case 53:
#line 210 "parser.y"
{ yyval.node = node_create_binary(ELSE_OPCODE, 0,
					  reverse_list_of_nodes(yyvsp[0].node));
		  yyval.node->next = yyvsp[-2].node;
	          yyval.node = reverse_list_of_nodes(yyval.node); ;
    break;}
case 54:
#line 218 "parser.y"
{ yyval.node = 0; ;
    break;}
case 55:
#line 220 "parser.y"
{ yyval.node = node_create_binary(ELSEIF_OPCODE, yyvsp[-2].node,
					  reverse_list_of_nodes(yyvsp[0].node));
		  yyval.node->next = yyvsp[-4].node; ;
    break;}
case 56:
#line 226 "parser.y"
{ yyval.node = node_create_binary(MATCHLIST_OPCODE,
					  reverse_list_of_nodes(yyvsp[-1].node),
					  reverse_list_of_nodes(yyvsp[0].node)); ;
    break;}
case 57:
#line 230 "parser.y"
{ yyval.node = node_create_binary(DEFAULT_OPCODE, 0,
					  reverse_list_of_nodes(yyvsp[0].node)); ;
    break;}
case 58:
#line 245 "parser.y"
{ yyval.node = 0; ;
    break;}
case 59:
#line 247 "parser.y"
{ yyval.node = yyvsp[0].node;
	       yyval.node->next = yyvsp[-1].node; ;
    break;}
case 60:
#line 252 "parser.y"
{ yyval.node = yyvsp[0].node; ;
    break;}
case 61:
#line 254 "parser.y"
{ yyval.node = yyvsp[0].node;
		   yyval.node->next = yyvsp[-2].node; ;
    break;}
case 62:
#line 259 "parser.y"
{ yyval.node = 0; ;
    break;}
case 63:
#line 261 "parser.y"
{ yyval.node = yyvsp[0].node;
	       yyval.node->next = yyvsp[-1].node; ;
    break;}
case 64:
#line 266 "parser.y"
{ yyval.node = 0; ;
    break;}
case 65:
#line 268 "parser.y"
{ yyval.node = yyvsp[0].node;
		  yyval.node->next = yyvsp[-1].node; ;
    break;}
case 66:
#line 273 "parser.y"
{ yyval.node = 0; ;
    break;}
case 67:
#line 275 "parser.y"
{ yyval.node = yyvsp[0].node;
	  yyval.node->next = yyvsp[-1].node; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/afs/bp.ncsu.edu/contrib/gnu/share/bison.simple"

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
#line 279 "parser.y"


/*
 * error_occured - Set to true when a parse error is reported.  If it is false
 *                 at the time a parse error is reported, a message is
 *                 printed on stderr.  See report_parse_error for more
 *                 details.
 */

static int error_occured = 0;

/*
 *  Parser-Lexer Internal Routine:
 *
 *    void report_parse_error(char *error_message, int line_number)
 *        Modifies: error_occured, stderr
 *        Effects: This routine is called to report a parser or lexer
 *                 error.  Error_message is the error message and line_number
 *                 the line number it occured on.  The reported error message
 *                 is of the form "....<error_message> on line <line #>.\n".
 *                 This routine sets error_occured (local to parser.y) to
 *                 true.  If it was previously false, the error message
 *                 is reported to the user via stderr. 
 */

void report_parse_error(error_message, line_number)
     char *error_message;
     int line_number;
{
    if (error_occured)
      return;
    error_occured = 1;

    fprintf(stderr, "jwgc: error in description file: %s on line %d.\n",
	    error_message, line_number);
    fflush(stderr);
}

/*
 *  yyerror - internal routine - used by yacc to report syntax errors and
 *            stack overflow errors.
 */
 
static void yyerror(message)
     char *message;
{
    report_parse_error(message, yylineno);
}

/*
 *    struct _Node *parse_file(FILE *input_file)
 *        Requires: input_file is opened for reading, no pointers to
 *                  existing nodes will ever be dereferened.
 *        Modifies: *input_file, stderr, all existing nodes
 *        Effects: First this routine destroys all nodes.  Then it parses
 *                 input_file as a jwgc description langauge file.  If
 *                 an error is encountered, an error message is printed
 *                 on stderr and NULL is returned.  If no error is
 *                 encountered, a pointer to the node representation of
 *                 the parsed program is returned, suitable for passing to
 *                 exec.c.  Note that NULL will also be returned for a
 *                 empty file & is a valid program.  Either way, input_file
 *                 is closed before this routine returns.
 */

struct _Node *parse_file(input_file)
     FILE *input_file;
{
    the_program = NULL;
    error_occured = 0;
    node_DestroyAllNodes();

    lex_open(input_file);
    yyparse();
    fclose(input_file);

    if (error_occured) {
	node_DestroyAllNodes();
	the_program = NULL;
    }

    return(the_program);
}

struct _Node *parse_buffer(input_buffer)
     char *input_buffer;
{
    the_program = NULL;
    error_occured = 0;
    node_DestroyAllNodes();

    lex_open_buffer(input_buffer);
    yyparse();

    if (error_occured) {
	node_DestroyAllNodes();
	the_program = NULL;
    }

    return(the_program);
}
