#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1992  Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psfax.c,v 1.1.1.1 1996-10-07 20:25:51 ghudson Exp $";
#endif
/* psfax.c
 *
 * Copyright (C) 1992 Adobe Systems Incorporated. All
 *  rights reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  1993/11/22  21:28:44  snichols
 * only process list of keys once, and keep the zero'th entry of
 * faxopts for common keys on multicast; start multicast entries at 1.
 *
 * Revision 1.9  1993/05/18  21:37:48  snichols
 * fixed memory-smasher on Sparcs.
 *
 * Revision 1.8  1992/10/15  22:12:32  snichols
 * should be opening output file for writing, not reading.
 *
 * Revision 1.7  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 1.6  1992/07/14  22:00:50  snichols
 * Added copyright
 *
 */
#include <stdio.h>
#ifdef XPG3
#include <stdlib.h>
#else
#include "compat.h"
#endif
#include <string.h>
#include <sys/wait.h>

#include "transcript.h"
#include "config.h"

#define TRUE 1
#define FALSE 0


extern char *optarg;
extern int optind;

extern char *lp;

char list[100][50];		/* holder for parsing keys in list */
int numlist=0;

FILE *db, *doc, *output;

char *dbfile = NULL;
char defdbfile[255];
char *infile = NULL;
char *outfile = NULL;
char *printername = NULL;
char *dialcallee;
char *toname;

int fdin, fdout;

struct entry{
    char option[128];
    char value[256];
    int filename;
};

struct listopts {
    int numopts;		/* number of options for this entry */
    int maxopts;		/* number allocated */
    struct entry **opts;	/* list of options */
} **faxopts;			/* a list, for multicast */
				/* if not multicast, all options are in the
				   zero'th entry; if multicast, zero'th
				   entry contains the options in common
				   among all the recipients */

int numfaxopts = 1;		/* number of faxopts entries */
int maxfaxopts = 1;		/* number allocated */
int multicast = FALSE;
int sendps = FALSE;
int usedb = TRUE;
int treatasstrings=TRUE;

char *comopts[20];		/* options from command line */
int ncomopts = 0;

typedef enum {string, integer, boolean, procedure, arrayint,
			     arraydict} valueTypes;
#define NUMOPTS 30

struct _optionTypes {		/* table for outputting various options */
    char *name;
    valueTypes valueType;
    char *begin;
    char *end;
} optionTypes[NUMOPTS] = {
    { "nooptionstr", string, "(", ")" },
    { "nooptionlit", integer, "", "" }, 
    { "DialCallee", string, "(", ")" },
    { "RecipientName", string, "(", ")" },
    { "RecipientPhone", string, "(", ")" }, 
    { "RecipientOrg", string, "(", ")" },
    { "RecipientMailStop", string, "(", ")" },
    { "RecipientID", string, "(", ")" },
    { "SenderName", string, "(", ")" },
    { "SenderPhone", string, "(", ")" }, 
    { "SenderOrg", string, "(", ")" },
    { "SenderMailStop", string, "(", ")" },
    { "SenderID", string, "(", ")" },
    { "CallerID", string, "(", ")" },
    { "CalleePhone", string, "(", ")" },
    { "CallerPhone", string, "(", ")" },
    { "FaxType", integer, "", "" },
    { "ErrorCorrect", boolean, "", "" },
    { "TrimWhite", boolean, "", "" },
    { "PageCaption", procedure, "{", "}" },
    { "MaxRetries", integer, "", "" },
    { "RetryInterval", integer, "", "" },
    { "MailingTime", arrayint, "[", "]" },
    { "CoverSheet", procedure, "{", "}" },
    { "Confirmation", procedure, "{", "}" },
    { "nPages", integer, "", "" },
    { "CoverSheetOnly", boolean, "", "" },
    { "RevertToRaster", boolean, "", "" },
    { "PostScriptPassword", string, "(", ")" },
    { "Copies", arraydict, "[", "]" }
};

/* forward dec'ls */

static struct listopts *NewFaxOpt();
static void AddOption();
static int FindOption();
static void HandleGroup();
static int FindEntry();

static char *myrealloc(ptr, size)
    char *ptr;
    int size;
{
    char *ret;

    if (ptr == NULL) return malloc(size);
    ret = (char *)realloc(ptr, (unsigned) size);
    if (ret) return ret;
    fprintf(stderr, "realloc failed\n");
    exit(1);
    return NULL;
}

static void HandleCommandLineOptions(opt)
    char *opt;
{
    char *option, *value;
    
    option = opt;
    value = strchr(option, '=');
    if (value) {
	*value = '\0';
	value++;
	AddOption(0, option, value, 0);
    }
    else {
	value = strchr(option, ':');
	if (value) {
	    *value = '\0';
	    value++;
	    AddOption(0, option, value, 1);
	}
    }
}

static int LookupOpt(opt)
    char *opt;
{
    int i;

    for (i = 0; i < NUMOPTS; i++) {
	if (strcmp(opt, optionTypes[i].name) == 0)
	    return i;
    }
    if (treatasstrings)
	return 0;
    else
	return 1;
}

static void DumpOpts(out)
    FILE *out;
{
    int i,j;
    FILE *fp;
    char buf[1024];
    int ndex;

    for (j = 0; j < faxopts[0]->numopts; j++) {
	ndex = LookupOpt(faxopts[0]->opts[j]->option);
	if (faxopts[0]->opts[j]->filename) {
	    if ((fp = fopen(faxopts[0]->opts[j]->value, "r")) == NULL) { 
		fprintf(stderr, "Couldn't open file %s\n",
			faxopts[0]->opts[j]->value); 
		exit(1);
	    }
	    fprintf(out, "/%s ", faxopts[0]->opts[j]->option);
	    fprintf(out, "%s\n",optionTypes[ndex].begin);
	    while (fgets(buf, 1024, fp))
		fputs(buf, out);
	    fprintf(out, "%s def\n", optionTypes[ndex].end);
	}
	else {
	    fprintf(out, "/%s %s%s%s def\n", faxopts[0]->opts[j]->option,
		    optionTypes[ndex].begin, faxopts[0]->opts[j]->value,
		    optionTypes[ndex].end); 
	}
    }
    if (multicast) {
	/* build copies array */
	fprintf(out, "/Copies [\n");
	for (i = 1; i < numfaxopts; i++) {
	    if (i > 1) fprintf(out, "\n");
	    fprintf(out, "<<");
	    for (j = 0; j < faxopts[i]->numopts; j++) {
		if (j > 0) fprintf(out, "\n");
		ndex = LookupOpt(faxopts[i]->opts[j]->option);
		if (faxopts[i]->opts[j]->filename) {
		    if ((fp = fopen(faxopts[i]->opts[j]->value, "r")) ==
			NULL) {  
			fprintf(stderr, "Couldn't open file %s\n",
				faxopts[i]->opts[j]->value); 
			exit(1);
		    }
		    fprintf(out, "/%s %s\n", faxopts[i]->opts[j]->option,
			    optionTypes[ndex].begin);  
		    while (fgets(buf, 1024, fp))
			fputs(buf, out);
		    fprintf(out, "%s", optionTypes[ndex].end);
		}
		else {
		    fprintf(out, "/%s %s%s%s",
			    faxopts[i]->opts[j]->option,
			    optionTypes[ndex].begin,
			    faxopts[i]->opts[j]->value,
			    optionTypes[ndex].end); 
		}
	    }
	    fprintf(out, ">>\n");
	}
	fprintf(out, "] def\n");
    }
}
	
static void DumpPS(out)
    FILE *out;
{

    fprintf(out, "currentfile %d dict dup begin\n",
	    faxopts[0]->numopts+multicast+5); 
    DumpOpts(out);
    fprintf(out, "end\n");
    fprintf(out, "/FaxOps /ProcSet findresource /faxsendps get exec\n");
}

static void DumpFax(out)
    FILE *out;
{

    fprintf(out, "2 dict dup begin /OutputDevice (Fax) def\n");
    fprintf(out, "/FaxOptions %d dict dup begin\n",
	    faxopts[0]->numopts+multicast+5);
    DumpOpts(out);
    fprintf(out, "end def end setpagedevice\n");
}

static void HandleList(s)
    char *s;
{
    char *p, *q;

    p = s;
    while (*p == ' ') p++;

    q = p;
    while ((p = strchr(q, ',')) != NULL) {
	*p = '\0'; p++;
	while (*p == ' ') p++;
	strcpy(list[numlist++], q);
	q = p;
    }
    /* get last one */
    strcpy(list[numlist++], q);
}
     
#define ARGS "P:k:d:f:s:p:n:tlc"

main(argc, argv)
    int argc;
    char **argv;
{
    char *key;
    char *p;
    char c;
    int i;
    char buf[1024];
    int cnt;
    int dumped = FALSE;
    int fdpipe[2];
    int spid;
    int status;
    char *tmp;

    while ((c = getopt(argc, argv, ARGS)) != EOF) {
	switch (c) {
	case 'k':
	    key = optarg;
	    if ((p = strchr(key, ',')) != NULL) {
		/* list of keys */
		HandleList(key);
		key = NULL;
	    }
	    break;
	case 'f':
	    dbfile = optarg;
	    break;
	case 'd':
	    /* DialCallee string */
	    dialcallee = optarg;
	    break;
	case 'S':
	    comopts[ncomopts++] = optarg;
	    break;
	case 't':
	    sendps = TRUE;
	    break;
	case 'c':
	    usedb = FALSE;
	    break;
	case 'n':
	    toname = optarg;
	    break;
	case 'l':
	    treatasstrings = FALSE;
	    break;
	case 'P':
	    printername = optarg;
	    break;
	case 'p':
	    outfile = optarg;
	    break;
	}
    }

    if (argc > optind) 
	infile = argv[optind];
    else infile = NULL;
    faxopts = (struct listopts **)malloc(sizeof(struct listopts *));
    faxopts[0] = (struct listopts *)NewFaxOpt();
    if (usedb) {
	if (dbfile == NULL) {
	    tmp = getenv("PSFAXDB");
	    if (tmp) {
		dbfile = tmp;
	    }
	    else {
		tmp = getenv("HOME");
		if (tmp) {
		    strcpy(defdbfile, tmp);
		    strcat(defdbfile, "/.faxdb");
		    dbfile = defdbfile;
		}
	    }
	}
	if ((db = fopen(dbfile, "r")) == NULL) {
	    fprintf(stderr, "psfax: couldn't open fax database file %s.\n",
		    dbfile); 
	    exit(1);
	}
	FindEntry(0, "always");
	if (key) {
	    if (!FindEntry(0, key)) {
		fprintf(stderr, "psfax: couldn't find entry for %s.\n", key);
		exit(1);
	    }
	}
    }
    if (dialcallee) 
	AddOption(0, "DialCallee", dialcallee, 0);
    if (toname)
	AddOption(0, "RecipientName", toname, 0);

    for (i = 0; i < ncomopts; i++)
	HandleCommandLineOptions(comopts[i]);

    if (numlist > 0)
	HandleGroup();

    if (FindOption(0, "DialCallee") == -1) {
	if (!multicast)  {
	    fprintf(stderr, "psfax: Must specify DialCallee field!\n");
	    exit(1);
	}
	else {
	    for (i = 1; i < numfaxopts; i++) {
		if (FindOption(i, "DialCallee") == -1) {
		    fprintf(stderr, "psfax: Must specify DialCallee field!\n");
		    exit(1);
		}
	    }
	}
    }

    if (sendps && FindOption(0, "PostScriptPassword") == -1) {
	fprintf(stderr,
		"psfax: password may be required when sending PostScript.\n");
    }
    else doc = stdin;
    if (printername && !outfile) {
	/* set up pipe to spooler */
	if (pipe(fdpipe)) {
	    fprintf(stderr, "psfax: pipe to lpr failed.\n");
	    exit(1);
	}
	if ((spid = fork()) < 0) {
	    fprintf(stderr, "psfax: fork failed.\n");
	    exit(1);
	}
	if (spid == 0) {
	    /* child process */
	    if (close(0) || (dup(fdpipe[0]) != 0) || close(fdpipe[0]) ||
		close(fdpipe[1])) {
		fprintf(stderr, "psfax: child failed.\n");
		exit(1);
	    }
	    sprintf(buf, "-P%s", printername);
	    execlp(lp, lp, buf, (char *) 0);
	    fprintf(stderr, "psfax: exec of %s failed\n", lp);
	    exit(1);
	}
	/* parent */
	if (close(1) || (dup(fdpipe[1]) != 1) || close(fdpipe[0]) ||
	    close(fdpipe[1])) {
	    fprintf(stderr, "psfax: parent failed.\n");
	    exit(1);
	}
	if ((output = fdopen(1, "w")) == NULL) {
	    fprintf(stderr, "psfax: error opening pipe.\n");
	    exit(1);
	}
    }
    else {
	if (outfile) {
	    if ((output = fopen(outfile, "w")) == NULL) {
		fprintf(stderr,
			"psfax: couldn't open output file %s.\n", outfile);
		exit(1);
	    }
	}
	else output = stdout;
    }
    if (infile) {
	if ((doc = fopen(infile, "r")) == NULL) {
	    fprintf(stderr, "psfax: couldn't open input file %s.\n", infile);
	    exit(1);
	}
    }
    fgets(buf, 1024, doc);
    if (strncmp(buf, "%!", 2) != 0) {
	fprintf(stderr, "psfax: input is not PostScript file.\n");
	exit(1);
    }
    fputs(buf, output);
    if (sendps)
	DumpPS(output);
    else
	DumpFax(output);
    while (fgets(buf, 1024, doc))
	fputs(buf, output);
    fclose(output);
    wait(&status);
}
		
static struct entry *NewEntry()
{
    struct entry *a;
    unsigned size;

    size = sizeof(struct entry);
    a = (struct entry *)malloc(size);
    if (a) {
	return a;
    }
    else {
	fprintf(stderr, "Error in malloc!");
	perror("");
	exit(1);
    }
}

static struct listopts *NewFaxOpt()
{
    struct listopts *ptr;

    ptr = (struct listopts *)malloc(sizeof(struct listopts));
    ptr->numopts = 0;
    ptr->maxopts = 0;
    ptr->opts = NULL;
    return ptr;
}


static void AddOption(ndex, option, value, name)
    int ndex;
    char *option;
    char *value;
    int name;
{
    /* if option is already in list, replace with new value;
       else, add to list. */
    int i;

    for (i = 0; i < faxopts[ndex]->numopts; i++) {
	if (!strcmp(faxopts[ndex]->opts[i]->option, option)) {
	    strncpy(faxopts[ndex]->opts[i]->value, value, 256);
	    faxopts[ndex]->opts[i]->filename = name;
	    return;
	}
    }
    if (faxopts[ndex]->numopts >= faxopts[ndex]->maxopts) {
	/* need more space, get 20 more */
	faxopts[ndex]->opts =
	    (struct entry **)myrealloc((char *) faxopts[ndex]->opts,  
				       (faxopts[ndex]->maxopts *
					sizeof(struct entry *) + 
					20*sizeof(struct entry *)));
	faxopts[ndex]->maxopts += 20;
    }
    faxopts[ndex]->opts[faxopts[ndex]->numopts] = NewEntry();
    strncpy(faxopts[ndex]->opts[faxopts[ndex]->numopts]->option, option, 128); 
    strncpy(faxopts[ndex]->opts[faxopts[ndex]->numopts]->value, value, 256);
    faxopts[ndex]->opts[faxopts[ndex]->numopts]->filename = name;
    faxopts[ndex]->numopts++;
}

static int FindOption(ndex, str)
    int ndex;
    char *str;
{
    int i;

    for (i = 0; i < faxopts[ndex]->numopts; i++) {
	if (strcmp(str, faxopts[ndex]->opts[i]->option) == 0) {
	    return i;
	}
    }
    return -1;;
}
	    
static void HandleOption(ndex, line)
    int ndex;
    char *line;
{
    char *p;
    char *q;
    int filename = FALSE;
    char *ptr;

    p = strchr(line, '=');
    if (p == NULL) {
	p = strchr(line, ':');
	if (p == NULL) {
	    fprintf(stderr, "readfdb: ill-formed option line %s\n", line);
	    exit(1);
	}
	filename = TRUE;
    }
    *p = '\0';
    p++;
    while (*p == ' ') p++;
    if (line[0] == '/') {
	/* special keyword */
	q = line;
	q++;
	if (strcmp(q, "list") == 0) {
	    /* list of users */
	}
        multicast = TRUE;
	q = p;
	while ((p = strchr(q, ',')) != NULL) {
	    *p = '\0'; p++;
	    while (*p == ' ') p++;
	    strcpy(list[numlist++], q);
	    q = p;
	}
	/* get last one */
	strcpy(list[numlist++], q);
    }	
    else {
	AddOption(ndex, line, p, filename);
    }
}

static int MatchKey(line, key)
    char *line, *key;
{
    char *p, *q;
    int foundit = FALSE;

    if (strcmp(line, key) == 0)
	return TRUE;
    q = line;
    while ((p = strchr(q, '|')) != NULL) {
	*p = '\0'; p++;
	if (strcmp(q, key) == 0) {
	    return TRUE;
	}
	q = p;
    }
    if (strcmp(q, key) == 0)
	return TRUE;
    return FALSE;
}

static char *myfgets(buf, cnt, fp)
    char *buf;
    int cnt;
    FILE *fp;
{
    char *ret;
    char *p;

    ret = fgets(buf, cnt, fp);

    if (ret) {
	p = strchr(buf, '\n');
	if (p) *p = '\0';
    }
    return ret;
}

static void HandleGroup()
{
    int i;

    rewind(db);
    multicast = FALSE; /* for now, to avoid confusing FindEntry */
    for (i = 0; i < numlist; i++) {
	if (numfaxopts >= maxfaxopts) {
	    faxopts =
		(struct listopts **) myrealloc((char *)faxopts,
					       (maxfaxopts *
						sizeof(struct listopts *) + 
						5*sizeof(struct listopts
							 *))); 
	    maxfaxopts += 5;
	}
	faxopts[numfaxopts] = (struct listopts *) NewFaxOpt();
	if (!FindEntry(numfaxopts, list[i])) {
	    fprintf(stderr, "Couldn't find entry for group member %s.\n",
		    list[i]);
	    exit(1);
	}
	numfaxopts++;
	rewind(db);
    }
    multicast = TRUE; /* to notify DumpOptions */
}

static int FindEntry(ndex, key)
    int ndex;
    char *key;
{
    char buf[1024];
    int match = FALSE;
    char *p, *q;

    while (myfgets(buf, 1024, db)) {
	if (buf[0] == '%')
	    continue;
	p = strchr(buf, '\n');
	if (p) *p = '\0';
	if (MatchKey(buf, key)) {
	    while (myfgets(buf, 1024, db)) {
		if (buf[0] == '%')
		    continue;
		if (buf[0] == '.') {
		    return TRUE;
		}
		HandleOption(ndex, buf);
	    }
	}
	else {
	    while (myfgets(buf, 1024, db)) {
		if (buf[0] == '.')
		    break;
	    }
	}
    }
    return FALSE;
}

        
