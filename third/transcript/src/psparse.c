#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psparse.c,v 1.2 1996-10-14 05:00:53 ghudson Exp $";
#endif
/* psparse.c
 *
 * Copyright (C) 1990,1991,1992 Adobe Systems Incorporated. All rights
 *  reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 *
 *
 * $/Log: psparse.c,v/$
 * Revision 3.20  1994/05/03  23:01:32  snichols
 * Handle null in name of resource, and handle files with neither prolog
 * nor setup comments.
 *
 * Revision 3.19  1994/02/16  00:31:48  snichols
 * support for Orientation comment
 *
 * Revision 3.18  1993/12/21  23:43:08  snichols
 * cast strlen for Solaris.
 *
 * Revision 3.17  1993/12/01  21:04:27  snichols
 * initialize some variables, and loop looking for resource files.
 *
 * Revision 3.16  1993/11/17  22:33:32  snichols
 * make sure that null bytes in the data don't prematurely terminate
 * the output.
 *
 * Revision 3.15  1993/08/06  22:56:51  snichols
 * fixed typos.
 *
 * Revision 3.14  1993/08/06  22:55:15  snichols
 * pname should be static.
 *
 * Revision 3.13  1993/07/27  17:50:17  snichols
 * Corrected a typo.
 *
 * Revision 3.12  1993/05/25  21:38:59  snichols
 * cleanup for Solaris.
 *
 * Revision 3.11  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.10  1992/08/20  00:38:41  snichols
 * Even if noparse is desired, should check for file type and conformance,
 * so clients can decide what to do.
 *
 * Revision 3.9  1992/07/27  20:02:19  snichols
 * Added support for BINARYOK environment variable, to allow control over
 * whether or not ASCII85 encoding is done after LZW compression.
 *
 * Revision 3.8  1992/07/14  22:37:51  snichols
 * Updated copyright.
 *
 * Revision 3.7  1992/06/29  23:57:31  snichols
 * line wrapped when I didn't want it to.
 *
 * Revision 3.6  1992/06/29  19:49:45  snichols
 * handle RequiresPageRegion, and warn user CustomPageSize isn't
 * supported.
 *
 * Revision 3.5  1992/06/23  19:41:28  snichols
 * fixed up problem where sometimes psparse was trying to open a null pathname.
 *
 * Revision 3.4  1992/05/28  21:20:04  snichols
 * support for handling Multiple Master fonts.
 *
 * Revision 3.3  1992/05/18  21:51:13  snichols
 * removed unnecessary debugging statement.
 *
 * Revision 3.2  1992/05/18  20:07:57  snichols
 * Added support for shrink to fit, and for translating by page length on
 * landscape as well as translating by page width.  Also has support for
 * long lines, as long as they aren't comments.
 *
 * Revision 3.1  1992/05/05  22:10:34  snichols
 * support for adding showpage to end of file.
 *
 * Revision 3.0  1991/06/17  16:59:26  snichols
 * Release 3.0
 *
 * Revision 1.29  1991/06/17  16:58:33  snichols
 * add more checks to make sure array limits aren't exceeded, and
 * increase array limits.
 *
 * Revision 1.28  1991/05/31  21:13:10  snichols
 * Since we're strncat'ing rather than strncpy'ing in StrCap, we need to
 * zero out r each time.
 *
 * Revision 1.27  1991/05/14  23:16:52  snichols
 * fixed bug in comment stripping code which was stripping out the prolog.
 *
 * Revision 1.26  1991/05/14  19:20:47  snichols
 * make sure command line feature code gets inserted properly.
 *
 * Revision 1.25  1991/05/13  20:50:39  snichols
 * The array that StrCap returns should be static.
 *
 * Revision 1.24  1991/04/29  22:52:44  snichols
 * if we get to end of CheckPrinter, return null, since we're checking for
 * null on the other end.
 *
 * Revision 1.23  1991/04/23  19:29:16  snichols
 * GetName would loop infinitely on names with ('s.
 *
 * Revision 1.22  1991/03/28  23:47:32  snichols
 * isolated code for finding PPD files to one routine, in psutil.c.
 *
 * Revision 1.21  1991/03/27  01:03:18  snichols
 * command line feature requests should override feature requests in file.
 *
 * Revision 1.20  1991/03/25  23:15:19  snichols
 * moved CheckForTray to appear before invocation.
 *
 * Revision 1.19  1991/03/21  22:58:02  snichols
 * if manual feed and no page specified anywhere else, use the one in
 * the PPD file as the default.
 *
 * Revision 1.18  1991/03/21  18:53:15  snichols
 * if printername is > 10 characters and can't find ppd, truncate to 10 and
 * check again.
 *
 * Revision 1.17  1991/03/06  21:54:54  snichols
 * missing arg to fprintf.
 *
 * Revision 1.16  1991/03/01  15:10:44  snichols
 * needed to declare ResourceDir extern.
 *
 * Revision 1.15  91/03/01  14:42:04  snichols
 * don't die if procset doesn't have version and revision.
 * 
 * Revision 1.14  91/02/19  16:46:45  snichols
 * support for additional resource types other than the currently defined ones,
 * new resource location package, and general clean-up for readability.
 * 
 * Revision 1.13  91/02/07  13:49:53  snichols
 * fixed landscape bugs.
 * 
 * Revision 1.12  91/01/30  14:32:21  snichols
 * fixed some bugs with parsing version and revision numbers on procsets.
 * 
 * Revision 1.11  91/01/23  16:32:25  snichols
 * Added support for landscape, cleaned up some defaults, and
 * handled *PageRegion features better.
 * 
 * Revision 1.10  91/01/17  13:23:08  snichols
 * extra debugging stuff.
 * 
 * Revision 1.9  91/01/16  16:46:15  snichols
 * SysV doesn't like union wait.
 * 
 * Revision 1.8  91/01/16  14:14:07  snichols
 * Added support for LZW compression and ascii85 encoding for Level 2 printers.
 * 
 * Revision 1.7  91/01/02  16:34:40  snichols
 * conditional compile for XPG3 to use SEEK_SET instead of L_SET in lseek.
 * 
 * Revision 1.6  90/12/12  10:29:36  snichols
 * new configuration stuff.
 * 
 * Revision 1.5  90/12/03  16:52:40  snichols
 * handled nested documents better.
 * 
 * Revision 1.4  90/11/16  14:45:22  snichols
 * Support for range printing: only download resource if it's going to be
 * used within the range.
 * 
 * Revision 1.2  90/09/17  17:46:54  snichols
 * Added support for PPD features; cleaned up output considerably
 * Now puts fonts, procsets in correct places (setup or prolog)
 * outputs appropriate setup and prolog comments.
 * 
 *
 */


#include <stdio.h>
#ifdef XPG3
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include "transcript.h"
#include "docman.h"
#include "psparse.h"
#include "PSres.h"


#ifdef XPG3
#define SEEKING SEEK_SET
#else
#define SEEKING L_SET
#endif

int SetupCompression();

static struct resource resources[MAXRESOURCES];  /* one entry per resource */

static struct dtable dloads[MAXRESOURCES];  /* one per potential download */

static struct feature features[MAXRESOURCES]; /* one per feature */

static int neededResources[MAXRESTYPES][MAXRESOURCES];  /* list by type of resouces
						 not supplied, points into
						 resources  */
static int suppliedResources[MAXRESTYPES][MAXRESOURCES]; /* list by type of resources
						  supplied, points into
						  resources  */

static long remcomment[MAXREM];  /* location of comments to be
					  removed from the original */

static int ndloads = 0;               /* counters for above data structures */
static int nresources = 0;
static int nfeatures = 0;
static int numrestypes = 6;
static int nNR[MAXRESTYPES];
static int nSR[MAXRESTYPES];
static int nrem = 0;


static int seenHeaderComment[HEADCOM]; /* only first header comment per
					  type is interesting, so if we've
					  seen one, we want to ignore rest */



static FILE *ppd;
static int noppd = FALSE;

static int alreadycompressed = FALSE;

static char docpagesize[50];
static char specialhandled = FALSE;

static int version;  /* version of DSC used */

static long endlast = -1;  
static long endpro = -1;
static int npages = 0;
static int ordered = TRUE;

static int defaultSection = FALSE;

static struct pageinfo {
    long location;
    char label[20];
    int  pageno;
} ptable[2000];


static long prologend = -1;
static long prologbegin = -1;
static long setupbegin = -1;
static long firstpages = -1;
static long setupend = -1;
static long vmtotal = 0;
static int exitonerror = FALSE;

int fd1pipe[2];
int fd2pipe[2];


static void VMUsage(min,max,fp)
    long *min, *max;
    FILE *fp;
{
    char buf[255];
    char *p, *q;
    
    while (fgets(buf,255,fp))
	if (!strncmp(buf,"%%VMusage:",10)) {
	p = strchr(buf,':');
	p++;
	while (*p == ' ') p++;
	q = strchr(p,' ');
	*q++ = '\0';
	*min = atol(p);
	p = strchr(q,'\n');
	if (p) *p = '\0';
	*max = atol(q);
	return;
    }
}

static int IncRemIndex()
{
    int old;
    if (nrem >= MAXREM) {
	fprintf(stderr,"psparse: too many comments to remove.\n");
	exit(2);
    }
    old = nrem++;
    return old;
}

static char *DecodeType(type)
    int type;
{
    if (!strcmp(resmap[type],"font"))
	return PSResFontOutline;
    if (!strcmp(resmap[type],"form"))
	return PSResForm;
    if (!strcmp(resmap[type],"pattern"))
	return PSResPattern;
    if (!strcmp(resmap[type],"encoding"))
	return PSResEncoding;
    if (!strcmp(resmap[type],"procset"))
	return PSResProcSet;
    return resmap[type];
}

    

static char *FindResource(resname,type, resourcepath, minvm, maxvm)
    char *resname;
    int type;
    char *resourcepath;
    long *minvm, *maxvm;
{
    FILE *tfp;
    char **pathnames;
    char **names;
    static char pname[TSPATHSIZE];
    char *r,*q;
    int cnt;
    char mmname[255];
    char *tmpresname;
    int i;

    if (type == RFILE) {
	/* if it's a file, just use file name. */
	strncpy(pname,resname,PSBUFSIZ);
	if ((tfp = fopen(pname,"r")) == NULL) 
	    return NULL;
	else {
	    fclose(tfp);
	    return pname;
	}
    }
    else {
	tmpresname = resname;
	if (type == FONT) {
	    /* might be a multiple master font, and if it is, we need to
	       strip down to the real name */
	    strcpy(mmname, resname);
	    r = strchr(mmname, '_');
	    if (r) {
		/* it is a multiple master font! */
		*r = '\0';
		tmpresname = mmname;
	    }
	}
	       
	cnt = ListPSResourceFiles(resourcepath, ResourceDir,
				  DecodeType(type), tmpresname, &names,
				  &pathnames); 
	if (cnt == 0)
	    return NULL;
	if (cnt > 1) 
	    fprintf(stderr,"Warning: more than one filename found for resource %s\n",
		    resname);
	for (i = 0; i < cnt; i++) {
	    if ((tfp = fopen(pathnames[i],"r")) != NULL) {
		VMUsage(minvm,maxvm,tfp);
		fclose(tfp);
		return pathnames[0];
	    }
	}
    }
    return NULL;
}

static void InitPrinterInfo(printer)
    char *printer;
{
    char ppdfile[255];

    ppd = GetPPD(printer);
    /* On Athena, we want to have a default printer description, so we
     * don't have to maintain a file for every single printer...
     * "default.ppd" is as good a name as any.
     * --bert 15apr1996
     */
    if (ppd == NULL)  ppd = GetPPD("default");

    if (ppd == NULL)
	noppd = TRUE;
    else
	noppd = FALSE;
}

    

static char *CheckPrinter(resname,type,printer)
    char *resname;
    int type;
    char *printer;
{
    char *q;
    int i;

    if (noppd) {
	if (type == FONT) {
	    for (i=0;i<35;i++) {
		if (!strcmp(standardfonts[i],resname))
		    return standardfonts[i];
	    }
	}
	return NULL;
    }
    else {
	if (type == FONT) {
	    rewind(ppd);
	    q = parseppd(ppd,"*Font",resname);
	    return q;
	}
    }
    return NULL;
}

static int NeededComment(comment)
    enum comtypes comment;
{
    if (comment == docneedfont || comment == docneedfile || comment ==
	docneedproc || comment == docneedres)
	return TRUE;
    return FALSE;
}

static int SuppliedComment(comment)
    enum comtypes comment;
{
    if (comment == docsupfont || comment == docsupfile || comment ==
	docsupproc || comment == docsupres)
	return TRUE;
    return FALSE;
}

static int BeginComment(comment)
    enum comtypes comment;
{
    if (comment == begfont || comment == begfile || comment == begproc ||
	comment == begres)
	return TRUE;
    return FALSE;
}

static int IncComment(comment)
    enum comtypes comment;
{
    if (comment == incfont || comment == incfile || comment == incproc ||
	comment == incres)
	return TRUE;
    return FALSE;
}

static int DocComment(comment)
    enum comtypes comment;
{
    if (comment == docfont || comment == docproc)
	return TRUE;
    return FALSE;
}

static int PageComment(comment)
    enum comtypes comment;
{
    if (comment == pagefont || comment == pagefile || comment == pageres)
	return TRUE;
    return FALSE;
}

static int ResourceType(q)
    char *q;
{
    int i;

    for (i=0; i<numrestypes; i++) 
	if (!strcmp(q, resmap[i]))
	    return i;
    /* new kind of resource */
    if (numrestypes == MAXRESTYPES) {
	fprintf(stderr,"psparse: too many different types of resources requested!\n");
	exit(10);
    }
    strncpy(resmap[numrestypes++],q,30);
    return numrestypes-1;
}

static int ResourceChange(q)
    char *q;
{
    int i;

    for (i=0; i<numrestypes; i++)
	if (!strcmp(q, resmap[i]))
	    return TRUE;
    return FALSE;
}

static void FillInResource(ndex,path,name,type,state,comment,minvm,maxvm,ver,rev)
    int ndex;
    char *path;
    char *name;
    int type;
    enum resstates state;
    enum comtypes comment;
    long minvm, maxvm;
    float ver;
    int rev;
{
    enum rangestates rangeinfo;

    if (ndex >= MAXRESOURCES) {
	fprintf(stderr,"psparse: too many resource requests.\n");
	exit(2);
    }
    if (DocComment(comment) || NeededComment(comment))
	rangeinfo = header;
    else if (defaultSection)
	rangeinfo = defaults;
    else if (PageComment(comment))
	rangeinfo = pageinrange;
    else rangeinfo = undefined;
    if (path)
	strncpy(resources[ndex].path,path,PSBUFSIZ);
    else resources[ndex].path[0] = '\0';
    if (name)
	strncpy(resources[ndex].name,name,PSBUFSIZ);
    else resources[ndex].name[0] = '\0';
    resources[ndex].type = type;
    resources[ndex].state = state;
    resources[ndex].comment = comment;
    resources[ndex].rangeinfo = rangeinfo;
    resources[ndex].inrange = FALSE;
    resources[ndex].minvm = minvm;
    resources[ndex].maxvm = maxvm;
    vmtotal += maxvm;
    resources[ndex].version = ver;
    resources[ndex].rev = rev;
}

static void FillInDLoad(ndex,location,type,rndex,comment)
    int ndex;
    long location;
    int type;
    int rndex;
    enum comtypes comment;
{
    if (ndex >= MAXRESOURCES) {
	fprintf(stderr,"psparse: too many download requests.\n");
	exit(2);
    }
    dloads[ndex].location = location;
    dloads[ndex].type = type;
    dloads[ndex].index = rndex;
    dloads[ndex].comment = comment;
}
    

static long checkatend(type)
    int type;
{
    int i;
    for (i=0;i<ndloads;i++)
	if (dloads[i].index == -1 && dloads[i].type == type) 
	    return dloads[i].location;
    return -1;
}

static int AlreadySupplied(ndex,type)
    int ndex;
    int type;
{
    int i;
    for (i=0; i<nSR[type];i++)
	if (suppliedResources[type][i] == ndex)
	    return TRUE;
    return FALSE;
}

static VOID MarkHeaderComment(comment)
    enum comtypes comment;
{
    /* this (and the following routine) are done this way because non-ANSI
       C compilers won't allow the use of enum's as array indices */

    switch (comment) {
    case docfont:
	seenHeaderComment[0]++;
	break;
    case docneedfont:
	seenHeaderComment[1]++;
	break;
    case docsupfont:
	seenHeaderComment[2]++;
	break;
    case docneedres:
	seenHeaderComment[3]++;
	break;
    case docsupres:
	seenHeaderComment[4]++;
	break;
    case docneedfile:
	seenHeaderComment[5]++;
	break;
    case docsupfile:
	seenHeaderComment[6]++;
	break;
    case docproc:
	seenHeaderComment[7]++;
	break;
    case docneedproc:
	seenHeaderComment[8]++;
	break;
    case docsupproc:
	seenHeaderComment[9]++;
	break;
    default: break;
    }
}
	


static int RepeatedHeaderComment(comment)
    enum comtypes comment;
{
    switch (comment) {
    case docfont:
	if (seenHeaderComment[0]) return TRUE;
	break;
    case docneedfont:
	if (seenHeaderComment[1]) return TRUE;
	break;
    case docsupfont:
	if (seenHeaderComment[2]) return TRUE;
	break;
    case docneedres:
	if (seenHeaderComment[3]) return TRUE;
	break;
    case docsupres:
	if (seenHeaderComment[4]) return TRUE;
	break;
    case docneedfile:
	if (seenHeaderComment[5]) return TRUE;
	break;
    case docsupfile:
	if (seenHeaderComment[6]) return TRUE;
	break;
    case docproc:
	if (seenHeaderComment[7]) return TRUE;
	break;
    case docneedproc:
	if (seenHeaderComment[8]) return TRUE;
	break;
    case docsupproc:
	if (seenHeaderComment[9]) return TRUE;
	break;
    default:
	return FALSE;
	break;
    }
    return FALSE;
}

static int InRange(pagenum, ranges)
    int pagenum;
    struct range ranges[];
{
    int i;

    /* if *no* ranges specified, i.e. all pages in range, return true */
    if (ranges[0].begin == 0 && ranges[0].end == 0)
	return TRUE;
    for (i = 0; i < MAXRANGES && ranges[i].begin > 0; i++)
	if (pagenum >= ranges[i].begin && pagenum <= ranges[i].end)
	    return TRUE;
    return FALSE;
}

static char *GetName(str)
    char **str;
{
    char *q, *p;
    char *name;
    int depth = 0;

    if (*str == NULL || **str == '\0') return NULL;
    while (**str == ' ') (*str)++;
    q = strchr(*str,'\n');
    if (q) *q = '\0';
    p = *str;
    name = p;
    if (*p == '(') {
	depth++;
	p++;
	while (q = strchr(p,'(')) {
	    depth++;
	    p = q;
	    p++;
	}
	while (q = strchr(p,')')) {
	    depth--;
	    p = q;
	    p++;
	}
	if (depth) {
	    fprintf(stderr,"psparse: mismatched parentheses in %s.\n",name);
	    if (exitonerror)
		exit(2);
	}
    }
    else
	p = strchr(*str,' ');
    if (p) *p++ = '\0';
    *str = p;
    return name;
}
	
    

static int CheckResources(reslist, kind, location, state, comment, printer,
			  resourcepath, cont, control)
    char *reslist;
    int kind;
    long location;
    enum resstates state;
    enum comtypes comment;
    char *printer;
    char *resourcepath;
    int cont;
    struct controlswitches control;
{
    char *p,*q;
    char *name;
    int i,j;
    long loc;
    long minvm = 0, maxvm = 0;
    int depth = 0;
    int first = TRUE;
    float version = 0.0;
    int rev = 0;
    
    if (RepeatedHeaderComment(comment) && !cont)
	return;
    p = reslist;
    while (name = GetName(&p)) {
	if (name[0] == '\0') continue;
        if (kind == UNKNOWN) {
	    kind = ResourceType(name);
	    if (p == NULL) break;
	    continue;
	}
	if (!strncmp(name,"(atend)",7)) {
	    /* need to save this location away, in case we need it again */
	    /* don't need to do this for pagefont, pagefile, etc. since */
	    /* we won't need it again */
	    if (!PageComment(comment)) 
		FillInDLoad(ndloads++,location,kind,-1,comment);
	    return;
	}
	/* is this a proc set?  if so, grab version and revision off input
	   now, unless we've run out of line, in which case we have to
	   assume that there isn't a version and revision */
	if (kind == PROCSET && p != NULL) {
	    while (*p == ' ') p++;
	    sscanf(p,"%f %d",&version, &rev);
	    /* now, skip over those two */
	    q = strchr(p,' ');
	    if (q) {
		p = q; p++;
		q = strchr(p,' ');
		if (q) {
		    p = q; p++;
		}
		else {
		    /* end of line */
		    *p = '\0';
		}
	    }
	}
	/* mark header comments as seen here, after the (atend) check */
	/* we don't want to ignore second one if first one was (atend) */
	MarkHeaderComment(comment);
	if (ResourceChange(name))
	    kind = ResourceType(name);
	/* have we seen this resource before? */
	for (i=0;i<nresources;i++) {
	    if (!strcmp(resources[i].name,name)) {
		/* begin comments */
		/* previously seen in a supplied comment, or even a
		   previous begin */
		if (BeginComment(comment)) {
		    if (resources[i].state != indocument) {
			/* first time we've seen this in a begin */
			resources[i].state = indocument;
			suppliedResources[kind][nSR[kind]++] = i;
		    }
		    break;
		}
		if (IncComment(comment)) {
		    /* include comm */
		    /* previously seen in a needed comment, or previous
		       include */
		    /* if resource is in printer, we're done with this one */
		    if (resources[i].state == inprinter) {
			resources[i].comment = comment;
			break;
		    }
		    /* if it's not in the printer, we're going to download
		       it, so get rid of the comment */
		    remcomment[IncRemIndex()] = location;
		    /* could have also been seen in a docfont or docproc,
		       so need to check for that */
		    for (j = 0; j < ndloads; j++) {
			if (dloads[j].comment == docfont && dloads[j].index
			    == i) {
			    dloads[j].location = location;
			    dloads[j].comment = comment;
			    break;
			}
			if (dloads[j].comment == docproc && dloads[j].index
			    == i) {
			    dloads[j].location = location;
			    dloads[j].comment = comment;
			    break;
			}
		    }
		    if (j != ndloads) {
			break;
		    }
		    /* if there had been a doc{font,proc}, all this is
		       already filled in */
		    resources[i].state = download;
		    if (!AlreadySupplied(i,kind))
			suppliedResources[kind][nSR[kind]++] = i;
		    FillInDLoad(ndloads++,location,kind,i,comment);
		    break;
		}
		if (comment == pagefont || comment == pagefile || comment
		    == pageres) {
		    if (defaultSection)
			resources[i].rangeinfo = defaults;
		    else {
			if (resources[i].rangeinfo == defaults)
			    break;
			if (InRange(ptable[npages - 1].pageno,control.ranges))
			    resources[i].rangeinfo = pageinrange;
			else
			    resources[i].rangeinfo = undefined;
		    }
		    break;
		}
		break;
	    }
	}
	if (i != nresources) {
	    if (p == NULL) return;
	    continue;
	}
	/* parse ppd file to check existence of resource in printer */
	q = CheckPrinter(name,kind,printer);
	if (q) {
	    FillInResource(nresources, (char *)NULL, name, kind, inprinter,
			   comment, minvm, maxvm, version, rev);
	    if (nNR[kind] >= MAXRESOURCES) {
		fprintf(stderr,"psparse: too many needed resources.\n");
		exit(2);
	    }
	    neededResources[kind][nNR[kind]++] = nresources++;
            if (p == NULL) return;
	    continue;
	}
	/* check along resource path for existence of resource */
	q = FindResource(name,kind,resourcepath,&minvm,&maxvm);
	if (q == NULL) {
	    if (NeededComment(comment) || IncComment(comment)) {
		fprintf(stderr,"psparse: Couldn't find resource %s\n",name);
		if (exitonerror)
		    exit(2);
	    }
	    if (DocComment(comment)) {
		FillInResource(nresources++, (char *)NULL, name, kind,
			       nowhere, comment, minvm, maxvm, version, rev);
	    }
	    if (SuppliedComment(comment)) {
	        FillInResource(nresources++, (char *)NULL, name, kind,
			       verify, comment, minvm, maxvm, version, rev);
	    }
	    if (BeginComment(comment)) {
		FillInResource(nresources, (char *)NULL, name, kind,
			       indocument, comment, minvm, maxvm, version,
			       rev);
		if (nSR[kind] >= MAXRESOURCES) {
		    fprintf(stderr,"psparse: too many supplied resources.\n");
		    exit(2);
		}
		suppliedResources[kind][nSR[kind]++] = nresources++;
	    }
	    if (p == NULL) return;
	    continue;
	}
	/* it's not in the printer, we found it, we've never seen it before */
	FillInResource(nresources, q, name, kind, state, comment, minvm,
		       maxvm, version, rev);
	if (state == download) {
	    /* need to download here */
	    suppliedResources[kind][nSR[kind]++] = nresources;
	    /* if it's an inc, don't need to check atends, just dload here */
	    if (IncComment(comment)) {
		FillInDLoad(ndloads++,location,kind,nresources,comment);
		remcomment[IncRemIndex()] = location;
	    }
	    if (DocComment(comment)) {
		 /* check for atend entries */
		loc = checkatend(kind);
		if (loc > 0) location = loc;
		FillInDLoad(ndloads++,location,kind,nresources,comment);
	    }
	}
	nresources++;
	if (p == NULL) return;
    }
    return;
}

static char *StrCap(str)
    char *str;
{
    char *q,*p;
    static char r[30];
    int i;

    if (str == NULL || *str == '\0') return NULL;
    for (i = 0; i < 30; i++) r[i] = '\0';
    p = str;
    q = str;
    q++;
    r[0] = toupper(*p);
    strncat(r,q,30);
    return r;
}

static void PrologAndSetup()
{
    int i;

    if (setupbegin == -1) {
	if (prologend != -1)
	    setupbegin = prologend;
	else
	    setupbegin = firstpages;
    }
    if (setupend == -1) {
	if (prologend != -1)
	    setupend = prologend;
	else
	    setupend = firstpages;
    }
    if (prologend == -1) return;
    for (i=0;i<ndloads;i++) {
	/* check procsets to make sure they're within prolog boundaries */
	if (dloads[i].type == PROCSET) {
	    if (dloads[i].location < prologbegin)
		dloads[i].location = prologbegin;
	    if (dloads[i].location > prologend)
		dloads[i].location = prologend;
	}
	/* check fonts to make sure they're within setup boundaries */
	if (dloads[i].type == FONT)
	    /* only check begin, because vm considerations may put a font
	       outside setup boundaries */
	    if (dloads[i].location < setupbegin)
		dloads[i].location = setupbegin;
    }
}

static int realcnt[MAXRESTYPES];

static void RangeResources()
{
    int i, j;

    for (i = 0; i < MAXRESTYPES; i++)
	realcnt[i] =0;
    for (i = 0; i< nresources; i++) {
	if (resources[i].rangeinfo == pageinrange || resources[i].rangeinfo
	    == defaults) {
	    resources[i].inrange = TRUE;
	    realcnt[resources[i].type]++;
	}
    }
    for (i = 0; i < numrestypes; i++) {
	if (realcnt[i] == 0) {
	    /* didn't find %%PageResources for this type in either the
	       defaults or the pages in the range, so must use everything
	       in header  */
	    for (j = 0; j < nresources; j++) {
		if (resources[j].type == i)
		    resources[j].inrange = TRUE;
	    }
	}
    }
}
	    


static void ReArrange(printer)
    char *printer;
{
    long printervm;
    char *value;
    int i;
    int moved[MAXRESOURCES];

    if (noppd)
	return;
    if (prologend == -1) return;
    for (i=0;i<MAXRESOURCES;i++)
	moved[i] = 0;
    rewind(ppd);
    value = parseppd(ppd,"*FreeVM",(char *)NULL);
    if (value == NULL) return;
    printervm = atol(value);
    if ((printervm - vmtotal) > 100000) {
	/* perhaps some rearranging? */
	for (i=0;i<ndloads;i++) {
	    if (dloads[i].type == FONT) {
		if (dloads[i].location > setupend) {
		    if (moved[dloads[i].index] > 0)
			dloads[i].index = -1;
		    dloads[i].location = setupbegin;
		    moved[dloads[i].index]++;
		}
	    }
	}
    }
}
    
    
static void DownLoadResource(resindex,output,strip)
    int resindex;
    FILE *output;
    int strip;
{
    FILE *fp;
    char buf[BUFSIZ];

    if (resources[resindex].state != download)
	return;
    if (resources[resindex].inrange != TRUE)
	return;
    if (resources[resindex].path[0] == '\0') {
	fprintf(stderr, "psparse: couldn't find resource file for %s\n",
		resources[resindex].name);
	if (exitonerror)
	    exit(2);
    }
    else {
	if ((fp = fopen(resources[resindex].path, "r")) == NULL) {
	    fprintf(stderr, "Couldn't open resource file %s.\n",
		    resources[resindex].path); 
	    if (exitonerror)
		exit(2);
	}
	if (!strip) {
	    if (version == VERS3) {
		fprintf(output, "%%%%BeginResource: ");
		fprintf(output, "%s ", resmap[resources[resindex].type]);
		fprintf(output, "%s", resources[resindex].name);
		if (resources[resindex].type == PROCSET)
		    fprintf(output,"%5.1f %d", resources[resindex].version,
			    resources[resindex].rev);
		fprintf(output,"\n");
	    }
	    else {
		fprintf(output, "%%%%Begin");
		fprintf(output, "%s: ", StrCap(resmap[resources[resindex].type]));
		fprintf(output, "%s", resources[resindex].name);
		if (resources[resindex].type == PROCSET)
		    fprintf(output,"%5.1f %d", resources[resindex].version,
			    resources[resindex].rev);
		fprintf(output, "\n");
	    }
	}
	while (fgets(buf, BUFSIZ, fp))
	    fputs(buf, output);
	if (!strip) {
	    if (version == VERS3)
		fprintf(output, "%%%%EndResource\n");
	    else {
		fprintf(output, "%%%%End");
		fprintf(output, "%s\n",
			StrCap(resmap[resources[resindex].type])); 
	    }
	}
    }
}

static void OutputInclude(output,ndex, type)
    FILE *output;
    int ndex;
    int type;
{
    if (version > VERS21) {
	fprintf(output, "%%%%IncludeResource: ");
	fprintf(output,"%s ",resmap[type]);
    }
    else {
	fprintf(output, "%%%%Include");
	fprintf(output,"%s: ",StrCap(resmap[type]));
    }
    fprintf(output,"%s\n",resources[ndex].name);
}

static void CheckIncludes(output)
    FILE *output;
{
    int i;

    for (i=0;i<nresources;i++) {
	if (resources[i].state == inprinter) {
	    if (NeededComment(resources[i].comment) || DocComment(resources[i].comment))
		OutputInclude(output,i,resources[i].type);
	}
    }
}

static void NewLine(output,count,s)
    FILE *output;
    int *count;
    char *s;
{
    fprintf(output,"\n%%%%+ ");
    *count = 3;
    if (s) {
	fprintf(output,"%s ",s);
	*count += strlen(s);
    }
}

static void OutputComments(output)
    FILE *output;
{
    int i;
    int k;
    int count = 0;
    int prev = FALSE;
    char *str;

    if (version == VERS3) {
	for (k = 0; k < numrestypes; k++) {
	    if (nNR[k] > 0) {
		if (!prev) {
		    fprintf(output, "%%%%DocumentNeededResources: ");
		    prev = TRUE;
		    count = 27;
		}
		else {
		    fprintf(output, "\n%%%%+ ");
		    count = 4;
		}
		str = resmap[k];
		fprintf(output, "%s ", str);
		count += strlen(str) + 1;
		for (i = 0; i < nNR[k]; i++) {
		    if (neededResources[k][i] >= 0 &&
		      resources[neededResources[k][i]].inrange) {
			count += strlen(resources[neededResources[k][i]].name);
			if (k == PROCSET) {
			    count += 10;
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s %5.1f %d ",
				    resources[neededResources[k][i]].name,
				    resources[neededResources[k][i]].version,
				    resources[neededResources[k][i]].rev); 
			}
			else {
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s ",
				    resources[neededResources[k][i]].name);
			}
		    }
		}
	    }
	}
	if (prev)
	    fprintf(output, "\n");
	prev = FALSE;
	for (k = 0; k < numrestypes; k++) {
	    if (nSR[k] > 0) {
		if (!prev) {
		    fprintf(output, "%%%%DocumentSuppliedResources: ");
		    prev = TRUE;
		    count = 29;
		}
		else {
		    fprintf(output, "\n%%%%+ ");
		    count = 4;
		}
		str = resmap[k];
		fprintf(output, "%s ", str );
		count += strlen(str) + 1;
		for (i = 0; i < nSR[k]; i++) {
		    if (suppliedResources[k][i] >= 0) {
			if (resources[suppliedResources[k][i]].state ==
			  download) {
			    if (!resources[suppliedResources[k][i]].inrange)
				continue;
			}
			count += strlen(resources[suppliedResources[k][i]].name);
			if (k == PROCSET) {
			    count += 10;
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s %5.1f %d ",
				    resources[suppliedResources[k][i]].name,
				    resources[suppliedResources[k][i]].version,
				    resources[suppliedResources[k][i]].rev); 
			}
			else {
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s ",
				    resources[suppliedResources[k][i]].name);
			}
		    }
		}
	    }
	}
	if (prev)
	    fprintf(output, "\n");
    }
    else {
	for (k = 0; k <= PROCSET; k++) {
	    if (nNR[k] > 0) {
		fprintf(output, "%%%%DocumentNeeded");
		count = 16;
		str = StrCap(resmap[k]);
		fprintf(output, "%ss: ", str);
		count += strlen(str) + 3;
		for (i = 0; i < nNR[k]; i++) {
		    if (neededResources[k][i] >= 0) {
			count += strlen(resources[neededResources[k][i]].name);
			if (k == PROCSET) {
			    count += 10;
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s %5.1f %d ",
				    resources[neededResources[k][i]].name,
				    resources[neededResources[k][i]].version,
				    resources[neededResources[k][i]].rev); 
			}
			else {
			    if (count > PSBUFSIZ)
				NewLine(output, &count, "");
			    fprintf(output, "%s ",
				    resources[neededResources[k][i]].name);
			}
		    }
		}
		fprintf(output, "\n");
		count = 0;
	    }
	}
	for (k = 0; k < PROCSET; k++) {
	    if (nSR[k] > 0) {
		fprintf(output, "%%%%DocumentSupplied");
		count = 18;
		str = StrCap(resmap[k]);
		fprintf(output, "%ss: ", str);
		count += strlen(str) + 3;
		for (i = 0; i < nSR[k]; i++) {
		    if (suppliedResources[k][i] >= 0) {
			count += strlen(resources[suppliedResources[k][i]].name);
			if (k == PROCSET) {
			    count += 10;
			    if (count > PSBUFSIZ)
				NewLine(output, &count, str);
			    fprintf(output, "%s %5.1f %d ",
				    resources[suppliedResources[k][i]].name,
				    resources[suppliedResources[k][i]].version,
				    resources[suppliedResources[k][i]].rev); 
			}
			else {
			    if (count > PSBUFSIZ)
				NewLine(output, &count, "");
			    fprintf(output, "%s ",
				    resources[suppliedResources[k][i]].name);
			}
		    }
		}
		fprintf(output, "\n");
		count = 0;
	    }
	}
    }
}

int dcompar(a,b)
#ifdef __STDC__
    const void *a, *b;
#else
    void *a, *b;
#endif    
{
    return(((struct dtable *)a)->location -
	   ((struct dtable *)b)->location);
}

int rcompar(a,b)
#ifdef __STDC__
    const void *a, *b;
#else
    void *a, *b;
#endif    
{
    return (*(long *)a - *(long *)b);
}

int fcompar(a,b)
#ifdef __STDC__
    const void *a, *b;
#else
    void *a, *b;
#endif    
{
    return(((struct feature *)a)->location -
	   ((struct feature *)b)->location);
}

static void BuildDownLoadTable()
{
/* sort download table for easier downloading */
    qsort((void *)dloads,ndloads,sizeof(struct dtable),dcompar);
    qsort((void *)remcomment,nrem,sizeof(long),rcompar);
    qsort((void *)features,nfeatures,sizeof(struct feature),fcompar);
}

static long finddloc(loc)
    long loc;
{
    int i;

    for (i = 0;i<ndloads;i++)
	if (dloads[i].location >= loc)
	    return i;
    return -1;
}

static long findrloc(loc)
    long loc;
{
    int i;

    for (i = 0;i<nrem;i++)
	if (remcomment[i] >= loc)
	    return i;
    return -1;
}

static long findfloc(loc)
    long loc;
{
    int i;

    for(i=0;i<nfeatures;i++)
	if (features[i].location >= loc)
	    return i;
    return -1;
}

static char *PageSizeGiven();

static int CheckForSpecial(feats, nfeats)
    struct comfeatures feats[];
    int nfeats;
{
    int i;
    for (i=0; i<nfeats; i++) {
	if (strncmp(feats[i].keyword,"*InputSlot",10) == 0)
	    return TRUE;
	if (strncmp(feats[i].keyword, "*ManualFeed", 11) == 0)
	    return TRUE;
    }
    return FALSE;
}

static int CheckOverrides(request, feats, nfeats)
    char *request;
    struct comfeatures feats[];
    int nfeats;
{
    int i;

    for (i = 0; i < nfeats; i++) {
	if (!strcmp(request, feats[i].keyword)) {
	    /* a match */
	    return TRUE;
	}
    }
    return FALSE;
}
			  
    

static void HandleFeatures(buf,location,command,control)
    char *buf;			/* string containing feature */
    long location;		/* where this feature should go */
    int command;		/* was this requested on command line? */
    struct controlswitches control;
{
    char *feature, *option, *p;
    char *value;
    char defaultvalue[255];
    char *size;
    int doregion = FALSE;


    if (noppd) {
	fprintf(stderr,"psparse: can't include printer specific feature without PPD file!\n");
	if (exitonerror)
	    exit(2);
	return;
    }
    if (nfeatures >= MAXRESOURCES) {
	fprintf(stderr,"psparse: too many feature requests.\n");
	exit(2);
    }
    
    feature = buf;
    while (*feature == ' ') feature++;
      p = strrchr(feature,'\n');
    if (p) *p = '\0';
    option = strchr(feature,' ');
    if (option) {
	*option = '\0';
	option++;
    }
    if (!command) {
	/* check for overrides; command line takes precedence */
	if (CheckOverrides(feature, control.features, control.nfeatures))
	    return;
    }
    if (strcmp(feature, "*CustomPageSize") == 0 && strcmp(option, "False")
	!= 0) {
	/* feature is CustomPageSize, and it's not set to False */
	fprintf(stderr, "psparse: CustomPageSize is not ");
	fprintf(stderr, "supported in this print manager\n");
	if (exitonerror)
	    exit(2);
	return;
    }
    
    /* check default first */
    p = feature; p++;
    if (option) {
	strncpy(defaultvalue,"*Default",255);
	strncat(defaultvalue,p,255);
	rewind(ppd);
	value = parseppd(ppd,defaultvalue,(char *)NULL);
	if (value) {
	    if (!strcmp(value,option)) {
		if (!strncmp(feature,"*PageSize",9))
		    strncpy(docpagesize,option,50);
		return;
	    }
	}
    }
    rewind(ppd);
    value = parseppd(ppd,feature,option);
    if (value) {
	if (!strncmp(feature,"*PageSize",9)) {
	    strncpy(docpagesize,option,50);
	    /* need to see if input tray or manual feed is or has been
	       requested */ 
	    /* if it has, then page size code is handled there with
	       *PageRegion, so we don't put in *PageSize code */
	    if (specialhandled)
		/* already seen a tray or manual feed request */
		return;
	    if (CheckForSpecial(control.features, control.nfeatures))
		/* will see a tray or manual feed request */
		return;
	}
	strncpy(features[nfeatures].name,feature,255);
	if (option)
	    strncpy(features[nfeatures].option,option,255);
	else
	    features[nfeatures].option[0] = '\0';
	features[nfeatures].code = value;
	features[nfeatures++].location = location;
	if (!command)
	    remcomment[IncRemIndex()] = location;
	if (!strncmp(feature,"*ManualFeed",11) &&
	    !strncmp(option,"True",4) && command) {
	    /* if manual feed is requested, need to use PageRegion as well */
	    specialhandled = TRUE;
	    size = PageSizeGiven(control.features,control.nfeatures);
	    if (size) {
		rewind(ppd);
		value = parseppd(ppd,"*PageRegion",size);
		if (value) {
		    strncpy(features[nfeatures].name,"*PageRegion",255);
		    strncpy(features[nfeatures].option,size,255);
		    features[nfeatures].code = value;
		    features[nfeatures++].location = location;
		}
		else {
		    fprintf(stderr,"psparse: no PageRegion of size %s available.\n",
			    size);
		    if (exitonerror)
			exit(2);
		}
	    }
	    else {
		fprintf(stderr,"psparse: size must be provided for manual feed.\n");
		if (exitonerror)
		    exit(2);
	    }
	}
	if (!strncmp(feature,"*InputSlot",10) && command) {
	    /* if input tray is requested, may need to use PageRegion as
	       well */ 
	    specialhandled = TRUE;
	    rewind(ppd);
	    value = parseppd(ppd, "*RequiresPageRegion", option);
	    if (value) {
		if (strcmp(value, "True") == 0) {
		    doregion = TRUE;
		}
		else {
		    doregion = FALSE;
		}
	    }
	    else {
		rewind(ppd);
		value = parseppd(ppd, "*RequiresPageRegion", "All");
		if (value) {
		    if (strcmp(value, "True") == 0) {
			doregion = TRUE;
		    }
		    else {
			doregion = FALSE;
		    }
		}
		else doregion = FALSE;
	    }
	    if (doregion) {
		doregion = FALSE;
		size = PageSizeGiven(control.features,control.nfeatures);
		if (size) {
		    rewind(ppd);
		    value = parseppd(ppd,"*PageRegion",size);
		    if (value) {
			strncpy(features[nfeatures].name,
				"*PageRegion",255); 
			strncpy(features[nfeatures].option,size,255);
			features[nfeatures].code = value;
			features[nfeatures++].location = location;
		    }
		    else {
			fprintf(stderr,
				"psparse: no PageRegion of size %s available.\n", size);
			if (exitonerror)
			    exit(2);
		    }
		}
		else {
		    fprintf(stderr,
			    "psparse: size must be provided for input slot.\n");
		    if (exitonerror)
			exit(2);
		}
	    }
	}
    }
    else {
	fprintf(stderr,"psparse: Unsupported printer feature: %s ",feature);
	if (option)
	    fprintf(stderr,"%s.\n",option);
	else fprintf(stderr,".\n");
	if (exitonerror) exit(2);
    }
}

static void IncludeFeature(output,ndex,strip)
    FILE *output;
    int ndex;
    int strip;
{
    if (!strip) {
	fprintf(output,"%%%%BeginFeature: %s ",features[ndex].name);
	if (features[ndex].option)
	    fprintf(output,"%s\n",features[ndex].option);
	else fprintf(output,"\n");
    }
    fputs(features[ndex].code,output);
    fprintf(output,"\n");
    if (!strip)
	fprintf(output,"%%%%EndFeature\n");
}

static void CommandLineFeatures(control)
    struct controlswitches control;
{
    int i;
    long loc;
    char tmp[100];

    if (setupbegin > -1)
	loc = setupbegin;
    else if (endpro > -1)
	loc = endpro;
    else loc = 0;

    for (i = 0; i < control.nfeatures; i++) {
	strncpy(tmp,control.features[i].keyword,100);
	if (control.features[i].option) {
	    strncat(tmp," ",100);
	    strncat(tmp,control.features[i].option,100);
	}
	HandleFeatures(tmp,loc,TRUE,control);
    }
}
    
static void CheckPrinterLevel(level,printer)
    int level;
    char *printer;
{
    char *q;
    int plevel;


    if (!noppd) {
	rewind(ppd);
	q = parseppd(ppd,"*LanguageLevel",NULL);
	if (q) 
	    plevel = atoi(q);
	else
	    plevel = 1;
	if (level > plevel) {
	    fprintf(stderr,"Warning: document language level greater than printer language level!\n");
	    if (exitonerror)
		exit(2);
	}
    }
}

static int Level2()
{
    char *q;
    int level;

    if (!noppd) {
	rewind(ppd);
	q = parseppd(ppd,"*LanguageLevel",NULL);
	if (q) {
	    level = atoi(q);
	    if (level == 2)
		return TRUE;
	}
    }
    return FALSE;
}

static void CheckSize(p)
    char *p;
{
    char *q;
    
    while (*p++ == ' ');
    q = strchr(p,' ');
    if (q) {
	*q++ = '\0';
	if (!strncmp(p,"*PageSize",9)) {
	    p = strchr(q,'\n');
	    *p = '\0';
	    strncpy(docpagesize,q,50);
	}
    }
}


static char *PageSizeGiven(feats, nfeats)
    struct comfeatures feats[];
    int nfeats;
{
    int i;
    char *size;

    for (i=0; i<nfeats; i++) {
	if (strncmp(feats[i].keyword,"*PageSize",9) == 0)
	    return feats[i].option;
	if (strncmp(feats[i].keyword, "*PageRegion", 11) == 0)
	    return feats[i].option;
    }
    if (docpagesize[0] != '\0')
	return docpagesize;
    if (!noppd) {
	rewind(ppd);
	size = parseppd(ppd,"*DefaultPageSize", NULL);
	if (size) {
	    strncpy(docpagesize, size, 50);
	    return docpagesize;
	}
    }
    return NULL;
}
static void ShrinkToFit(out, control)
    FILE *out;
    struct controlswitches control;
{
    float x, y;
    float scale;
    char *str;
    char *size;

    if (noppd) {
	fprintf(stderr, "psparse: must specify a printer for squeezing.\n");
	if (exitonerror)
	    exit(2);
    }
    else {
	rewind(ppd);
	size = PageSizeGiven(control.features, control.nfeatures);
	if (size) {
	    rewind(ppd);
	    str = parseppd(ppd, "*PaperDimension", size);
	    if (str) {
		sscanf(str, "%f %f", &x, &y);
		scale = x/y;
		fprintf(out, "%5.1f %5.1f scale\n", scale, scale);
	    }
	    else {
		fprintf(stderr,
			"psparse: no *PaperDimension entry! using default.\n");
		scale = 612.0/792.0;
		fprintf(out, "%5.1f %5.1f scale\n", scale, scale);
	    }
	}
	else {
	    fprintf(stderr,
		    "psparse: must specify a page size for squeezing.\n");
	    if (exitonerror)
		exit(2);
	}
    }
}
		
static void DoLandscape(out,control)
    FILE *out;
    struct controlswitches control;
{
    float x, y;
    char *str;
    char *size;

    if (!noppd) {
	rewind(ppd);
	size = PageSizeGiven(control.features, control.nfeatures);
	if (size) {
	    rewind(ppd);
	    str = parseppd(ppd,"*PaperDimension",size);
	    if (str) {
		sscanf(str,"%f %f",&x,&y);
		if (control.upper)
		    fprintf(out,"90 rotate 0 %5.1f translate\n",-y);
		else
		    fprintf(out,"90 rotate 0 %5.1f translate\n",-x);
	    }
	    else {
		fprintf(stderr,
			"psparse: no *PaperDimension entry! using defaults.\n");
		if (control.upper)
		    fprintf(out,"90 rotate 0 %5.1f translate\n",-612);
		else
		    fprintf(out,"90 rotate 0 %5.1f translate\n",-792);
		
	    }
	}
	else {
	    fprintf(stderr,
		    "psparse: must specify a page size for landscape.\n");
	    if (exitonerror)
		exit(2);
	}
    }
    else {
	fprintf(stderr,"psparse: must specify a printer for landscape.\n");
	if (exitonerror)
	    exit(2);
    }
}
		

static enum comtypes ParseComment(line)
    char *line;
{
    if (!strncmp(line,"%%LanguageLevel:",16))
	return langlevel;
    if (!strncmp(line,"%%DocumentFonts:",16))
	return docfont;
    if (!strncmp(line,"%%Page:",7))
	return page;
    if (!strncmp(line, "%%Pages:", 8))
	return pages;
    if (!strncmp(line,"%%Trailer",9))
	return trailer;
    if (!strncmp(line,"%%BeginDocument",15))
	return begdoc;
    if (!strncmp(line,"%%EndDocument",13))
	return enddoc;
	/* version 2.0 (2.1?) comments */
    if (!strncmp(line,"%%DocumentNeededFonts:",22))
	return docneedfont;
    if (!strncmp(line,"%%DocumentSuppliedFonts:",24)) 
	return docsupfont;
    if (!strncmp(line,"%%IncludeFont:",14)) 
	return incfont;
    if (!strncmp(line,"%%BeginFont:",12)) 
	return begfont;
    if (!strncmp(line,"%%EndFont",9))
	return endfont;
    if (!strncmp(line,"%%PageFonts:", 12))
	return pagefont;
    if (!strncmp(line,"%%DocumentNeededFiles:",22))
	return docneedfile;
    if (!strncmp(line,"%%DocumentSuppliedFiles:",24))
	return docsupfile;
    if (!strncmp(line,"%%PageFiles:",12))
	return pagefile;
    if (!strncmp(line,"%%IncludeFile:",14)) 
	return incfile;
    if (!strncmp(line,"%%BeginFile:",12)) 
	return begfile;
    if (!strncmp(line,"%%EndFile",9))
	return endfile;
    if (!strncmp(line,"%%DocumentProcSets:",19))
	return docproc;
    if (!strncmp(line,"%%DocumentNeededProcSets:",25))
	return docneedproc;
    if (!strncmp(line,"%%DocumentSuppliedProcSets:",27))
	return docsupproc;
    if (!strncmp(line,"%%IncludeProcSet:",17)) 
	return incproc;
    if (!strncmp(line,"%%BeginProcSet:",15)) 
	return begproc;
    if (!strncmp(line,"%%EndProcSet",12))
	return endproc;
       /* version 3 comments */
    if (!strncmp(line,"%%DocumentNeededResources:",26))
	return docneedres;
    if (!strncmp(line,"%%DocumentSuppliedResources:",28))
	return docsupres;
    if (!strncmp(line,"%%PageResources:",16))
	return pageres;
    if (!strncmp(line,"%%IncludeResource:",18))
	return incres;
    if (!strncmp(line,"%%BeginResource:",15))
	return begres;
    if (!strncmp(line,"%%EndResource",13))
	return endres;
    if (!strncmp(line,"%%IncludeFeature:",17))
	return incfeature;
    if (!strncmp(line,"%%Feature:",10))
	return incfeature;
    if (!strncmp(line,"%%BeginFeature:",15))
	return begfeature;
    if (!strncmp(line,"%%EndFeature",12))
	return endfeature;
    if (!strncmp(line,"%%BeginBinary",13))
	return begbin;
    if (!strncmp(line,"%%EndBinary",11))
	return endbin;
    if (!strncmp(line,"%%BeginData",11))
	return begdata;
    if (!strncmp(line,"%%EndData",9))
	return enddata;
    if (!strncmp(line,"%%BeginProlog", 13))
	return begprolog;
    if (!strncmp(line,"%%EndProlog",11))
	return endprolog;
    if (!strncmp(line,"%%BeginSetup",12))
	return begsetup;
    if (!strncmp(line,"%%EndSetup",10))
	return endsetup;
    if (!strncmp(line,"%%EndComments",13))
	return endcomment;
    if (!strncmp(line,"%%PageOrder:",11))
	return pageorder;
    if (!strncmp(line,"%%BeginDefaults",15))
	return begdef;
    if (!strncmp(line,"%%EndDefaults",13))
	return enddef;
    if (!strncmp(line, "%%Orientation:", 14))
	return orient;
    return invalid;
}

enum status HandleComments(input,printer,resourcepath,control)
    FILE *input;
    char *printer;
    char *resourcepath;
    struct controlswitches control;
{
    char buf[PSBUFSIZ];
    int i;
    long location;
    char *p,*q;
    enum comtypes context;
    int skip = 0;
    int dontchecklength = 0;
    int continuation = FALSE;
    int level;
    char tempstr[20];


    InitPrinterInfo(printer);
    exitonerror = !(control.force);
    fgets(buf,PSBUFSIZ,input);
    if (strncmp(buf,"%!",2))
	return notps;
    if (strncmp(buf,"%!PS-Adobe-",11))
	return notcon;
    if (control.noparse) return success;

    for (i = 0; i < MAXRESTYPES; i++) 
	nNR[i] = nSR[i] = 0;

    /* okay, we've got a conforming file, let's find out the version */
    for (i = 11; buf[i] != ' ' && buf[i] != '\n'; i++)
	if (buf[i] != '.')
	    version = version * 10 + buf[i] - '0';

    /* parse on! */
    CommandLineFeatures(control);
    location = ftell(input);
    while (fgets(buf,PSBUFSIZ,input)) {
	if ((p = strchr(buf, '\n')) == NULL) {
	    /* tolerate long lines, except on comments */
	    if (buf[0] == '%') {
		fprintf(stderr,
			"psparse: comment line too long!\n");
		if (exitonerror) 
		    exit(2);
	    }
	    else {
		fprintf(stderr,
			"psparse: line longer than 255 characters\n");
		fprintf(stderr, "psparse: proceeding anyway\n");
		while (fgets(buf, PSBUFSIZ, input)) {
		    if ((p = strchr(buf, '\n')) != NULL)
			break;
		}
	    }
	}
	if (!strncmp(buf,"%%%NOPARSE",10))
	    return handled;
	if (!strncmp(buf,"%%%PARSED:",10)) {
	    /* check printer: might have been handled for different printer */
	    p = strchr(buf,':'); p++;
	    while (*p == ' ') p++;
	    if (!strcmp(printer,p))
		return handled;
	}
	if (!strncmp(buf,"%%%CMP",6))
	    alreadycompressed = TRUE;
	if (prologbegin == -1)
	    if (buf[0] != '%' || !strncmp(buf,"% ",2) ||
		!strncmp(buf,"%\n",2) || !strncmp(buf,"%\t",2)) {
		prologbegin = location;
	    }
	if (strncmp(buf,"%%",2)) {
	    location = ftell(input);
	    continue;
	}
	p = strchr(buf,':');
	if (p) p++;
	if (strncmp(buf,"%%+",3)) {
	    context = ParseComment(buf);
	    continuation = FALSE;
	}
	else {
	    continuation = TRUE;
	    p = strchr(buf,'+');
	    p++;
	}
	switch (context) {
	case begdef:
	    defaultSection = TRUE;
	    break;
	case enddef:
	    defaultSection = FALSE;
	    break;
	case langlevel:
	    q = strrchr(p,'\n');
	    if (q) *q = '\0';
	    level = atoi(p);
	    CheckPrinterLevel(level,printer);
	    break;
	case pageorder:
	    if (!strncmp(p,"(atend)",7))
		break;
	    if (!strncmp(p,"Special",7))
		ordered = FALSE;
	    break;
	case pages:
	    if (firstpages == -1)
		firstpages = location;
	    break;
	case page:
	    if (!skip) {
		ptable[npages].location = location;
		sscanf(buf,"%s %s %d",tempstr, ptable[npages].label,
		       &ptable[npages].pageno);
		npages++;
		if (endpro == -1) endpro = location;
	    }
	    break;
	case trailer:
	    if (!skip) {
		endlast = location;
		ptable[npages].location = location;
	    }
	    break;
	case begdoc:
	    skip++;
	    break;
	case enddoc:
	    skip--;
	    break;
	case incfeature:
	    HandleFeatures(p,location, FALSE, control);
	case begfeature:
	    CheckSize(p);
	case orient:
	    /* check to see if it matches what we want */
	    /* we're only going to modify things if control.landscape is
	       true *and* the Orientation comment says Portrait.  If
	       control.landscape is false, we're not changing the
	       orientation of the original document, so we shouldn't change
	       the comment */
	    if (control.landscape) {
		if (strncmp(p, "Portrait", 8) == 0) {
		    /* it's not what we want */
		    remcomment[IncRemIndex()] = location;
		}
	    }
	    break;
	case begsetup:
	    if (!skip) {
		remcomment[IncRemIndex()] = location;
		setupbegin = location;
	    }
	    break;
	case endsetup:
	    if (!skip) {
		remcomment[IncRemIndex()] = location;
		setupend = location;
	    }
	    break;
	case endcomment:
	case begprolog:
	    if (!skip) {
		remcomment[IncRemIndex()] = location;
		prologbegin = location;
	    }
	    break;
	case endprolog:
	    if (!skip) {
		remcomment[IncRemIndex()] = location;
		prologend = location;
	    }
	    break;
	case docfont:
	    if (skip) break;
	    CheckResources(p, FONT, location, download, context, printer,
			   resourcepath, continuation, control);
	    remcomment[IncRemIndex()] = location;
	    break;
	case docneedfont:
	    if (skip) break;
	    CheckResources(p, FONT, location, future, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case docsupfont:
	    if (skip) break;
	    CheckResources(p, FONT, location, verify, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case pagefont:
	    if (skip) break;
	    CheckResources(p, FONT, location, future, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case incfont:
	    if (skip) break;
	    CheckResources(p, FONT, location, download, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case begfont:
	    if (skip) break;
	    CheckResources(p, FONT, location, indocument, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case endfont:
	    break;
	case docneedfile:
	    if (skip) break;
	    CheckResources(p, RFILE, location, future, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case docsupfile:
	    if (skip) break;
	    CheckResources(p, RFILE, location, verify, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case pagefile:
	    if (skip) break;
	    CheckResources(p, RFILE, location, future, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case incfile:
	    CheckResources(p, RFILE, location, download, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case begfile:
	    if (skip) break;
	    CheckResources(p, RFILE, location, indocument, context,
			   printer, resourcepath, continuation, control); 
	    break;
	case endfile:
	    break;
	case docproc:
	    if (skip) break;
	    CheckResources(p, PROCSET, location, download, context,
			   printer, resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case docneedproc:
	    if (skip) break;
	    CheckResources(p, PROCSET, location,  future,  context,
			   printer, resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case docsupproc:
	    if (skip) break;
	    CheckResources(p, PROCSET, location, verify, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case incproc:
	    CheckResources(p, PROCSET, location, download, context,
			   printer, resourcepath, continuation, control); 
	    break;
	case begproc:
	    if (skip) break;
	    CheckResources(p, PROCSET, location, indocument, context,
			   printer, resourcepath, continuation, control); 
	    break;
	case endproc:
	    break;
	case docneedres:
	    if (skip) break;
	    CheckResources(p, UNKNOWN, location, future, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case docsupres:
	    if (skip) break;
	    CheckResources(p, UNKNOWN, location, verify, context, printer,
			   resourcepath, continuation, control); 
	    remcomment[IncRemIndex()] = location;
	    break;
	case pageres:
	    if (skip) break;
	    CheckResources(p, UNKNOWN, location, future, context, printer,
			   resourcepath, continuation, control); 
	    break;
	case incres:
	    CheckResources(p, UNKNOWN, location, download, context,
			   printer, resourcepath, continuation, control); 
	    break;
	case begres:
	    if (skip) break;
	    CheckResources(p, UNKNOWN, location, indocument, context,
			   printer, resourcepath, continuation, control); 
	    break;
	case begbin:
	case begdata:
	    dontchecklength++;
	    break;
	case endbin:
	case enddata:
	    dontchecklength--;
	    break;
	case endres: break;
	case invalid: break;
	default: break;
	}
	location = ftell(input);
    }
    location = ftell(input);
    if (endlast == -1) ptable[npages].location = location;
    if (!control.norearrange) {
	PrologAndSetup();
	ReArrange(printer);
    }
    BuildDownLoadTable();
    RangeResources();
    return success;
}

static int CheckOrientation(buffer, output, landscape)
    char *buffer;
    FILE *output;
{
    if (strncmp(buffer, "%%Orientation:", 14) == 0) {
	/* we already know that this orientation comment doesn't match what
	   we want, or we wouldn't be here, so put out correct comment.  We
	   also know that we're only changing to Landscape. */ 
	if (landscape) {
	    fprintf(output, "%%%%Orientation: Landscape\n");
	}
	return TRUE;
    }
    return FALSE;
}
	    

static void PutBuf(buffer, output)
    char *buffer;
    FILE *output;
{
    char *p;
    int i;

    p = strchr(buffer, '\n');
    if (((int) strlen(buffer) < PSBUFSIZ - 1) && !p) {
	for (i = 0; i < PSBUFSIZ - 1; i++) {
	    fputc(buffer[i], output);
	    if (buffer[i] == '\n')
		break;
	}
    }
    else
	fputs(buffer, output);
    memset(buffer, 0, PSBUFSIZ);
}
	    

void HandleOutput(input, output, printer, control)
    FILE *input, *output;
    char *printer;
    struct controlswitches control;
{
    int downndx, remndx, pagendx, featndx;
    char buf[PSBUFSIZ];
    long location;
    int i;
    int reverse;
    int fdin, fdout;
    FILE *out;
    int cnt;
    char bigbuf[TSBUFSIZE];
    int status;
    int filtering = FALSE;
    int dolandscape = FALSE;

    /* now, let's ship result to output */
    fdin = fileno(input);
    if (!(filtering = SetupCompression(&out,fileno(output), control)))
	out = output;
    reverse = control.reverse;
    if (endpro == -1 || npages == 0)
	reverse = FALSE;
    downndx = remndx = featndx = 0;
    if (reverse)
	pagendx = npages - 1;
    else pagendx = 0;
    if (!control.noparse) {
	fgets(buf, PSBUFSIZ, input);
	if (control.strip) {
	    sprintf(buf, "%%!\n");
	    PutBuf(buf, out);
	}
	else {
	    if (version == VERS1)
		sprintf(buf, "%%!PS-Adobe-2.1\n");
	    PutBuf(buf, out);
	    if (printer)
		fprintf(out, "%%%%%%PARSED: %s\n", printer);
	    OutputComments(out);
	}
	location = ftell(input);
	while (fgets(buf, PSBUFSIZ, input)) {
	    if (location == prologbegin && !control.strip && prologend > -1)
		fprintf(out, "%%%%BeginProlog\n");
	    /* check for procsets to download */
	    if (downndx < ndloads) {
		if (location == dloads[downndx].location) {
		    for (i = downndx; location == dloads[i].location; i++)
			if (dloads[i].index >= 0 &&
			    dloads[i].type == PROCSET) 
			    DownLoadResource(dloads[i].index, out, control.strip);
		}
	    }
	    if (location == prologend && !control.strip)
		fprintf(out, "%%%%EndProlog\n");
	    if (location == setupbegin) {
		if (!control.strip) {
		    fprintf(out, "%%%%BeginSetup\n");
		    CheckIncludes(out);
		}
		if (featndx < nfeatures && features[featndx].location == 0) {
		    /* features specified on command line that don't replace
		       something specified in document */
		    for (; features[featndx].location == 0 && featndx <
			 nfeatures; featndx++) 
			if (features[featndx].code)
			    IncludeFeature(out, featndx, control.strip);
		}
	    }
	    if (reverse) {
		if (location == endpro && pagendx > 0) {
		    fseek(input, ptable[pagendx].location, 0);
		    location = ftell(input);
		    downndx = finddloc(location);
		    remndx = findrloc(location);
		    featndx = findfloc(location);
		    continue;
		}
		if (location == ptable[pagendx].location && pagendx >= 0) {
		    if (!InRange(ptable[pagendx].pageno,control.ranges)) {
			/* outside range, skip to next page */
			if (pagendx == 0) {
			    fseek(input,endlast, 0);
			    pagendx--;
			}
			else
			    fseek(input,ptable[--pagendx].location,0);
			location = ftell(input);
			downndx = finddloc(location);
			remndx = findrloc(location);
			featndx = findfloc(location);
			continue;
		    }
		    if (control.landscape)
			dolandscape = TRUE;
		}
		if (location == ptable[pagendx + 1].location && pagendx >= 0) {
		    if (pagendx == 0) {
			fseek(input, endlast, 0);
			pagendx--;
		    }
		    else
			fseek(input, ptable[--pagendx].location, 0);
		    location = ftell(input);
		    downndx = finddloc(location);
		    remndx = findrloc(location);
		    featndx = findfloc(location);
		    continue;
		}
	    }
	    else {
		if (location == ptable[pagendx].location && pagendx != npages) {
		    if (!InRange(ptable[pagendx].pageno, control.ranges)) {
			/* outside the range, skip to next page */
			fseek(input,ptable[++pagendx].location,0);
			location = ftell(input);
			downndx = finddloc(location);
			remndx = findrloc(location);
			featndx = findfloc(location);
			continue;
		    }
		    else {
			/* inside the range, keep on going */
			pagendx++;
			if (control.landscape)
			    dolandscape = TRUE;
		    }
		}
	    }
	    if (downndx < ndloads) {
		if (location == dloads[downndx].location) {
		    for (; location == dloads[downndx].location; downndx++)
			if (dloads[downndx].index >= 0 &&
			    dloads[downndx].type != PROCSET)
			    DownLoadResource(dloads[downndx].index, out, control.strip);
		}
	    }
	    if (featndx < nfeatures) {
		if (location == features[featndx].location) {
		    for (; location == features[featndx].location; featndx++)
			if (features[featndx].code)
			    IncludeFeature(out, featndx, control.strip);
		}
	    }
	    if (location == setupend && !control.strip)
		fprintf(out, "%%%%EndSetup\n");
	    if (remndx < nrem) {
		if (location == remcomment[remndx]) {
		    if (!CheckOrientation(buf, out, control.landscape)) 
			remndx++;
		}
		else if (!(control.strip && *buf == '%'))
		    PutBuf(buf, out);
	    }
	    else if (!(control.strip && *buf == '%'))
		PutBuf(buf, out);
	    if (dolandscape) {
		DoLandscape(out, control);
		dolandscape = FALSE;
		if (control.squeeze)
		    ShrinkToFit(out, control);
	    }
	    location = ftell(input);
	}
	if (control.addshowpage) {
	    fprintf(out, "\nshowpage\n");
	}
	fflush(out);
    }
    else {
	fgets(buf,PSBUFSIZ,input);
	cnt = strlen(buf);
	PutBuf(buf,out);
	fprintf(out,"%%%%%%NOPARSE\n");
	fflush(out);
	fdout = fileno(out);
	lseek(fdin,cnt,SEEKING);
	while ((cnt = read(fdin,bigbuf,BUFSIZ)) > 0)
	    write(fdout,bigbuf,cnt);
    }
    if (filtering) {
	fclose(out);
	wait(&status);
    }
}

int SetupCompression(fptr, outfd, control)
    FILE **fptr;
    int outfd;
    struct controlswitches control;
{
    int fpid;
    int f2pid;
    char pathname[TSPATHSIZE];
    FILE *outptr;
    int binok = FALSE;
    char *value;

    if (alreadycompressed)
	return FALSE;
    if (value = getenv("BINARYOK")) {
	if (strcmp(value, "1") == 0) {
	    binok = TRUE;
	}
    }
    /* if it's a Level 2 printer, can compress */
    if ((Level2() && control.compress) || (control.compress && control.force)) {
	outptr = fdopen(outfd, "w");
	fprintf(outptr, "%%!\n");
	fprintf(outptr, "%%%%%%CMP\n");
	if (!binok) {
	    fprintf(outptr, "currentfile /ASCII85Decode filter ");
	}
	else {
	    fprintf(outptr, "currentilfe ");
	}
	fprintf(outptr, "/LZWDecode filter cvx exec\n");
	fflush(outptr);
	if (pipe(fd1pipe)) {
	    fprintf(stderr,"Error in pipe.\n");
	    perror("");
	}
	if (!binok) {
	    if (pipe(fd2pipe)) {
		fprintf(stderr,"Error in 2nd pipe.\n");
		perror("");
		exit(2);
	    }
	}
	if ((fpid = fork()) < 0) {
	    fprintf(stderr, "Error in compression fork.\n");
	    perror("");
	    exit(2);
	}
	if (fpid == 0) {
	    /* compression fork */
	    if (!binok) {
		/* need to encode the binary */
		if ((f2pid = fork()) < 0) {
		    fprintf(stderr, "Error in encoding fork.\n");
		    perror("");
		    exit(2);
		}
		if (f2pid == 0) {
		    /* encoding fork */
		    if (dup2(outfd, 1) < 0) {
			fprintf(stderr, "Error connecting output.\n");
			perror("");
			exit(2);
		    }
		    if ((dup2(fd2pipe[0], 0) < 0) || close(fd2pipe[0]) ||
			close(fd2pipe[1]) || close(fd1pipe[0]) ||
			close(fd1pipe[1])) { 
			fprintf(stderr, "Error in encoding child.\n");
			perror("");
			exit(2);
		    }
		    sprintf(pathname, "%s/asc85ec", PSLibDir);
		    execl(pathname, (char *) 0);
		    fprintf(stderr, "Error exec'ing %s\n", pathname);
		    perror("");
		    exit(2);
		}
		if ((dup2(fd2pipe[1], 1) < 0) || close(fd2pipe[1]) ||
		    close(fd2pipe[0])) {
		    fprintf(stderr,
			    "Error in compressing child (output pipe)\n");
		    perror("");
		    exit(2);
		}
	    }
	    else {
		/* don't need to encode */
		if (dup2(outfd, 1) < 0) {
		    fprintf(stderr,
			    "Error in compressing child (output)\n");
		    perror("");
		    exit(2);
		}
	    }
	    if ((dup2(fd1pipe[0], 0) < 0) || close(fd1pipe[0]) ||
		close(fd1pipe[1])) {
		fprintf(stderr, "Error in compressing child (input).\n");
		perror("");
		exit(2);
	    }
	    sprintf(pathname, "%s/lzwec", PSLibDir);
	    execl(pathname, (char *) 0);
	    fprintf(stderr, "Error exec'ing %s\n", pathname);
	    perror("");
	    exit(2);
	}
	*fptr = fdopen(fd1pipe[1], "w");
	if (!binok) {
	    close(fd2pipe[0]); close(fd2pipe[1]);
	}
	close(fd1pipe[0]);
	return TRUE;
    }
    return FALSE;
}
