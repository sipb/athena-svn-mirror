/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions common to all parts of OLC.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/macros.h,v $
 *      $Author: raeburn $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/macros.h,v 1.3 1990-01-04 14:37:04 raeburn Exp $
 */


/* 
 * Error codes
 */

#define SUCCESS          0
#define ERROR           -1
#define FAILURE         -2
#define ABORT           -3
#define FATAL           -4
#define NO_ACTION        1


/* 
 * Useful constants 
 */

#ifndef FALSE
#define FALSE            0
#endif  FALSE

#ifndef TRUE
#define TRUE             1
#endif  TRUE


/* 
 * Type declarations 
 */

typedef int     ERRCODE;        /* An error code. */
typedef ERRCODE (*FUNCTION)();  /* A pointer to a function. */
typedef int     RESPONSE;       /* A response code. */

/* Useful macros. */

#define string_eq(a,b)  (!strcmp((a),(b)))      /* Compare two strings.- */
#define string_equiv(a,b,c)   (!strncmp((a),(b),(c) < strlen((a)) \
					? strlen((a)) : (c)))		 

#ifndef min
#define min(a,b) a < b ? a : b
#endif  min

#ifndef max
#define max(a,b) a > b ? a : b
#endif max

#define is_option(r,option) r & option
#define set_option(r,option) r |= option
#define unset_option(r,option) r &= ~option

#define isme(r) string_eq(r->target.username, r->requester.username)

/* Size constants. */

#define STRING_LENGTH   16
#define LABEL_LENGTH    32
#define NAME_LENGTH     64      /* Maximum length for a filename. */
#define LINE_LENGTH     128     /* Maximum length for an input line. */
#define BUFSIZE         1024    /* Size of a message buffer. */
#define ERRSIZE         512     /* Size of an error message. */

/* the above names will be phased out */

#define LOGIN_SIZE      9       /* Length of a username, plus NULL. */
#define REALNAME_SIZE   40      /* Arbitrary length of a real name. */
#define IDENT_SIZE      REALNAME_SIZE + LOGIN_SIZE + LINE_LENGTH + 5  
                                /* Length of identification string. */
#define TOPIC_SIZE      24      /* Length of one-word topic. */
#define TIME_SIZE       30      /* Size of a time string. */
#define STATUS_SIZE     20
#define STRING_SIZE     16
#define LABEL_SIZE      32
#define NAME_SIZE       64      /* Maximum length for a filename. */
#define NOTE_SIZE       64
#define LINE_SIZE      128      /* Maximum length for an input line. */
#define ERROR_SIZE     512      /* Size of an error message. */
#define COMMENT_SIZE   512
#define BUF_SIZE      1024      /* Size of a message buffer. */

#define MAX_BYTES       512     /* Max. bytes to use on a socket. */
#define COMMAND_LENGTH  1000    /* Maximum length of command line. */
