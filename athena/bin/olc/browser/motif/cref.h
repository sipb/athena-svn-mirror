/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>
#include <sys/param.h>

/* Structure describing a CREF entry. */

#define  TITLE_SIZE	128		/* Size of a section title. */
#define  MAX_TITLES	25		/* Maximum number of titles. */
#define  LINE_LENGTH	200		/* Length of a line of text. */
#define  SUBDIR		1		/* Subdirectory in menu. */
#define  PLAINFILE	0		/* Entry in menu. */
#define  MAX_ENTRIES	100		/* Maximum number of entries. */

typedef struct tENTRY {
        int type;			/* Type of entry. */
        char title[TITLE_SIZE];		/* Title of entry. */
        char filename[MAXPATHLEN];	/* Filename for entry. */
        char formatter[LINE_LENGTH];	/* Text formatter to use. */
        char maintainer[MAXPATHLEN];	/* Maintainer of file. */
        } ENTRY;

/* Other CREF stuff. */

#define  CONTENTS	".index"	/* Where the contents are kept. */
#define  B_ENTRY	"entry"		/* String for a file entry. */
#define  B_SUBDIR	"directory"	/* String for a directory entry. */
#define  COMMENT_CHAR	'#'		/* Comment char in contents. */
#define  CONTENTS_DELIM	':'		/* Delimiter in contents. */

#ifdef ATHENA
/* Root of the stock answer tree */
#define	BASE_DIR	"/mit/olc-stock"
#else
#define BASE_DIR	"/your/local/tree"
#endif
