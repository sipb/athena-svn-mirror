/*
 * Copyright (C) 1992 Adobe Systems Incorporated.  All rights reserved.
 *
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/readppd.c,v 1.1.1.1 1996-10-07 20:25:55 ghudson Exp $
 *
 * RCSLog:
 * $Log: not supported by cvs2svn $
 * Revision 4.4  1993/08/23  22:58:34  snichols
 * Ignore constraints that aren't UI features.
 *
 * Revision 4.3  1993/08/19  17:26:20  snichols
 * updated for Motif 1.2, and added missing check for NULL.
 *
 * Revision 4.2  1993/06/18  16:48:15  snichols
 * Check before dereferencing.
 *
 * Revision 4.1  1992/12/15  17:36:24  snichols
 * handle constraints where all options are constrained.
 *
 * Revision 4.0  1992/08/21  16:24:13  snichols
 * Release 4.0
 *
 * Revision 1.6  1992/07/14  23:08:22  snichols
 * Added copyright.
 *
 *
 */


#include <stdio.h>
#include <string.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <Xm/Xm.h>
#if XmVersion > 1001
#include <Xm/ManagerP.h>
#else
#include <Xm/XmP.h>
#endif
#include <Xm/XmP.h>
#include "PrintPaneP.h"

#define FALSE 0
#define TRUE 1

extern char *PPDDir;
/* temporary storage for constrainst */
static char constraints[100][125];
static int nCons = 0;

void InitPPD(p)
    PrintPanelWidget p;
{
    int i;
    int j;
    
    nCons = 0;
/*
    memset(&p->printpanel.features, 0, sizeof(p->printpanel.features));
*/    
    p->printpanel.features.num_uis = 0;
    p->printpanel.features.supports_fax = FALSE;
    for (i = 0; i < 20; i++) {
	p->printpanel.features.uis[i].num_options = 0;
	p->printpanel.features.uis[i].default_option = -1;
	p->printpanel.features.uis[i].key_tran[0] = '\0';
	p->printpanel.features.uis[i].display = TRUE;
	for (j = 0; j < 50; j++) {
	    p->printpanel.features.uis[i].options[j].num_constraints = 0;
	    p->printpanel.features.uis[i].options[j].gray = FALSE;
	    p->printpanel.features.uis[i].options[j].name_tran[0] = '\0';
	}
    }
}

static void ParseConstraintLine(line, keyword, option, nokeyword, nooption)
    char *line;
    char **keyword, **option, **nokeyword, **nooption;
{
    char *p, *q;

    p = line;
    while (*p != '*') p++;
    *keyword = p;
    p = strchr(*keyword, ' ');
    *p = '\0';
    p++;
    while (*p == ' ') p++;
    if (*p != '*') {
	/* we have a constraining option */
	*option = p;
	p = strchr(*option, ' ');
	*p = '\0';
	p++;
	while (*p == ' ') p++;
    }
    else *option = NULL;
    /* constrained keyword */
    *nokeyword = p;
    p = strchr(*nokeyword, ' ');
    if (p) {
	/* possibly a constrained option */
	*p = '\0';
	p++;
	while (*p == ' ') p++;
	if (*p != '\0' && *p != '\n') {
	    *nooption = p;
	    p = strchr(*nooption, ' ');
	    if (p) *p = '\0';
	}
	else
	    *nooption = NULL;
    }
    else *nooption = NULL;
}

static UIData *FindKeyword(p, key)
    PrintPanelWidget p;
    char *key;
{
    int i;
    UIData *uiptr;

    if (key == NULL)
	return NULL;
    
    for (i = 0; i < p->printpanel.features.num_uis; i++) {
	uiptr = &p->printpanel.features.uis[i];
	if (strcmp(uiptr->keyword, key) != 0) {
	    continue;
	}
	/* found the keyword */
	return uiptr;
    }
    return NULL;
}

static OptionData *FindOption(uiptr, opt)
    UIData *uiptr;
    char *opt;
{
    int i;
    OptionData *optptr;

    if (opt == NULL)
	return NULL;
    if (uiptr == NULL)
	return NULL;
    for (i = 0; i < uiptr->num_options; i++) {
	optptr = &(uiptr->options[i]);
	if (strcmp(optptr->name, opt) != 0)
	    continue;
	/* found the option */
	return optptr;
    }
    return NULL;
}

static void AddConstraint(p, optptr, key, opt)
    PrintPanelWidget p;
    OptionData *optptr;
    char *key;
    char *opt;
{
    UIData *up;
    OptionData *op;
    /* if the constraint isn't a UI feature, ignore it */
    if ((up = FindKeyword(p, key)) != NULL)
	optptr->constraints[optptr->num_constraints].no_key = up;
    if ((op = FindOption(up, opt)) != NULL) 
	optptr->constraints[optptr->num_constraints].no_option = op;
    if (up && op)
	optptr->num_constraints++;
}


static void HandleConstraints(p)
    PrintPanelWidget p;
{
    int i, j, k;
    char *key, *opt, *nokey, *noopt;
    UIData *uiptr;
    UIData *noptr;
    OptionData *optptr;

    for (i = 0; i < nCons; i++) {
	ParseConstraintLine(constraints[i], &key, &opt, &nokey, &noopt);
	uiptr = FindKeyword(p, key);
	if (uiptr) {
	    if (opt) {
		optptr = FindOption(uiptr, opt);
		if (optptr) {
		    if (noopt) {
			AddConstraint(p, optptr, nokey, noopt);
		    }
		    else {
			/* all options are constrained */
			noptr = FindKeyword(p, nokey);
			if (noptr) {
			    for (j = 0; j < noptr->num_options; j++) {
				AddConstraint(p, optptr, nokey,
					      noptr->options[j].name);
			    }
			}
		    }
		}	    
	    }
	    else {
		/* all options are constraining */
		for (j = 0; j < uiptr->num_options; j++) {
		    optptr = &(uiptr->options[j]);
		    if ((strcmp(optptr->name, "None") == 0) ||
			(strcmp(optptr->name, "False") == 0))
			continue;
		    AddConstraint(p, optptr, nokey, noopt);
		}
	    }
	}
    }
    /* now, go ahead and mark as gray those options constrained by a
       installable option that isn't installed */

    for (i = 0; i < p->printpanel.features.num_uis; i++) {
	uiptr = &p->printpanel.features.uis[i];
	if (uiptr->display) {
	    /* regular one, not installable */
	    continue;
	}
	if (strcmp(uiptr->options[uiptr->default_option].name, "True") ==
	    0) { 
	    /* it's installed */
	    continue;
	}
	for (j = 0; j < uiptr->num_options; j++) {
	    if (j != uiptr->default_option)
		continue;
	    for (k = 0; k < uiptr->options[j].num_constraints; k++) {
		uiptr->options[j].constraints[k].no_option->gray =
			TRUE; 
	    }
	}
    }
}

int HandleContents(p, fp)
    PrintPanelWidget p;
    FILE *fp;
{
    char *s, *q;
    int ret;
    char *key, *option, *value, *otran, *vtran;
    int midUI = FALSE;
    int midinstall = FALSE;
    char buf[1024];
    char *pbuf;
    char defname[255];
    char tmpname[255];
    int i, j, k;
    UIData *uiptr;
    OptionData *popt;
    char *beg, *end;
    char incname[1024];
    FILE *incfile;
    int tmp;
    int multiline = FALSE;
    int quotestring = FALSE;

    pbuf = buf;
    while (fgets(pbuf, 1024, fp)) {
	if (buf[0] == '*') {
	    if (buf[1] == '%') {
		/* comment */
		continue;
	    }
	    s = strchr(buf, '\n');
	    if (s) *s = '\0';
	    ret = ParseLine(buf, &key, &option, &otran, &value, &vtran);
	    if (ret > 0) {
		if (strncmp(key, "*Include", 8) == 0) {
		    /* strip off delimiters */
		    beg = value;
		    beg++;
		    end = strrchr(beg, *value);
		    if (end) *end = '\0';
		    if (*beg != '/') {
			/* tack on ppddir */
			strcpy(incname, PPDDir);
			strcat(incname, beg);
		    }
		    else
			strcpy(incname, beg);
		    if ((incfile = fopen(incname, "r")) == NULL) {
			fprintf(stderr, "couldn't open include file %s\n",
				incname); 
			continue;
		    }
		    tmp = HandleContents(p, incfile);
		    if (tmp == FALSE)
			return tmp;
		}
		if (*value == '"') {
		    s = value;
		    value++;
		    q = strrchr(value, '"');
		    if (q) 
			*q = '\0';
		    *s = '\0';
		}
		if (strncmp(key, "*FormatVersion", 14) == 0) {
		    strcpy(p->printpanel.features.format_version, value);
		}
		if (strncmp(key, "*Product", 8) == 0) {
		    strcpy(p->printpanel.features.product, value);
		}
		if (strncmp(key, "*PSVersion", 10) == 0) {
		    strcpy(p->printpanel.features.ps_version, value);
		}
		if (strncmp(key, "*ModelName", 10) == 0) {
		    strcpy(p->printpanel.features.model, value);
		}
		if (strncmp(key, "*Nickname", 9) == 0) {
		    strcpy(p->printpanel.features.nickname, value);
		}
		if (strncmp(key, "*FreeVM", 7) == 0) {
		    p->printpanel.features.free_vm = atoi(value);
		}
		if (strncmp(key, "*LanguageLevel", 15) == 0) {
		    p->printpanel.features.language_level = atoi(value);
		}
		if (strncmp(key, "*ColorDevice", 12) == 0) {
		    if (strcmp(value, "True") == 0)
			p->printpanel.features.color_device = TRUE;
		}
		if (strncmp(key, "*FaxSupport", 11) == 0) {
		    p->printpanel.features.supports_fax = TRUE;
		}
		if (strncmp(key, "*DefaultResolution", 18) == 0) {
		    strcpy(p->printpanel.features.default_resolution, value);
		}
		if (strncmp(key, "*UIConstraints", 14) == 0) {
		    strcpy(constraints[nCons++], value);
		}
		if (strcmp(key, "*OpenGroup") == 0 &&
		    strcmp(value,"InstallableOptions") == 0 ) {
		    /* special case of group */
		    midinstall = TRUE;
		}
		if (strncmp(key, "*OpenUI", 7) == 0) {
		    uiptr = FindKeyword(p, option);
		    if (uiptr == NULL) {
			uiptr =
			    &p->printpanel.features.uis[p->printpanel.features.num_uis];
			/* we've already seen this one, so we've already
			   gotten these values */
			if (strncmp(value, "PickOne", 7) == 0)
			    uiptr->type = PICKONE;
			if (strncmp(value, "PickMany", 8) == 0)
			    uiptr->type = PICKMANY;
			if (strncmp(value, "Boolean", 7) == 0)
			    uiptr->type = BOOL;
			if (otran) {
			    strcpy(uiptr->key_tran, otran);
			}
			else {
			    uiptr->key_tran[0] = '\0';
			}
			strcpy(uiptr->keyword, option);
		    }
		    strcpy(defname, "*Default");
		    s = uiptr->keyword + 1;
		    strcat(defname, s);
		    midUI = TRUE;
		    if (midinstall) {
			uiptr->display = FALSE;
		    }
		    else {
			uiptr->display = TRUE;
		    }
		}
		if (midUI) {
		    if (strncmp(key, defname, strlen(defname)) == 0) {
			if (FindOption(uiptr, tmpname) == NULL) {
			    strcpy(tmpname, value);
			}
			else tmpname[0] = '\0';
			continue;
		    }
		    if (strncmp(key, uiptr->keyword,
				strlen(uiptr->keyword)) == 0) {
			/* option! */
			if (FindOption(uiptr, option) == NULL) {
			    if (otran) {
				strcpy(uiptr->options[uiptr->num_options].name_tran,
				       otran);  
			    }
			    else {
				uiptr->options[uiptr->num_options].name_tran[0]
				    = '\0';
			    }
			    strcpy(uiptr->options[uiptr->num_options].name,
				   option);
			    if (strcmp(option, tmpname) == 0) {
				uiptr->default_option = uiptr->num_options;
			    }
			    uiptr->num_options++;
			}
		    }
		    if (strncmp(key, "*CloseUI", 8) == 0) {
			if (strncmp(value, uiptr->keyword,
				    strlen(uiptr->keyword)) == 0) { 
			    midUI = FALSE;
			    defname[0] = '\0';
			    tmpname[0] = '\0';
			    p->printpanel.features.num_uis++;
			    uiptr =
				&p->printpanel.features.uis[p->printpanel.features.num_uis];
			}
			else {
			    fprintf(stderr,
				    "CloseUI doesn't match OpenUI\n");
			    return FALSE;
			}
		    }
		}
		if (strcmp(key, "*CloseGroup") == 0 &&
		    strcmp(value,"InstallableOptions") == 0 ) {
		    midinstall = FALSE;
		}
	    }
	}
    }
}
    
    
		
int ReadPPD(p, fp)
    PrintPanelWidget p;
    FILE *fp;
{
    if (fp == NULL) return FALSE;
    InitPPD(p);
    HandleContents(p, fp);
    HandleConstraints(p);
    return TRUE;
}
    
