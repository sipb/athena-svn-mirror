#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 * Copyright 1992, 1993, 1999, 2001, 2005, Geoff Kuenning, Claremont, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. The code that causes the 'ispell -v' command to display a prominent
 *    link to the official ispell Web site may not be removed.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This is the configuration file for ispell.  Thanks to Bob McQueer
 * for creating it and making the necessary changes elsewhere to
 * support it, and to George Sipe for figuring out how to make it easier
 * to use.
 *
 * WARNING:  The Makefile edits this file (config.X) to produce config.h.
 * If you are looking at config.h, you're in the wrong file.
 *
 * Look through this file from top to bottom.  If anything needs changing,
 * create the header file "local.h" and define the correct values there;
 * they will override this file.  If you don't make any changes to this
 * file, future patching will be easier.
 */

/*
 * $Id: config.X,v 1.3 2007-02-01 19:58:05 ghudson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.2  2007/02/01 19:50:34  ghudson
 * Import ispell 3.3.02.
 *
 * Revision 1.99  2005/05/25 14:13:53  geoff
 * Correct the spelling of the default for EXEEXT.
 *
 * Revision 1.98  2005/05/01 23:07:59  geoff
 * Add #ifndef around the definition of LINK (how the heck did I blow
 * THAT one?  Thanks to Eli Zaretskii for fixing it).  Add EXTEXT (again
 * courtesy of Eli) to support MS-DOS and similar systems.
 *
 * Revision 1.97  2005/04/28 14:46:51  geoff
 * Add DEFLOGDIR.
 *
 * Revision 1.96  2005/04/28 01:03:11  geoff
 * Include defhash.h after local.h so local.h can override its
 * definitions.  Put back the definitions of MASTERHASH, DEFHASH, and
 * DEFLANG so they'll be documented.
 *
 * Revision 1.95  2005/04/28 00:26:06  geoff
 * Get hash-file names from defhash.h (but let local.h override them).
 * Document the fact that the master hash file is the first language in
 * the language list.
 *
 * Revision 1.94  2005/04/26 22:40:07  geoff
 * Add double-inclusion protection.
 *
 * Revision 1.93  2005/04/21 14:08:58  geoff
 * Don't put an explicit path on egrep, and assume that it takes the -i
 * switch (since that's true on most modern systems).
 *
 * Revision 1.92  2005/04/14 21:25:52  geoff
 * Add a prominent warning against using symbolic links.  Add
 * DICTIONARYVAR and CHARESETVAR.  Fix a typo in determining word size
 * for masks.  Try to get the masktype width from the system word size if
 * possible.
 *
 * Revision 1.91  2005/04/14 15:19:37  geoff
 * Add shortcite to both versions of TEXSKIP1.
 *
 * Revision 1.90  2005/04/14 14:38:23  geoff
 * Update license.  Integrate Ed Avis's changes.  Make "ln" configurable.
 * Use -lncurses by default, rather than -ltermcap.  Make manual
 * extensions configures.  Also make some of the manual contents
 * configurable.  Make words default to /usr/share/dict.  Add a warning
 * against /usr/share/dict/web2.  Make the determination of MAXPATHLEN
 * and MAXNAMLEN smarter.  Increase a bunch of parameters to reflect the
 * increased power of modern machines.  Add support for 64-bit word size.
 * Expand the known TeX keywords.  Add DEFAULT_FILE_MODE.
 *
 * Revision 1.89  2001/09/06 00:30:28  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.88  2001/07/25 21:51:45  geoff
 * Minor license update.
 *
 * Revision 1.87  2001/07/23 20:24:02  geoff
 * Update the copyright and the license.
 *
 * Revision 1.86  2001/06/14 09:11:11  geoff
 * Use non-conflicting macros for bcopy and bzero to avoid compilation
 * problems on smarter compilers.
 *
 * Revision 1.85  2001/05/30 21:14:47  geoff
 * Invert the fcntl/mkstemp options so they will default to being used.
 *
 * Revision 1.84  2001/05/30 21:04:25  geoff
 * Add HAS_FCNTL_H and HAS_MKSTEMP.
 *
 * Revision 1.83  2001/01/24 01:18:27  geoff
 * Don't build the + dictionaries by default any more, because too many
 * distributions no longer supply /usr/dict/words.
 *
 * Revision 1.82  2000/08/22 10:52:25  geoff
 * Fix a spelling error in an ifdef, improve comments on TERMLIB, and
 * bump MAXSTRINGCHARS to 512 to reflect the increased number used in the
 * english.aff file (and presumably others).
 *
 * Revision 1.81  2000/06/30 08:32:47  geoff
 * Bump the default MAXSTRINGCHARS to handle more complex languages
 *
 * Revision 1.80  2000/01/21 06:57:38  geoff
 * Add a bunch of new TeX keywords
 *
 * Revision 1.79  1999/01/13  01:34:18  geoff
 * Get rid of EMACS, which is now obsolete.
 *
 * Revision 1.78  1999/01/08  04:34:11  geoff
 * Get rid of ELISPDIR and TEXINFODIR
 *
 * Revision 1.77  1999/01/07  01:22:34  geoff
 * Update the copyright.
 *
 * Revision 1.76  1998/07/12  20:42:12  geoff
 * Switch the sense of IGNOREBIB for TEXSKIP1.
 *
 * Revision 1.75  1998/07/06  06:55:11  geoff
 * Add TEXSKIP1/2 and associated variables.
 *
 * Revision 1.74  1997/12/02  06:24:35  geoff
 * Get rid of some compile options that really shouldn't be optional.
 *
 * Revision 1.73  1997/12/01  00:53:41  geoff
 * Add HTML support variables.
 *
 * Revision 1.72  1995/11/08  05:09:09  geoff
 * Use symbolic values for DEFTEXFLAG.
 *
 * Revision 1.71  1995/01/08  23:23:28  geoff
 * Add some more configuration variables: HAS_RENAME, MSDOS_BINARY_OPEN,
 * HOME, PDICTHOME, HASHSUFFIX, STATSUFFIX, and COUNTSUFFIX.  These are
 * all to make it easier to port ispell to MS-DOS.  Change DEFPAFF back
 * to "words" so that only .ispell_words will be independent of language.
 *
 * Revision 1.70  1994/10/25  05:45:57  geoff
 * Fix a tiny typo in a comment.  Add a configurable install command.
 *
 * Revision 1.69  1994/09/01  06:06:30  geoff
 * Improve the the documentation of LANGUAGES to include working examples.
 *
 * Revision 1.68  1994/07/28  05:11:34  geoff
 * Log message for previous revision: force MASKBITS to greater than or
 * equal to MASKTYPE_WIDTH (simplifies configuration for 64-bit
 * machines).
 *
 * Revision 1.67  1994/07/28  04:53:34  geoff
 *
 * Revision 1.66  1994/04/27  02:50:46  geoff
 * Change the documentation and defaults for the languages variable to
 * reflect the new method of making American and British dictionary
 * variants.
 *
 * Revision 1.65  1994/04/27  01:50:28  geoff
 * Add MAX_CAPS.
 *
 * Revision 1.64  1994/02/07  08:10:42  geoff
 * Add GENERATE_LIBRARY_PROTOS as a special variable for use only by me.
 *
 * Revision 1.63  1994/01/26  07:44:45  geoff
 * Make yacc configurable through local.h.
 *
 * Revision 1.62  1994/01/25  07:11:20  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

/* You may wish to specify your local definitions in this file: */

#include "local.h"	/* local definitions for options */

#include "defhash.h"	/* Automatically generated options */

/*
** Major-differences selection.  The default system is BSD;  for USG
** or non-UNIX systems you should add the appropriate #define to local.h.
*/
#ifndef USG
#undef USG		/* Define this in local.h for System V machines */
#endif /* USG */

#include <sys/param.h>
#include <sys/types.h>
#ifndef USG
#include <sys/dir.h>
#endif /* USG */

/*
** Things that normally go in a Makefile.  Define these just like you
** might in the Makefile, except you should use #define instead of
** make's assignment syntax.  Everything must be double-quoted, and
** (unlike make) you can't use any sort of $-syntax to pick up the
** values of other definitions.
*/
#ifndef CC
#define CC	"cc"
#endif /* CC */
#ifndef LINT
#define LINT	"lint"
#endif /* LINT */
#ifndef CFLAGS
#define CFLAGS	"-O"
#endif /* CFLAGS */
#ifndef LINTFLAGS
#define LINTFLAGS ""
#endif /* LINTFLAGS */
#ifndef YACC
#define YACC	"yacc"
#endif /* YACC */

/*
** For some systems, it might be better to use symbolic links rather
** than traditional hard links.  Usually hard links are better,
** because they are more efficient.  However, hard links don't work
** across physical devices.  In such cases, you may prefer to set LINK
** to "ln -s" to work around that deficiency.
**
** NOTE TO PERSONS WHO FIRST LEARNED UNIX AFTER 1980: many Unix
** novices seem to think that symbolic links are the only kind of
** links, and seem to use them reflexively.  Remember that symbolic
** links are inferior to hard links in almost every way.  The
** exception is when you're dealing either with (a) links to
** directories or (b) cross-device links.  Ispell uses neither.  Don't
** change LINK to "ln -s" unless you are working on a system that
** simply doesn't support hard links.
*/
#ifndef LINK
#define LINK	"ln"
#endif

/*
** Libraries that may need to be added to the cc line to get ispell to
** link.  Normally, this should be null.
*/
#ifndef LIBES
#define LIBES	""
#endif

/*
** TERMLIB - where to get the termcap library.  Should be -ltermcap,
** -lcurses, or -lncurses on most systems.
*/
#ifndef TERMLIB
#define TERMLIB	"-lncurses"
#endif

/*
** REGLIB - where to get the regular-expression routines, if
** REGEX_LOOKUP is defined.  Should be -lPW on USG systems, null on
** BSD systems.
*/
#ifndef REGLIB
#define REGLIB	""
#endif

/*
** Where to install various components of ispell.  BINDIR contains
** binaries.  LIBDIR contains hash tables and affix files.
**
** If you intend to use multiple dictionary files, I would suggest
** LIBDIR be a directory which will contain nothing else, so sensible
** names can be constructed for the -d option without conflict.
*/
#ifndef BINDIR
#define BINDIR	"/usr/local/bin"
#endif
#ifndef LIBDIR
#define LIBDIR	"/usr/local/lib"
#endif

/*
** MAN1DIR and MAN1EXT control installing the section 1 (executable
** programs) manual pages.  There are also manual pages for file
** formats, but there seems to be some controversy over whether these
** belong in section 4 or section 5 of the manual.  To make the
** cross-references correct you also need to change MAN45SECT.  (The
** built files are always called .5X and .5; the extension changes
** only when it's finally installed.)
** 
** If your system likes to use ".1l" and such for local manual pages,
** you can change MAN1EXT and MAN45EXT appropriately.
*/
#ifndef MAN1DIR
#define MAN1DIR	"/usr/local/man/man1"
#endif
#ifndef MAN1EXT
#define MAN1EXT	".1"
#endif

#ifndef MAN45DIR
#define MAN45DIR "/usr/local/man/man5"
#endif
#ifndef MAN45EXT
#define MAN45EXT ".5"
#endif
#ifndef MAN45SECT
#define MAN45SECT "5"
#endif

/*
** Ispell's manual page refers to some programs, look, spell, and tib, that
** may be absent on some systems.  Look used to be optional; spell
** used to be a part of standard Unix, but has been obsoleted by
** ispell; tib is part of a rarely used TeX package that is not
** available on most systems.  To avoid having a cross reference to a
** manual page that doesn't exist, they are changeable here.
**
** If you have either on your system, define these as follows:
**
** #define SPELL_XREF ".IR spell (1),"
** #define TIB_XREF ".IR tib (1),"
**
** By default, SPELL_XREF is defined as a special string that
** evaluates to nothing in a manual page, because the program is not
** otherwise mentioned in the ispell manaul.  Since tib is mentioned
** elsewhere, its default definition is more complex.
**
** Incidentally, there is no need to do the same for sort, join, and
** so on, since they are standard on any Unix-like system.
*/
#ifndef LOOK_XREF
#define LOOK_XREF ".IR look (1),"
#endif
#ifndef SPELL_XREF
#define SPELL_XREF	".\\\""
#endif
#ifndef TIB_XREF
#define TIB_XREF	".IR tib \" (if available on your system)\","
#endif

/*
** List of all hash files (languages) which will be supported by ispell.
**
** This variable has a complex format so that many options can be
** specified.  The format is as follows:
**
**	<language>[,<make-options>...] [<language> [,<make-options> ...] ...]
**
** where
**
**	language	is the name of a subdirectory of the
**			"languages" directory
**	make-options	are options that are to be passed to "make" in
**			the specified directory.  The make-options
**			should not, in general, specify a target, as
**			this will be provided by the make process.
**
** For example, if LANGUAGES is:
**
**	"{american,MASTERDICTS=american.med+,HASHFILES=americanmed+.hash,EXTRADICT=/usr/share/dict/words /usr/share/dict/web2} {deutsch,DICTALWAYS=deutsch.sml,DICTOPTIONS=}"
**
** then the American-English and Deutsch (German) languages will be supported,
** and the following variable settings will be passed to the two Makefiles:
**
**	American:
**
**	    MASTERDICTS='american.med+'
**	    HASHFILES='americanmed+.hash'
**	    EXTRADICT='/usr/share/dict/words /usr/share/dict/web2'
**
**	Deutsch:
**
**	    DICTALWAYS='deutsch.sml'
**	    DICTOPTIONS=''
**
** Incidentally, it is rarely a good idea to include /usr/share/dict/web2
** in an ispell dictionary even if the file exists on your system,
** because it contains obscure words that may mask misspellings of
** more common words.
**
** Notes on the syntax: The makefile is not very robust.  If you have
** make problems, or if make seems to fail in the language-subdirs
** dependency, check your syntax.  The makefile adds single quotes to
** the individual variables in the LANGUAGES specification, so don't
** use quotes of any kind.
**
** The first language listed in this variable MUST contain a HASHFILES
** declaration.  That language will become the default for
** spell-checking if no dictionary is explicitly specified.  As a
** special (and kludgey) feature, if that language begins with
** "american" or "british", it will also be aliased to "english", and
** "english" will become the default dictionary.  (All of these
** defaults can be overridden by defining MASTERHASH, DEFHASH, and
** DEFLANG, but that should rarely be necessary.)
**
** On MSDOS systems, you must use names that are consistent with the
** 8+3 filename restrictions, or you will risk filename collision.  See
** the file pc/local.djgpp for details.
*/
#ifndef LANGUAGES
#define LANGUAGES "{american,MASTERDICTS=american.med,HASHFILES=americanmed.hash,EXTRADICT=}"
#endif /* LANGUAGES */

/*
** Master hash file for DEFHASH.  This is the name of a hash file
** built by a language Makefile.  It should be the most-popular hash
** file on your system, because it is the one that will be used by
** default.  It must be listed in LANGUAGES, above.
**
** Normally, it is not necessary to define this variable in local.h,
** because it will automatically be extracted from LANGUAGES, above.
*/
#ifndef MASTERHASH
#define MASTERHASH      "americanmed.hash"
#endif

/*
** Default native-language hash file.  This is the name given to the
** hash table that will be used if no language is specified to
** ispell.  It is a link to MASTERHASH, above.
**
** Normally, it is not necessary to define this variable in local.h,
** because it will automatically be extracted from LANGUAGES, above.
*/
#ifndef DEFHASH
#define DEFHASH "english.hash"
#endif

/*
** Language tables for the default language.  This must be the name of
** the affix file that was used to generate the MASTERHASH/DEFHASH,
** above.
**
** Normally, it is not necessary to define this variable in local.h,
** because it will automatically be extracted from LANGUAGES, above.
*/
#ifndef DEFLANG
#define DEFLANG "english.aff"
#endif
/*
** Language to use for error messages.  If there are no messages in this
** language, English will be used instead.
*/
#ifndef MSGLANG
#define MSGLANG	"english"
#endif /* MSGLANG */

/*
** If your sort command accepts the -T switch to set temp file
** locations (try it out; it exists but is undocumented on some
** systems), make the following variable the null string.  Otherwise
** leave it as the sed script.
*/
#ifndef SORTTMP
#define SORTTMP	"-e '/!!SORTTMP!!/s/=.*$/=/'"
#endif

/*
** If your sort command accepts the -T switch (see above), make the
** following variable refer to a temporary directory with lots of
** space.  Otherwise make it the null string.
*/
#ifndef MAKE_SORTTMP
#define MAKE_SORTTMP "-T ${TMPDIR-/tmp}"
#endif

/*
** Sequence that indicates the beginning of a shell script.  Most
** systems use "#!/bin/sh" here.  Some very old machines, however,
** need ": Use /bin/sh".
*/
#ifndef POUNDBANG
#define POUNDBANG "#!/bin/sh"
#endif


/*
** INSTALL program. Could be a copy program like cp or something fancier
** like /usr/ucb/install -c
*/
#ifndef INSTALL
#define INSTALL		"cp"
#endif

/*
** If your system has the rename(2) system call, define HAS_RENAME and
** ispell will use that call to rename backup files.  Otherwise, it
** will use link/unlink.  There is no harm in this except on MS-DOS,
** which doesn't support link/unlink.
*/
#ifndef HAS_RENAME
#undef HAS_RENAME
#endif /* HAS_RENAME */

/*
** If your system doesn't have the fcntl.h header file (most modern
** systems do), define this.
*/
#ifndef NO_FCNTL_H
#undef NO_FCNTL_H
#endif /* NO_FCNTL_H */

/*
** If your system doesn't have the mkstemp library call, define this.
*/
#ifndef NO_MKSTEMP
#undef NO_MKSTEMP
#endif /* NO_MKSTEMP */

/* Aliases for some routines */
#ifdef USG
#define BCOPY(s, d, n)	memcpy (d, s, n)
#define BZERO(d, n)	memset (d, 0, n)
#define index strchr
#define rindex strrchr
#else
#define BCOPY(s, d, n)	bcopy (s, d, n)
#define BZERO(d, n)	bzero (d, n)
#endif

/* type given to signal() by signal.h */
#ifndef SIGNAL_TYPE
#define SIGNAL_TYPE void
#endif

/* environment variable for user's word list */
#ifndef PDICTVAR
#define PDICTVAR "WORDLIST"
#endif

/* prefix part of default word list */
#ifndef DEFPDICT
#define DEFPDICT ".ispell_"
#endif

/*
** suffix part of default word list
*/
#ifndef DEFPAFF
#define DEFPAFF "words"
#endif

/* old place to look for default word list */
#ifndef OLDPDICT
#define OLDPDICT ".ispell_"
#endif /* OLDPDICT */
#ifndef OLDPAFF
#define OLDPAFF "words"
#endif /* OLDPAFF */

/* where to put log files (inside home */
#define DEFLOGDIR ".ispell_logs"

/* environment variable for include file string */
#ifndef INCSTRVAR
#define INCSTRVAR "INCLUDE_STRING"
#endif

/* default include string */
#ifndef DEFINCSTR
#define DEFINCSTR "&Include_File&"
#endif

/* Environment variable for getting extra ispell options */
#define OPTIONVAR         "ISPELL_OPTIONS"

/* Environment variable for controlling where to look for dictionaries */
#define LIBRARYVAR        "ISPELL_DICTDIR"

/* environment variable for preferred dictionary */
#ifndef DICTIONARYVAR
#define DICTIONARYVAR "DICTIONARY"
#endif

/* environment variable for preferred character set */
#ifndef CHARSETVAR
#define CHARSETVAR "ISPELL_CHARSET"
#endif

/* mktemp template for temporary file - MUST contain 6 consecutive X's */
#ifndef TEMPNAME
#define TEMPNAME "/tmp/ispellXXXXXX"
#endif

/*
** If REGEX_LOOKUP is undefined, the lookup command (L) will use the look(1)
** command (if available) or the egrep command.  If REGEX_LOOKUP is defined,
** the lookup command will use the internal dictionary and the
** regular-expression library (which you must supply separately.  There is
** a public-domain library available;  libraries are also distributed with
** both BSD and System V.
**
** The advantage of no REGEX_LOOKUP is that it is often much faster, especially
** if the look(1) command is available, that the words found are presented
** in alphabetical order, and that the list of words searched is larger.
** The advantage of REGEX_LOOKUP is that ispell doesn't need to spawn another
** program, and the list of words searched is exactly the list of (root) words
** that ispell will accept.  (However, note that words formed with affixes will
** not be found;  this can produce some artifacts.  For example, since
** "brother" can be formed as "broth+er", a lookup command might fail to
** find "brother.")
*/
#ifndef REGEX_LOOKUP
#undef REGEX_LOOKUP
#endif /* REGEX_LOOKUP */

/*
** Choose the proper type of regular-expression routines here.  BSD
** and public-domain systems have routines called re_comp and re_exec;
** System V uses regcmp and regex.
**
** REGCTYPE             is the type of a variable to hold the compiled regexp
** REGCMP(re,str)	is the function that compiles a regexp in `str' into
**		        a compiled regexp `re' declared as REGCTYPE and
**			returns `re'.
** REGEX(re, str, dummy) is a function that matches `str' against a compiled
**			regexp in `re' and returns a non-NULL pointer if it
**			matches, NULL otherwise.
** REGFREE(re)		is anything that should be done when the compiled
**			regexp `re' is no longer needed.  It can be also used
**			to allocate any structures required to compile a
**			regexp, since it is called *before* compiling a new
**			regexp (that's where we know that the old one will no
**			longer be used).
**
** Here is one way of defining these if you have POSIX regexp functions:
**
**  #include <sys/types.h>
**  #include <regex.h>
**  #define REGCTYPE		regex_t *
**  #define REGCMP(re,str)	(regcomp (re, str, 0), re)
**  #define REGEX(re, str, dummy) \
**    (re != 0 && regexec (re, str, 0, 0, 0) == 0 ? (char *) 1 : NULL)
**  #define REGFREE(re) \
**    do							\
**	  { 							\
**        if (re == 0)						\
**            re = (regex_t *) calloc (1, sizeof (regex_t));	\
**        else							\
**            regfree (re);					\
**	  } while (0)
**
*/
#ifdef REGEX_LOOKUP
#ifndef REGCMP
#ifdef USG
#define REGCTYPE		char *
#define REGCMP(re, str)		regcmp (str, (char *) 0)
#define REGEX(re, str, dummy)	regex (re, str, dummy, dummy, dummy, dummy, \
				    dummy, dummy, dummy, dummy, dummy, dummy)
#define REGFREE(re)		do \
				    { \
				    if (re != NULL) \
					free (re); \
				    re = NULL; \
				    } \
				while (0)
#else /* USG */
#define REGCTYPE		char *
#define REGFREE(re)		/* Do nothing */
#define REGCMP(re, str)		re_comp (str)
#define REGEX(re, str, dummy)	re_exec (str)
#endif /* USG */
#endif /* REGCMP */
#endif /* REGEX_LOOKUP */

/* look command (if look(1) MAY BE available - ignored if not) */
#ifndef REGEX_LOOKUP
#ifndef LOOK
#define	LOOK	"look -df"
#endif
#endif /* REGEX_LOOKUP */

/* path to egrep (use speeded up version if available) */
#ifndef EGREPCMD
#define EGREPCMD "egrep -i"
#endif

/*
** Path to wordlist for Lookup command (typically /usr/dict/words,
** /usr/dict/web2, or /usr/share/dict/words or web2).
**
** Note that it is usually a bad idea to specify the web2 dictionary
** because it contains obscure words.
*/
/* note that /usr/share/dict/web2 is usually a bad idea due to obscure words */
#ifndef WORDS
#define WORDS	"/usr/share/dict/words"
#endif

/* buffer size to use for file names if not in sys/param.h */
#ifndef MAXPATHLEN
#ifdef PATH_MAX
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 240
#endif
#endif

/* max file name length (will truncate to fit BAKEXT) if not in sys/param.h */
#ifndef MAXNAMLEN
#ifdef NAME_MAX
#define MAXNAMLEN NAME_MAX
#else
#define MAXNAMLEN 14
#endif
#endif

/* define if you want .bak file names truncated to MAXNAMLEN characters */
#ifndef TRUNCATEBAK
#undef TRUNCATEBAK
#endif /* TRUNCATEBAK */

/* largest word accepted from a file by any input routine, plus one */
#ifndef	INPUTWORDLEN
#define INPUTWORDLEN 100
#endif

/* largest amount that a word might be extended by adding affixes */
#ifndef MAXAFFIXLEN
#define MAXAFFIXLEN 40
#endif

/*
** Number of mask bits (affix flags) supported.  Must be 32, 64, 128, or
** 256.  If MASKBITS is 32 or 64, there are really only 26 or 58 flags
** available, respectively.  If it is 32, the flags are named with the
** 26 English uppercase letters;  lowercase will be converted to uppercase.
** If MASKBITS is 64, the 58 flags are named 'A' through 'z' in ASCII
** order, including the 6 special characters from 'Z' to 'a': "[\]^_`".
** If MASKBITS is 128 or 256, all the 7-bit or 8-bit characters,
** respectively, are theoretically available, though a few (newline, slash,
** null byte) are pretty hard to actually use successfully.
**
** Note that most non-English affix files depend on having MASKBITS
** equal to 64 or larger.  It is for this reason that MASKBITS
** defaults to 64.  If you are only going to be using ispell for
** English, and you are very tight on memory space, you might want to
** reduce MASKBITS to 32.
*/
#ifndef MASKBITS
#define MASKBITS	64
#endif

/*
** C type to use for masks.  This should be a type that the processor
** accesses efficiently.
**
** MASKTYPE_WIDTH must correctly reflect the number of bits in a
** MASKTYPE.  Unfortunately, it is also required to be a constant at
** preprocessor time, which means you can't use the sizeof operator to
** define it.
**
** Note that MASKTYPE *must* match MASKTYPE_WIDTH or you may get
** division-by-zero errors! 
*/
#ifndef MASKTYPE
#ifdef __WORDSIZE
#if __WORDSIZE == 64
#define MASKTYPE long
#define MASKTYPE_WIDTH 64
#else
#define MASKTYPE int
#define MASKTYPE_WIDTH __WORDSIZE
#endif
#else
#define MASKTYPE	long
#endif
#endif

#ifndef MASKTYPE_WIDTH
#ifdef __WORDSIZE
#define MASKTYPE_WIDTH __WORDSIZE
#else
#define MASKTYPE_WIDTH	32
#endif /* __WORDSIZE */
#endif /* MASKTYPE_WIDTH */
#if MASKBITS < MASKTYPE_WIDTH
#undef MASKBITS
#define MASKBITS	MASKTYPE_WIDTH
#endif /* MASKBITS < MASKTYPE_WIDTH */

/* maximum number of include files supported by xgets;  set to 0 to disable */
#ifndef MAXINCLUDEFILES
#define MAXINCLUDEFILES	5
#endif

/*
** Maximum hash table fullness percentage.  Larger numbers trade space
** for time.
**/
#ifndef MAXPCT
#define MAXPCT	70		/* Expand table when 70% full */
#endif

/*
** Maximum number of "string" characters that can be defined in a
** language (affix) file.  Don't forget that an upper/lower string
** character counts as two!
*/
#ifndef MAXSTRINGCHARS
#define MAXSTRINGCHARS 512
#endif /* MAXSTRINGCHARS */

/*
** Maximum length of a "string" character.  The default is appropriate for
** nroff-style characters starting with a backslash.
*/
#ifndef MAXSTRINGCHARLEN
#define MAXSTRINGCHARLEN 10
#endif /* MAXSTRINGCHARLEN */

/*
** the terminal mode for ispell, set to CBREAK or RAW
**
*/
#ifndef TERM_MODE
#define TERM_MODE	CBREAK
#endif

/*
** Define this if you want your columns of words to be of equal length.
** This will spread short word lists across the screen instead of down it.
*/
#ifndef EQUAL_COLUMNS
#undef EQUAL_COLUMNS
#endif /* EQUAL_COLUMNS */

/*
** This is the extension that will be added to backup files
*/
#ifndef	BAKEXT
#define	BAKEXT	".bak"
#endif

/*
** Define this if you want your personal dictionary sorted.  This may take
** a long time for very large dictionaries.  Dictionaries larger than
** SORTPERSONAL words will not be sorted.  Define SORTPERSONAL as zero
** to disable this feature.
*/
#ifndef SORTPERSONAL
#define SORTPERSONAL	4000
#endif

/*
** Define this if you want to use the shell for interpretation of commands
** issued via the "L" command, "^Z" under System V, and "!".  If this is
** not defined then a direct fork()/exec() will be used in place of the
** normal system().  This may speed up these operations greatly on some
** systems.
*/
#ifndef USESH
#undef USESH
#endif /* USESH */

/*
** Maximum language-table search size.  Smaller numbers make ispell
** run faster, at the expense of more memory (the lowest reasonable value
** is 2).  If a given character appears in a significant position in
** more than MAXSEARCH suffixes, it will be given its own index table.
** If you change this, define INDEXDUMP in lookup.c to be sure your
** index table looks reasonable.
*/
#ifndef MAXSEARCH
#define MAXSEARCH 4
#endif

/*
** Define this if you want to be able to type any command at a "type space
** to continue" prompt.
*/
#ifndef COMMANDFORSPACE
#undef COMMANDFORSPACE
#endif /* COMMANDFORSPACE */

/*
** Memory-allocation increment.  Buildhash allocates memory in chunks
** of this size, and then subdivides it to get its storage.  This saves
** much malloc execution time.  A good number for this is the system
** page size less the malloc storage overhead.
**
** Define this to zero to revert to using malloc/realloc.  This is normally
** useful only on systems with limited memory.
*/
#ifndef MALLOC_INCREMENT
#define MALLOC_INCREMENT	(4096 - 8)
#endif

/*
** Maximum number of "hits" expected on a word.  This is basically the
** number of different ways different affixes can produce the same word.
** For example, with "english.aff", "brothers" can be produced 3 ways:
** "brothers," "brother+s", or "broth+ers".  If this is too low, no major
** harm will be done, but ispell may occasionally forget a capitalization.
*/
#ifndef MAX_HITS
#define MAX_HITS	10
#endif

/*
** Maximum number of capitalization variations expected in any word.
** Besides the obvious all-lower, all-upper, and capitalized versions,
** this includes followcase variants.  If this is too low, no real
** harm will be done, but ispell may occasionally fail to suggest a
** correct capitalization.
*/
#ifndef MAX_CAPS
#define MAX_CAPS	10
#endif /* MAX_CAPS */

/* Define this to ignore spelling check of entire LaTeX bibliography listings */
#ifndef IGNOREBIB
#undef IGNOREBIB
#endif

/*
** Default nroff and TeX special characters.  Normally, you won't want to
** change this;  instead you would override it in the language-definition
** file.
*/
#ifndef TEXSPECIAL
#define TEXSPECIAL	"()[]{}<>\\$*.%"
#endif

#ifndef NRSPECIAL
#define NRSPECIAL	"().\\*"
#endif

/*
** Default tags after which TeX mode ignores one (skip1) and two (skip2)
** arguments
*/
#ifndef TEXSKIP1
#ifdef IGNOREBIB
#define TEXSKIP1	"end,vspace,hspace,cite,shortcite,ref,eqref,parbox,label,input,nocite,include,includeonly,documentstyle,documentclass,usepackage,selectlanguage,pagestyle,thispagestyle,pagenumbering,value,arabic,roman,Roman,alph,Alph,fnsymbol,stepcounter,refstepcounter,newcounter,addtocontents,newenvironment,renewenvironment,newtheorem,theoremstyle,color,colorbox,textcolor,pagecolor,enlargethispage,stretch,scalebox,rotatebox,includegraphics,symbol,settowidth,settoheight,settodepth,bibliography,bibitem,hyphenation,pageref,psfig"
#else /* IGNOREBIB */
#define TEXSKIP1	"end,vspace,hspace,cite,shortcite,ref,eqref,parbox,label,input,nocite,include,includeonly,documentstyle,documentclass,usepackage,selectlanguage,pagestyle,thispagestyle,pagenumbering,value,arabic,roman,Roman,alph,Alph,fnsymbol,stepcounter,refstepcounter,newcounter,addtocontents,newenvironment,renewenvironment,newtheorem,theoremstyle,color,colorbox,textcolor,pagecolor,enlargethispage,stretch,scalebox,rotatebox,includegraphics,symbol,settowidth,settoheight,settodepth,hyphenation,pageref,psfig"
#endif /* IGNOREBIB */
#endif /* TEXSKIP1 */
#ifndef TEXSKIP2
#define TEXSKIP2	"rule,setcounter,addtocounter,setlength,addtolength,settowidth"
#endif /* TEXSKIP2 */

/*
** Environment variables used to override TEXSKIP1 and TEXSKIP2
*/
#ifndef TEXSKIP1VAR
#define TEXSKIP1VAR	"TEXSKIP1"
#endif /* TEXSKIP1VAR */
#ifndef TEXSKIP2VAR
#define TEXSKIP2VAR	"TEXSKIP2"
#endif /* TEXSKIP2VAR */

/*
** Default tags between which HTML mode ignores text
*/
#ifndef HTMLIGNORE
#define HTMLIGNORE	"code,samp,kbd,pre,listing,address"
#endif

/*
** Variable used to override HTMLIGNORE
*/
#ifndef HTMLIGNOREVAR
#define HTMLIGNOREVAR	"HTMLIGNORE"
#endif /* HTMLIGNOREVAR */

/*
** Default tag fields which HTML mode always checks
*/
#ifndef HTMLCHECK
#define HTMLCHECK	"alt"
#endif /* HTMLCHECK */

/*
** Variable used to override HTMLCHECK
*/
#ifndef HTMLCHECKVAR
#define HTMLCHECKVAR	"HTMLCHECK"
#endif /* HTMLCHECKVAR */

/*
** Defaults for certain command-line flags.
*/
#ifndef DEFNOBACKUPFLAG
#define DEFNOBACKUPFLAG	0		    /* Don't suppress backup file */
#endif

/*
** DEFTEXFLAG should really be named DEFAULT_DEFORMATTER, but isn't for
** historical reasons.
**/
#ifndef DEFTEXFLAG
#define DEFTEXFLAG	DEFORMAT_NROFF	    /* Default to nroff mode */
#endif

/*
** Define this if you want ispell to place a limitation on the maximum
** size of the screen.  On windowed workstations with very large windows,
** the size of the window can be too much of a good thing, forcing the
** user to look back and forth between the bottom and top of the screen.
** If MAX_SCREEN_SIZE is nonzero, screens larger than this will be treated
** as if they have only MAX_SCREEN_SIZE lines.  A good value for this
** variable is 24 or 30.  Define it as zero to suppress the feature.
*/
#ifndef MAX_SCREEN_SIZE
#define MAX_SCREEN_SIZE 0
#endif

/*
** The next three variables are used to provide a variable-size context
** display at the bottom of the screen.  Normally, the user will see
** a number of lines equal to CONTEXTPCT of his screen, rounded down
** (thus, with CONTEXTPCT == 10, a 24-line screen will produce two lines
** of context).  The context will never be greater than MAXCONTEXT or
** less than MINCONTEXT.  To disable this feature entirely, set MAXCONTEXT
** and MINCONTEXT to the same value.  To round context percentages up,
** define CONTEXTROUNDUP.
**
** Warning: don't set MAXCONTEXT ridiculously large.  There is a
** static buffer of size MAXCONTEXT*BUFSIZ; since BUFSIZ is frequently
** 1K or larger, this can create a remarkably large executable.
*/
#ifndef CONTEXTPCT
#define CONTEXTPCT	10	/* Use 10% of the screen for context */
#endif
#ifndef MINCONTEXT
#define MINCONTEXT	2	/* Always show at least 2 lines of context */
#endif
#ifndef MAXCONTEXT
#define MAXCONTEXT	10	/* Never show more than 10 lines of context */
#endif
#ifndef CONTEXTROUNDUP
#undef CONTEXTROUNDUP		/* Don't round context up */
#endif

/*
** Define this if you want the context lines to be displayed at the
** bottom of the screen, the way they used to be, rather than at the top.
*/
#ifndef BOTTOMCONTEXT
#undef BOTTOMCONTEXT
#endif /* BOTTOMCONTEXT */

/*
** Define this if you want the "mini-menu," which gives the most important
** options at the bottom of the screen, to be the default (in any case, it
** can be controlled with the "-M" switch).
*/
#ifndef MINIMENU
#undef MINIMENU
#endif

/*
** You might want to change this to zero if your users want to check
** single-letter words against the dictionary.  However, you should try
** some sample runs using the -W switch before you try it out;  you'd
** be surprised how many single letters appear in documents.  If you increase
** MINWORD beyond 1, don't say I didn't warn you that it was a bad idea.
*/
#ifndef MINWORD
#define MINWORD		1	/* Words this short and shorter are always ok */
#endif

/*
** ANSI C compilers are supposed to provide an include file,
** "stdlib.h", which gives function prototypes for all library
** routines.  Define NO_STDLIB_H if you have a compiler that claims to
** be ANSI, but doesn't provide this include file.
*/
#ifndef NO_STDLIB_H
#ifndef __STDC__
#define NO_STDLIB_H
#endif /* __STDC__ */
#endif /* NO_STDLIB_H */

/*
** Not all systems have the same definitions for R_OK and W_OK
** flags for calling `access'.  If the defaults below are not
** good for your system, define different values in "local.h".
** Usually, the system header <unistd.h> defines these.
*/
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif

/*
** Default mode to give to output files if the mode of the input file
** cannot be determined.  There should be no need to change this.
*/
#ifndef DEFAULT_FILE_MODE
#define DEFAULT_FILE_MODE 0644
#endif /* DEFAULT_FILE_MODE */

/*
** The following define is used by the ispell developer to help
** double-check the software.  Leave it undefined on other systems
** unless you are especially fond of warning messages, or are pursuing
** an inexplicable bug that might be related to a type mismatch.
*/
#ifndef GENERATE_LIBRARY_PROTOS
#undef GENERATE_LIBRARY_PROTOS
#endif /* GENERATE_LIBRARY_PROTOS */

/*
** Symbols below this point are generally intended to cater to
** idiosyncracies of specific machines and operating systems.
**
** MS-DOS users should also define HAS_RENAME, above, if appropriate.
**
** Define PIECEMEAL_HASH_WRITES if your system can't handle huge write
** operations.  This is known to be a problem on MS-DOS systems when
** a 16-bit compiler is used to compile Ispell (but not with DJGPP or
** EMX).
*/
#ifndef PIECEMEAL_HASH_WRITES
#undef PIECEMEAL_HASH_WRITES
#endif /* PIECEMEAL_HASH_WRITES */

/*
** Redefine GETKEYSTROKE() to getkey() on some MS-DOS systems where
** getchar() doesn't operate properly in raw mode.
*/
#ifndef GETKEYSTROKE
#define GETKEYSTROKE()	getchar ()
#endif /* GETKEYSTROKE */

/*
** Define MSDOS_BINARY_OPEN to 0x8000 on MS-DOS systems.  This can be
** done by adding "#include fcntl.h" to your local.h file.
*/
#ifndef MSDOS_BINARY_OPEN
#ifdef O_BINARY
#define MSDOS_BINARY_OPEN   O_BINARY
#else /* O_BINARY */
#define MSDOS_BINARY_OPEN   0
#endif /* O_BINARY */
#endif /* MSDOS_BINARY_OPEN */

/*
** Environment variable to use to locate the home directory.  On DOS
** systems you might want to set this to ISPELL_HOME to avoid
** conflicts with other programs that look for a HOME environment
** variable; on all other systems it should be just HOME.
*/
#ifndef HOME
#define HOME	"HOME"
#endif /* HOME */

/*
** On MS-DOS systems, define PDICTHOME to be the name of a directory
** to be used to contain the personal dictionary (.ispell_english,
** etc.).  On other systems, you can leave it undefined.
*/
#ifndef PDICTHOME
#undef PDICTHOME
#endif /* PDICTHOME */

/*
** On MS-DOS systems, you can rename the following variables so that
** ispell's files have 3-character suffixes.  Note that, if you do
** this, you will have to redefine any variable above that incorporates
** one of the suffixes.  (Most MS-DOS environments will silently truncate
** excess characters beyond the 8+3 limit, so you usually don't need to
** change the suffixes just because they are longer than 3 characters.)
*/
#ifndef HASHSUFFIX
#define HASHSUFFIX	".hash"
#endif /* HASHSUFFIX */
#ifndef STATSUFFIX
#define STATSUFFIX	".stat"
#endif /* STATSUFFIX */

/*
** MS-DOS and MS-Windows systems can use either '/' or '\\' for
** a directory separator in file names.
*/
#ifndef IS_SLASH
#ifdef MSDOS
#define IS_SLASH(c)	((c) == '/'  ||  (c) == '\\')
#else /* MSDOS */
#define IS_SLASH(c)	((c) == '/')
#endif /* MSDOS */
#endif /* IS_SLASH */

#ifndef EXEEXT
#define EXEEXT ""
#endif

#endif /* CONFIG_H_INCLUDED */
