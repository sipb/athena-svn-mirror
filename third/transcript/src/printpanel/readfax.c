/*
 * Copyright (C) 1992 Adobe Systems Incorporated.  All rights reserved.
 *
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/readfax.c,v 1.1.1.1 1996-10-07 20:25:55 ghudson Exp $
 *
 * RCSLog:
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1992/08/21  16:24:13  snichols
 * Release 4.0
 *
 * Revision 1.5  1992/07/14  23:08:22  snichols
 * Added copyright.
 *
 *
 */
#include <stdio.h>
#ifdef XPG3
#include <stdlib.h>
#else
#include "../compat.h"
#endif
#include <string.h>

#define TRUE 1
#define FALSE 0

struct FaxPhoneEntry {
    char option[200];
    char value[200];
};

int GetPhonebookKeys(klist, nkeys, maxkeys, filename)
    char *klist[];
    int *nkeys;
    int *maxkeys;
    char *filename;
{
    FILE *fp;
    char buf[1024];
    char *p;
    int inentry = FALSE;
    int i;

    if ((fp = fopen(filename, "r")) == NULL)
	return -1;

    i = 0;

    while (fgets(buf, 1024, fp)) {
	if (buf[0] == '%')
	    continue;
	if (inentry) {
	    if (buf[0] == '.')
		inentry = FALSE;
	    continue;
	}
	if ((p = (char *)strchr(buf, '\n')) != NULL)
	    *p = '\0';
	klist[i] = (char *) malloc(strlen(buf)+1);
	strcpy(klist[i], buf);
	i++;
	inentry = TRUE;
    }
    *nkeys = i;
    close(fp);
    return 0;
}

static int HandlePhoneOption(line, list, numlist, maxlist)
    char *line;
    struct FaxPhoneEntry list[];
    int *numlist, *maxlist;
{
    char *p, *q;

     p = strchr(line, '=');
    if (p == NULL) {
	p = strchr(line, ':');
	if (p == NULL)
	    return -1;
    }
    *p = '\0';
    p++;
    while (*p == ' ') p++;
    if (line[0] == '/') {
	/* special keyword */
	return -2; /* for now */
    }
    else {
	if (*numlist >= *maxlist) {
	    list = (struct FaxPhoneEntry *)
		realloc(list, 10*sizeof(struct FaxPhoneEntry));  
	    *maxlist += 10;
	}
	strcpy(list[*numlist].option, line);
	strcpy(list[*numlist].value, p);
	(*numlist)++;
    }
}
    

int FindPhoneEntry(key, entlist, nentries, maxentries, dbfile)
    char *key;
    struct FaxPhoneEntry entlist[];
    int *nentries, *maxentries;
    char *dbfile;
{
    FILE *fp;
    char buf[1024];
    char *p;

    if ((fp = fopen(dbfile, "r")) == NULL) {
	fprintf(stderr, "couldn't open %s\n", dbfile);
	return 0;
    }
    while (fgets(buf, 1024, fp)) {
	if (buf[0] == '%')
	    continue;
	p = strchr(buf, '\n');
	if (p) *p = '\0';
	if (strcmp(buf, key) == 0) {
	    while (fgets(buf, 1024, fp)) {
		p = strchr(buf, '\n');
		if (p) *p = '\0';
		if (buf[0] == '%')
		    continue;
		if (buf[0] == '.')
		    break;
		HandlePhoneOption(buf, entlist, nentries, maxentries);
	    }
	}
    }
    return 1;
}
