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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: macros.h,v 1.14 1999-06-28 22:52:32 ghudson Exp $
 */

#ifndef OLC__OLC_MACROS_H
#define OLC__OLC_MACROS_H

#include <mit-copyright.h>
#include <olc/lang.h>

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
#endif  /* FALSE */

#ifndef TRUE
#define TRUE             1
#endif  /* TRUE */


/* 
 * Type declarations 
 */

typedef long     ERRCODE;        /* An error code. */

/* Useful macros. */

#define string_eq(a,b) (((a)[0]==(b)[0])?(!strcmp((a),(b))):FALSE)
/* Compare two strings.- */
#define string_equiv(a,b,c)   (!strncmp((a),(b),(c) < strlen((a)) \
					? strlen((a)) : (c)))

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif  /* MIN */

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif /* MAX */

#define is_option(r,option) r & option
#define set_option(r,option) r |= option
#define unset_option(r,option) r &= ~option

#define isme(r) string_eq(r->target.username, r->requester.username)

/* Size constants. */

#define LOGIN_SIZE      9       /* Length of a username, plus NULL. */
#define REALNAME_SIZE   40      /* Arbitrary length of a real name. */
#define IDENT_SIZE      REALNAME_SIZE + LOGIN_SIZE + LINE_LENGTH + 5  
                                /* Length of identification string. */
#define TOPIC_SIZE      24      /* Length of one-word topic. */
#define TIME_SIZE       30      /* Size of a time string. */
#define STATUS_SIZE     20
#define STRING_SIZE     16
#define TITLE_SIZE      32
#define NAME_SIZE       64      /* Maximum length for a filename. */
#define NOTE_SIZE       64
#define LINE_SIZE      128      /* Maximum length for an input line. */
#define HOSTNAME_SIZE  256	/* Maximum length of a fully-qual. hostname */
#define ERROR_SIZE     512      /* Size of an error message. */
#define COMMENT_SIZE   512
#define BUF_SIZE      1024      /* Size of a message buffer. */

#define MAX_BYTES       512     /* Max. bytes to use on a socket. */
#define COMMAND_LENGTH  1000    /* Maximum length of command line. */

#endif /* OLC__OLC_MACROS_H */
