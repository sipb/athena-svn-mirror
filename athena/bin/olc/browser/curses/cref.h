/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *      Permission to use, copy, modify, and distribute this program
 *      for any purpose and without fee is hereby granted, provided
 *      that this copyright and permission notice appear on all copies
 *      and supporting documentation, the name of M.I.T. not be used
 *      in advertising or publicity pertaining to distribution of the
 *      program without specific prior permission, and notice be given
 *      in supporting documentation that copying and distribution is
 *      by permission of M.I.T.  M.I.T. makes no representations about
 *      the suitability of this software for any purpose.  It is pro-
 *      vided "as is" without express or implied warranty.
 */


/* This file is part of the CREF finder.  It contains global variable
 * definitions.
 *
 *	$Source:
 *	$Author:
 *	$Header:
 */


#include <mit-copyright.h>

/* General type definitions. */

typedef int (*PROC)();				/* A pointer to a function. */
typedef int ERRCODE;				/* An error code. */

/* Size variables. */

#define TITLE_SIZE	128		/* Size of a section title. */
#define	FILENAME_SIZE	120		/* Size of a filename. */
#define	MAX_ENTRIES	100		/* Maximum number of entries.*/
#define	MAX_ABBREVS	100		/* Maximum number of abbreviations. */
#define LINE_LENGTH	200		/* Length of a line of text. */
#define LOG_LENGTH      500             /* Length of log string*/
#define	ERRSIZE		200		/* Size of an error message. */
#define MAX_INDEX_LINES	(LINES - 9)	/* Number of index lines. */
#define LOGIN_NAMESIZE  20              /* Maximum size of login name. */
/* Structure describing a CREF entry. */

typedef struct tENTRY {
	int type;			/* Type of entry. */
	char title[TITLE_SIZE];		/* Title of entry. */
	char filename[FILENAME_SIZE];	/* Filename for entry. */
	char formatter[LINE_LENGTH];	/* Text formatter to use. */
	char maintainer[FILENAME_SIZE];	/* Maintainer of file. */
	} ENTRY;

/* Structure describing a CREF command. */

typedef struct tCOMMAND {
	char command;				/* Command character. */
	PROC procedure;				/* Procedure to execute. */
	char *help_string;			/* Command help string. */
	} COMMAND;

/* Structure describing a CREF abbreviation. */

typedef struct tABBREV {
	char abbrev[LINE_LENGTH];	/* Abbrev. for a particular place. */
	char filename[FILENAME_SIZE];	/* Filename belonging to abbrev. */
	} ABBREV;

/* Where things are. */

#define	STOCK_ROOT	"/mit/olc-stock"
#define	CREF_ROOT	"/mit/cref/Ref"

#define	CONTENTS	".index"
#define GLOBAL_ABBREV	"cref_abbrevs"
#define USER_ABBREV	".crefrc"

/* Other important definitions. */

#define	STOCK_HEADER	"On-Line Consulting Browser"
#define	STOCK_PROMPT	"olc_browser> "	/* Prompt string. */

#define	CREF_HEADER	"Consultants' On-line Reference System"
#define	CREF_PROMPT	"cref> "	/* Prompt string. */


#define	CREF_ENTRY	"entry"		/* String for a file entry. */
#define CREF_SUBDIR	"directory"	/* String for a directory entry. */
#define	CREF_FILE	100		/* Code for a file entry. */
#define CREF_DIR	101		/* Code for a directory. */

#define	COMMENT_CHAR	'#'		/* Comment char in contents. */
#define CONTENTS_DELIM	':'		/* Delimiter in contents. */

#define CLOSED_FILE	0640		/* Limited access file */
#define CLOSED_DIR	0751		/* Limited access directory */
#define OPEN_FILE       0644            /* Open access file */
#define OPEN_DIR        0755            /* Open access directory */


/* Error Codes. */

#define	SUCCESS		0			/* Success! */
#define	ERROR		1			/* An error occurred. */
#define	PERM_DENIED	20			/* Permission denied. */

/* Function declarations. */

ENTRY *get_entry();
