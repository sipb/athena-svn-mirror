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
 *	$Id: cref.h,v 1.2 1999-01-22 23:13:33 ghudson Exp $
 */



#include <mit-copyright.h>
#include <sys/param.h>

/* General type definitions. */

typedef int (*PROC)();				/* A pointer to a function. */
typedef int ERRCODE;				/* An error code. */

/* Size variables. */

#define TITLE_SIZE	128		/* Size of a section title. */
#define	MAX_ENTRIES	100		/* Maximum number of entries.*/
#define	MAX_ABBREVS	100		/* Maximum number of abbreviations. */
#define LINE_LENGTH	200		/* Length of a line of text. */
#define LOG_LENGTH      500             /* Length of log string*/
#define	ERRSIZE		200		/* Size of an error message. */
#define MAX_INDEX_LINES	(LINES - 9)	/* Number of index lines. */
#define LOGIN_NAMESIZE  20              /* Maximum size of login name. */
/* The following are only used by the Motif client. */
#define  MAX_TITLES	25		/* Maximum number of titles. */
#define  SUBDIR		1		/* Subdirectory in menu. */
#define  PLAINFILE	0		/* Entry in menu. */

/* Structure describing a CREF entry. */

typedef struct tENTRY {
	int type;			/* Type of entry. */
	char title[TITLE_SIZE];		/* Title of entry. */
	char filename[MAXPATHLEN];	/* Filename for entry. */
	char formatter[LINE_LENGTH];	/* Text formatter to use. */
	char maintainer[MAXPATHLEN];	/* Maintainer of file. */
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
	char filename[MAXPATHLEN];	/* Filename belonging to abbrev. */
	} ABBREV;

/* Definitions still used by the Motif browser. */
#define STOCK_ROOT      "/mit/olc-stock/stock_answers"
#define STOCK_HEADER    "On-Line Consulting Browser"

/* If configuration fails. */
#define	FALLBACK_HEADER	"Generic Stock Answers Browser"
#define	FALLBACK_PROMPT	"browser> "	/* Prompt string. */

/* Default path/extension for the configuration files. */
#define BROWSER_CONFIG_PATH "/usr/athena/lib/olc:/mit/olc-stock/config:/mit/library/config"
#define BROWSER_CONFIG_EXT  ".cfg"

#define  CONTENTS	".index"	/* Where the contents are kept. */
#define GLOBAL_ABBREV	"cref_abbrevs"
#define USER_ABBREV	".crefrc"

/* Other important definitions. */

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
