/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.h,v $
 *	$Author: treese $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref.h,v 1.1 1986-01-18 18:09:46 treese Exp $
 */

/* This file is part of the CREF finder.  It contains general definitions.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 *
 */

/* General type definitions. */

typedef int (*PROC)();				/* A pointer to a function. */
typedef int ERRCODE;				/* An error code. */

/* Size variables. */

#define TITLE_SIZE	128			/* Size of a section title. */
#define	FILENAME_SIZE	80			/* Size of a filename. */
#define	MAX_ENTRIES	100			/* Maximum number of entries.*/
#define LINE_LENGTH	80			/* Length of a line of text. */
#define	ERRSIZE		200			/* Size of an error message. */
#define MAX_INDEX_LINES	(LINES - 12)		/* Number of index lines. */

/* Structure describing a CREF entry. */

typedef struct tENTRY {
	int type;				/* Type of entry. */
	char title[TITLE_SIZE];			/* Title of entry. */
	char filename[FILENAME_SIZE];		/* Filename for entry. */
	} ENTRY;

/* Structure describing a CREF command. */

typedef struct tCOMMAND {
	char command;				/* Command character. */
	PROC procedure;				/* Procedure to execute. */
	char *help_string;			/* Command help string. */
	} COMMAND;

/* Where things are. */

#define	ROOT_DIR	"/mit/c/r/cref"
#define	CONTENTS	"Contents.index"
#define MANUAL_DIR	"Crm"

/* Other important definitions. */

#define	CREF_HEADER	"Consultants On-line Reference System"
#define	CREF_PROMPT	"ufi> "			/* Prompt string. */

#define	CREF_ENTRY	"entry"			/* String for a file entry. */
#define	CREF_FILE	100			/* Code for a file entry. */
#define CREF_DIR	101			/* Code for a directory. */

#define	COMMENT_CHAR	'#'			/* Comment char in contents. */
#define CONTENTS_DELIM	':'			/* Delimiter in contents. */

#define	CONSULT_GROUP	"consult"		/* Consultants group. */
#define	ALT_GROUP	"econsult"		/* Alternate group. */

/* Error Codes. */

#define	SUCCESS		0			/* Success! */
#define	ERROR		1			/* An error occurred. */
#define	PERM_DENIED	20			/* Permission denied. */

/* Function declarations. */

ENTRY *get_entry();
