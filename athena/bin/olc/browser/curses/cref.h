/* This file is part of the CREF finder.  It contains general definitions.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.h,v $
 *	$Author: treese $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.h,v 1.3 1986-01-25 15:08:49 treese Exp $
 */


/* General type definitions. */

typedef int (*PROC)();				/* A pointer to a function. */
typedef int ERRCODE;				/* An error code. */

/* Size variables. */

#define TITLE_SIZE	128		/* Size of a section title. */
#define	FILENAME_SIZE	120		/* Size of a filename. */
#define	MAX_ENTRIES	100		/* Maximum number of entries.*/
#define	MAX_ABBREVS	100		/* Maximum number of abbreviations. */
#define LINE_LENGTH	200		/* Length of a line of text. */
#define	ERRSIZE		200		/* Size of an error message. */
#define MAX_INDEX_LINES	(LINES - 12)	/* Number of index lines. */

/* Structure describing a CREF entry. */

typedef struct tENTRY {
	int type;			/* Type of entry. */
	char title[TITLE_SIZE];		/* Title of entry. */
	char filename[FILENAME_SIZE];	/* Filename for entry. */
	char formatter[LINE_LENGTH];	/* Text formatter to use. */
	char spare[FILENAME_SIZE];	/* Extra space. */
	} ENTRY;

/* Structure describing a CREF command. */

typedef struct tCOMMAND {
	char command;				/* Command character. */
	PROC procedure;				/* Procedure to execute. */
	char *help_string;			/* Command help string. */
	} COMMAND;

/* Structure describing a CREF abbreviation. */

typedef struct tABBREV {
	char label[LINE_LENGTH];	/* Label for a particular place. */
	char filename[FILENAME_SIZE];	/* Filename belonging to label. */
	} ABBREV;

/* Where things are. */

#define	CREF_ROOT	"/mit/c/r/cref/Ref"
#define	CONTENTS	"Contents.index"
#define GLOBAL_ABBREV	"cref_abbrevs"
#define USER_ABBREV	".crefrc"

/* Other important definitions. */

#define	CREF_HEADER	"Consultant's On-line Reference System"
#define	CREF_PROMPT	"cref> "	/* Prompt string. */

#define	CREF_ENTRY	"entry"		/* String for a file entry. */
#define	CREF_FILE	100		/* Code for a file entry. */
#define CREF_DIR	101		/* Code for a directory. */

#define	COMMENT_CHAR	'#'		/* Comment char in contents. */
#define CONTENTS_DELIM	':'		/* Delimiter in contents. */

#define	CONSULT_GROUP	"consult"	/* Consultants group. */

/* Error Codes. */

#define	SUCCESS		0			/* Success! */
#define	ERROR		1			/* An error occurred. */
#define	PERM_DENIED	20			/* Permission denied. */

/* Function declarations. */

ENTRY *get_entry();
