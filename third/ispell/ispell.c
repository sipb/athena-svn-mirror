#ifndef lint
static char Rcs_Id[] =
    "$Id: ispell.c,v 1.1.1.2 2007-02-01 19:50:34 ghudson Exp $";
#endif

#define MAIN

/*
 * ispell.c - An interactive spelling corrector.
 *
 * Copyright (c), 1983, by Pace Willisson
 *
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
 * $Log: not supported by cvs2svn $
 * Revision 1.161  2005/05/25 14:13:53  geoff
 * Report the value of EXEEXT in "ispell -vv".
 *
 * Revision 1.160  2005/04/28 14:46:51  geoff
 * Open a correction log file in command mode.
 *
 * Revision 1.159  2005/04/28 00:26:06  geoff
 * Don't print the count-file suffix, since it's no longer used.
 *
 * Revision 1.158  2005/04/20 23:16:32  geoff
 * Rename some variables to make them more meaningful.
 *
 * Revision 1.157  2005/04/14 23:11:36  geoff
 * Move initckch to makedent.c.
 *
 * Revision 1.156  2005/04/14 21:25:52  geoff
 * Make DICTIONARYVAR configurable, add CHARSETVAR, and add both to ispell -vv.
 *
 * Revision 1.155  2005/04/14 15:19:37  geoff
 * Make sure all current config.X variables are represented in ispell -vv
 * (with the exception of a couple that are function macros), and clean
 * up the sorting of the printout.
 *
 * Revision 1.154  2005/04/14 14:38:23  geoff
 * Update license.  Incorporate Ed Avis's changes.  Add new config.X
 * variables to -vv output.  For backwards compatibility, always put
 * !NO8BIT in config.X output.  Add the -o switch.  Add -e5.  Be
 * intelligent about inferring the output file mode if there is an
 * external deformatter.
 *
 * Revision 1.153  2001/09/06 00:30:28  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.152  2001/07/25 21:51:46  geoff
 * Minor license update.
 *
 * Revision 1.151  2001/07/23 20:24:03  geoff
 * Update the copyright and the license.
 *
 * Revision 1.150  2001/07/19 07:01:46  geoff
 * Fix a bug that effectively disabled the -w switch.
 *
 * Revision 1.149  2001/06/14 09:11:11  geoff
 * Use a non-conflicting macro for bzero to avoid compilation problems on
 * smarter compilers.
 *
 * Revision 1.148  2001/05/30 21:14:47  geoff
 * Invert the fcntl/mkstemp options so they will default to being used.
 *
 * Revision 1.147  2001/05/30 21:04:25  geoff
 * Add fcntl.h support (needed for the previous change), security
 * commentary, and mkstemp support.
 *
 * Revision 1.146  2001/05/30 09:38:26  geoff
 * Werner Fink's patch to get rid of a temp-file race condition.
 *
 * Revision 1.145  2001/05/07 04:47:18  geoff
 * Flush stdout after expansion, for use in pipes (Ed Avis)
 *
 * Revision 1.144  2000/08/22 10:52:25  geoff
 * Fix some compiler warnings.
 *
 * Revision 1.143  1999/01/13 01:34:21  geoff
 * Get rid of some obsolete variables in the -vv switch.
 *
 * Revision 1.142  1999/01/08  05:45:03  geoff
 * Allow shtml files to force HTML mode.
 *
 * Revision 1.141  1999/01/07  01:22:47  geoff
 * Update the copyright.
 *
 * Revision 1.140  1999/01/03  01:46:35  geoff
 * Add support for external deformatters.
 *
 * Revision 1.139  1998/07/12  20:42:18  geoff
 * Change the -i switch to -k, and make it take the name of a keyword
 * table.
 *
 * Revision 1.138  1998/07/06  06:55:16  geoff
 * Use the new general-keyword-lookup support to initialize HTML and TEX
 * tags from environment variables or defaults.
 *
 * Revision 1.137  1997/12/02  06:24:48  geoff
 * Get rid of some compile options that really shouldn't be optional.
 * Add HTML support ('i' switch).
 *
 * Revision 1.136  1995/11/08  05:09:14  geoff
 * Set "aflag" and "askverbose" (new interactive mode) if invoked without
 * filenames or mode arguments.
 *
 * Revision 1.135  1995/11/08  04:27:59  geoff
 * Improve the HTML support to interoperate better with nroff/troff, clean
 * it up, and use '-H' for the switch.
 *
 * Revision 1.134  1995/10/25  04:05:23  geoff
 * Modifications made by Gerry Tierney <gtierney@nova.ucd.ie> to allow
 * checking of html code.  Adds -h switch and checking for html files by
 * .html or .htm extension.  14th of October 1995.
 *
 * Revision 1.133  1995/10/11  04:30:29  geoff
 * Get rid of an unused variable.
 *
 * Revision 1.132  1995/08/05  23:19:36  geoff
 * If the DICTIONARY environment variable is set, derive the default
 * personal-dictionary name from it.
 *
 * Revision 1.131  1995/01/08  23:23:39  geoff
 * Support variable hashfile suffixes for DOS purposes.  Report all the
 * new configuration variables in the -vv switch.  Do some better error
 * checking for mktemp failures.  Support the rename system call.  All of
 * this is to help make DOS porting easier.
 *
 * Revision 1.130  1995/01/03  19:24:08  geoff
 * When constructing a personal-dictioary name from the hash file name,
 * don't stupidly include path directory components.
 *
 * Revision 1.129  1995/01/03  02:23:19  geoff
 * Disable the setbuf call on BSDI systems, sigh.
 *
 * Revision 1.128  1994/10/26  05:12:28  geoff
 * Include boundary characters in the list of characters to be tried in
 * corrections.
 *
 * Revision 1.127  1994/10/25  05:46:07  geoff
 * Allow the default dictionary to be specified by an environment
 * variable (DICTIONARY) as well as a switch.
 *
 * Revision 1.126  1994/09/16  03:32:34  geoff
 * Issue an error message for bad affix flags
 *
 * Revision 1.125  1994/07/28  05:11:36  geoff
 * Log message for previous revision: fix backup-file checks to correctly
 * test for exceeding MAXNAMLEN.
 *
 * Revision 1.124  1994/07/28  04:53:39  geoff
 *
 * Revision 1.123  1994/05/17  06:44:12  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.122  1994/04/27  01:50:37  geoff
 * Print MAX_CAPS in -vv mode.
 *
 * Revision 1.121  1994/03/16  03:49:10  geoff
 * Fix -vv to display the value of NO_STDLIB_H.
 *
 * Revision 1.120  1994/03/15  06:24:28  geoff
 * Allow the -t, -n, and -T switches to override each other, as follows:
 * if no switches are given, the deformatter and string characters are
 * chosen based on the file suffix.  If only -t/-n are given, the
 * deformatter is forced but string characters come from the file suffix.
 * If only -T is given, the deformatter is chosen based on the value
 * given in the -T switch.  Finally, if both -T and -t/-n are given,
 * string characters are controlled by -T and the deformatter by -t/-n.
 *
 * Revision 1.119  1994/03/15  05:58:07  geoff
 * Get rid of a gcc warning
 *
 * Revision 1.118  1994/03/15  05:30:37  geoff
 * Get rid of an unused-variable complaint by proper ifdeffing
 *
 * Revision 1.117  1994/03/12  21:26:48  geoff
 * Correctly limit maximum name lengths for files that have directory paths
 * included.  Also don't use a wired-in 256 for the size of the backup file
 * name.
 *
 * Revision 1.116  1994/02/07  08:10:44  geoff
 * Print GENERATE_LIBRARY_PROTOS in the -vv switch.
 *
 * Revision 1.115  1994/01/26  07:44:47  geoff
 * Make yacc configurable through local.h.
 *
 * Revision 1.114  1994/01/25  07:11:44  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include "config.h"
#include "ispell.h"
#include "fields.h"
#include "proto.h"
#include "msgs.h"
#include "version.h"
#include <ctype.h>
#ifndef NO_FCNTL_H
#include <fcntl.h>
#endif /* NO_FCNTL_H */
#include <sys/stat.h>

static void	usage P ((void));
int		main P ((int argc, char * argv[]));
static void	dofile P ((char * filename));
static FILE *	setupdefmt P ((char * filename, struct stat * statbuf));
static void	update_file P ((char * filename, struct stat * statbuf));
static void	expandmode P ((int printorig));
char *		last_slash P ((char * file));

static char *	Cmd;
static char *	LibDict = NULL;		/* Pointer to name of $(LIBDIR)/dict */

static void usage ()
    {

    (void) fprintf (stderr, ISPELL_C_USAGE1, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE2, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE3, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE4, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE5, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE6, Cmd);
    (void) fprintf (stderr, ISPELL_C_USAGE7, Cmd);
    givehelp (0);
    exit (1);
    }

int main (argc, argv)
    int		argc;
    char *	argv[];
    {
    char *	p;
    char	libdir[MAXPATHLEN];
    char *	cpd;
    field_t *	extra_args;	/* Extra arguments from OPTIONVAR */
    char **	versionp;
    char *	wchars = NULL;
    char *	preftype = NULL;
    static char	libdictname[sizeof DEFHASH];
    char	logfilename[MAXPATHLEN];
    static char	outbuf[BUFSIZ];
    int		argno;
    int		arglen;

    Cmd = *argv;

    Trynum = 0;

    p = getenv (LIBRARYVAR);
    if (p == NULL)
	(void) strcpy (libdir, LIBDIR);
    else
	{
	(void) strncpy (libdir, p, sizeof libdir);
	libdir[sizeof libdir - 1] = '\0';
	}

    p = getenv (DICTIONARYVAR);
    if (p != NULL)
	{
	if (last_slash (p) != NULL)
	    (void) strcpy (hashname, p);
	else
	    (void) sprintf (hashname, "%s/%s", libdir, p);
	(void) strcpy (libdictname, p);
	p = rindex (p, '.');
	if (p == NULL  ||  strcmp (p, HASHSUFFIX) != 0)
	    (void) strcat (hashname, HASHSUFFIX);
	LibDict = last_slash (libdictname);
	if (LibDict != NULL)
	    LibDict++;
	else
	    LibDict = libdictname;
	p = rindex (LibDict, '.');
	if (p != NULL)
	    *p = '\0';
	}
   else
	(void) sprintf (hashname, "%s/%s", libdir, DEFHASH);

    cpd = NULL;

    /*
    ** If any options were given in OPTIONVAR, prepend them to the
    ** command-line arguments.  We prepend so that the command-line
    ** arguments can override those from the environment.
    */
    p = getenv (OPTIONVAR);
    if (p != NULL)
	{
	char **		newargv;

	extra_args = fieldmake (p, 0, " \t",
	  FLD_RUNS | FLD_SNGLQUOTES | FLD_DBLQUOTES | FLD_SHQUOTES
	    | FLD_STRIPQUOTES | FLD_BACKSLASH,
	  0);
	if (extra_args == NULL)
	    {
	    (void) fprintf (stderr, ISPELL_C_NO_OPTIONS_SPACE);
	    return 1;
	    }
	else
	    {
	    newargv =
	      (char **) calloc (argc + extra_args->nfields, sizeof (char *));
	    if (newargv == NULL)
		{
		(void) fprintf (stderr, ISPELL_C_NO_OPTIONS_SPACE);
		return 1;
		}

	    /* Copy arguments over */
	    newargv[0] = argv[0];
	    for (argno = 0;  argno < (int) extra_args->nfields;  argno++)
		newargv[argno + 1] = extra_args->fields[argno];
	    for (argc += extra_args->nfields, argno++;  argno < argc;  argno++)
		newargv[argno] = argv[argno - extra_args->nfields];
	    }

	argv = newargv;
	}

    argv++;
    argc--;
    while (argc && **argv == '-')
	{
	/*
	 * Trying to add a new flag?  Can't remember what's been used?
	 * Here's a handy guide:
	 *
	 * Used:
	 *
	 *	ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789
	 *	^^^^ ^ ^   ^^^ ^  ^^ ^^
	 *	abcdefghijklmnopqrstuvwxyz
	 *	^^^^^^  ^   ^^^ ^ ^^^ ^^^
	 */
	arglen = strlen (*argv);
	switch ((*argv)[1])
	    {
	    case 'v':
		if (arglen > 3)
		    usage ();
		for (versionp = Version_ID;  *versionp;  )
		    {
		    p = *versionp++;
		    if (strncmp (p, "(#) ", 5) == 0)
		      p += 5;
		    (void) printf ("%s\n", p);
		    }
		if ((*argv)[2] == 'v')
		    {
		    (void) printf (ISPELL_C_OPTIONS_ARE);
		    /*
		     * We print USG first because it's mis-set so often.
		     * All others are in alphabetical order.
		     */
#ifdef USG
		    (void) printf ("\tUSG\n");
#else /* USG */
		    (void) printf ("\t!USG (BSD)\n");
#endif /* USG */
		    (void) printf ("\tBAKEXT = \"%s\"\n", BAKEXT);
		    (void) printf ("\tBINDIR = \"%s\"\n", BINDIR);
#ifdef BOTTOMCONTEXT
		    (void) printf ("\tBOTTOMCONTEXT\n");
#else /* BOTTOMCONTEXT */
		    (void) printf ("\t!BOTTOMCONTEXT\n");
#endif /* BOTTOMCONTEXT */
		    (void) printf ("\tCC = \"%s\"\n", CC);
		    (void) printf ("\tCFLAGS = \"%s\"\n", CFLAGS);
		    (void) printf ("\tCHARSETVAR = \"%s\"\n", CHARSETVAR);
#ifdef COMMANDFORSPACE
		    (void) printf ("\tCOMMANDFORSPACE\n");
#else /* COMMANDFORSPACE */
		    (void) printf ("\t!COMMANDFORSPACE\n");
#endif /* COMMANDFORSPACE */
		    (void) printf ("\tCONTEXTPCT = %d\n", CONTEXTPCT);
#ifdef CONTEXTROUNDUP
		    (void) printf ("\tCONTEXTROUNDUP\n");
#else /* CONTEXTROUNDUP */
		    (void) printf ("\t!CONTEXTROUNDUP\n");
#endif /* CONTEXTROUNDUP */
		    (void) printf ("\tDEFAULT_FILE_MODE = 0%3.3o\n",
		      DEFAULT_FILE_MODE);
		    (void) printf ("\tDEFHASH = \"%s\"\n", DEFHASH);
		    (void) printf ("\tDEFINCSTR = \"%s\"\n", DEFINCSTR);
		    (void) printf ("\tDEFLANG = \"%s\"\n", DEFLANG);
		    (void) printf ("\tDEFNOBACKUPFLAG = %d\n",
		      DEFNOBACKUPFLAG);
		    (void) printf ("\tDEFPAFF = \"%s\"\n", DEFPAFF);
		    (void) printf ("\tDEFPDICT = \"%s\"\n", DEFPDICT);
		    (void) printf ("\tDEFTEXFLAG = %d\n", DEFTEXFLAG);
		    (void) printf ("\tDICTIONARYVAR = \"%s\"\n",
		      DICTIONARYVAR);
		    (void) printf ("\tEGREPCMD = \"%s\"\n", EGREPCMD);
#ifdef EQUAL_COLUMNS
		    (void) printf ("\tEQUAL_COLUMNS\n");
#else /* EQUAL_COLUMNS */
		    (void) printf ("\t!EQUAL_COLUMNS\n");
#endif /* EQUAL_COLUMNS */
		    (void) printf ("\tEXEEXT = \"%s\"\n", EXEEXT);
#ifdef GENERATE_LIBRARY_PROTOS
		    (void) printf ("\tGENERATE_LIBRARY_PROTOS\n");
#else /* GENERATE_LIBRARY_PROTOS */
		    (void) printf ("\t!GENERATE_LIBRARY_PROTOS\n");
#endif /* GENERATE_LIBRARY_PROTOS */
		    (void) printf ("\tHASHSUFFIX = \"%s\"\n", HASHSUFFIX);
#ifdef HAS_RENAME
		    (void) printf ("\tHAS_RENAME\n");
#else /* HAS_RENAME */
		    (void) printf ("\t!HAS_RENAME\n");
#endif /* HAS_RENAME */
		    (void) printf ("\tHOME = \"%s\"\n", HOME);
		    (void) printf ("\tHTMLCHECK = \"%s\"\n", HTMLCHECK);
		    (void) printf ("\tHTMLCHECKVAR = \"%s\"\n", HTMLCHECKVAR);
		    (void) printf ("\tHTMLIGNORE = \"%s\"\n", HTMLIGNORE);
		    (void) printf ("\tHTMLIGNOREVAR = \"%s\"\n",
		      HTMLIGNOREVAR);
#ifdef IGNOREBIB
		    (void) printf ("\tIGNOREBIB\n");
#else /* IGNOREBIB */
		    (void) printf ("\t!IGNOREBIB\n");
#endif /* IGNOREBIB */
		    (void) printf ("\tINCSTRVAR = \"%s\"\n", INCSTRVAR);
		    (void) printf ("\tINPUTWORDLEN = %d\n", INPUTWORDLEN);
		    (void) printf ("\tINSTALL = \"%s\"\n", INSTALL);
		    (void) printf ("\tLANGUAGES = \"%s\"\n", LANGUAGES);
		    (void) printf ("\tLIBDIR = \"%s\"\n", LIBDIR);
		    (void) printf ("\tLIBES = \"%s\"\n", LIBES);
		    (void) printf ("\tLIBRARYVAR = \"%s\"\n", LIBRARYVAR);
		    (void) printf ("\tLINK = \"%s\"\n",  LINK);
		    (void) printf ("\tLINT = \"%s\"\n", LINT);
		    (void) printf ("\tLINTFLAGS = \"%s\"\n", LINTFLAGS);
#ifndef REGEX_LOOKUP
		    (void) printf ("\tLOOK = \"%s\"\n", LOOK);
#endif /* REGEX_LOOKUP */
		    (void) printf ("\tLOOK_XREF = \"%s\"\n", LOOK_XREF);
		    (void) printf ("\tMAKE_SORTTMP = \"%s\"\n", MAKE_SORTTMP);
		    (void) printf ("\tMALLOC_INCREMENT = %d\n",
		      MALLOC_INCREMENT);
		    (void) printf ("\tMAN1DIR = \"%s\"\n", MAN1DIR);
		    (void) printf ("\tMAN1EXT = \"%s\"\n", MAN1EXT);
		    (void) printf ("\tMAN45DIR = \"%s\"\n", MAN45DIR);
		    (void) printf ("\tMAN45EXT = \"%s\"\n", MAN45EXT);
		    (void) printf ("\tMAN45SECT = \"%s\"\n", MAN45SECT);
		    (void) printf ("\tMASKBITS = %d\n", MASKBITS);
		    (void) printf ("\tMASKTYPE = \"%s\"\n", MASKTYPE_STRING);
		    (void) printf ("\tMASKTYPE_WIDTH = %d\n", MASKTYPE_WIDTH);
		    (void) printf ("\tMASTERHASH = \"%s\"\n", MASTERHASH);
		    (void) printf ("\tMAXAFFIXLEN = %d\n", MAXAFFIXLEN);
#ifdef MAXBASENAMELEN
		    (void) printf ("\tMAXBASENAMELEN = %d\n", MAXBASENAMELEN);
#endif
		    (void) printf ("\tMAXCONTEXT = %d\n", MAXCONTEXT);
#ifdef MAXEXTLEN
		    (void) printf ("\tMAXEXTLEN = %d\n", MAXEXTLEN);
#endif
		    (void) printf ("\tMAXINCLUDEFILES = %d\n",
		      MAXINCLUDEFILES);
		    (void) printf ("\tMAXNAMLEN = %d\n", MAXNAMLEN);
		    (void) printf ("\tMAXPATHLEN = %d\n", MAXPATHLEN);
		    (void) printf ("\tMAXPCT = %d\n", MAXPCT);
		    (void) printf ("\tMAXSEARCH = %d\n", MAXSEARCH);
		    (void) printf ("\tMAXSTRINGCHARLEN = %d\n",
		      MAXSTRINGCHARLEN);
		    (void) printf ("\tMAXSTRINGCHARS = %d\n", MAXSTRINGCHARS);
		    (void) printf ("\tMAX_CAPS = %d\n", MAX_CAPS);
		    (void) printf ("\tMAX_HITS = %d\n", MAX_HITS);
		    (void) printf ("\tMAX_SCREEN_SIZE = %d\n",
		      MAX_SCREEN_SIZE);
		    (void) printf ("\tMINCONTEXT = %d\n", MINCONTEXT);
#ifdef MINIMENU
		    (void) printf ("\tMINIMENU\n");
#else /* MINIMENU */
		    (void) printf ("\t!MINIMENU\n");
#endif /* MINIMENU */
		    (void) printf ("\tMINWORD = %d\n", MINWORD);
#ifdef MSDOS
		    (void) printf ("\tMSDOS\n");
#else /* MSDOS */
		    (void) printf ("\t!MSDOS\n");
#endif /* MSDSO */
		    (void) printf ("\tMSDOS_BINARY_OPEN = 0x%x\n",
		      (unsigned int) MSDOS_BINARY_OPEN);
		    (void) printf ("\tMSGLANG = \"%s\"\n", MSGLANG);
		    /*
		     * NO8BIT is an obsolete option, but some people depend
		     * on it being in the ispell -vv output.
		     */
		    (void) printf ("\t!NO8BIT (8BIT)\n");
#ifdef NO_FCNTL_H
		    (void) printf ("\tNO_FCNTL_H\n");
#else /* NO_FCNTL_H */
		    (void) printf ("\t!NO_FCNTL_H (FCNTL_H)\n");
#endif /* NO_FCNTL_H */
#ifdef NO_MKSTEMP
		    (void) printf ("\tNO_MKSTEMP\n");
#else /* NO_MKSTEMP */
		    (void) printf ("\t!NO_MKSTEMP (MKSTEMP)\n");
#endif /* NO_STDLIB_H */
#ifdef NO_STDLIB_H
		    (void) printf ("\tNO_STDLIB_H\n");
#else /* NO_STDLIB_H */
		    (void) printf ("\t!NO_STDLIB_H (STDLIB_H)\n");
#endif /* NO_STDLIB_H */
		    (void) printf ("\tNRSPECIAL = \"%s\"\n", NRSPECIAL);
		    (void) printf ("\tOLDPAFF = \"%s\"\n", OLDPAFF);
		    (void) printf ("\tOLDPDICT = \"%s\"\n", OLDPDICT);
		    (void) printf ("\tOPTIONVAR = \"%s\"\n", OPTIONVAR);
#ifdef PDICTHOME
		    (void) printf ("\tPDICTHOME = \"%s\"\n", PDICTHOME);
#else /* PDICTHOME */
		    (void) printf ("\tPDICTHOME = (undefined)\n");
#endif /* PDICTHOME */
		    (void) printf ("\tPDICTVAR = \"%s\"\n", PDICTVAR);
#ifdef PIECEMEAL_HASH_WRITES
		    (void) printf ("\tPIECEMEAL_HASH_WRITES\n");
#else /* PIECEMEAL_HASH_WRITES */
		    (void) printf ("\t!PIECEMEAL_HASH_WRITES\n");
#endif /* PIECEMEAL_HASH_WRITES */
		    (void) printf ("\tPOUNDBANG = \"%s\"\n", POUNDBANG);
#ifdef REGEX_LOOKUP
		    (void) printf ("\tREGEX_LOOKUP\n");
#else /* REGEX_LOOKUP */
		    (void) printf ("\t!REGEX_LOOKUP\n");
#endif /* REGEX_LOOKUP */
		    (void) printf ("\tREGLIB = \"%s\"\n", REGLIB);
		    (void) printf ("\tR_OK = %d\n", R_OK);
		    (void) printf ("\tSIGNAL_TYPE = \"%s\"\n",
		      SIGNAL_TYPE_STRING);
		    (void) printf ("\tSORTPERSONAL = %d\n", SORTPERSONAL);
		    (void) printf ("\tSORTTMP = \"%s\"\n", SORTTMP);
		    (void) printf ("\tSPELL_XREF = \"%s\"\n", SPELL_XREF);
		    (void) printf ("\tSTATSUFFIX = \"%s\"\n", STATSUFFIX);
		    (void) printf ("\tTEMPNAME = \"%s\"\n", TEMPNAME);
		    (void) printf ("\tTERMLIB = \"%s\"\n", TERMLIB);
#if TERM_MODE == CBREAK
		    (void) printf ("\tTERM_MODE = CBREAK\n");
#else /* TERM_MODE */
		    (void) printf ("\tTERM_MODE = RAW\n");
#endif /* TERM_MODE */
		    (void) printf ("\tTEXSKIP1 = \"%s\"\n", TEXSKIP1);
		    (void) printf ("\tTEXSKIP1VAR = \"%s\"\n", TEXSKIP1VAR);
		    (void) printf ("\tTEXSKIP2 = \"%s\"\n", TEXSKIP2);
		    (void) printf ("\tTEXSKIP2VAR = \"%s\"\n", TEXSKIP2VAR);
		    (void) printf ("\tTEXSPECIAL = \"%s\"\n", TEXSPECIAL);
		    (void) printf ("\tTIB_XREF = \"%s\"\n", TIB_XREF);
#ifdef TRUNCATEBAK
		    (void) printf ("\tTRUNCATEBAK\n");
#else /* TRUNCATEBAK */
		    (void) printf ("\t!TRUNCATEBAK\n");
#endif /* TRUNCATEBAK */
#ifdef USESH
		    (void) printf ("\tUSESH\n");
#else /* USESH */
		    (void) printf ("\t!USESH\n");
#endif /* USESH */
		    (void) printf ("\tWORDS = \"%s\"\n", WORDS);
		    (void) printf ("\tW_OK = %d\n", W_OK);
		    (void) printf ("\tYACC = \"%s\"\n", YACC);
		    }
		exit (0);
		break;
	    case 'n':
		if (arglen > 2)
		    usage ();
		tflag = DEFORMAT_NROFF;	/* nroff/troff mode */
		deftflag = DEFORMAT_NROFF;
		if (preftype == NULL)
		    preftype = "nroff";
		break;
	    case 't':			/* TeX mode */
		if (arglen > 2)
		    usage ();
		tflag = DEFORMAT_TEX;
		deftflag = DEFORMAT_TEX;
		if (preftype == NULL)
		    preftype = "tex";
		break;
	    case 'H':			/* HTML mode */
		if (arglen > 2)
		    usage ();
		tflag = DEFORMAT_SGML;	/* non-TeX mode */
		deftflag = DEFORMAT_SGML;
		if (preftype == NULL)
		    preftype = "sgml";
		break;
	    case 'o':                   /* Ordinary text mode */
		if (arglen > 2)
		    usage ();
		tflag = DEFORMAT_NONE;
		deftflag = DEFORMAT_NONE;
		if (preftype == NULL)
		    preftype = "plain";
		break;
	    case 'k':			/* Set keyword tables */
		p = (*argv) + 2;
		argv++;
		argc--;
		if (argc == 0)
		    usage ();
		if (strcmp (p, "texskip1") == 0)
		    {
		    if (init_keyword_table (*argv, TEXSKIP1VAR, TEXSKIP1,
		      0, &texskip1list))
			usage ();
		    }
		else if (strcmp (p, "texskip2") == 0)
		    {
		    if (init_keyword_table (*argv, TEXSKIP2VAR, TEXSKIP2,
		      0, &texskip2list))
			usage ();
		    }
		else if (strcmp (p, "htmlignore") == 0)
		    {
		    if (init_keyword_table (*argv, HTMLIGNOREVAR, HTMLIGNORE,
		      1, &htmlignorelist))
			usage ();
		    }
		else if (strcmp (p, "htmlcheck") == 0)
		    {
		    if (init_keyword_table (*argv, HTMLCHECKVAR, HTMLCHECK,
		      1, &htmlchecklist))
			usage ();
		    }
		break;
	    case 'T':			/* Set preferred file type */
		p = (*argv) + 2;
		if (*p == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    p = *argv;
		    }
		preftype = p;
		break;
	    case 'F':			/* Set external deformatting program */
		p = (*argv) + 2;
		if (*p == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    p = *argv;
		    }
		defmtpgm = p;
		tflag = DEFORMAT_NONE;
		deftflag = DEFORMAT_NONE;
		if (preftype == NULL)
		    preftype = "plain";
		break;
	    case 'A':
		if (arglen > 2)
		    usage ();
		incfileflag = 1;
		aflag = 1;
		break;
	    case 'a':
		if (arglen > 2)
		    usage ();
		aflag++;
		break;
	    case 'D':
		if (arglen > 2)
		    usage ();
		dumpflag++;
		nodictflag++;
		break;
	    case 'e':
		if (arglen > 3)
		    usage ();
		eflag = 1;
		if ((*argv)[2] == 'e')
		    eflag = 2;
		else if ((*argv)[2] >= '1'  &&  (*argv)[2] <= '5')
		    eflag = (*argv)[2] - '0';
		else if ((*argv)[2] != '\0')
		    usage ();
		nodictflag++;
		break;
	    case 'c':
		if (arglen > 2)
		    usage ();
		cflag++;
		lflag++;
		nodictflag++;
		break;
	    case 'b':
		if (arglen > 2)
		    usage ();
		xflag = 0;		/* Keep a backup file */
		break;
	    case 'x':
		if (arglen > 2)
		    usage ();
		xflag = 1;		/* Don't keep a backup file */
		break;
	    case 'f':
		fflag++;
		p = (*argv) + 2;
		if (*p == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    p = *argv;
		    }
		askfilename = p;
		if (*askfilename == '\0')
		    askfilename = NULL;
		break;
	    case 'L':
		p = (*argv) + 2;
		if (*p == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    p = *argv;
		    }
		contextsize = atoi (p);
		break;
	    case 'l':
		if (arglen > 2)
		    usage ();
		lflag++;
		break;
#ifndef USG
	    case 's':
		if (arglen > 2)
		    usage ();
		sflag++;
		break;
#endif
	    case 'S':
		if (arglen > 2)
		    usage ();
		sortit = 0;
		break;
	    case 'B':		/* -B:  report missing blanks */
		if (arglen > 2)
		    usage ();
		compoundflag = COMPOUND_NEVER;
		break;
	    case 'C':		/* -C:  compound words are acceptable */
		if (arglen > 2)
		    usage ();
		compoundflag = COMPOUND_ANYTIME;
		break;
	    case 'P':		/* -P:  don't gen non-dict poss's */
		if (arglen > 2)
		    usage ();
		tryhardflag = 0;
		break;
	    case 'm':		/* -m:  make all poss affix combos */
		if (arglen > 2)
		    usage ();
		tryhardflag = 1;
		break;
	    case 'N':		/* -N:  suppress minimenu */
		if (arglen > 2)
		    usage ();
		minimenusize = 0;
		break;
	    case 'M':		/* -M:  force minimenu */
		if (arglen > 2)
		    usage ();
		minimenusize = 2;
		break;
	    case 'p':
		cpd = (*argv) + 2;
		if (*cpd == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    cpd = *argv;
		    if (*cpd == '\0')
			cpd = NULL;
		    }
		LibDict = NULL;
		break;
	    case 'd':
		p = (*argv) + 2;
		if (*p == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    p = *argv;
		    }
		if (last_slash (p) != NULL)
		    (void) strcpy (hashname, p);
		else
		    (void) sprintf (hashname, "%s/%s", libdir, p);
		if (cpd == NULL  &&  *p != '\0')
		    LibDict = p;
		p = rindex (p, '.');
		if (p != NULL  &&  strcmp (p, HASHSUFFIX) == 0)
		    *p = '\0';	/* Don't want ext. in LibDict */
		else
		    (void) strcat (hashname, HASHSUFFIX);
		if (LibDict != NULL)
		    {
		    p = last_slash (LibDict);
		    if (p != NULL)
			LibDict = p + 1;
		    }
		break;
	    case 'V':		/* Display 8-bit characters as M-xxx */
		if (arglen > 2)
		    usage ();
		vflag = 1;
		break;
	    case 'w':
		wchars = (*argv) + 2;
		if (*wchars == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    wchars = *argv;
		    }
		break;
	    case 'W':
		if ((*argv)[2] == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    minword = atoi (*argv);
		    }
		else
		    minword = atoi (*argv + 2);
		break;
	    default:
		usage ();
	    }
	argv++;
	argc--;
	}

    if (!argc  &&  !lflag  &&  !aflag   &&  !eflag  &&  !dumpflag)
	{
	if (argc != 0)
	    usage ();
	else
	    {
	    aflag = 1;
	    askverbose = 1;
	    }
	}

    /*
     * Because of the high cost of reading the dictionary, we stat
     * the files specified first to see if they exist.  If at least
     * one exists, we continue.
     */
    for (argno = 0;  argno < argc;  argno++)
	{
	if (access (argv[argno], R_OK) >= 0)
	    break;
	}
    if (argno >= argc  &&  !lflag  &&  !aflag  &&  !eflag  &&  !dumpflag)
	{
	(void) fprintf (stderr,
	  argc == 1 ? ISPELL_C_NO_FILE : ISPELL_C_NO_FILES);
	exit (1);
	}
    if (linit () < 0)
	exit (1);

    if (preftype == NULL)
	preftype = getenv (CHARSETVAR);

    if (preftype != NULL)
	{
	prefstringchar =
	  findfiletype (preftype, 1, deftflag < 0 ? &deftflag : (int *) NULL);
	if (prefstringchar < 0
	  &&  strcmp (preftype, "plain") != 0
	  &&  strcmp (preftype, "tex") != 0
	  &&  strcmp (preftype, "nroff") != 0
	  &&  strcmp (preftype, "sgml") != 0)
	    {
	    (void) fprintf (stderr, ISPELL_C_BAD_TYPE, preftype);
	    exit (1);
	    }
	}
    if (prefstringchar < 0)
	defstringgroup = 0;
    else
	defstringgroup = prefstringchar;

    if (compoundflag < 0)
	compoundflag = hashheader.compoundflag;
    if (tryhardflag < 0)
	tryhardflag = hashheader.defhardflag;

    /*
     * Set up the various tables of keywords to be treated specially.
     *
     * TeX/LaTeX mode:
     */
    (void) init_keyword_table (NULL, TEXSKIP1VAR, TEXSKIP1, 0, &texskip1list);
    (void) init_keyword_table (NULL, TEXSKIP2VAR, TEXSKIP2, 0, &texskip2list);
    /*
     * HTML mode:
     */
    (void) init_keyword_table (NULL, HTMLIGNOREVAR, HTMLIGNORE, 1,
      &htmlignorelist);
    (void) init_keyword_table (NULL, HTMLCHECKVAR, HTMLCHECK, 1,
      &htmlchecklist);

    initckch(wchars);

    if (LibDict == NULL)	
	{
	(void) strcpy (libdictname, DEFHASH);
	LibDict = libdictname;
	p = rindex (libdictname, '.');
	if (p != NULL  &&  strcmp (p, HASHSUFFIX) == 0)
	    *p = '\0';	/* Don't want ext. in LibDict */
	}
    if (!nodictflag)
	treeinit (cpd, LibDict);

    if (aflag)
	{
	askmode ();
	treeoutput ();
	exit (0);
	}
    else if (eflag)
	{
	expandmode (eflag);
	exit (0);
	}
    else if (dumpflag)
	{
	dumpmode ();
	exit (0);
	}

#ifndef __bsdi__
    setbuf (stdout, outbuf);
#endif /* __bsdi__ */
    if (lflag)
	{
	infile = setupdefmt(NULL, NULL);
	outfile = stdout;
	checkfile ();
	exit (0);
	}

    /*
     * If there is a log directory, open a log file.  If the open
     * fails, we just won't log.
     */
    (void) sprintf (logfilename, "%s/%s/%s",
      getenv ("HOME") == NULL ? "" : getenv ("HOME"),
      DEFLOGDIR, LibDict);
    logfile = fopen (logfilename, "a");

    terminit ();

    while (argc--)
	dofile (*argv++);

    done (0);
    /* NOTREACHED */
    return 0;
    }

static void dofile (filename)
    char *	filename;
    {
    struct stat	statbuf;
    char *	cp;
    int		outfd;			/* Used in opening temp file */
					/* ..might produce not-used warnings */

    currentfile = filename;

    /* Guess a deformatter based on the file extension */
    tflag = deftflag;
    if (tflag < 0)
	{
	tflag = DEFORMAT_NONE;		/* Default to none */
	cp = rindex (filename, '.');
	if (cp != NULL)
	    {
	    if (strcmp (cp, ".ms") == 0  ||  strcmp (cp, ".mm") == 0
	      ||  strcmp (cp, ".me") == 0  ||  strcmp (cp, ".man") == 0
	      ||  isdigit(*cp))
		tflag = DEFORMAT_NROFF;
	    else if (strcmp (cp, ".tex") == 0)
		tflag = DEFORMAT_TEX;
	    else if (strcmp (cp, ".html") == 0  ||  strcmp (cp, ".htm") == 0
	      ||  strcmp (cp, ".shtml") == 0)
		tflag = DEFORMAT_SGML;
	    }
	}
    if (prefstringchar < 0)
	{
	defstringgroup =
	  findfiletype (filename, 0, deftflag < 0 ? &tflag : (int *) NULL);
	if (defstringgroup < 0)
	    defstringgroup = 0;
	}

    if ((infile = setupdefmt (filename, &statbuf)) == NULL)
	{
	(void) fprintf (stderr, CANT_OPEN, filename, MAYBE_CR (stderr));
	(void) sleep ((unsigned) 2);
	return;
	}

    readonly = access (filename, W_OK) < 0;
    if (readonly)
	{
	(void) fprintf (stderr, ISPELL_C_CANT_WRITE, filename,
	  MAYBE_CR (stderr));
	(void) sleep ((unsigned) 2);
	}

    /*
     * Security notes: TEMPNAME must be less than MAXPATHLEN - 1.  If
     * the system has O_EXCL but not mkstemp, the temporary file will
     * be opened securely as in the manner of mkstemp (which
     * unfortunately isn't available anywhere).  In other words, don't
     * worry about the security of this hunk of code.
     */
    if (last_slash (TEMPNAME) != NULL)
	(void) strcpy (tempfile, TEMPNAME);
    else
	{
	char *tmp = getenv ("TMPDIR");
	int   lastchar;

	if (tmp == NULL)
	    tmp = getenv ("TEMP");
	if (tmp == NULL)
	    tmp = getenv ("TMP");
	if (tmp == NULL)
#ifdef P_tmpdir
	    tmp = P_tmpdir;
#else
	    tmp = "/tmp";
#endif
	lastchar = tmp[strlen (tmp) - 1];
	(void) sprintf (tempfile, "%s%s%s", tmp,
			IS_SLASH (lastchar) ? "" : "/",
			TEMPNAME);
	}
#ifdef NO_MKSTEMP
    if (mktemp (tempfile) == NULL  ||  tempfile[0] == '\0'
#ifdef O_EXCL
      ||  (outfd = open (tempfile, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0
      ||  (outfile = fdopen (outfd, "w")) == NULL)
#else /* O_EXCL */
      ||  (outfile = fopen (tempfile, "w")) == NULL)
#endif /* O_EXCL */
#else /* NO_MKSTEMP */
    if ((outfd = mkstemp (tempfile)) < 0
      ||  (outfile = fdopen (outfd, "w")) == NULL)
#endif /* NO_MKSTEMP */
	{
	(void) fprintf (stderr, CANT_CREATE,
	  (tempfile == NULL  ||  tempfile[0] == '\0')
	    ? "temporary file" : tempfile,
	  MAYBE_CR (stderr));
	(void) sleep ((unsigned) 2);
	return;
	}
#ifndef MSDOS
    /*
    ** This is usually a no-op on MS-DOS, but with file-sharing
    ** installed, it was reported to produce empty spelled files!
    ** Apparently, the file-sharing module would close the file when
    ** `chmod' is called.
    */
    (void) chmod (tempfile, statbuf.st_mode);
#endif

    quit = 0;
    changes = 0;

    checkfile ();

    (void) fclose (infile);
    (void) fclose (outfile);

    if (!cflag)
	treeoutput ();

    if (changes && !readonly)
	update_file (filename, &statbuf);
    (void) unlink (tempfile);
    }

/*
 * Set up to externally deformat a file.
 *
 * If no deformatter is provided, we return either standard input (if
 * filename is NULL) or the result of opening the specified file.
 *
 * If a deformatter is provided, but no filename is provided, we
 * assume that the input comes from stdin, and we set up the
 * deformatter to filter that descriptor, and return stdin as our
 * result.  In such a case, there is no dual input: we will read the
 * filtered data via the deformatter and ignore the unfiltered
 * version.
 *
 * If there is a deformatter and we are given a filename, then we set
 * up the deformatter to read the file from its own standard input and
 * write to its standard output.  In addition, we set up "sourcefile"
 * so that it can read the unfiltered data directly from the input
 * file.
 *
 * If "statbuf" is non-NULL, it will be filled in with the result of
 * stat-ing the file.  If the stat fails, statbuf->st_mode will be set
 * to a reasonable default value.  The contents of statbuf are
 * undefined if the input file or the filter cannot be opened.
 *
 * Note that external deformatters do not work with named pipes as input.
 */
static FILE * setupdefmt (filename, statbuf)
    char *		filename;	/* File to open, if non-NULL */
    struct stat *	statbuf;	/* Buffer to hold file status */
    {
    FILE*		filteredfile;	/* Access to the filtered file */
    int			inputfd;	/* Fd for access to file to open */
    int			savedstdin;	/* File descriptor saving stdin */

    sourcefile = NULL;
    if (defmtpgm == NULL)
	{
	/*
	 * There is no deformatter.  Return either stdin or an open file.
	 */
	if (filename == NULL)
	    filteredfile = stdin;
	else
	    filteredfile = fopen (filename, "r");
	if (statbuf != NULL  &&  filteredfile != NULL
	  &&  fstat (fileno (filteredfile), statbuf) == -1)
	    statbuf->st_mode = DEFAULT_FILE_MODE;
	return filteredfile;
	}
    else if (filename == NULL)
	{
	/*
	 * We are reading from standard input.  Switch over to a
	 * filtered version of stdin.
	 */
	if (statbuf != NULL  &&  fstat (fileno (stdin), statbuf) == -1)
	    statbuf->st_mode = DEFAULT_FILE_MODE;
	return popen (defmtpgm, "r");
	}
    else
	{
	/*
	 * This is the tricky case.  We need to get the deformatter to
	 * read from the input file and filter it to us.  Doing so
	 * requires several steps:
	 *
	 *  1.	Preserve file descriptor 0 by duplicating it.
	 *  2.	Close file descriptor 0, and reopen it on the file to be
	 *	filtered.
	 *  3.	Open a pipe to the deformat program.
	 *  4.	Restore file descriptor 0.
	 *
	 * Because we do all of this without ever letting the stdio
	 * library know that we've been mucking around with file
	 * descriptors, we won't bother its access to stdin.
	 */
	sourcefile = fopen (filename, "r");
	if (sourcefile == NULL)
	    return NULL;
	if (statbuf != NULL  &&  fstat (fileno (sourcefile), statbuf) == -1)
	    statbuf->st_mode = DEFAULT_FILE_MODE;
	savedstdin = dup (0);
	inputfd = open (filename, 0);
	if (inputfd < 0)
	    return NULL;		/* Failed to open the file */
	else if (dup2 (inputfd, 0) != 0)
	    {
	    (void) fprintf (stderr, ISPELL_C_UNEXPECTED_FD, filename,
	      MAYBE_CR (stderr));
	    exit (1);
	    }
	filteredfile = popen (defmtpgm, "r");
	if (dup2 (savedstdin, 0) != 0)
	    {
	    (void) fprintf (stderr, ISPELL_C_UNEXPECTED_FD, filename,
	      MAYBE_CR (stderr));
	    exit (1);
	    }
	close (savedstdin);
	return filteredfile;
	}
    }

static void update_file (filename, statbuf)
    char *		filename;
    struct stat *	statbuf;
    {
    char		bakfile[MAXPATHLEN];
    int			c;
    char *		pathtail;

    if ((infile = fopen (tempfile, "r")) == NULL)
	{
	(void) fprintf (stderr, ISPELL_C_TEMP_DISAPPEARED, tempfile,
	  MAYBE_CR (stderr));
	(void) sleep ((unsigned) 2);
	return;
	}

#ifdef TRUNCATEBAK
    (void) strncpy (bakfile, filename, sizeof bakfile - 1);
    bakfile[sizeof bakfile - 1] = '\0';
#else /* TRUNCATEBAK */
    (void) sprintf (bakfile, "%.*s%s", (int) (sizeof bakfile - sizeof BAKEXT),
      filename, BAKEXT);
#endif /* TRUNCATEBAK */
    pathtail = last_slash (bakfile);
    if (pathtail == NULL)
	pathtail = bakfile;
    else
	pathtail++;
#ifdef TRUNCATEBAK
    if (strcmp(BAKEXT, filename + strlen(filename) - sizeof BAKEXT + 1) != 0)
	{
	if (strlen (pathtail) > MAXNAMLEN - sizeof BAKEXT + 1)
	    pathtail[MAXNAMLEN - sizeof BAKEXT + 1] = '\0';
	(void) strcat (pathtail, BAKEXT);
	}
#endif /* TRUNCATEBAK */


#ifdef MSDOS
    if (pathconf (filename, _PC_NAME_MAX) <= MAXBASENAMELEN + MAXEXTLEN + 1)
	{
	/*
	** Excessive characters beyond 8+3 will be truncated by the
	** OS.  Ensure the backup extension won't be truncated, and
	** that we don't create an invalid filename (e.g., more than
	** one dot).  Allow use of BAKEXT without a leading dot (such
	** as "~").
	*/
	char *last_dot = rindex (pathtail, '.');

	/*
	** If no dot in backup filename, make BAKEXT be the extension.
	** This ensures we don't truncate the name more than necessary.
	*/
	if (last_dot == NULL  &&  strlen (pathtail) > MAXBASENAMELEN)
	    {
	    pathtail[MAXBASENAMELEN] = '.';
	    /*
	    ** BAKEXT cannot include a dot here (or we would have
	    ** found it above, and last_dot would not be NULL).
	    */
	    strcpy (pathtail + MAXBASENAMELEN + 1, BAKEXT);
	    }
	else if (last_dot != NULL)
	    {
	    char *p = pathtail;
	    size_t ext_len = strlen (last_dot);

	    /* Convert all dots but the last to underscores. */
	    while (p < last_dot && *p)
		{
		if (*p == '.')
		    *p = '_';
		p++;
		}

	    /* Make sure we preserve as much of BAKEXT as we can. */
	    if (ext_len > MAXEXTLEN && ext_len > sizeof (BAKEXT) - 1)
		strcpy (MAXEXTLEN <= sizeof (BAKEXT) - 1
		    ? last_dot + 1
		    : last_dot + MAXEXTLEN - sizeof (BAKEXT) + 1,
		  BAKEXT);
	    }
	}
#endif /* MSDOS */

    if (strncmp (filename, bakfile, pathtail - bakfile + MAXNAMLEN) != 0)
	(void) unlink (bakfile);	/* unlink so we can write a new one. */
#ifdef HAS_RENAME
    (void) rename (filename, bakfile);
#else /* HAS_RENAME */
    if (link (filename, bakfile) == 0)
	(void) unlink (filename);
#endif /* HAS_RENAME */

    /* if we can't write new, preserve .bak regardless of xflag */
    if ((outfile = fopen (filename, "w")) == NULL)
	{
	(void) fprintf (stderr, CANT_CREATE, filename, MAYBE_CR (stderr));
	(void) sleep ((unsigned) 2);
	return;
	}

#ifndef MSDOS
    /*
    ** This is usually a no-op on MS-DOS, but with file-sharing
    ** installed, it was reported to produce empty spelled files!
    ** Apparently, the file-sharing module would close the file when
    ** `chmod' is called.
    */
    (void) chmod (filename, statbuf->st_mode);
#endif

    while ((c = getc (infile)) != EOF)
	(void) putc (c, outfile);

    (void) fclose (infile);
    (void) fclose (outfile);

    if (xflag
      &&  strncmp (filename, bakfile, pathtail - bakfile + MAXNAMLEN) != 0)
	(void) unlink (bakfile);
    }

static void expandmode (option)
    int			option;		/* How to print: */
					/* 1 = expansions only */
					/* 2 = original line + expansions */
					/* 3 = original paired w/ expansions */
					/* 4 = add length ratio */
                                        /* 5 = root + flags used, expansion */
    {
    char		buf[BUFSIZ];
    int			explength;	/* Total length of all expansions */
    register char *	flagp;		/* Pointer to next flag char */
    ichar_t		ibuf[BUFSIZ];
    MASKTYPE		mask[MASKSIZE];
    char		origbuf[BUFSIZ]; /* Original contents of buf */
    char		ratiobuf[20];	/* Expansion/root length ratio */
    int			rootlength;	/* Length of root word */
    register int	temp;

    while (xgets (buf, sizeof buf, stdin) != NULL)
	{
	rootlength = strlen (buf);
	if (buf[rootlength - 1] == '\n')
	  buf[--rootlength] = '\0';
	(void) strcpy (origbuf, buf);
	if ((flagp = index (buf, hashheader.flagmarker)) != NULL)
	    {
	    rootlength = flagp - buf;
	    *flagp++ = '\0';
	    }
	if (option == 2  ||  option == 3  ||  option == 4)
	    (void) printf ("%s ", origbuf);
	if (flagp != NULL)
	    {
	    if (flagp - buf > INPUTWORDLEN)
		buf[INPUTWORDLEN] = '\0';
	    }
	else
	    {
	    if ((int) strlen (buf) > INPUTWORDLEN - 1)
		buf[INPUTWORDLEN] = '\0';
	    }
	(void) fputs (buf, stdout);
	if (flagp != NULL)
	    {
	    (void) BZERO ((char *) mask, sizeof (mask));
	    while (*flagp != '\0'  &&  *flagp != '\n')
		{
		temp = CHARTOBIT ((unsigned char) *flagp);
		if (temp >= 0  &&  temp <= LARGESTFLAG)
		    SETMASKBIT (mask, temp);
		else
		    (void) fprintf (stderr, BAD_FLAG, MAYBE_CR (stderr),
		      (unsigned char) *flagp, MAYBE_CR (stderr));
		flagp++;
		/* Accept old-format dicts with extra slashes */
		if (*flagp == hashheader.flagmarker)
		    flagp++;
		}
	    if (strtoichar (ibuf, (unsigned char *) buf, sizeof ibuf, 1))
		(void) fprintf (stderr, WORD_TOO_LONG (buf));
	    explength = expand_pre ((unsigned char *) origbuf, ibuf, mask,
	      option, (unsigned char *) "");
	    explength += expand_suf ((unsigned char *) origbuf, ibuf, mask, 0,
	      option, (unsigned char *) "");
	    explength += rootlength;
	    if (option == 4)
		{
		(void) sprintf (ratiobuf, " %f",
		  (double) explength / (double) rootlength);
		(void) fputs (ratiobuf, stdout);
		(void) expand_pre ((unsigned char *) origbuf, ibuf, mask, 3,
		  (unsigned char *) ratiobuf);
		(void) expand_suf ((unsigned char *) origbuf, ibuf, mask, 0, 3,
		  (unsigned char *) ratiobuf);
		}
	    }
	(void) putchar ('\n');
	(void) fflush (stdout);
	}
    }

/*
** A trivial wrapper for rindex (file '/') on Unix, but
** saves a lot of ifdef-ing on MS-DOS.
*/
char * last_slash (file)
    char *		file;		/* String to search for / or \ */
    {
#ifdef MSDOS
    char *		backslash;	/* Position of last backslash */
#endif /* MSDOS */
    char *		slash;		/* Position of last slash */

    slash = rindex (file, '/');

#ifdef MSDOS
    /*
    ** We can have both forward- and backslashes; find the rightmost
    ** one of either type.
    */
    backslash = rindex (file, '\\');
    if (slash == NULL  ||  (backslash != NULL  &&  backslash > slash))
	slash = backslash;

    /*
    ** If there is no backslash, but the first two characters are a
    ** letter and a colon, the basename begins right after them.
    */
    if (slash == NULL  &&  file[0] != '\0'  &&  file[1] == ':')
	slash = file + 1;
#endif

    return slash;
    }
