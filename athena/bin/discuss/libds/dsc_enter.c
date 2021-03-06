/*
 *
 *	Copyright (C) 1988, 1989 by the Massachusetts Institute of Technology
 *    	Developed by the MIT Student Information Processing Board (SIPB).
 *    	For copying information, see the file mit-copyright.h in this release.
 *
 */
/*
 * dsc_enter.c - enter a transaction from a file into discuss.
 *
 *	$Id: dsc_enter.c,v 1.13 1999-02-02 20:40:26 kcr Exp $
 */

#ifndef	lint
static char rcsid[] =
    "$Id: dsc_enter.c,v 1.13 1999-02-02 20:40:26 kcr Exp $";
#endif

#include <stdio.h>
#include <string.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <regex.h>

#include <discuss/tfile.h>
#include <discuss/types.h>

/*
 * Externals.
 */

extern tfile unix_tfile();

#define DEFAULT_SUBJECT "No subject found in mail header"

char *dsc_enter_deflist[] = {
	"^cc$", "^date$", "^from$","^to$","-cc$","-to$","-from$",
	NULL
};
		
static void strndowncpy();
static bool list_compare();

/*
 * The stream `source' is assumed to contain a transaction in
 * approximate RFC822 format.
 *
 * An attempt is made to `censor' its headers and enter it in the
 * discuss meeting `mtg_path' on `mtg_host'.
 */

dsc_enter(file, mtg_host, mtg_path, trn_no)
	FILE *file;
	char *mtg_host, *mtg_path;
	int *trn_no;
{
	return dsc_enter_filter(file, mtg_host, mtg_path,
				dsc_enter_deflist, (char **)NULL, trn_no);
}

/*
 * Macro which defines the matching algorithm.
 */
#define field_ok(key, save, reject) \
	((((save) == NULL) || list_compare((key), (save))) && \
	 (((reject) == NULL) || !list_compare((key), (reject))))

/*
 * The stream `source' is assumed to contain a transaction in
 * approximate RFC822 format.
 *
 * An attempt is made to scan its headers and
 * enter it in the discuss meeting `mtg_path' on `mtg_host'.
 *
 * If `accept_headers' is non-NULL, only headers maching the regexps
 * are allowed.
 *
 * If `reject_headers' is non-NULL, headers maching the regexps in it
 * are explicitly rejected.
 */

dsc_enter_filter(source, mtg_host, mtg_path, accept_headers,
		 reject_headers, trn_no)
	FILE *source;
	char *mtg_host;
	char *mtg_path;
	char *accept_headers[], *reject_headers[]; /* NULL terminated. */
	int *trn_no;			/* Transaction number entered. */
{
	int fd;				/* Temporary file descriptor */
	FILE *f;			/* Temporary file pointer */
	tfile transaction;		/* Also points to temp file. */

	char line[BUFSIZ+1];		/* Current line. */
	char key[BUFSIZ+1];		/* Keyword for line. */
	char subject[BUFSIZ+1];		/* Transaction subject. */

	bool no_subject_yet = TRUE; 	/* No subject seen yet? */
	bool ok_prev = FALSE;		/* Prev header line ok? */
	bool iscont = FALSE;		/* This line is continuation? */

	int reply_to = 0;		/* Transaction number to reply to. */
	int header_count = 0;		/* Header line count */
	
	int fatal;			/* Do we lose contacting the mtg? */
	int result;			/* Return code */

	char filename[60];		/* temporary filename */
	char module[1024];		/* discuss RPC module name */

	register int i;			/* iteration variable */

	static char *subjlist[] = {	/* matches subject: */
		"subject",  NULL
	};

	static char *inreplyto[] = {	/* matches in-reply-to: */
		"in-reply-to",  NULL
	};

	/*
	 * Find a temporary file.
	 */
	(void) mktemp(strcpy(filename, "/tmp/DSXXXXXX"));
	
	if ((fd = open(filename, O_RDWR|O_EXCL|O_CREAT, 0600)) < 0 ||
	    (f = fdopen(fd, "w+")) == NULL) {
		result = errno;
		goto out;
	}
	
	reply_to = 0;
	
	for (;;) {
		if (fgets(line, BUFSIZ, source) == NULL ||
		    line[0] == '\n' ||
		    strcmp(line, "--text follows this line--") == 0)
			break;
		else if (isspace(line[0]))
			iscont = TRUE;
		else if (!iscont) {
			/* skip to colon, if any */
			for (i=0; line[i] && line[i] != ':'; i++)
				continue;
			
			strndowncpy(key, line, i);
			
			if (list_compare (key, subjlist)) {
				/*
				 * skip whitespace at start of 
				 * subject.
				 */	

				while(isspace(line[++i])) ;

				/*
				 * if luser tries two subject lines, we
				 * ignore the second subject line
				 */

				if (no_subject_yet) {
					int len;
					(void) strcpy(subject, line+i);
					len = strlen(subject);
					if (len && (subject[--len] == '\n'))
						subject[len]='\0';
					no_subject_yet = FALSE;
				}	
			} else if (list_compare (key, inreplyto)) {
				char *cp;
				/*
				 * Look for a trn number between [].
				 */
				if ((cp = strchr(line,'[')) && strchr(cp, ']')) {
					cp++;
					if (isdigit(*cp))
						reply_to = atoi(cp);
				}
			}
		}
		ok_prev = ((iscont && ok_prev) ||
			   (!iscont && field_ok(key, accept_headers, reject_headers)));
		if (ok_prev) {
			header_count++;
			fputs(line, f);
		}
		iscont = line[strlen(line)-1] != '\n';
	}
	if (header_count > 0) {
		fputs("\n", f);
	}
	/* copy rest of message */
	while (fgets(line,BUFSIZ,source) != NULL) {
		fputs(line, f);
	}
	/* did it really get there? */
	if (fflush(f) == EOF) {
		result = errno;
		goto out;
	}
	/* overwrite subject */
	if (no_subject_yet)
		(void) strcpy(subject, DEFAULT_SUBJECT);

	/* find the server */
	(void) strcpy(module, "discuss@");
	(void) strcat(&module[8], mtg_host);

	init_rpc();
	set_module(module, &fatal, &result);
	if (result && fatal) goto out;

	/* back to the beginning */
	(void) rewind(f);
	(void) lseek(fileno(f), (long)0, SEEK_SET);
	/* create transaction stream */
	transaction = unix_tfile(fileno(f));

	/* drop it in the meeting */
	add_trn(mtg_path, transaction, subject, reply_to, trn_no, &result);
	
out:
	if (f) {
		(void) fclose(f);
	}
	if (transaction) tdestroy(transaction);
	(void) unlink(filename); 

	return result;
}

static bool list_compare(s,list)
	char *s,**list;
{
	regex_t reg;

	while (*list!=NULL) {
		if (regcomp(&reg, *list++, REG_NOSUB) != 0)
			return(FALSE);
		if (regexec(&reg, s, 0, NULL, 0) == 0) {
			regfree(&reg);
			return(TRUE);
		}
		regfree(&reg);
	}
	return(FALSE);
}

static void strndowncpy(dp, sp, count)
	register char *dp, *sp;
	register int count;
{
	register unsigned char c;

	while ((c = *sp++) && count > 0) {
	    *dp++ = isupper(c) ? tolower(c) : c;
	    count--;
	}
	*dp++ = '\0';
}
