/*
 * This is the configuration file for ispell.  Thanks to Bob McQueer
 * for creating it and making the necessary changes elsewhere to
 * support it.
 * Look through this file from top to bottom, and edit anything that
 * needs editing.  There are also five or six variables in the
 * Makefile that you must edit.  Note that the Makefile edits this
 * file (config.X) to produce config.h.  If you are looking at
 * config.h, you're in the wrong file.
 *
 * Don't change the funny-looking lines with !!'s in them;  see the
 * Makefile!
 */

/*
** library directory for hash table(s) / default hash table name
** If you intend to use multiple dictionary files, I would suggest
** LIBDIR be a directory which will contain nothing else, so sensible
** names can be constructed for the -d option without conflict.
*/
#define LIBDIR "!!LIBDIR!!"
#define DEFHASH "!!DEFHASH!!"

#ifdef USG
#define index strchr
#endif

/* environment variable for user's word list */
#define PDICTVAR "WORDLIST"

/* default word list */
#define DEFPDICT ".ispell.words"

/* mktemp template for temporary file - MUST contain 6 consecutive X's */
#define TEMPNAME "/tmp/ispellXXXXXX"

/* default dictionary file */
#define DEFDICT "!!DEFDICT!!"

/* define LOOK if look(1) command is available */
#define LOOK	"/usr/bin/look -df"

/* path to egrep (use speeded up version if available) */
#define EGREPCMD "/usr/bin/egrep"

/* path to wordlist for Lookup command (typically /usr/dict/{words|web2} */
#define WORDS	"/usr/dict/words"

/* buffer size to use for file names if not in sys/param.h */
#ifndef MAXPATHLEN
#define MAXPATHLEN 240
#endif

/* word length allowed in dictionary by buildhash */
#define WORDLEN 30

/* hash table magic number */
#define MAGIC 1

/* suppress the 8-bit character feature */
#define NO8BIT

/* Approximate number of words in the full dictionary, after munching.
** Err on the high side unless you are very short on memory, in which
** case you might want to change the tables in tree.c and also increase
** MAXPCT.
**
** (Note:  dict.191 is a bit over 15000 words.  dict.191 munched with
** /usr/dict/words is a little over 28000).
*/
#define BIG_DICT	29000

/*
** Maximum hash table fullness percentage.  Larger numbers trade space
** for time.
**/
#define MAXPCT	70		/* Expand table when 70% full */

/*
** the isXXXX macros normally only check ASCII range.  These are used
** instead for text characters, which we assume may be 8 bit.  The
** NO8BIT ifdef shuts off significance of 8 bit characters.  If you are
** using this, and your ctype.h already masks, you can simplify.
*/
#ifdef NO8BIT
#define myupper(X) isupper((X)&0x7f)
#define mylower(X) islower((X)&0x7f)
#define myspace(X) isspace((X)&0x7f)
#define myalpha(X) isalpha((X)&0x7f)
#else
#define myupper(X) (!((X)&0x80) && isupper(X))
#define mylower(X) (!((X)&0x80) && islower(X))
#define myspace(X) (!((X)&0x80) && isspace(X))
#define myalpha(X) (!((X)&0x80) && isalpha(X))
#endif

/*
** the NOPARITY mask is applied to user input characters from the terminal
** in order to mask out the parity bit.
*/
#define NOPARITY 0x7f
