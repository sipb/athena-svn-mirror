#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93 (BSDI)";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYEMPTY (-1)
#define YYLEX yylex()
#define yyclearin (yychar=YYEMPTY)
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 2 "ns_parser.y"
#if !defined(lint) && !defined(SABER)
static char rcsid[] = "$Id: ns_parser.c,v 1.1.1.1 1998-05-04 22:23:36 ghudson Exp $";
#endif /* not lint */

/*
 * Copyright (c) 1996, 1997 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Global C stuff goes here. */

#include "port_before.h"

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include <isc/eventlib.h>
#include <isc/logging.h>

#include "port_after.h"

#include "named.h"
#include "ns_parseutil.h"
#include "ns_lexer.h"

#define SYM_ZONE	0x010000
#define SYM_SERVER	0x020000
#define SYM_KEY		0x030000
#define SYM_ACL		0x040000
#define SYM_CHANNEL	0x050000
#define SYM_PORT	0x060000

#define SYMBOL_TABLE_SIZE 29989		/* should always be prime */
static symbol_table symtab;

#define AUTH_TABLE_SIZE 397		/* should always be prime */
static symbol_table authtab = NULL;

static zone_config current_zone;
static int seen_zone;

static options current_options;
static int seen_options;

static topology_config current_topology;
static int seen_topology;

static server_config current_server;
static int seen_server;

static char *current_algorithm;
static char *current_secret;

static log_config current_logging;
static int current_category;
static int chan_type;
static int chan_level;
static u_int chan_flags;
static int chan_facility;
static char *chan_name;
static int chan_versions;
static u_long chan_max_size;

static log_channel lookup_channel(char *);
static void define_channel(char *, log_channel);
static char *canonical_name(char *);

int yyparse();

#line 96 "ns_parser.y"
typedef union {
	char *			cp;
	int			s_int;
	long			num;
	u_long			ul_int;
	u_int16_t		us_int;
	struct in_addr		ip_addr;
	ip_match_element	ime;
	ip_match_list		iml;
	key_info		keyi;
	enum axfr_format	axfr_fmt;
} YYSTYPE;
#line 121 "y.tab.c"
#define L_EOS 257
#define L_IPADDR 258
#define L_NUMBER 259
#define L_STRING 260
#define L_QSTRING 261
#define L_END_INCLUDE 262
#define T_INCLUDE 263
#define T_OPTIONS 264
#define T_DIRECTORY 265
#define T_PIDFILE 266
#define T_NAMED_XFER 267
#define T_DUMP_FILE 268
#define T_STATS_FILE 269
#define T_FAKE_IQUERY 270
#define T_RECURSION 271
#define T_FETCH_GLUE 272
#define T_QUERY_SOURCE 273
#define T_LISTEN_ON 274
#define T_PORT 275
#define T_ADDRESS 276
#define T_DATASIZE 277
#define T_STACKSIZE 278
#define T_CORESIZE 279
#define T_DEFAULT 280
#define T_UNLIMITED 281
#define T_FILES 282
#define T_TRANSFERS_IN 283
#define T_TRANSFERS_OUT 284
#define T_TRANSFERS_PER_NS 285
#define T_TRANSFER_FORMAT 286
#define T_MAX_TRANSFER_TIME_IN 287
#define T_ONE_ANSWER 288
#define T_MANY_ANSWERS 289
#define T_NOTIFY 290
#define T_AUTH_NXDOMAIN 291
#define T_MULTIPLE_CNAMES 292
#define T_CLEAN_INTERVAL 293
#define T_INTERFACE_INTERVAL 294
#define T_STATS_INTERVAL 295
#define T_LOGGING 296
#define T_CATEGORY 297
#define T_CHANNEL 298
#define T_SEVERITY 299
#define T_DYNAMIC 300
#define T_FILE 301
#define T_VERSIONS 302
#define T_SIZE 303
#define T_SYSLOG 304
#define T_DEBUG 305
#define T_NULL_OUTPUT 306
#define T_PRINT_TIME 307
#define T_PRINT_CATEGORY 308
#define T_PRINT_SEVERITY 309
#define T_TOPOLOGY 310
#define T_SERVER 311
#define T_LONG_AXFR 312
#define T_BOGUS 313
#define T_TRANSFERS 314
#define T_KEYS 315
#define T_ZONE 316
#define T_IN 317
#define T_CHAOS 318
#define T_HESIOD 319
#define T_TYPE 320
#define T_MASTER 321
#define T_SLAVE 322
#define T_STUB 323
#define T_RESPONSE 324
#define T_HINT 325
#define T_MASTERS 326
#define T_ALSO_NOTIFY 327
#define T_ACL 328
#define T_ALLOW_UPDATE 329
#define T_ALLOW_QUERY 330
#define T_ALLOW_TRANSFER 331
#define T_SEC_KEY 332
#define T_ALGID 333
#define T_SECRET 334
#define T_CHECK_NAMES 335
#define T_WARN 336
#define T_FAIL 337
#define T_IGNORE 338
#define T_FORWARD 339
#define T_FORWARDERS 340
#define T_ONLY 341
#define T_FIRST 342
#define T_IF_NO_ANSWER 343
#define T_IF_NO_DOMAIN 344
#define T_YES 345
#define T_TRUE 346
#define T_NO 347
#define T_FALSE 348
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   25,   25,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   27,   34,   28,   35,   35,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   38,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,    5,    5,    4,
    4,    3,    3,   43,   44,   40,   40,   40,   40,    2,
    2,   23,   23,   23,   23,   23,   21,   21,   21,   22,
   22,   22,   37,   37,   37,   37,   41,   41,   41,   41,
   20,   20,   20,   20,   42,   42,   42,   39,   39,   45,
   45,   46,   47,   29,   48,   48,   48,   50,   49,   52,
   49,   54,   54,   54,   54,   55,   55,   56,   57,   57,
   57,   57,   57,   58,    9,    9,   10,   10,   59,   60,
   60,   60,   60,   60,   60,   60,   53,   53,   53,    8,
    8,   61,   51,   51,   51,    7,    7,    7,    6,   62,
   30,   63,   63,   64,   64,   64,   64,   64,   14,   14,
   12,   12,   11,   11,   11,   11,   11,   13,   17,   66,
   65,   65,   65,   67,   33,   68,   68,   68,   18,   19,
   32,   70,   31,   69,   69,   15,   15,   16,   16,   16,
   16,   71,   71,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   72,   72,   73,   73,   75,   74,   74,   76,
   76,   77,    1,   24,   24,
};
short yylen[] = {                                         2,
    1,    1,    2,    1,    2,    2,    2,    2,    2,    2,
    1,    2,    2,    3,    0,    5,    2,    3,    0,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    3,    5,    2,    0,    5,    2,    4,    4,    4,    1,
    1,    2,    2,    2,    2,    2,    1,    1,    1,    1,
    1,    1,    1,    2,    2,    1,    1,    2,    2,    0,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    2,    2,    2,    2,
    1,    1,    1,    1,    2,    2,    2,    0,    1,    2,
    3,    1,    0,    5,    2,    3,    1,    0,    6,    0,
    6,    1,    1,    2,    1,    2,    2,    2,    0,    1,
    1,    2,    2,    3,    1,    1,    0,    1,    2,    1,
    1,    1,    2,    2,    2,    2,    2,    3,    1,    1,
    1,    1,    2,    3,    1,    1,    1,    1,    1,    0,
    6,    2,    3,    2,    2,    2,    4,    1,    2,    3,
    1,    2,    1,    3,    3,    1,    3,    1,    1,    1,
    2,    3,    1,    0,    6,    2,    2,    1,    3,    3,
    5,    0,    5,    0,    3,    0,    1,    1,    1,    1,
    1,    2,    3,    2,    2,    4,    2,    4,    4,    4,
    2,    2,    4,    1,    2,    3,    1,    0,    1,    2,
    3,    1,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,   11,    0,   15,   93,    0,    0,    0,  164,    0,
    0,    2,    4,    0,    0,    0,    0,    0,    0,   12,
   13,    0,    0,    0,  140,    0,  204,  205,    0,    0,
    3,    5,    6,    7,    8,    9,   10,   14,    0,    0,
    0,  172,  177,    0,    0,   47,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   34,    0,    0,   40,
   41,   97,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  151,    0,  156,    0,  158,    0,   20,   22,
   21,   24,   23,   66,   62,   63,   64,   65,   25,   26,
   27,    0,    0,   36,    0,    0,    0,    0,   82,   83,
   84,   77,   81,   78,   79,   80,   85,   86,   87,   48,
   49,   42,   43,   28,   29,   30,   44,   45,   46,    0,
    0,    0,   67,   68,   69,    0,   73,   74,   75,   76,
   33,    0,   16,    0,   17,  137,  138,   98,  139,  136,
  131,  100,  130,   94,    0,   95,  148,    0,    0,    0,
    0,    0,    0,    0,  173,    0,    0,    0,  152,  149,
  171,    0,  168,    0,    0,    0,    0,    0,  203,   53,
   52,   55,   50,   51,   54,   58,   59,   61,    0,    0,
    0,    0,   70,   71,   72,   31,    0,   18,    0,    0,
   96,  146,  144,  145,    0,  141,    0,  142,  194,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  154,  155,  157,  150,    0,    0,  166,  167,  165,
    0,   39,   37,   38,   92,    0,    0,    0,    0,    0,
  163,  160,  159,    0,    0,  143,  191,  192,  185,  178,
  179,  181,  180,  184,    0,    0,    0,    0,    0,  187,
  175,    0,  182,  169,  170,   32,   35,    0,   90,  135,
  132,    0,    0,  129,    0,    0,    0,  122,    0,    0,
    0,    0,  120,  121,    0,  147,    0,  161,  197,    0,
    0,  202,    0,    0,    0,    0,    0,    0,  183,   91,
   99,    0,  133,  105,    0,  102,  123,    0,  116,  118,
  119,  115,  124,  125,  126,  101,    0,  127,  162,  186,
    0,  195,  193,    0,  200,  188,  189,  190,  134,  104,
    0,    0,    0,    0,  114,  128,  196,  201,  106,  107,
  108,  112,  113,
};
short yydgoto[] = {                                      10,
  191,  118,  192,  195,  132,  158,  159,  281,  320,  321,
   93,   94,   95,   96,   42,  264,  252,  186,  187,  122,
  146,  206,  109,   97,   11,   12,   13,   14,   15,   16,
   17,   18,   19,   23,   78,   79,  151,  152,  246,  114,
   80,   81,  115,  116,  247,  248,   24,   85,   86,  209,
  282,  210,  292,  317,  343,  344,  345,  293,  294,  295,
  283,   41,  172,  173,  254,  255,   30,  188,  175,   88,
  230,  231,  300,  303,  301,  304,  305,
};
short yysindex[] = {                                     89,
 -192,    0, -244,    0,    0, -210, -207, -176,    0,    0,
   89,    0,    0, -195, -188, -180, -168, -166, -164,    0,
    0, -162,  -93,  -90,    0, -176,    0,    0,  -63, -176,
    0,    0,    0,    0,    0,    0,    0,    0,  104, -231,
  -44,    0,    0,   22,  -26,    0, -160, -154, -152, -145,
 -143, -191, -191, -191, -108, -155,  -80,  -80,  -80,  -80,
 -135, -133, -131, -123, -129, -191, -191, -191, -127, -125,
 -117,  -24,   29,   36, -178, -205,    0,   28,  -84,    0,
    0,    0, -219, -214, -109,  -79, -227,   68,  150,  151,
   22,  -97,    0,  -55,    0,  -29,    0, -228,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -37,  -33,    0,  -72,  -70,  -46,   81,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   22,
   22,   22,    0,    0,    0, -264,    0,    0,    0,    0,
    0,   94,    0,  -39,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -38,    0,    0, -123, -191,  -35,
   98,  -87,  -12,  128,    0,   13,   14,  -25,    0,    0,
    0,   18,    0, -176, -176,  -58,  -86,  153,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   22,  -21,
  -10,   -6,    0,    0,    0,    0,   33,    0,  163,  164,
    0,    0,    0,    0, -203,    0,   35,    0,    0,   44,
 -191,   43, -211,  185,  186,  193,  194,  201, -264, -115,
   69,    0,    0,    0,    0,   70,   71,    0,    0,    0,
   -2,    0,    0,    0,    0,  200,   33,   72, -216,  124,
    0,    0,    0, -112,   73,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   74,   75,   22,   22,   22,    0,
    0,   78,    0,    0,    0,    0,    0,   79,    0,    0,
    0, -110,   80,    0, -197,   82, -224,    0, -191, -191,
 -191, -114,    0,    0,   83,    0,   84,    0,    0, -111,
   85,    0,  206,   75,   87,    2,    6,   10,    0,    0,
    0,   90,    0,    0,   91,    0,    0, -132,    0,    0,
    0,    0,    0,    0,    0,    0,   92,    0,    0,    0,
   97,    0,    0,   99,    0,    0,    0,    0,    0,    0,
 -225,  -80,   45,   37,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  346,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -99,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  100,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  232,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  100,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  105,  107,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  108,  109,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  236,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  254,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  267,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  136,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  276,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  145,    0,    0,  146,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  147,  149,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
short yygindex[] = {                                      0,
  290,    0,    0,    0,  240,    0,    0,  325,    0,    0,
  318,  -18,    0,  -59,    0,    0,    0,  224,  226,  -57,
    0,  184,  -47,   -8,    0,  405,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  341,    0,    0,    0,    0,
    0,    0,  304,  307,    0,  177,    0,    0,  342,    0,
    0,    0,    0,    0,   93,   95,    0,    0,    0,  134,
  154,    0,    0,  268,    0,  187,    0,    0,    0,    0,
    0,  212,    0,    0,  152,    0,  141,
};
#define YYTABLESIZE 463
short yytable[] = {                                      29,
  124,  125,  126,   92,  190,  110,  111,   92,  194,  271,
  326,   92,  296,  330,  311,  164,   22,   43,  134,  135,
  136,   45,   92,  176,   82,   91,   92,  183,  167,   39,
   92,  178,   40,  349,   92,   27,   28,  216,   92,  280,
   27,   28,   92,   27,   28,   27,   28,   25,  123,  123,
  123,  123,  251,   26,   92,  350,   27,   28,  168,   44,
  156,   32,   27,   28,   20,   83,   84,  104,   33,   21,
  157,  203,  204,  205,  160,  163,   34,  182,   87,  319,
  200,  201,  202,   27,   28,  169,  170,  171,   35,  161,
   36,  161,   37,   91,   38,  181,   98,   91,  140,  234,
   99,   91,  314,  242,  184,  185,  100,  315,  101,  260,
  261,  262,   91,  263,  243,  102,   91,  103,  244,  117,
   91,  213,  276,  127,   91,  128,  336,  129,   91,  133,
  337,  137,   91,  138,  338,  147,  148,  149,  150,  241,
  219,  139,  143,  144,   91,  145,  299,   27,   28,   27,
   28,  141,  153,  105,  106,  107,  108,  176,  142,  182,
   89,   90,   27,   28,  130,  131,  112,  113,  167,  341,
  342,  220,  155,  258,  221,  236,  237,  166,  119,   27,
   28,  182,  182,  182,  285,  222,  286,   83,   84,  287,
  174,  288,  289,  290,  291,  161,  176,  177,  168,  120,
  121,  180,  112,  199,  223,  113,  253,  306,  307,  308,
  224,  225,  189,  226,  227,  228,  207,  208,  211,  229,
  215,  189,  182,  214,  193,  169,  170,  171,   89,   90,
   27,   28,   89,   90,   27,   28,   89,   90,   27,   28,
  163,  323,  324,  325,  218,  253,  184,   89,   90,   27,
   28,   89,   90,   27,   28,   89,   90,   27,   28,   89,
   90,   27,   28,   89,   90,   27,   28,   89,   90,   27,
   28,  232,  233,  163,  235,  185,  316,  240,  322,   89,
   90,   27,   28,   46,  351,  249,  250,  182,  182,  182,
  245,  256,   47,   48,   49,   50,   51,   52,   53,   54,
   55,   56,  257,  259,   57,   58,   59,  265,  266,   60,
   61,   62,   63,   64,   65,  267,  268,   66,   67,   68,
   69,   70,   71,  269,  277,  273,  274,  275,  279,  298,
  333,  299,  302,  123,  309,  310,  313,   72,  341,  328,
  329,  332,  318,  335,    1,    1,  339,  342,  346,  340,
    2,    3,    4,  347,   60,  348,   19,   73,   74,   46,
   88,  174,   75,  153,   56,   57,   76,   77,   47,   48,
   49,   50,   51,   52,   53,   54,   55,   56,   89,  284,
   57,   58,   59,  219,    5,   60,   61,   62,   63,   64,
   65,  198,  117,   66,   67,   68,   69,   70,   71,    6,
  199,  103,  109,  110,    7,  111,  198,  212,  162,  179,
  239,  238,  270,   72,  220,   31,    8,  221,  154,  197,
    9,  196,  285,  278,  286,  327,  165,  287,  222,  288,
  289,  290,  291,   73,   74,  312,  353,  352,   75,  217,
  297,  272,   76,   77,  334,    0,    0,  223,    0,    0,
    0,  331,    0,  224,  225,    0,  226,  227,  228,    0,
    0,    0,  229,
};
short yycheck[] = {                                       8,
   58,   59,   60,   33,   42,   53,   54,   33,   42,  125,
  125,   33,  125,  125,  125,  125,  261,   26,   66,   67,
   68,   30,   33,  123,  256,  123,   33,  256,  256,  123,
   33,   91,  123,  259,   33,  260,  261,  125,   33,  256,
  260,  261,   33,  260,  261,  260,  261,  258,   57,   58,
   59,   60,  256,  261,   33,  281,  260,  261,  286,  123,
  280,  257,  260,  261,  257,  297,  298,  259,  257,  262,
  290,  336,  337,  338,   83,   84,  257,   96,  123,  304,
  140,  141,  142,  260,  261,  313,  314,  315,  257,  306,
  257,  306,  257,  123,  257,  125,  123,  123,  123,  125,
  261,  123,  300,  125,  333,  334,  261,  305,  261,  321,
  322,  323,  123,  325,  125,  261,  123,  261,  125,  275,
  123,  169,  125,  259,  123,  259,  125,  259,  123,  259,
  125,  259,  123,  259,  125,  341,  342,  343,  344,  199,
  256,  259,  321,  322,  123,  324,  258,  260,  261,  260,
  261,  123,  125,  345,  346,  347,  348,  257,  123,  178,
  258,  259,  260,  261,  288,  289,  275,  276,  256,  302,
  303,  287,  257,  221,  290,  184,  185,  257,  259,  260,
  261,  200,  201,  202,  299,  301,  301,  297,  298,  304,
  123,  306,  307,  308,  309,  306,   47,   47,  286,  280,
  281,  257,  275,  123,  320,  276,  215,  267,  268,  269,
  326,  327,  259,  329,  330,  331,  123,  257,  257,  335,
  123,  259,  241,  259,  258,  313,  314,  315,  258,  259,
  260,  261,  258,  259,  260,  261,  258,  259,  260,  261,
  249,  289,  290,  291,  257,  254,  333,  258,  259,  260,
  261,  258,  259,  260,  261,  258,  259,  260,  261,  258,
  259,  260,  261,  258,  259,  260,  261,  258,  259,  260,
  261,  259,  259,  282,  257,  334,  285,  125,  287,  258,
  259,  260,  261,  256,  342,  123,  123,  306,  307,  308,
  258,  257,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  259,  261,  277,  278,  279,  123,  123,  282,
  283,  284,  285,  286,  287,  123,  123,  290,  291,  292,
  293,  294,  295,  123,  125,  257,  257,  257,  257,  257,
  125,  258,  258,  342,  257,  257,  257,  310,  302,  257,
  257,  257,  261,  257,  256,    0,  257,  303,  257,  259,
  262,  263,  264,  257,  123,  257,  257,  330,  331,  256,
  125,  257,  335,  257,  257,  257,  339,  340,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  125,  256,
  277,  278,  279,  256,  296,  282,  283,  284,  285,  286,
  287,  125,  257,  290,  291,  292,  293,  294,  295,  311,
  125,  257,  257,  257,  316,  257,  117,  168,   84,   92,
  187,  186,  229,  310,  287,   11,  328,  290,   78,  116,
  332,  115,  299,  247,  301,  292,   85,  304,  301,  306,
  307,  308,  309,  330,  331,  282,  344,  343,  335,  172,
  254,  230,  339,  340,  304,   -1,   -1,  320,   -1,   -1,
   -1,  300,   -1,  326,  327,   -1,  329,  330,  331,   -1,
   -1,   -1,  335,
};
#define YYFINAL 10
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 348
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,0,0,0,0,"'*'",0,0,0,0,"'/'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"L_EOS",
"L_IPADDR","L_NUMBER","L_STRING","L_QSTRING","L_END_INCLUDE","T_INCLUDE",
"T_OPTIONS","T_DIRECTORY","T_PIDFILE","T_NAMED_XFER","T_DUMP_FILE",
"T_STATS_FILE","T_FAKE_IQUERY","T_RECURSION","T_FETCH_GLUE","T_QUERY_SOURCE",
"T_LISTEN_ON","T_PORT","T_ADDRESS","T_DATASIZE","T_STACKSIZE","T_CORESIZE",
"T_DEFAULT","T_UNLIMITED","T_FILES","T_TRANSFERS_IN","T_TRANSFERS_OUT",
"T_TRANSFERS_PER_NS","T_TRANSFER_FORMAT","T_MAX_TRANSFER_TIME_IN",
"T_ONE_ANSWER","T_MANY_ANSWERS","T_NOTIFY","T_AUTH_NXDOMAIN",
"T_MULTIPLE_CNAMES","T_CLEAN_INTERVAL","T_INTERFACE_INTERVAL",
"T_STATS_INTERVAL","T_LOGGING","T_CATEGORY","T_CHANNEL","T_SEVERITY",
"T_DYNAMIC","T_FILE","T_VERSIONS","T_SIZE","T_SYSLOG","T_DEBUG","T_NULL_OUTPUT",
"T_PRINT_TIME","T_PRINT_CATEGORY","T_PRINT_SEVERITY","T_TOPOLOGY","T_SERVER",
"T_LONG_AXFR","T_BOGUS","T_TRANSFERS","T_KEYS","T_ZONE","T_IN","T_CHAOS",
"T_HESIOD","T_TYPE","T_MASTER","T_SLAVE","T_STUB","T_RESPONSE","T_HINT",
"T_MASTERS","T_ALSO_NOTIFY","T_ACL","T_ALLOW_UPDATE","T_ALLOW_QUERY",
"T_ALLOW_TRANSFER","T_SEC_KEY","T_ALGID","T_SECRET","T_CHECK_NAMES","T_WARN",
"T_FAIL","T_IGNORE","T_FORWARD","T_FORWARDERS","T_ONLY","T_FIRST",
"T_IF_NO_ANSWER","T_IF_NO_DOMAIN","T_YES","T_TRUE","T_NO","T_FALSE",
};
char *yyrule[] = {
"$accept : config_file",
"config_file : statement_list",
"statement_list : statement",
"statement_list : statement_list statement",
"statement : include_stmt",
"statement : options_stmt L_EOS",
"statement : logging_stmt L_EOS",
"statement : server_stmt L_EOS",
"statement : zone_stmt L_EOS",
"statement : acl_stmt L_EOS",
"statement : key_stmt L_EOS",
"statement : L_END_INCLUDE",
"statement : error L_EOS",
"statement : error L_END_INCLUDE",
"include_stmt : T_INCLUDE L_QSTRING L_EOS",
"$$1 :",
"options_stmt : T_OPTIONS $$1 '{' options '}'",
"options : option L_EOS",
"options : options option L_EOS",
"option :",
"option : T_DIRECTORY L_QSTRING",
"option : T_NAMED_XFER L_QSTRING",
"option : T_PIDFILE L_QSTRING",
"option : T_STATS_FILE L_QSTRING",
"option : T_DUMP_FILE L_QSTRING",
"option : T_FAKE_IQUERY yea_or_nay",
"option : T_RECURSION yea_or_nay",
"option : T_FETCH_GLUE yea_or_nay",
"option : T_NOTIFY yea_or_nay",
"option : T_AUTH_NXDOMAIN yea_or_nay",
"option : T_MULTIPLE_CNAMES yea_or_nay",
"option : T_CHECK_NAMES check_names_type check_names_opt",
"option : T_LISTEN_ON maybe_port '{' address_match_list '}'",
"option : T_FORWARD forward_opt",
"$$2 :",
"option : T_FORWARDERS $$2 '{' opt_forwarders_list '}'",
"option : T_QUERY_SOURCE query_source",
"option : T_ALLOW_QUERY '{' address_match_list '}'",
"option : T_ALLOW_TRANSFER '{' address_match_list '}'",
"option : T_TOPOLOGY '{' address_match_list '}'",
"option : size_clause",
"option : transfer_clause",
"option : T_TRANSFER_FORMAT transfer_format",
"option : T_MAX_TRANSFER_TIME_IN L_NUMBER",
"option : T_CLEAN_INTERVAL L_NUMBER",
"option : T_INTERFACE_INTERVAL L_NUMBER",
"option : T_STATS_INTERVAL L_NUMBER",
"option : error",
"transfer_format : T_ONE_ANSWER",
"transfer_format : T_MANY_ANSWERS",
"maybe_wild_addr : L_IPADDR",
"maybe_wild_addr : '*'",
"maybe_wild_port : in_port",
"maybe_wild_port : '*'",
"query_source_address : T_ADDRESS maybe_wild_addr",
"query_source_port : T_PORT maybe_wild_port",
"query_source : query_source_address",
"query_source : query_source_port",
"query_source : query_source_address query_source_port",
"query_source : query_source_port query_source_address",
"maybe_port :",
"maybe_port : T_PORT in_port",
"yea_or_nay : T_YES",
"yea_or_nay : T_TRUE",
"yea_or_nay : T_NO",
"yea_or_nay : T_FALSE",
"yea_or_nay : L_NUMBER",
"check_names_type : T_MASTER",
"check_names_type : T_SLAVE",
"check_names_type : T_RESPONSE",
"check_names_opt : T_WARN",
"check_names_opt : T_FAIL",
"check_names_opt : T_IGNORE",
"forward_opt : T_ONLY",
"forward_opt : T_FIRST",
"forward_opt : T_IF_NO_ANSWER",
"forward_opt : T_IF_NO_DOMAIN",
"size_clause : T_DATASIZE size_spec",
"size_clause : T_STACKSIZE size_spec",
"size_clause : T_CORESIZE size_spec",
"size_clause : T_FILES size_spec",
"size_spec : any_string",
"size_spec : L_NUMBER",
"size_spec : T_DEFAULT",
"size_spec : T_UNLIMITED",
"transfer_clause : T_TRANSFERS_IN L_NUMBER",
"transfer_clause : T_TRANSFERS_OUT L_NUMBER",
"transfer_clause : T_TRANSFERS_PER_NS L_NUMBER",
"opt_forwarders_list :",
"opt_forwarders_list : forwarders_in_addr_list",
"forwarders_in_addr_list : forwarders_in_addr L_EOS",
"forwarders_in_addr_list : forwarders_in_addr_list forwarders_in_addr L_EOS",
"forwarders_in_addr : L_IPADDR",
"$$3 :",
"logging_stmt : T_LOGGING $$3 '{' logging_opts_list '}'",
"logging_opts_list : logging_opt L_EOS",
"logging_opts_list : logging_opts_list logging_opt L_EOS",
"logging_opts_list : error",
"$$4 :",
"logging_opt : T_CATEGORY category $$4 '{' channel_list '}'",
"$$5 :",
"logging_opt : T_CHANNEL channel_name $$5 '{' channel_opt_list '}'",
"channel_severity : any_string",
"channel_severity : T_DEBUG",
"channel_severity : T_DEBUG L_NUMBER",
"channel_severity : T_DYNAMIC",
"version_modifier : T_VERSIONS L_NUMBER",
"version_modifier : T_VERSIONS T_UNLIMITED",
"size_modifier : T_SIZE size_spec",
"maybe_file_modifiers :",
"maybe_file_modifiers : version_modifier",
"maybe_file_modifiers : size_modifier",
"maybe_file_modifiers : version_modifier size_modifier",
"maybe_file_modifiers : size_modifier version_modifier",
"channel_file : T_FILE L_QSTRING maybe_file_modifiers",
"facility_name : any_string",
"facility_name : T_SYSLOG",
"maybe_syslog_facility :",
"maybe_syslog_facility : facility_name",
"channel_syslog : T_SYSLOG maybe_syslog_facility",
"channel_opt : channel_file",
"channel_opt : channel_syslog",
"channel_opt : T_NULL_OUTPUT",
"channel_opt : T_SEVERITY channel_severity",
"channel_opt : T_PRINT_TIME yea_or_nay",
"channel_opt : T_PRINT_CATEGORY yea_or_nay",
"channel_opt : T_PRINT_SEVERITY yea_or_nay",
"channel_opt_list : channel_opt L_EOS",
"channel_opt_list : channel_opt_list channel_opt L_EOS",
"channel_opt_list : error",
"channel_name : any_string",
"channel_name : T_NULL_OUTPUT",
"channel : channel_name",
"channel_list : channel L_EOS",
"channel_list : channel_list channel L_EOS",
"channel_list : error",
"category_name : any_string",
"category_name : T_DEFAULT",
"category_name : T_NOTIFY",
"category : category_name",
"$$6 :",
"server_stmt : T_SERVER L_IPADDR $$6 '{' server_info_list '}'",
"server_info_list : server_info L_EOS",
"server_info_list : server_info_list server_info L_EOS",
"server_info : T_BOGUS yea_or_nay",
"server_info : T_TRANSFERS L_NUMBER",
"server_info : T_TRANSFER_FORMAT transfer_format",
"server_info : T_KEYS '{' key_list '}'",
"server_info : error",
"address_match_list : address_match_element L_EOS",
"address_match_list : address_match_list address_match_element L_EOS",
"address_match_element : address_match_simple",
"address_match_element : '!' address_match_simple",
"address_match_simple : L_IPADDR",
"address_match_simple : L_IPADDR '/' L_NUMBER",
"address_match_simple : L_NUMBER '/' L_NUMBER",
"address_match_simple : address_name",
"address_match_simple : '{' address_match_list '}'",
"address_name : any_string",
"key_ref : any_string",
"key_list_element : key_ref",
"key_list : key_list_element L_EOS",
"key_list : key_list key_list_element L_EOS",
"key_list : error",
"$$7 :",
"key_stmt : T_SEC_KEY $$7 any_string '{' key_definition '}'",
"key_definition : algorithm_id secret",
"key_definition : secret algorithm_id",
"key_definition : error",
"algorithm_id : T_ALGID any_string L_EOS",
"secret : T_SECRET any_string L_EOS",
"acl_stmt : T_ACL any_string '{' address_match_list '}'",
"$$8 :",
"zone_stmt : T_ZONE L_QSTRING optional_class $$8 optional_zone_options_list",
"optional_zone_options_list :",
"optional_zone_options_list : '{' zone_option_list '}'",
"optional_class :",
"optional_class : any_string",
"zone_type : T_MASTER",
"zone_type : T_SLAVE",
"zone_type : T_HINT",
"zone_type : T_STUB",
"zone_option_list : zone_option L_EOS",
"zone_option_list : zone_option_list zone_option L_EOS",
"zone_option : T_TYPE zone_type",
"zone_option : T_FILE L_QSTRING",
"zone_option : T_MASTERS '{' master_in_addr_list '}'",
"zone_option : T_CHECK_NAMES check_names_opt",
"zone_option : T_ALLOW_UPDATE '{' address_match_list '}'",
"zone_option : T_ALLOW_QUERY '{' address_match_list '}'",
"zone_option : T_ALLOW_TRANSFER '{' address_match_list '}'",
"zone_option : T_MAX_TRANSFER_TIME_IN L_NUMBER",
"zone_option : T_NOTIFY yea_or_nay",
"zone_option : T_ALSO_NOTIFY '{' opt_notify_in_addr_list '}'",
"zone_option : error",
"master_in_addr_list : master_in_addr L_EOS",
"master_in_addr_list : master_in_addr_list master_in_addr L_EOS",
"master_in_addr : L_IPADDR",
"opt_notify_in_addr_list :",
"opt_notify_in_addr_list : notify_in_addr_list",
"notify_in_addr_list : notify_in_addr L_EOS",
"notify_in_addr_list : notify_in_addr_list notify_in_addr L_EOS",
"notify_in_addr : L_IPADDR",
"in_port : L_NUMBER",
"any_string : L_STRING",
"any_string : L_QSTRING",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
struct yystack {
    short *ssp;
    YYSTYPE *vsp;
    short *ss;
    YYSTYPE *vs;
    int stacksize;
    short *sslim;
};
int yychar; /* some people use this, so we copy it in & out */
int yyerrflag; /* must be global for yyerrok & YYRECOVERING */
YYSTYPE yylval;
#line 1259 "ns_parser.y"

static char *
canonical_name(char *name) {
	char canonical[MAXDNAME];
	
	if (strlen(name) >= MAXDNAME)
		return (NULL);
	strcpy(canonical, name);
	if (makename(canonical, ".", sizeof canonical) < 0)
		return (NULL);
	return (savestr(canonical));
}

static void
init_acls() {
	ip_match_element ime;
	ip_match_list iml;
	struct in_addr address;

	/* Create the predefined ACLs */

	address.s_addr = 0U;

	/* ACL "any" */
	ime = new_ip_match_pattern(address, 0);
	iml = new_ip_match_list();
	add_to_ip_match_list(iml, ime);
	/* define_acl expects the name to be malloc'd memory */
	define_acl(savestr("any"), iml);

	/* ACL "none" */
	ime = new_ip_match_pattern(address, 0);
	ip_match_negate(ime);
	iml = new_ip_match_list();
	add_to_ip_match_list(iml, ime);
	define_acl(savestr("none"), iml);

	/* ACL "localhost" */
	ime = new_ip_match_localhost();
	iml = new_ip_match_list();
	add_to_ip_match_list(iml, ime);
	define_acl(savestr("localhost"), iml);

	/* ACL "localnets" */
	ime = new_ip_match_localnets();
	iml = new_ip_match_list();
	add_to_ip_match_list(iml, ime);
	define_acl(savestr("localnets"), iml);
}

static void
free_sym_value(int type, void *value) {
	ns_debug(ns_log_parser, 99, "free_sym_value: type %06x value %p",
		 type, value);
	type &= ~0xffff;
	switch (type) {
	case SYM_ACL:
		free_ip_match_list(value);
		break;
	case SYM_KEY:
		free_key_info(value);
		break;
	case SYM_CHANNEL:
		ns_panic(ns_log_parser, 1, "channel free in free_sym_value()");
	default:
		if (value != NULL)
			free(value);
	}
}

static log_channel
lookup_channel(char *name) {
	symbol_value value;

	if (lookup_symbol(symtab, name, SYM_CHANNEL, &value))
		return ((log_channel)(value.pointer));
	return (NULL);
}

static void
define_channel(char *name, log_channel channel) {
	symbol_value value;

	value.pointer = channel;  
	define_symbol(symtab, name, SYM_CHANNEL, value, SYMBOL_FREE_KEY);
}

static void
define_builtin_channels() {
	define_channel(savestr("default_syslog"), syslog_channel);
	define_channel(savestr("default_debug"), debug_channel);
	define_channel(savestr("default_stderr"), stderr_channel);
	define_channel(savestr("null"), null_channel);
}

static void
parser_initialize() {
	seen_options = 0;
	seen_topology = 0;
	symtab = new_symbol_table(SYMBOL_TABLE_SIZE, NULL);
	if (authtab != NULL)
		free_symbol_table(authtab);
	authtab = new_symbol_table(AUTH_TABLE_SIZE, free_sym_value);
	init_acls();
	define_builtin_channels();
}

static void
parser_cleanup() {
	if (symtab != NULL)
		free_symbol_table(symtab);
	symtab = NULL;
	/*
	 * We don't clean up authtab here because the ip_match_lists are in
	 * use.
	 */
}

/*
 * Public Interface
 */

ip_match_list
lookup_acl(char *name) {
	symbol_value value;

	if (lookup_symbol(authtab, name, SYM_ACL, &value))
		return ((ip_match_list)(value.pointer));
	return (NULL);
}

void
define_acl(char *name, ip_match_list iml) {
	symbol_value value;

	INSIST(name != NULL);
	INSIST(iml != NULL);

	value.pointer = iml;
	define_symbol(authtab, name, SYM_ACL, value,
		      SYMBOL_FREE_KEY|SYMBOL_FREE_VALUE);
	ns_debug(ns_log_parser, 7, "acl %s", name);
	dprint_ip_match_list(ns_log_parser, iml, 2, "allow ", "deny ");
}

key_info
lookup_key(char *name) {
	symbol_value value;

	if (lookup_symbol(authtab, name, SYM_KEY, &value))
		return ((key_info)(value.pointer));
	return (NULL);
}

void
define_key(char *name, key_info ki) {
	symbol_value value;

	INSIST(name != NULL);
	INSIST(ki != NULL);

	value.pointer = ki;
	define_symbol(authtab, name, SYM_KEY, value, SYMBOL_FREE_VALUE);
	dprint_key_info(ki);
}

void
parse_configuration(const char *filename) {
	FILE *config_stream;

	config_stream = fopen(filename, "r");
	if (config_stream == NULL)
		ns_panic(ns_log_parser, 0, "can't open '%s'", filename);

	lexer_initialize();
	parser_initialize();
	lexer_begin_file(filename, config_stream);
	(void)yyparse();
	lexer_end_file();
	parser_cleanup();
}
#line 942 "y.tab.c"
/* allocate initial stack */
#if defined(__STDC__) || defined(__cplusplus)
static int yyinitstack(struct yystack *sp)
#else
static int yyinitstack(sp)
    struct yystack *sp;
#endif
{
    int newsize;
    short *newss;
    YYSTYPE *newvs;

    newsize = YYINITSTACKSIZE;
    newss = (short *)malloc(newsize * sizeof *newss);
    newvs = (YYSTYPE *)malloc(newsize * sizeof *newvs);
    sp->ss = sp->ssp = newss;
    sp->vs = sp->vsp = newvs;
    if (newss == NULL || newvs == NULL) return -1;
    sp->stacksize = newsize;
    sp->sslim = newss + newsize - 1;
    return 0;
}

/* double stack size, up to YYMAXDEPTH */
#if defined(__STDC__) || defined(__cplusplus)
static int yygrowstack(struct yystack *sp)
#else
static int yygrowstack(sp)
    struct yystack *sp;
#endif
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = sp->stacksize) >= YYMAXDEPTH) return -1;
    if ((newsize *= 2) > YYMAXDEPTH) newsize = YYMAXDEPTH;
    i = sp->ssp - sp->ss;
    if ((newss = (short *)realloc(sp->ss, newsize * sizeof *newss)) == NULL)
        return -1;
    sp->ss = newss;
    sp->ssp = newss + i;
    if ((newvs = (YYSTYPE *)realloc(sp->vs, newsize * sizeof *newvs)) == NULL)
        return -1;
    sp->vs = newvs;
    sp->vsp = newvs + i;
    sp->stacksize = newsize;
    sp->sslim = newss + newsize - 1;
    return 0;
}

#define YYFREESTACK(sp) { free((sp)->ss); free((sp)->vs); }

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate, yych;
    register YYSTYPE *yyvsp;
    YYSTYPE yyval;
    struct yystack yystk;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = yych = YYEMPTY;

    if (yyinitstack(&yystk)) goto yyoverflow;
    *yystk.ssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yych < 0)
    {
        if ((yych = YYLEX) < 0) yych = 0;
        yychar = yych;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yych <= YYMAXTOKEN) yys = yyname[yych];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yych, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yych) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yych)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystk.ssp >= yystk.sslim && yygrowstack(&yystk))
            goto yyoverflow;
        *++yystk.ssp = yystate = yytable[yyn];
        *++yystk.vsp = yylval;
        yychar = yych = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yych) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yych)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystk.ssp]) &&
                    (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystk.ssp, yytable[yyn]);
#endif
                if (yystk.ssp >= yystk.sslim && yygrowstack(&yystk))
                    goto yyoverflow;
                *++yystk.ssp = yystate = yytable[yyn];
                *++yystk.vsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystk.ssp);
#endif
                if (yystk.ssp <= yystk.ss) goto yyabort;
                --yystk.ssp;
                --yystk.vsp;
            }
        }
    }
    else
    {
        if (yych == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yych <= YYMAXTOKEN) yys = yyname[yych];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yych, yys);
        }
#endif
        yychar = yych = YYEMPTY;
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyvsp = yystk.vsp; /* for speed in code under switch() */
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 205 "ns_parser.y"
{
		/* nothing */
	}
break;
case 14:
#line 226 "ns_parser.y"
{ lexer_begin_file(yyvsp[-1].cp, NULL); }
break;
case 15:
#line 234 "ns_parser.y"
{
		if (seen_options)
			parser_error(0, "cannot redefine options");
		current_options = new_options();
	}
break;
case 16:
#line 240 "ns_parser.y"
{
		if (!seen_options)
			set_options(current_options, 0);
		else
			free_options(current_options);
		current_options = NULL;
		seen_options = 1;
	}
break;
case 20:
#line 256 "ns_parser.y"
{
		if (current_options->directory != NULL)
			free(current_options->directory);
		current_options->directory = yyvsp[0].cp;
	}
break;
case 21:
#line 262 "ns_parser.y"
{
		if (current_options->named_xfer != NULL)
			free(current_options->named_xfer);
		current_options->named_xfer = yyvsp[0].cp;
	}
break;
case 22:
#line 268 "ns_parser.y"
{
		if (current_options->pid_filename != NULL)
			free(current_options->pid_filename);
		current_options->pid_filename = yyvsp[0].cp;
	}
break;
case 23:
#line 274 "ns_parser.y"
{
		if (current_options->stats_filename != NULL)
			free(current_options->stats_filename);
		current_options->stats_filename = yyvsp[0].cp;
	}
break;
case 24:
#line 280 "ns_parser.y"
{
		if (current_options->dump_filename != NULL)
			free(current_options->dump_filename);
		current_options->dump_filename = yyvsp[0].cp;
	}
break;
case 25:
#line 286 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_FAKE_IQUERY, yyvsp[0].num);
	}
break;
case 26:
#line 290 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_NORECURSE, !yyvsp[0].num);
	}
break;
case 27:
#line 294 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_NOFETCHGLUE, !yyvsp[0].num);
	}
break;
case 28:
#line 298 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_NONOTIFY, !yyvsp[0].num);
	}
break;
case 29:
#line 302 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_NONAUTH_NXDOMAIN,
				   !yyvsp[0].num);
	}
break;
case 30:
#line 307 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_MULTIPLE_CNAMES,
				   yyvsp[0].num);
	}
break;
case 31:
#line 312 "ns_parser.y"
{
		current_options->check_names[yyvsp[-1].s_int] = yyvsp[0].s_int;
	}
break;
case 32:
#line 316 "ns_parser.y"
{
		char port_string[10];
		symbol_value value;

		(void)sprintf(port_string, "%u", yyvsp[-3].us_int);
		if (lookup_symbol(symtab, port_string, SYM_PORT, NULL))
			parser_error(0,
				     "cannot redefine listen-on for port %u",
				     ntohs(yyvsp[-3].us_int));
		else {
			add_listen_on(current_options, yyvsp[-3].us_int, yyvsp[-1].iml);
			value.pointer = NULL;
			define_symbol(symtab, savestr(port_string), SYM_PORT,
				      value, SYMBOL_FREE_KEY);
		}

	}
break;
case 34:
#line 335 "ns_parser.y"
{
		if (current_options->fwdtab) {
			free_forwarders(current_options->fwdtab);
			current_options->fwdtab = NULL;
		}
	}
break;
case 37:
#line 344 "ns_parser.y"
{
		if (current_options->query_acl)
			free_ip_match_list(current_options->query_acl);
		current_options->query_acl = yyvsp[-1].iml;
	}
break;
case 38:
#line 350 "ns_parser.y"
{
		if (current_options->transfer_acl)
			free_ip_match_list(current_options->transfer_acl);
		current_options->transfer_acl = yyvsp[-1].iml;
	}
break;
case 39:
#line 356 "ns_parser.y"
{
		if (current_options->topology)
			free_ip_match_list(current_options->topology);
		current_options->topology = yyvsp[-1].iml;
	}
break;
case 40:
#line 362 "ns_parser.y"
{
		/* To get around the $$ = $1 default rule. */
	}
break;
case 42:
#line 367 "ns_parser.y"
{
		current_options->transfer_format = yyvsp[0].axfr_fmt;
	}
break;
case 43:
#line 371 "ns_parser.y"
{
		current_options->max_transfer_time_in = yyvsp[0].num * 60;
	}
break;
case 44:
#line 375 "ns_parser.y"
{
		current_options->clean_interval = yyvsp[0].num * 60;
	}
break;
case 45:
#line 379 "ns_parser.y"
{
		current_options->interface_interval = yyvsp[0].num * 60;
	}
break;
case 46:
#line 383 "ns_parser.y"
{
		current_options->stats_interval = yyvsp[0].num * 60;
	}
break;
case 48:
#line 390 "ns_parser.y"
{
		yyval.axfr_fmt = axfr_one_answer;
	}
break;
case 49:
#line 394 "ns_parser.y"
{
		yyval.axfr_fmt = axfr_many_answers;
	}
break;
case 50:
#line 399 "ns_parser.y"
{ yyval.ip_addr = yyvsp[0].ip_addr; }
break;
case 51:
#line 400 "ns_parser.y"
{ yyval.ip_addr.s_addr = htonl(INADDR_ANY); }
break;
case 52:
#line 403 "ns_parser.y"
{ yyval.us_int = yyvsp[0].us_int; }
break;
case 53:
#line 404 "ns_parser.y"
{ yyval.us_int = htons(0); }
break;
case 54:
#line 408 "ns_parser.y"
{
		current_options->query_source.sin_addr = yyvsp[0].ip_addr;
	}
break;
case 55:
#line 414 "ns_parser.y"
{
		current_options->query_source.sin_port = yyvsp[0].us_int;
	}
break;
case 60:
#line 425 "ns_parser.y"
{ yyval.us_int = htons(NS_DEFAULTPORT); }
break;
case 61:
#line 426 "ns_parser.y"
{ yyval.us_int = yyvsp[0].us_int; }
break;
case 62:
#line 430 "ns_parser.y"
{ 
		yyval.num = 1;	
	}
break;
case 63:
#line 434 "ns_parser.y"
{ 
		yyval.num = 1;	
	}
break;
case 64:
#line 438 "ns_parser.y"
{ 
		yyval.num = 0;	
	}
break;
case 65:
#line 442 "ns_parser.y"
{ 
		yyval.num = 0;	
	}
break;
case 66:
#line 446 "ns_parser.y"
{ 
		if (yyvsp[0].num == 1 || yyvsp[0].num == 0) {
			yyval.num = yyvsp[0].num;
		} else {
			parser_warning(0,
				       "number should be 0 or 1; assuming 1");
			yyval.num = 1;
		}
	}
break;
case 67:
#line 458 "ns_parser.y"
{
		yyval.s_int = primary_trans;
	}
break;
case 68:
#line 462 "ns_parser.y"
{
		yyval.s_int = secondary_trans;
	}
break;
case 69:
#line 466 "ns_parser.y"
{
		yyval.s_int = response_trans;
	}
break;
case 70:
#line 472 "ns_parser.y"
{
		yyval.s_int = warn;
	}
break;
case 71:
#line 476 "ns_parser.y"
{
		yyval.s_int = fail;
	}
break;
case 72:
#line 480 "ns_parser.y"
{
		yyval.s_int = ignore;
	}
break;
case 73:
#line 486 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_FORWARD_ONLY, 1);
	}
break;
case 74:
#line 490 "ns_parser.y"
{
		set_boolean_option(current_options, OPTION_FORWARD_ONLY, 0);
	}
break;
case 75:
#line 494 "ns_parser.y"
{
		parser_warning(0, "forward if-no-answer is unimplemented");
	}
break;
case 76:
#line 498 "ns_parser.y"
{
		parser_warning(0, "forward if-no-domain is unimplemented");
	}
break;
case 77:
#line 504 "ns_parser.y"
{
		current_options->data_size = yyvsp[0].ul_int;
	}
break;
case 78:
#line 508 "ns_parser.y"
{
		current_options->stack_size = yyvsp[0].ul_int;
	}
break;
case 79:
#line 512 "ns_parser.y"
{
		current_options->core_size = yyvsp[0].ul_int;
	}
break;
case 80:
#line 516 "ns_parser.y"
{
		current_options->files = yyvsp[0].ul_int;
	}
break;
case 81:
#line 522 "ns_parser.y"
{
		u_long result;

		if (unit_to_ulong(yyvsp[0].cp, &result))
			yyval.ul_int = result;
		else {
			parser_error(0, "invalid unit string '%s'", yyvsp[0].cp);
			/* 0 means "use default" */
			yyval.ul_int = 0;
		}
		free(yyvsp[0].cp);
	}
break;
case 82:
#line 535 "ns_parser.y"
{	
		yyval.ul_int = (u_long)yyvsp[0].num;
	}
break;
case 83:
#line 539 "ns_parser.y"
{
		yyval.ul_int = 0;
	}
break;
case 84:
#line 543 "ns_parser.y"
{
		yyval.ul_int = ULONG_MAX;
	}
break;
case 85:
#line 549 "ns_parser.y"
{
		current_options->transfers_in = (u_long) yyvsp[0].num;
	}
break;
case 86:
#line 553 "ns_parser.y"
{
		current_options->transfers_out = (u_long) yyvsp[0].num;
	}
break;
case 87:
#line 557 "ns_parser.y"
{
		current_options->transfers_per_ns = (u_long) yyvsp[0].num;
	}
break;
case 90:
#line 567 "ns_parser.y"
{
		/* nothing */
	}
break;
case 91:
#line 571 "ns_parser.y"
{
		/* nothing */
	}
break;
case 92:
#line 577 "ns_parser.y"
{
	  	add_forwarder(current_options, yyvsp[0].ip_addr);
	}
break;
case 93:
#line 587 "ns_parser.y"
{
		current_logging = begin_logging();
	}
break;
case 94:
#line 591 "ns_parser.y"
{
		end_logging(current_logging, 1);
	}
break;
case 98:
#line 602 "ns_parser.y"
{
		current_category = yyvsp[0].s_int;
	}
break;
case 100:
#line 607 "ns_parser.y"
{
		chan_type = log_null;
		chan_flags = 0;
		chan_level = log_info;
	}
break;
case 101:
#line 613 "ns_parser.y"
{
		log_channel current_channel = NULL;

		if (lookup_channel(yyvsp[-4].cp) != NULL) {
			parser_error(0, "can't redefine channel '%s'", yyvsp[-4].cp);
			free(yyvsp[-4].cp);
		} else {
			switch (chan_type) {
			case log_file:
				current_channel =
					log_new_file_channel(chan_flags,
							     chan_level,
							     chan_name, NULL,
							     chan_versions,
							     chan_max_size);
				break;
			case log_syslog:
				current_channel =
					log_new_syslog_channel(chan_flags,
							       chan_level,
							       chan_facility);
				break;
			case log_null:
				current_channel = log_new_null_channel();
				break;
			default:
				ns_panic(ns_log_parser, 1,
					 "unknown channel type: %d",
					 chan_type);
			}
			if (current_channel == NULL)
				ns_panic(ns_log_parser, 0,
					 "couldn't create channel");
			define_channel(yyvsp[-4].cp, current_channel);
		}
	}
break;
case 102:
#line 652 "ns_parser.y"
{
		symbol_value value;

		if (lookup_symbol(constants, yyvsp[0].cp, SYM_LOGGING, &value)) {
			chan_level = value.integer;
		} else {
			parser_error(0, "unknown severity '%s'", yyvsp[0].cp);
			chan_level = log_debug(99);
		}
		free(yyvsp[0].cp);
	}
break;
case 103:
#line 664 "ns_parser.y"
{
		chan_level = log_debug(1);
	}
break;
case 104:
#line 668 "ns_parser.y"
{
		chan_level = yyvsp[0].num;
	}
break;
case 105:
#line 672 "ns_parser.y"
{
		chan_level = 0;
		chan_flags |= LOG_USE_CONTEXT_LEVEL|LOG_REQUIRE_DEBUG;
	}
break;
case 106:
#line 679 "ns_parser.y"
{
		chan_versions = yyvsp[0].num;
		chan_flags |= LOG_TRUNCATE;
	}
break;
case 107:
#line 684 "ns_parser.y"
{
		chan_versions = LOG_MAX_VERSIONS;
		chan_flags |= LOG_TRUNCATE;
	}
break;
case 108:
#line 691 "ns_parser.y"
{
		chan_max_size = yyvsp[0].ul_int;
	}
break;
case 109:
#line 697 "ns_parser.y"
{
		chan_versions = 0;
		chan_max_size = ULONG_MAX;
	}
break;
case 110:
#line 702 "ns_parser.y"
{
		chan_max_size = ULONG_MAX;
	}
break;
case 111:
#line 706 "ns_parser.y"
{
		chan_versions = 0;
	}
break;
case 114:
#line 714 "ns_parser.y"
{
		chan_flags |= LOG_CLOSE_STREAM;
		chan_type = log_file;
		chan_name = yyvsp[-1].cp;
	}
break;
case 115:
#line 722 "ns_parser.y"
{ yyval.cp = yyvsp[0].cp; }
break;
case 116:
#line 723 "ns_parser.y"
{ yyval.cp = savestr("syslog"); }
break;
case 117:
#line 726 "ns_parser.y"
{ yyval.s_int = LOG_DAEMON; }
break;
case 118:
#line 728 "ns_parser.y"
{
		symbol_value value;

		if (lookup_symbol(constants, yyvsp[0].cp, SYM_SYSLOG, &value)) {
			yyval.s_int = value.integer;
		} else {
			parser_error(0, "unknown facility '%s'", yyvsp[0].cp);
			yyval.s_int = LOG_DAEMON;
		}
		free(yyvsp[0].cp);
	}
break;
case 119:
#line 742 "ns_parser.y"
{
		chan_type = log_syslog;
		chan_facility = yyvsp[0].s_int;
	}
break;
case 120:
#line 748 "ns_parser.y"
{ /* nothing to do */ }
break;
case 121:
#line 749 "ns_parser.y"
{ /* nothing to do */ }
break;
case 122:
#line 751 "ns_parser.y"
{
		chan_type = log_null;
	}
break;
case 123:
#line 754 "ns_parser.y"
{ /* nothing to do */ }
break;
case 124:
#line 756 "ns_parser.y"
{
		if (yyvsp[0].num)
			chan_flags |= LOG_TIMESTAMP;
		else
			chan_flags &= ~LOG_TIMESTAMP;
	}
break;
case 125:
#line 763 "ns_parser.y"
{
		if (yyvsp[0].num)
			chan_flags |= LOG_PRINT_CATEGORY;
		else
			chan_flags &= ~LOG_PRINT_CATEGORY;
	}
break;
case 126:
#line 770 "ns_parser.y"
{
		if (yyvsp[0].num)
			chan_flags |= LOG_PRINT_LEVEL;
		else
			chan_flags &= ~LOG_PRINT_LEVEL;
	}
break;
case 131:
#line 784 "ns_parser.y"
{ yyval.cp = savestr("null"); }
break;
case 132:
#line 788 "ns_parser.y"
{
		log_channel channel;
		symbol_value value;

		if (current_category >= 0) {
			channel = lookup_channel(yyvsp[0].cp);
			if (channel != NULL) {
				add_log_channel(current_logging,
						current_category, channel);
			} else
				parser_error(0, "unknown channel '%s'", yyvsp[0].cp);
		}
		free(yyvsp[0].cp);
	}
break;
case 137:
#line 810 "ns_parser.y"
{ yyval.cp = savestr("default"); }
break;
case 138:
#line 811 "ns_parser.y"
{ yyval.cp = savestr("notify"); }
break;
case 139:
#line 815 "ns_parser.y"
{
		symbol_value value;

		if (lookup_symbol(constants, yyvsp[0].cp, SYM_CATEGORY, &value))
			yyval.s_int = value.integer;
		else {
			parser_error(0, "invalid logging category '%s'",
				     yyvsp[0].cp);
			yyval.s_int = -1;
		}
		free(yyvsp[0].cp);
	}
break;
case 140:
#line 834 "ns_parser.y"
{
		char *ip_printable;
		symbol_value value;
		
		ip_printable = inet_ntoa(yyvsp[0].ip_addr);
		value.pointer = NULL;
		if (lookup_symbol(symtab, ip_printable, SYM_SERVER, NULL))
			seen_server = 1;
		else
			seen_server = 0;
		if (seen_server)
			parser_error(0, "cannot redefine server '%s'", 
				     ip_printable);
		else
			define_symbol(symtab, savestr(ip_printable),
				      SYM_SERVER, value,
				      SYMBOL_FREE_KEY);
		current_server = begin_server(yyvsp[0].ip_addr);
	}
break;
case 141:
#line 854 "ns_parser.y"
{
		end_server(current_server, !seen_server);
	}
break;
case 144:
#line 864 "ns_parser.y"
{
		set_server_option(current_server, SERVER_INFO_BOGUS, yyvsp[0].num);
	}
break;
case 145:
#line 868 "ns_parser.y"
{
		set_server_transfers(current_server, (int)yyvsp[0].num);
	}
break;
case 146:
#line 872 "ns_parser.y"
{
		set_server_transfer_format(current_server, yyvsp[0].axfr_fmt);
	}
break;
case 149:
#line 884 "ns_parser.y"
{
		ip_match_list iml;
		
		iml = new_ip_match_list();
		if (yyvsp[-1].ime != NULL)
			add_to_ip_match_list(iml, yyvsp[-1].ime);
		yyval.iml = iml;
	}
break;
case 150:
#line 893 "ns_parser.y"
{
		if (yyvsp[-1].ime != NULL)
			add_to_ip_match_list(yyvsp[-2].iml, yyvsp[-1].ime);
		yyval.iml = yyvsp[-2].iml;
	}
break;
case 152:
#line 902 "ns_parser.y"
{
		if (yyvsp[0].ime != NULL)
			ip_match_negate(yyvsp[0].ime);
		yyval.ime = yyvsp[0].ime;
	}
break;
case 153:
#line 910 "ns_parser.y"
{
		yyval.ime = new_ip_match_pattern(yyvsp[0].ip_addr, 32);
	}
break;
case 154:
#line 914 "ns_parser.y"
{
		if (yyvsp[0].num < 0 || yyvsp[0].num > 32) {
			parser_error(0, "mask bits out of range; skipping");
			yyval.ime = NULL;
		} else {
			yyval.ime = new_ip_match_pattern(yyvsp[-2].ip_addr, yyvsp[0].num);
			if (yyval.ime == NULL)
				parser_error(0, 
					   "address/mask mismatch; skipping");
		}
	}
break;
case 155:
#line 926 "ns_parser.y"
{
		struct in_addr ia;

		if (yyvsp[-2].num > 255) {
			parser_error(0, "address out of range; skipping");
			yyval.ime = NULL;
		} else {
			if (yyvsp[0].num < 0 || yyvsp[0].num > 32) {
				parser_error(0,
					"mask bits out of range; skipping");
					yyval.ime = NULL;
			} else {
				ia.s_addr = htonl((yyvsp[-2].num & 0xff) << 24);
				yyval.ime = new_ip_match_pattern(ia, yyvsp[0].num);
				if (yyval.ime == NULL)
					parser_error(0, 
					   "address/mask mismatch; skipping");
			}
		}
	}
break;
case 157:
#line 948 "ns_parser.y"
{
		char name[256];

		/*
		 * We want to be able to clean up this iml later so
		 * we give it a name and treat it like any other acl.
		 */
		sprintf(name, "__internal_%p", yyvsp[-1].iml);
		define_acl(savestr(name), yyvsp[-1].iml);
  		yyval.ime = new_ip_match_indirect(yyvsp[-1].iml);
	}
break;
case 158:
#line 962 "ns_parser.y"
{
		ip_match_list iml;

		iml = lookup_acl(yyvsp[0].cp);
		if (iml == NULL) {
			parser_error(0, "unknown ACL '%s'", yyvsp[0].cp);
			yyval.ime = NULL;
		} else
			yyval.ime = new_ip_match_indirect(iml);
		free(yyvsp[0].cp);
	}
break;
case 159:
#line 980 "ns_parser.y"
{
		key_info ki;

		ki = lookup_key(yyvsp[0].cp);
		if (ki == NULL) {
			parser_error(0, "unknown key '%s'", yyvsp[0].cp);
			yyval.keyi = NULL;
		} else
			yyval.keyi = ki;
		free(yyvsp[0].cp);
	}
break;
case 160:
#line 994 "ns_parser.y"
{
		if (yyvsp[0].keyi == NULL)
			parser_error(0, "empty key not added to server list ");
		else
			add_server_key_info(current_server, yyvsp[0].keyi);
	}
break;
case 164:
#line 1008 "ns_parser.y"
{
		current_algorithm = NULL;
		current_secret = NULL;
	}
break;
case 165:
#line 1013 "ns_parser.y"
{
		key_info ki;

		if (lookup_key(yyvsp[-3].cp) != NULL) {
			parser_error(0, "can't redefine key '%s'", yyvsp[-3].cp);
			free(yyvsp[-3].cp);
		} else {
			if (current_algorithm == NULL ||
			    current_secret == NULL)
				parser_error(0, "skipping bad key '%s'", yyvsp[-3].cp);
			else {
				ki = new_key_info(yyvsp[-3].cp, current_algorithm,
						  current_secret);
				define_key(yyvsp[-3].cp, ki);
			}
		}
	}
break;
case 166:
#line 1033 "ns_parser.y"
{
		current_algorithm = yyvsp[-1].cp;
		current_secret = yyvsp[0].cp;
	}
break;
case 167:
#line 1038 "ns_parser.y"
{
		current_algorithm = yyvsp[0].cp;
		current_secret = yyvsp[-1].cp;
	}
break;
case 168:
#line 1043 "ns_parser.y"
{
		current_algorithm = NULL;
		current_secret = NULL;
	}
break;
case 169:
#line 1049 "ns_parser.y"
{ yyval.cp = yyvsp[-1].cp; }
break;
case 170:
#line 1052 "ns_parser.y"
{ yyval.cp = yyvsp[-1].cp; }
break;
case 171:
#line 1060 "ns_parser.y"
{
		if (lookup_acl(yyvsp[-3].cp) != NULL) {
			parser_error(0, "can't redefine ACL '%s'", yyvsp[-3].cp);
			free(yyvsp[-3].cp);
		} else
			define_acl(yyvsp[-3].cp, yyvsp[-1].iml);
	}
break;
case 172:
#line 1074 "ns_parser.y"
{
		int sym_type;
		symbol_value value;
		char *zone_name;

		if (!seen_options)
			parser_error(0,
             "no options statement before first zone; using previous/default");
		sym_type = SYM_ZONE | (yyvsp[0].num & 0xffff);
		value.pointer = NULL;
		zone_name = canonical_name(yyvsp[-1].cp);
		if (zone_name == NULL) {
			parser_error(0, "can't make zone name '%s' canonical",
				     yyvsp[-1].cp);
			seen_zone = 1;
			zone_name = savestr("__bad_zone__");
		} else {
			seen_zone = lookup_symbol(symtab, zone_name, sym_type,
						  NULL);
			if (seen_zone) {
				parser_error(0,
					"cannot redefine zone '%s' class %d",
					     zone_name, yyvsp[0].num);
			} else
				define_symbol(symtab, zone_name, sym_type,
					      value, 0);
		}
		free(yyvsp[-1].cp);
		current_zone = begin_zone(zone_name, yyvsp[0].num); 
	}
break;
case 173:
#line 1105 "ns_parser.y"
{ end_zone(current_zone, !seen_zone); }
break;
case 176:
#line 1113 "ns_parser.y"
{
		yyval.num = C_IN;
	}
break;
case 177:
#line 1117 "ns_parser.y"
{
		symbol_value value;

		if (lookup_symbol(constants, yyvsp[0].cp, SYM_CLASS, &value))
			yyval.num = value.integer;
		else {
			/* the zone validator will give the error */
			yyval.num = C_NONE;
		}
		free(yyvsp[0].cp);
	}
break;
case 178:
#line 1131 "ns_parser.y"
{
		yyval.s_int = Z_MASTER;
	}
break;
case 179:
#line 1135 "ns_parser.y"
{
		yyval.s_int = Z_SLAVE;
	}
break;
case 180:
#line 1139 "ns_parser.y"
{
		yyval.s_int = Z_HINT;
	}
break;
case 181:
#line 1143 "ns_parser.y"
{
		yyval.s_int = Z_STUB;
	}
break;
case 184:
#line 1153 "ns_parser.y"
{
		if (!set_zone_type(current_zone, yyvsp[0].s_int))
			parser_warning(0, "zone type already set; skipping");
	}
break;
case 185:
#line 1158 "ns_parser.y"
{
		if (!set_zone_filename(current_zone, yyvsp[0].cp))
			parser_warning(0,
				       "zone filename already set; skipping");
	}
break;
case 187:
#line 1165 "ns_parser.y"
{
		if (!set_zone_checknames(current_zone, yyvsp[0].s_int))
			parser_warning(0,
	                              "zone checknames already set; skipping");
	}
break;
case 188:
#line 1171 "ns_parser.y"
{
		if (!set_zone_update_acl(current_zone, yyvsp[-1].iml))
			parser_warning(0,
				      "zone update acl already set; skipping");
	}
break;
case 189:
#line 1177 "ns_parser.y"
{
		if (!set_zone_query_acl(current_zone, yyvsp[-1].iml))
			parser_warning(0,
				      "zone query acl already set; skipping");
	}
break;
case 190:
#line 1183 "ns_parser.y"
{
		if (!set_zone_transfer_acl(current_zone, yyvsp[-1].iml))
			parser_warning(0,
				    "zone transfer acl already set; skipping");
	}
break;
case 191:
#line 1189 "ns_parser.y"
{
		if (!set_zone_transfer_time_in(current_zone, yyvsp[0].num*60))
			parser_warning(0,
		       "zone max transfer time (in) already set; skipping");
	}
break;
case 192:
#line 1195 "ns_parser.y"
{
		set_zone_notify(current_zone, yyvsp[0].num);
	}
break;
case 195:
#line 1203 "ns_parser.y"
{
		/* nothing */
	}
break;
case 196:
#line 1207 "ns_parser.y"
{
		/* nothing */
	}
break;
case 197:
#line 1213 "ns_parser.y"
{
	  	add_zone_master(current_zone, yyvsp[0].ip_addr);
	}
break;
case 200:
#line 1223 "ns_parser.y"
{
		/* nothing */
	}
break;
case 201:
#line 1227 "ns_parser.y"
{
		/* nothing */
	}
break;
case 202:
#line 1233 "ns_parser.y"
{
	  	add_zone_notify(current_zone, yyvsp[0].ip_addr);
	}
break;
case 203:
#line 1243 "ns_parser.y"
{
		if (yyvsp[0].num < 0 || yyvsp[0].num > 65535) {
		  	parser_warning(0, 
			  "invalid IP port number '%d'; setting port to 0",
			               yyvsp[0].num);
			yyvsp[0].num = 0;
		} else
			yyval.us_int = htons(yyvsp[0].num);
	}
break;
#line 2266 "y.tab.c"
    }
    yystk.ssp -= yym;
    yystate = *yystk.ssp;
    yystk.vsp -= yym;
    yym = yylhs[yyn];
    yych = yychar;
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystk.ssp = YYFINAL;
        *++yystk.vsp = yyval;
        if (yych < 0)
        {
            if ((yych = YYLEX) < 0) yych = 0;
            yychar = yych;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yych <= YYMAXTOKEN) yys = yyname[yych];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yych, yys);
            }
#endif
        }
        if (yych == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystk.ssp, yystate);
#endif
    if (yystk.ssp >= yystk.sslim && yygrowstack(&yystk))
        goto yyoverflow;
    *++yystk.ssp = yystate;
    *++yystk.vsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    YYFREESTACK(&yystk);
    return (1);
yyaccept:
    YYFREESTACK(&yystk);
    return (0);
}
