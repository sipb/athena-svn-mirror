/** \ingroup rpmts
 * \file lib/transaction.c
 */

#include "system.h"
#include <rpmlib.h>

#include <rpmmacro.h>	/* XXX for rpmExpand */

#include "fsm.h"
#include "psm.h"

#include "rpmdb.h"

#include "rpmds.h"

#define	_RPMFI_INTERNAL
#include "rpmfi.h"

#define	_RPMTE_INTERNAL
#include "rpmte.h"

#define	_RPMTS_INTERNAL
#include "rpmts.h"

#include "cpio.h"
#include "fprint.h"
#include "legacy.h"	/* XXX domd5 */
#include "misc.h" /* XXX stripTrailingChar, splitString, currentDirectory */

#include "debug.h"

/*@access Header @*/		/* XXX ts->notify arg1 is void ptr */
/*@access rpmps @*/	/* XXX need rpmProblemSetOK() */
/*@access dbiIndexSet @*/

/*@access rpmpsm @*/

/*@access alKey @*/
/*@access fnpyKey @*/

/*@access rpmfi @*/

/*@access rpmte @*/
/*@access rpmtsi @*/
/*@access rpmts @*/

/**
 */
static int archOkay(/*@null@*/ const char * pkgArch)
	/*@*/
{
    if (pkgArch == NULL) return 0;
    return (rpmMachineScore(RPM_MACHTABLE_INSTARCH, pkgArch) ? 1 : 0);
}

/**
 */
static int osOkay(/*@null@*/ const char * pkgOs)
	/*@*/
{
    if (pkgOs == NULL) return 0;
    return (rpmMachineScore(RPM_MACHTABLE_INSTOS, pkgOs) ? 1 : 0);
}

/**
 */
static int sharedCmp(const void * one, const void * two)
	/*@*/
{
    sharedFileInfo a = (sharedFileInfo) one;
    sharedFileInfo b = (sharedFileInfo) two;

    if (a->otherPkg < b->otherPkg)
	return -1;
    else if (a->otherPkg > b->otherPkg)
	return 1;

    return 0;
}

/**
 */
/*@-boundsread@*/
static fileAction decideFileFate(const rpmts ts,
		const rpmfi ofi, rpmfi nfi)
	/*@globals fileSystem, internalState @*/
	/*@modifies nfi, fileSystem, internalState @*/
{
    const char * fn = rpmfiFN(nfi);
    int newFlags = rpmfiFFlags(nfi);
    char buffer[1024];
    fileTypes dbWhat, newWhat, diskWhat;
    struct stat sb;
    int save = (newFlags & RPMFILE_NOREPLACE) ? FA_ALTNAME : FA_SAVE;

    if (lstat(fn, &sb)) {
	/*
	 * The file doesn't exist on the disk. Create it unless the new
	 * package has marked it as missingok, or allfiles is requested.
	 */
	if (!(rpmtsFlags(ts) & RPMTRANS_FLAG_ALLFILES)
	 && (newFlags & RPMFILE_MISSINGOK))
	{
	    rpmMessage(RPMMESS_DEBUG, _("%s skipped due to missingok flag\n"),
			fn);
	    return FA_SKIP;
	} else {
	    return FA_CREATE;
	}
    }

    diskWhat = whatis((int_16)sb.st_mode);
    dbWhat = whatis(rpmfiFMode(ofi));
    newWhat = whatis(rpmfiFMode(nfi));

    /*
     * RPM >= 2.3.10 shouldn't create config directories -- we'll ignore
     * them in older packages as well.
     */
    if (newWhat == XDIR)
	return FA_CREATE;

    if (diskWhat != newWhat)
	return save;
    else if (newWhat != dbWhat && diskWhat != dbWhat)
	return save;
    else if (dbWhat != newWhat)
	return FA_CREATE;
    else if (dbWhat != LINK && dbWhat != REG)
	return FA_CREATE;

    /*
     * This order matters - we'd prefer to CREATE the file if at all
     * possible in case something else (like the timestamp) has changed.
     */
    if (dbWhat == REG) {
	const unsigned char * omd5, * nmd5;
	if (domd5(fn, buffer, 0, NULL))
	    return FA_CREATE;	/* assume file has been removed */
	omd5 = rpmfiMD5(ofi);
	if (omd5 && !memcmp(omd5, buffer, 16))
	    return FA_CREATE;	/* unmodified config file, replace. */
	nmd5 = rpmfiMD5(nfi);
/*@-nullpass@*/
	if (omd5 && nmd5 && !memcmp(omd5, nmd5, 16))
	    return FA_SKIP;	/* identical file, don't bother. */
/*@=nullpass@*/
    } else /* dbWhat == LINK */ {
	const char * oFLink, * nFLink;
	memset(buffer, 0, sizeof(buffer));
	if (readlink(fn, buffer, sizeof(buffer) - 1) == -1)
	    return FA_CREATE;	/* assume file has been removed */
	oFLink = rpmfiFLink(ofi);
	if (oFLink && !strcmp(oFLink, buffer))
	    return FA_CREATE;	/* unmodified config file, replace. */
	nFLink = rpmfiFLink(nfi);
/*@-nullpass@*/
	if (oFLink && nFLink && !strcmp(oFLink, nFLink))
	    return FA_SKIP;	/* identical file, don't bother. */
/*@=nullpass@*/
    }

    /*
     * The config file on the disk has been modified, but
     * the ones in the two packages are different. It would
     * be nice if RPM was smart enough to at least try and
     * merge the difference ala CVS, but...
     */
    return save;
}
/*@=boundsread@*/

/**
 */
/*@-boundsread@*/
static int filecmp(rpmfi afi, rpmfi bfi)
	/*@*/
{
    fileTypes awhat = whatis(rpmfiFMode(afi));
    fileTypes bwhat = whatis(rpmfiFMode(bfi));

    if (awhat != bwhat) return 1;

    if (awhat == LINK) {
	const char * alink = rpmfiFLink(afi);
	const char * blink = rpmfiFLink(bfi);
	if (alink == blink) return 0;
	if (alink == NULL) return 1;
	if (blink == NULL) return -1;
	return strcmp(alink, blink);
    } else if (awhat == REG) {
	const unsigned char * amd5 = rpmfiMD5(afi);
	const unsigned char * bmd5 = rpmfiMD5(bfi);
	if (amd5 == bmd5) return 0;
	if (amd5 == NULL) return 1;
	if (bmd5 == NULL) return -1;
	return memcmp(amd5, bmd5, 16);
    }

    return 0;
}
/*@=boundsread@*/

/**
 */
/* XXX only ts->{probs,rpmdb} modified */
/*@-bounds@*/
static int handleInstInstalledFiles(const rpmts ts,
		rpmte p, rpmfi fi,
		sharedFileInfo shared,
		int sharedCount, int reportConflicts)
	/*@globals rpmGlobalMacroContext, fileSystem, internalState @*/
	/*@modifies ts, fi, rpmGlobalMacroContext, fileSystem, internalState @*/
{
    uint_32 tscolor = rpmtsColor(ts);
    uint_32 otecolor, tecolor;
    uint_32 oficolor, ficolor;
    const char * altNEVR = NULL;
    rpmfi otherFi = NULL;
    int numReplaced = 0;
    rpmps ps;
    int i;

    {	rpmdbMatchIterator mi;
	Header h;
	int scareMem = 0;

	mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES,
			&shared->otherPkg, sizeof(shared->otherPkg));
	while ((h = rpmdbNextIterator(mi)) != NULL) {
	    altNEVR = hGetNEVR(h, NULL);
	    otherFi = rpmfiNew(ts, h, RPMTAG_BASENAMES, scareMem);
	    break;
	}
	mi = rpmdbFreeIterator(mi);
    }

    /* Compute package color. */
    tecolor = rpmteColor(p);
    tecolor &= tscolor;

    /* Compute other pkg color. */
    otecolor = 0;
    otherFi = rpmfiInit(otherFi, 0);
    if (otherFi != NULL)
    while (rpmfiNext(otherFi) >= 0)
	otecolor |= rpmfiFColor(otherFi);
    otecolor &= tscolor;

    if (otherFi == NULL)
	return 1;

    fi->replaced = xcalloc(sharedCount, sizeof(*fi->replaced));

    ps = rpmtsProblems(ts);
    for (i = 0; i < sharedCount; i++, shared++) {
	int otherFileNum, fileNum;
	int isCfgFile;

	otherFileNum = shared->otherFileNum;
	(void) rpmfiSetFX(otherFi, otherFileNum);
	oficolor = rpmfiFColor(otherFi);
	oficolor &= tscolor;

	fileNum = shared->pkgFileNum;
	(void) rpmfiSetFX(fi, fileNum);
	ficolor = rpmfiFColor(fi);
	ficolor &= tscolor;

	isCfgFile = ((rpmfiFFlags(otherFi) | rpmfiFFlags(fi)) & RPMFILE_CONFIG);

#ifdef	DYING
	/* XXX another tedious segfault, assume file state normal. */
	if (otherStates && otherStates[otherFileNum] != RPMFILE_STATE_NORMAL)
	    continue;
#endif

	if (XFA_SKIPPING(fi->actions[fileNum]))
	    continue;

	if (filecmp(otherFi, fi)) {
	    /* Report conflicts only for packages/files of same color. */
	    if (tscolor == 0 || (tecolor == otecolor && ficolor == oficolor))
	    if (reportConflicts) {
		rpmpsAppend(ps, RPMPROB_FILE_CONFLICT,
			rpmteNEVR(p), rpmteKey(p),
			rpmfiDN(fi), rpmfiBN(fi),
			altNEVR,
			0);
	    }
	    if (!isCfgFile) {
		/*@-assignexpose@*/ /* FIX: p->replaced, not fi */
		if (!shared->isRemoved)
		    fi->replaced[numReplaced++] = *shared;
		/*@=assignexpose@*/
	    }
	}

	if (isCfgFile) {
	    fileAction action;
	    action = decideFileFate(ts, otherFi, fi);
	    fi->actions[fileNum] = action;
	}
	fi->replacedSizes[fileNum] = rpmfiFSize(otherFi);
    }
    ps = rpmpsFree(ps);

    altNEVR = _free(altNEVR);
    otherFi = rpmfiFree(otherFi);

    fi->replaced = xrealloc(fi->replaced,	/* XXX memory leak */
			   sizeof(*fi->replaced) * (numReplaced + 1));
    fi->replaced[numReplaced].otherPkg = 0;

    return 0;
}
/*@=bounds@*/

/**
 */
/* XXX only ts->rpmdb modified */
static int handleRmvdInstalledFiles(const rpmts ts, rpmfi fi,
		sharedFileInfo shared, int sharedCount)
	/*@globals rpmGlobalMacroContext, fileSystem, internalState @*/
	/*@modifies ts, fi, rpmGlobalMacroContext, fileSystem, internalState @*/
{
    HGE_t hge = fi->hge;
    Header h;
    const char * otherStates;
    int i, xx;
   
    rpmdbMatchIterator mi;

    mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES,
			&shared->otherPkg, sizeof(shared->otherPkg));
    h = rpmdbNextIterator(mi);
    if (h == NULL) {
	mi = rpmdbFreeIterator(mi);
	return 1;
    }

    xx = hge(h, RPMTAG_FILESTATES, NULL, (void **) &otherStates, NULL);

/*@-boundswrite@*/
    for (i = 0; i < sharedCount; i++, shared++) {
	int otherFileNum, fileNum;
	otherFileNum = shared->otherFileNum;
	fileNum = shared->pkgFileNum;

	if (otherStates[otherFileNum] != RPMFILE_STATE_NORMAL)
	    continue;

	fi->actions[fileNum] = FA_SKIP;
    }
/*@=boundswrite@*/

    mi = rpmdbFreeIterator(mi);

    return 0;
}

#define	ISROOT(_d)	(((_d)[0] == '/' && (_d)[1] == '\0') ? "" : (_d))

/*@unchecked@*/
int _fps_debug = 0;

static int fpsCompare (const void * one, const void * two)
	/*@*/
{
    const struct fingerPrint_s * a = (const struct fingerPrint_s *)one;
    const struct fingerPrint_s * b = (const struct fingerPrint_s *)two;
    int adnlen = strlen(a->entry->dirName);
    int asnlen = (a->subDir ? strlen(a->subDir) : 0);
    int abnlen = strlen(a->baseName);
    int bdnlen = strlen(b->entry->dirName);
    int bsnlen = (b->subDir ? strlen(b->subDir) : 0);
    int bbnlen = strlen(b->baseName);
    char * afn, * bfn, * t;
    int rc = 0;

    if (adnlen == 1 && asnlen != 0) adnlen = 0;
    if (bdnlen == 1 && bsnlen != 0) bdnlen = 0;

/*@-boundswrite@*/
    afn = t = alloca(adnlen+asnlen+abnlen+2);
    if (adnlen) t = stpcpy(t, a->entry->dirName);
    *t++ = '/';
    if (a->subDir && asnlen) t = stpcpy(t, a->subDir);
    if (abnlen) t = stpcpy(t, a->baseName);
    if (afn[0] == '/' && afn[1] == '/') afn++;

    bfn = t = alloca(bdnlen+bsnlen+bbnlen+2);
    if (bdnlen) t = stpcpy(t, b->entry->dirName);
    *t++ = '/';
    if (b->subDir && bsnlen) t = stpcpy(t, b->subDir);
    if (bbnlen) t = stpcpy(t, b->baseName);
    if (bfn[0] == '/' && bfn[1] == '/') bfn++;
/*@=boundswrite@*/

    rc = strcmp(afn, bfn);
/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "\trc(%d) = strcmp(\"%s\", \"%s\")\n", rc, afn, bfn);
/*@=modfilesys@*/

/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "\t%s/%s%s\trc %d\n",
ISROOT(b->entry->dirName),
(b->subDir ? b->subDir : ""),
b->baseName,
rc
);
/*@=modfilesys@*/

    return rc;
}

/*@unchecked@*/
static int _linear_fps_search = 0;

static int findFps(const struct fingerPrint_s * fiFps,
		const struct fingerPrint_s * otherFps,
		int otherFc)
	/*@*/
{
    int otherFileNum;

/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "==> %s/%s%s\n",
ISROOT(fiFps->entry->dirName),
(fiFps->subDir ? fiFps->subDir : ""),
fiFps->baseName);
/*@=modfilesys@*/

  if (_linear_fps_search) {

linear:
    for (otherFileNum = 0; otherFileNum < otherFc; otherFileNum++, otherFps++) {

/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "\t%4d %s/%s%s\n", otherFileNum,
ISROOT(otherFps->entry->dirName),
(otherFps->subDir ? otherFps->subDir : ""),
otherFps->baseName);
/*@=modfilesys@*/

	/* If the addresses are the same, so are the values. */
	if (fiFps == otherFps)
	    break;

	/* Otherwise, compare fingerprints by value. */
	/*@-nullpass@*/	/* LCL: looks good to me */
	if (FP_EQUAL((*fiFps), (*otherFps)))
	    break;
	/*@=nullpass@*/
    }

if (otherFileNum == otherFc) {
/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "*** FP_EQUAL NULL %s/%s%s\n",
ISROOT(fiFps->entry->dirName),
(fiFps->subDir ? fiFps->subDir : ""),
fiFps->baseName);
/*@=modfilesys@*/
}

    return otherFileNum;

  } else {

    const struct fingerPrint_s * bingoFps;

/*@-boundswrite@*/
    bingoFps = bsearch(fiFps, otherFps, otherFc, sizeof(*otherFps), fpsCompare);
/*@=boundswrite@*/
    if (bingoFps == NULL) {
/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "*** bingoFps NULL %s/%s%s\n",
ISROOT(fiFps->entry->dirName),
(fiFps->subDir ? fiFps->subDir : ""),
fiFps->baseName);
/*@=modfilesys@*/
	goto linear;
    }

    /* If the addresses are the same, so are the values. */
    /*@-nullpass@*/	/* LCL: looks good to me */
    if (!(fiFps == bingoFps || FP_EQUAL((*fiFps), (*bingoFps)))) {
/*@-modfilesys@*/
if (_fps_debug)
fprintf(stderr, "***  BAD %s/%s%s\n",
ISROOT(bingoFps->entry->dirName),
(bingoFps->subDir ? bingoFps->subDir : ""),
bingoFps->baseName);
/*@=modfilesys@*/
	goto linear;
    }

    otherFileNum = (bingoFps != NULL ? (bingoFps - otherFps) : 0);

  }

    return otherFileNum;
}

/**
 * Update disk space needs on each partition for this package's files.
 */
/* XXX only ts->{probs,di} modified */
static void handleOverlappedFiles(const rpmts ts,
		const rpmte p, rpmfi fi)
	/*@globals fileSystem, internalState @*/
	/*@modifies ts, fi, fileSystem, internalState @*/
{
    uint_32 fixupSize = 0;
    rpmps ps;
    const char * fn;
    int i, j;
  
    ps = rpmtsProblems(ts);
    fi = rpmfiInit(fi, 0);
    if (fi != NULL)
    while ((i = rpmfiNext(fi)) >= 0) {
	struct fingerPrint_s * fiFps;
	int otherPkgNum, otherFileNum;
	rpmfi otherFi;
	int_32 FFlags;
	int_16 FMode;
	const rpmfi * recs;
	int numRecs;

	if (XFA_SKIPPING(fi->actions[i]))
	    continue;

	fn = rpmfiFN(fi);
	fiFps = fi->fps + i;
	FFlags = rpmfiFFlags(fi);
	FMode = rpmfiFMode(fi);

	fixupSize = 0;

	/*
	 * Retrieve all records that apply to this file. Note that the
	 * file info records were built in the same order as the packages
	 * will be installed and removed so the records for an overlapped
	 * files will be sorted in exactly the same order.
	 */
	(void) htGetEntry(ts->ht, fiFps,
			(const void ***) &recs, &numRecs, NULL);

	/*
	 * If this package is being added, look only at other packages
	 * being added -- removed packages dance to a different tune.
	 *
	 * If both this and the other package are being added, overlapped
	 * files must be identical (or marked as a conflict). The
	 * disposition of already installed config files leads to
	 * a small amount of extra complexity.
	 *
	 * If this package is being removed, then there are two cases that
	 * need to be worried about:
	 * If the other package is being added, then skip any overlapped files
	 * so that this package removal doesn't nuke the overlapped files
	 * that were just installed.
	 * If both this and the other package are being removed, then each
	 * file removal from preceding packages needs to be skipped so that
	 * the file removal occurs only on the last occurence of an overlapped
	 * file in the transaction set.
	 *
	 */

	/* Locate this overlapped file in the set of added/removed packages. */
	for (j = 0; j < numRecs && recs[j] != fi; j++)
	    {};

	/* Find what the previous disposition of this file was. */
	otherFileNum = -1;			/* keep gcc quiet */
	otherFi = NULL;
	for (otherPkgNum = j - 1; otherPkgNum >= 0; otherPkgNum--) {
	    struct fingerPrint_s * otherFps;
	    int otherFc;

	    otherFi = recs[otherPkgNum];

	    /* Added packages need only look at other added packages. */
	    if (rpmteType(p) == TR_ADDED && rpmteType(otherFi->te) != TR_ADDED)
		/*@innercontinue@*/ continue;

	    otherFps = otherFi->fps;
	    otherFc = rpmfiFC(otherFi);

	    otherFileNum = findFps(fiFps, otherFps, otherFc);
	    (void) rpmfiSetFX(otherFi, otherFileNum);

	    /* XXX Happens iff fingerprint for incomplete package install. */
	    if (otherFi->actions[otherFileNum] != FA_UNKNOWN)
		/*@innerbreak@*/ break;
	}

/*@-boundswrite@*/
	switch (rpmteType(p)) {
	case TR_ADDED:
	  { struct stat sb;
	    if (otherPkgNum < 0) {
		/* XXX is this test still necessary? */
		if (fi->actions[i] != FA_UNKNOWN)
		    /*@switchbreak@*/ break;
		if ((FFlags & RPMFILE_CONFIG) && !lstat(fn, &sb)) {
		    /* Here is a non-overlapped pre-existing config file. */
		    fi->actions[i] = (FFlags & RPMFILE_NOREPLACE)
			? FA_ALTNAME : FA_BACKUP;
		} else {
		    fi->actions[i] = FA_CREATE;
		}
		/*@switchbreak@*/ break;
	    }

assert(otherFi != NULL);
	    /* Mark added overlapped non-identical files as a conflict. */
	    if (!(rpmtsFilterFlags(ts) & RPMPROB_FILTER_REPLACENEWFILES)
	     && filecmp(otherFi, fi))
	    {
		rpmpsAppend(ps, RPMPROB_NEW_FILE_CONFLICT,
			rpmteNEVR(p), rpmteKey(p),
			fn, NULL,
			rpmteNEVR(otherFi->te),
			0);
	    }

	    /* Try to get the disk accounting correct even if a conflict. */
	    fixupSize = rpmfiFSize(otherFi);

	    if ((FFlags & RPMFILE_CONFIG) && !lstat(fn, &sb)) {
		/* Here is an overlapped  pre-existing config file. */
		fi->actions[i] = (FFlags & RPMFILE_NOREPLACE)
			? FA_ALTNAME : FA_SKIP;
	    } else {
		fi->actions[i] = FA_CREATE;
	    }
	  } /*@switchbreak@*/ break;

	case TR_REMOVED:
	    if (otherPkgNum >= 0) {
assert(otherFi != NULL);
		/* Here is an overlapped added file we don't want to nuke. */
		if (otherFi->actions[otherFileNum] != FA_ERASE) {
		    /* On updates, don't remove files. */
		    fi->actions[i] = FA_SKIP;
		    /*@switchbreak@*/ break;
		}
		/* Here is an overlapped removed file: skip in previous. */
		otherFi->actions[otherFileNum] = FA_SKIP;
	    }
	    if (XFA_SKIPPING(fi->actions[i]))
		/*@switchbreak@*/ break;
	    if (rpmfiFState(fi) != RPMFILE_STATE_NORMAL)
		/*@switchbreak@*/ break;
	    if (!(S_ISREG(FMode) && (FFlags & RPMFILE_CONFIG))) {
		fi->actions[i] = FA_ERASE;
		/*@switchbreak@*/ break;
	    }
		
	    /* Here is a pre-existing modified config file that needs saving. */
	    {	char md5sum[50];
		const unsigned char * MD5 = rpmfiMD5(fi);
		if (!domd5(fn, md5sum, 0, NULL) && memcmp(MD5, md5sum, 16)) {
		    fi->actions[i] = FA_BACKUP;
		    /*@switchbreak@*/ break;
		}
	    }
	    fi->actions[i] = FA_ERASE;
	    /*@switchbreak@*/ break;
	}
/*@=boundswrite@*/

	/* Update disk space info for a file. */
	rpmtsUpdateDSI(ts, fiFps->entry->dev, rpmfiFSize(fi),
		fi->replacedSizes[i], fixupSize, fi->actions[i]);

    }
    ps = rpmpsFree(ps);
}

/**
 * Ensure that current package is newer than installed package.
 * @param ts		transaction set
 * @param p		current transaction element
 * @param h		installed header
 * @return		0 if not newer, 1 if okay
 */
static int ensureOlder(rpmts ts,
		const rpmte p, const Header h)
	/*@modifies ts @*/
{
    int_32 reqFlags = (RPMSENSE_LESS | RPMSENSE_EQUAL);
    const char * reqEVR;
    rpmds req;
    char * t;
    int nb;
    int rc;

    if (p == NULL || h == NULL)
	return 1;

/*@-boundswrite@*/
    nb = strlen(rpmteNEVR(p)) + (rpmteE(p) != NULL ? strlen(rpmteE(p)) : 0) + 1;
    t = alloca(nb);
    *t = '\0';
    reqEVR = t;
    if (rpmteE(p) != NULL)	t = stpcpy( stpcpy(t, rpmteE(p)), ":");
    if (rpmteV(p) != NULL)	t = stpcpy(t, rpmteV(p));
    *t++ = '-';
    if (rpmteR(p) != NULL)	t = stpcpy(t, rpmteR(p));
/*@=boundswrite@*/
    
    req = rpmdsSingle(RPMTAG_REQUIRENAME, rpmteN(p), reqEVR, reqFlags);
    rc = rpmdsNVRMatchesDep(h, req, _rpmds_nopromote);
    req = rpmdsFree(req);

    if (rc == 0) {
	rpmps ps = rpmtsProblems(ts);
	const char * altNEVR = hGetNEVR(h, NULL);
	rpmpsAppend(ps, RPMPROB_OLDPACKAGE,
		rpmteNEVR(p), rpmteKey(p),
		NULL, NULL,
		altNEVR,
		0);
	altNEVR = _free(altNEVR);
	ps = rpmpsFree(ps);
	rc = 1;
    } else
	rc = 0;

    return rc;
}

/**
 * Skip any files that do not match install policies.
 * @param ts		transaction set
 * @param fi		file info set
 */
/*@-mustmod@*/ /* FIX: fi->actions is modified. */
/*@-bounds@*/
static void skipFiles(const rpmts ts, rpmfi fi)
	/*@globals rpmGlobalMacroContext @*/
	/*@modifies fi, rpmGlobalMacroContext @*/
{
    uint_32 tscolor = rpmtsColor(ts);
    uint_32 ficolor;
    int noConfigs = (rpmtsFlags(ts) & RPMTRANS_FLAG_NOCONFIGS);
    int noDocs = (rpmtsFlags(ts) & RPMTRANS_FLAG_NODOCS);
    char ** netsharedPaths = NULL;
    const char ** languages;
    const char * dn, * bn;
    int dnlen, bnlen, ix;
    const char * s;
    int * drc;
    char * dff;
    int dc;
    int i, j;

    if (!noDocs)
	noDocs = rpmExpandNumeric("%{_excludedocs}");

    {	const char *tmpPath = rpmExpand("%{_netsharedpath}", NULL);
	/*@-branchstate@*/
	if (tmpPath && *tmpPath != '%')
	    netsharedPaths = splitString(tmpPath, strlen(tmpPath), ':');
	/*@=branchstate@*/
	tmpPath = _free(tmpPath);
    }

    s = rpmExpand("%{_install_langs}", NULL);
    /*@-branchstate@*/
    if (!(s && *s != '%'))
	s = _free(s);
    if (s) {
	languages = (const char **) splitString(s, strlen(s), ':');
	s = _free(s);
    } else
	languages = NULL;
    /*@=branchstate@*/

    /* Compute directory refcount, skip directory if now empty. */
    dc = rpmfiDC(fi);
    drc = alloca(dc * sizeof(*drc));
    memset(drc, 0, dc * sizeof(*drc));
    dff = alloca(dc * sizeof(*dff));
    memset(dff, 0, dc * sizeof(*dff));

    fi = rpmfiInit(fi, 0);
    if (fi != NULL)	/* XXX lclint */
    while ((i = rpmfiNext(fi)) >= 0)
    {
	char ** nsp;

	bn = rpmfiBN(fi);
	bnlen = strlen(bn);
	ix = rpmfiDX(fi);
	dn = rpmfiDN(fi);
	dnlen = strlen(dn);
	if (dn == NULL)
	    continue;	/* XXX can't happen */

	drc[ix]++;

	/* Don't bother with skipped files */
	if (XFA_SKIPPING(fi->actions[i])) {
	    drc[ix]--; dff[ix] = 1;
	    continue;
	}

	/* Ignore colored files not in our rainbow. */
	ficolor = rpmfiFColor(fi);
	if (tscolor && ficolor && !(tscolor & ficolor)) {
	    drc[ix]--;	dff[ix] = 1;
	    fi->actions[i] = FA_SKIPCOLOR;
	    continue;
	}

	/*
	 * Skip net shared paths.
	 * Net shared paths are not relative to the current root (though
	 * they do need to take package relocations into account).
	 */
	for (nsp = netsharedPaths; nsp && *nsp; nsp++) {
	    int len;

	    len = strlen(*nsp);
	    if (dnlen >= len) {
		if (strncmp(dn, *nsp, len))
		    /*@innercontinue@*/ continue;
		/* Only directories or complete file paths can be net shared */
		if (!(dn[len] == '/' || dn[len] == '\0'))
		    /*@innercontinue@*/ continue;
	    } else {
		if (len < (dnlen + bnlen))
		    /*@innercontinue@*/ continue;
		if (strncmp(dn, *nsp, dnlen))
		    /*@innercontinue@*/ continue;
		if (strncmp(bn, (*nsp) + dnlen, bnlen))
		    /*@innercontinue@*/ continue;
		len = dnlen + bnlen;
		/* Only directories or complete file paths can be net shared */
		if (!((*nsp)[len] == '/' || (*nsp)[len] == '\0'))
		    /*@innercontinue@*/ continue;
	    }

	    /*@innerbreak@*/ break;
	}

	if (nsp && *nsp) {
	    drc[ix]--;	dff[ix] = 1;
	    fi->actions[i] = FA_SKIPNETSHARED;
	    continue;
	}

	/*
	 * Skip i18n language specific files.
	 */
	if (fi->flangs && languages && *fi->flangs[i]) {
	    const char **lang, *l, *le;
	    for (lang = languages; *lang != NULL; lang++) {
		if (!strcmp(*lang, "all"))
		    /*@innerbreak@*/ break;
		for (l = fi->flangs[i]; *l != '\0'; l = le) {
		    for (le = l; *le != '\0' && *le != '|'; le++)
			{};
		    if ((le-l) > 0 && !strncmp(*lang, l, (le-l)))
			/*@innerbreak@*/ break;
		    if (*le == '|') le++;	/* skip over | */
		}
		if (*l != '\0')
		    /*@innerbreak@*/ break;
	    }
	    if (*lang == NULL) {
		drc[ix]--;	dff[ix] = 1;
		fi->actions[i] = FA_SKIPNSTATE;
		continue;
	    }
	}

	/*
	 * Skip config files if requested.
	 */
	if (noConfigs && (rpmfiFFlags(fi) & RPMFILE_CONFIG)) {
	    drc[ix]--;	dff[ix] = 1;
	    fi->actions[i] = FA_SKIPNSTATE;
	    continue;
	}

	/*
	 * Skip documentation if requested.
	 */
	if (noDocs && (rpmfiFFlags(fi) & RPMFILE_DOC)) {
	    drc[ix]--;	dff[ix] = 1;
	    fi->actions[i] = FA_SKIPNSTATE;
	    continue;
	}
    }

    /* Skip (now empty) directories that had skipped files. */
#ifndef	NOTYET
    if (fi != NULL)	/* XXX can't happen */
    for (j = 0; j < dc; j++)
#else
    if ((fi = rpmfiInitD(fi)) != NULL)
    while (j = rpmfiNextD(fi) >= 0)
#endif
    {

	if (drc[j]) continue;	/* dir still has files. */
	if (!dff[j]) continue;	/* dir was not emptied here. */
	
	/* Find parent directory and basename. */
	dn = fi->dnl[j];	dnlen = strlen(dn) - 1;
	bn = dn + dnlen;	bnlen = 0;
	while (bn > dn && bn[-1] != '/') {
		bnlen++;
		dnlen--;
		bn--;
	}

	/* If explicitly included in the package, skip the directory. */
	fi = rpmfiInit(fi, 0);
	if (fi != NULL)		/* XXX lclint */
	while ((i = rpmfiNext(fi)) >= 0) {
	    const char * fdn, * fbn;
	    int_16 fFMode;

	    if (XFA_SKIPPING(fi->actions[i]))
		/*@innercontinue@*/ continue;

	    fFMode = rpmfiFMode(fi);

	    if (whatis(fFMode) != XDIR)
		/*@innercontinue@*/ continue;
	    fdn = rpmfiDN(fi);
	    if (strlen(fdn) != dnlen)
		/*@innercontinue@*/ continue;
	    if (strncmp(fdn, dn, dnlen))
		/*@innercontinue@*/ continue;
	    fbn = rpmfiBN(fi);
	    if (strlen(fbn) != bnlen)
		/*@innercontinue@*/ continue;
	    if (strncmp(fbn, bn, bnlen))
		/*@innercontinue@*/ continue;
	    rpmMessage(RPMMESS_DEBUG, _("excluding directory %s\n"), dn);
	    fi->actions[i] = FA_SKIPNSTATE;
	    /*@innerbreak@*/ break;
	}
    }

    if (netsharedPaths) freeSplitString(netsharedPaths);
#ifdef	DYING	/* XXX freeFi will deal with this later. */
    fi->flangs = _free(fi->flangs);
#endif
    if (languages) freeSplitString((char **)languages);
}
/*@=bounds@*/
/*@=mustmod@*/

/**
 * Return transaction element's file info.
 * @todo Take a rpmfi refcount here.
 * @param tsi		transaction element iterator
 * @return		transaction element file info
 */
static /*@null@*/
rpmfi rpmtsiFi(const rpmtsi tsi)
	/*@*/
{
    rpmfi fi = NULL;

    if (tsi != NULL && tsi->ocsave != -1) {
	/*@-type -abstract@*/ /* FIX: rpmte not opaque */
	rpmte te = rpmtsElement(tsi->ts, tsi->ocsave);
	/*@-assignexpose@*/
	if (te != NULL && (fi = te->fi) != NULL)
	    fi->te = te;
	/*@=assignexpose@*/
	/*@=type =abstract@*/
    }
    /*@-compdef -refcounttrans -usereleased @*/
    return fi;
    /*@=compdef =refcounttrans =usereleased @*/
}

#define	NOTIFY(_ts, _al) /*@i@*/ if ((_ts)->notify) (void) (_ts)->notify _al

int rpmtsRun(rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet)
{
    uint_32 tscolor = rpmtsColor(ts);
    int i, j;
    int ourrc = 0;
    int totalFileCount = 0;
    rpmfi fi;
    sharedFileInfo shared, sharedList;
    int numShared;
    int nexti;
    alKey lastFailKey;
    fingerPrintCache fpc;
    rpmps ps;
    rpmpsm psm;
    rpmtsi pi;	rpmte p;
    rpmtsi qi;	rpmte q;
    int numAdded;
    int numRemoved;
    int xx;

    /* XXX programmer error segfault avoidance. */
    if (rpmtsNElements(ts) <= 0)
	return -1;

    if (rpmtsFlags(ts) & RPMTRANS_FLAG_NOSCRIPTS)
	(void) rpmtsSetFlags(ts, (rpmtsFlags(ts) | _noTransScripts | _noTransTriggers));
    if (rpmtsFlags(ts) & RPMTRANS_FLAG_NOTRIGGERS)
	(void) rpmtsSetFlags(ts, (rpmtsFlags(ts) | _noTransTriggers));

    if (rpmtsFlags(ts) & RPMTRANS_FLAG_JUSTDB)
	(void) rpmtsSetFlags(ts, (rpmtsFlags(ts) | _noTransScripts | _noTransTriggers));

    ts->probs = rpmpsFree(ts->probs);
    ts->probs = rpmpsCreate();

    /* XXX Make sure the database is open RDWR for package install/erase. */
    {	int dbmode = (rpmtsFlags(ts) & RPMTRANS_FLAG_TEST)
		? O_RDONLY : (O_RDWR|O_CREAT);

	/* Open database RDWR for installing packages. */
	if (rpmtsOpenDB(ts, dbmode))
	    return -1;	/* XXX W2DO? */
    }

    ts->ignoreSet = ignoreSet;
    {	const char * currDir = currentDirectory();
	rpmtsSetCurrDir(ts, currDir);
	currDir = _free(currDir);
    }

    (void) rpmtsSetChrootDone(ts, 0);

    {	int_32 tid = (int_32) time(NULL);
	(void) rpmtsSetTid(ts, tid);
    }

    /* Get available space on mounted file systems. */
    xx = rpmtsInitDSI(ts);

    /* ===============================================
     * For packages being installed:
     * - verify package arch/os.
     * - verify package epoch:version-release is newer.
     * - count files.
     * For packages being removed:
     * - count files.
     */

rpmMessage(RPMMESS_DEBUG, _("sanity checking %d elements\n"), rpmtsNElements(ts));
    ps = rpmtsProblems(ts);
    /* The ordering doesn't matter here */
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, TR_ADDED)) != NULL) {
	rpmdbMatchIterator mi;
	int fc;

	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	fc = rpmfiFC(fi);

	if (!(rpmtsFilterFlags(ts) & RPMPROB_FILTER_IGNOREARCH) && !tscolor)
	    if (!archOkay(rpmteA(p)))
		rpmpsAppend(ps, RPMPROB_BADARCH,
			rpmteNEVR(p), rpmteKey(p),
			rpmteA(p), NULL,
			NULL, 0);

	if (!(rpmtsFilterFlags(ts) & RPMPROB_FILTER_IGNOREOS))
	    if (!osOkay(rpmteO(p)))
		rpmpsAppend(ps, RPMPROB_BADOS,
			rpmteNEVR(p), rpmteKey(p),
			rpmteO(p), NULL,
			NULL, 0);

	if (!(rpmtsFilterFlags(ts) & RPMPROB_FILTER_OLDPACKAGE)) {
	    Header h;
	    mi = rpmtsInitIterator(ts, RPMTAG_NAME, rpmteN(p), 0);
	    while ((h = rpmdbNextIterator(mi)) != NULL)
		xx = ensureOlder(ts, p, h);
	    mi = rpmdbFreeIterator(mi);
	}

	if (!(rpmtsFilterFlags(ts) & RPMPROB_FILTER_REPLACEPKG)) {
	    mi = rpmtsInitIterator(ts, RPMTAG_NAME, rpmteN(p), 0);
	    xx = rpmdbSetIteratorRE(mi, RPMTAG_EPOCH, RPMMIRE_DEFAULT,
				rpmteE(p));
	    xx = rpmdbSetIteratorRE(mi, RPMTAG_VERSION, RPMMIRE_DEFAULT,
				rpmteV(p));
	    xx = rpmdbSetIteratorRE(mi, RPMTAG_RELEASE, RPMMIRE_DEFAULT,
				rpmteR(p));
	    if (tscolor) {
		xx = rpmdbSetIteratorRE(mi, RPMTAG_ARCH, RPMMIRE_DEFAULT,
				rpmteA(p));
		xx = rpmdbSetIteratorRE(mi, RPMTAG_OS, RPMMIRE_DEFAULT,
				rpmteO(p));
	    }

	    while (rpmdbNextIterator(mi) != NULL) {
		rpmpsAppend(ps, RPMPROB_PKG_INSTALLED,
			rpmteNEVR(p), rpmteKey(p),
			NULL, NULL,
			NULL, 0);
		/*@innerbreak@*/ break;
	    }
	    mi = rpmdbFreeIterator(mi);
	}

	/* Count no. of files (if any). */
	totalFileCount += fc;

    }
    pi = rpmtsiFree(pi);
    ps = rpmpsFree(ps);

    /* The ordering doesn't matter here */
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, TR_REMOVED)) != NULL) {
	int fc;

	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	fc = rpmfiFC(fi);

	totalFileCount += fc;
    }
    pi = rpmtsiFree(pi);

    /* ===============================================
     * Initialize transaction element file info for package:
     */

    /*
     * FIXME?: we'd be better off assembling one very large file list and
     * calling fpLookupList only once. I'm not sure that the speedup is
     * worth the trouble though.
     */
rpmMessage(RPMMESS_DEBUG, _("computing %d file fingerprints\n"), totalFileCount);
    numAdded = numRemoved = 0;
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, 0)) != NULL) {
	int fc;

	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	fc = rpmfiFC(fi);

	/*@-branchstate@*/
	switch (rpmteType(p)) {
	case TR_ADDED:
	    numAdded++;
	    fi->record = 0;
	    /* Skip netshared paths, not our i18n files, and excluded docs */
	    if (fc > 0)
		skipFiles(ts, fi);
	    /*@switchbreak@*/ break;
	case TR_REMOVED:
	    numRemoved++;
	    fi->record = rpmteDBOffset(p);
	    /*@switchbreak@*/ break;
	}
	/*@=branchstate@*/

	fi->fps = (fc > 0 ? xmalloc(fc * sizeof(*fi->fps)) : NULL);
    }
    pi = rpmtsiFree(pi);

    if (!rpmtsChrootDone(ts)) {
	const char * rootDir = rpmtsRootDir(ts);
	xx = chdir("/");
	/*@-superuser -noeffect @*/
	if (rootDir != NULL)
	    xx = chroot(rootDir);
	/*@=superuser =noeffect @*/
	(void) rpmtsSetChrootDone(ts, 1);
    }

    ts->ht = htCreate(totalFileCount * 2, 0, 0, fpHashFunction, fpEqual);
    fpc = fpCacheCreate(totalFileCount);

    /* ===============================================
     * Add fingerprint for each file not skipped.
     */
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, 0)) != NULL) {
	int fc;

	(void) rpmdbCheckSignals();

	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	fc = rpmfiFC(fi);

	fpLookupList(fpc, fi->dnl, fi->bnl, fi->dil, fc, fi->fps);
	/*@-branchstate@*/
 	fi = rpmfiInit(fi, 0);
 	if (fi != NULL)		/* XXX lclint */
	while ((i = rpmfiNext(fi)) >= 0) {
	    if (XFA_SKIPPING(fi->actions[i]))
		/*@innercontinue@*/ continue;
	    /*@-dependenttrans@*/
	    htAddEntry(ts->ht, fi->fps + i, (void *) fi);
	    /*@=dependenttrans@*/
	}
	/*@=branchstate@*/
    }
    pi = rpmtsiFree(pi);

    NOTIFY(ts, (NULL, RPMCALLBACK_TRANS_START, 6, ts->orderCount,
	NULL, ts->notifyData));

    /* ===============================================
     * Compute file disposition for each package in transaction set.
     */
rpmMessage(RPMMESS_DEBUG, _("computing file dispositions\n"));
    ps = rpmtsProblems(ts);
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, 0)) != NULL) {
	dbiIndexSet * matches;
	int knownBad;
	int fc;

	(void) rpmdbCheckSignals();

	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	fc = rpmfiFC(fi);

	NOTIFY(ts, (NULL, RPMCALLBACK_TRANS_PROGRESS, rpmtsiOc(pi),
			ts->orderCount, NULL, ts->notifyData));

	if (fc == 0) continue;

	/* Extract file info for all files in this package from the database. */
	matches = xcalloc(fc, sizeof(*matches));
	if (rpmdbFindFpList(rpmtsGetRdb(ts), fi->fps, matches, fc)) {
	    ps = rpmpsFree(ps);
	    return 1;	/* XXX WTFO? */
	}

	numShared = 0;
 	fi = rpmfiInit(fi, 0);
	while ((i = rpmfiNext(fi)) >= 0)
	    numShared += dbiIndexSetCount(matches[i]);

	/* Build sorted file info list for this package. */
	shared = sharedList = xcalloc((numShared + 1), sizeof(*sharedList));

 	fi = rpmfiInit(fi, 0);
	while ((i = rpmfiNext(fi)) >= 0) {
	    /*
	     * Take care not to mark files as replaced in packages that will
	     * have been removed before we will get here.
	     */
	    for (j = 0; j < dbiIndexSetCount(matches[i]); j++) {
		int ro;
		ro = dbiIndexRecordOffset(matches[i], j);
		knownBad = 0;
		qi = rpmtsiInit(ts);
		while ((q = rpmtsiNext(qi, TR_REMOVED)) != NULL) {
		    if (ro == knownBad)
			/*@innerbreak@*/ break;
		    if (rpmteDBOffset(q) == ro)
			knownBad = ro;
		}
		qi = rpmtsiFree(qi);

		shared->pkgFileNum = i;
		shared->otherPkg = dbiIndexRecordOffset(matches[i], j);
		shared->otherFileNum = dbiIndexRecordFileNumber(matches[i], j);
		shared->isRemoved = (knownBad == ro);
		shared++;
	    }
	    matches[i] = dbiFreeIndexSet(matches[i]);
	}
	numShared = shared - sharedList;
	shared->otherPkg = -1;
	matches = _free(matches);

	/* Sort file info by other package index (otherPkg) */
	qsort(sharedList, numShared, sizeof(*shared), sharedCmp);

	/* For all files from this package that are in the database ... */
	/*@-branchstate@*/
	for (i = 0; i < numShared; i = nexti) {
	    int beingRemoved;

	    shared = sharedList + i;

	    /* Find the end of the files in the other package. */
	    for (nexti = i + 1; nexti < numShared; nexti++) {
		if (sharedList[nexti].otherPkg != shared->otherPkg)
		    /*@innerbreak@*/ break;
	    }

	    /* Is this file from a package being removed? */
	    beingRemoved = 0;
	    if (ts->removedPackages != NULL)
	    for (j = 0; j < ts->numRemovedPackages; j++) {
		if (ts->removedPackages[j] != shared->otherPkg)
		    /*@innercontinue@*/ continue;
		beingRemoved = 1;
		/*@innerbreak@*/ break;
	    }

	    /* Determine the fate of each file. */
	    switch (rpmteType(p)) {
	    case TR_ADDED:
		xx = handleInstInstalledFiles(ts, p, fi, shared, nexti - i,
	!(beingRemoved || (rpmtsFilterFlags(ts) & RPMPROB_FILTER_REPLACEOLDFILES)));
		/*@switchbreak@*/ break;
	    case TR_REMOVED:
		if (!beingRemoved)
		    xx = handleRmvdInstalledFiles(ts, fi, shared, nexti - i);
		/*@switchbreak@*/ break;
	    }
	}
	/*@=branchstate@*/

	free(sharedList);

	/* Update disk space needs on each partition for this package. */
	handleOverlappedFiles(ts, p, fi);

	/* Check added package has sufficient space on each partition used. */
	switch (rpmteType(p)) {
	case TR_ADDED:
	    rpmtsCheckDSIProblems(ts, p);
	    /*@switchbreak@*/ break;
	case TR_REMOVED:
	    /*@switchbreak@*/ break;
	}
    }
    pi = rpmtsiFree(pi);
    ps = rpmpsFree(ps);

    if (rpmtsChrootDone(ts)) {
	const char * currDir = rpmtsCurrDir(ts);
	/*@-superuser -noeffect @*/
	xx = chroot(".");
	/*@=superuser =noeffect @*/
	(void) rpmtsSetChrootDone(ts, 0);
	if (currDir != NULL)
	    xx = chdir(currDir);
    }

    NOTIFY(ts, (NULL, RPMCALLBACK_TRANS_STOP, 6, ts->orderCount,
	NULL, ts->notifyData));

    /* ===============================================
     * Free unused memory as soon as possible.
     */
    pi = rpmtsiInit(ts);
    while ((p = rpmtsiNext(pi, 0)) != NULL) {
	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	if (rpmfiFC(fi) == 0)
	    continue;
	fi->fps = _free(fi->fps);
    }
    pi = rpmtsiFree(pi);

    fpc = fpCacheFree(fpc);
    ts->ht = htFree(ts->ht);

    /* ===============================================
     * If unfiltered problems exist, free memory and return.
     */
    if ((rpmtsFlags(ts) & RPMTRANS_FLAG_BUILD_PROBS)
     || (ts->probs->numProblems &&
		(okProbs != NULL || rpmpsTrim(ts->probs, okProbs)))
       )
    {
	return ts->orderCount;
    }

    /* ===============================================
     * Save removed files before erasing.
     */
    if (rpmtsFlags(ts) & (RPMTRANS_FLAG_DIRSTASH | RPMTRANS_FLAG_REPACKAGE)) {
	int progress;
	progress = 0;
	pi = rpmtsiInit(ts);
	while ((p = rpmtsiNext(pi, 0)) != NULL) {

	    (void) rpmdbCheckSignals();

	    if ((fi = rpmtsiFi(pi)) == NULL)
		continue;	/* XXX can't happen */
	    switch (rpmteType(p)) {
	    case TR_ADDED:
		/*@switchbreak@*/ break;
	    case TR_REMOVED:
		if (!(rpmtsFlags(ts) & RPMTRANS_FLAG_REPACKAGE))
		    /*@switchbreak@*/ break;
		if (!progress)
		    NOTIFY(ts, (NULL, RPMCALLBACK_REPACKAGE_START,
				7, numRemoved, NULL, ts->notifyData));

		NOTIFY(ts, (NULL, RPMCALLBACK_REPACKAGE_PROGRESS, progress,
			numRemoved, NULL, ts->notifyData));
		progress++;

	/* XXX TR_REMOVED needs CPIO_MAP_{ABSOLUTE,ADDDOT} CPIO_ALL_HARDLINKS */
		fi->mapflags |= CPIO_MAP_ABSOLUTE;
		fi->mapflags |= CPIO_MAP_ADDDOT;
		fi->mapflags |= CPIO_ALL_HARDLINKS;
		psm = rpmpsmNew(ts, p, fi);
		xx = rpmpsmStage(psm, PSM_PKGSAVE);
		psm = rpmpsmFree(psm);
		fi->mapflags &= ~CPIO_MAP_ABSOLUTE;
		fi->mapflags &= ~CPIO_MAP_ADDDOT;
		fi->mapflags &= ~CPIO_ALL_HARDLINKS;

		/*@switchbreak@*/ break;
	    }
	}
	pi = rpmtsiFree(pi);
	if (progress) {
	    NOTIFY(ts, (NULL, RPMCALLBACK_REPACKAGE_STOP, 7, numRemoved,
			NULL, ts->notifyData));
	}
    }

    /* ===============================================
     * Install and remove packages.
     */
    lastFailKey = (alKey)-2;	/* erased packages have -1 */
    pi = rpmtsiInit(ts);
    /*@-branchstate@*/ /* FIX: fi reload needs work */
    while ((p = rpmtsiNext(pi, 0)) != NULL) {
	alKey pkgKey;
	int gotfd;

	(void) rpmdbCheckSignals();

	gotfd = 0;
	if ((fi = rpmtsiFi(pi)) == NULL)
	    continue;	/* XXX can't happen */
	
	psm = rpmpsmNew(ts, p, fi);
	psm->unorderedSuccessor =
		(rpmtsiOc(pi) >= rpmtsUnorderedSuccessors(ts, -1) ? 1 : 0);

	switch (rpmteType(p)) {
	case TR_ADDED:

	    pkgKey = rpmteAddedKey(p);

	    rpmMessage(RPMMESS_DEBUG, "========== +++ %s\n", rpmteNEVR(p));
	    p->h = NULL;
	    /*@-type@*/ /* FIX: rpmte not opaque */
	    {
		/*@-noeffectuncon@*/ /* FIX: notify annotations */
		p->fd = ts->notify(p->h, RPMCALLBACK_INST_OPEN_FILE, 0, 0,
				rpmteKey(p), ts->notifyData);
		/*@=noeffectuncon@*/
		if (rpmteFd(p) != NULL) {
		    rpmVSFlags ovsflags = rpmtsVSFlags(ts);
		    rpmVSFlags vsflags = ovsflags | RPMVSF_NEEDPAYLOAD;
		    rpmRC rpmrc;

		    ovsflags = rpmtsSetVSFlags(ts, vsflags);
		    rpmrc = rpmReadPackageFile(ts, rpmteFd(p),
				rpmteNEVR(p), &p->h);
		    vsflags = rpmtsSetVSFlags(ts, ovsflags);

		    switch (rpmrc) {
		    default:
			/*@-noeffectuncon@*/ /* FIX: notify annotations */
			p->fd = ts->notify(p->h, RPMCALLBACK_INST_CLOSE_FILE,
					0, 0,
					rpmteKey(p), ts->notifyData);
			/*@=noeffectuncon@*/
			p->fd = NULL;
			ourrc++;
			/*@innerbreak@*/ break;
		    case RPMRC_NOTTRUSTED:
		    case RPMRC_NOKEY:
		    case RPMRC_OK:
			/*@innerbreak@*/ break;
		    }
		    if (rpmteFd(p) != NULL) gotfd = 1;
		}
	    }
	    /*@=type@*/

	    if (rpmteFd(p) != NULL) {
		/*
		 * XXX Sludge necessary to tranfer existing fstates/actions
		 * XXX around a recreated file info set.
		 */
		psm->fi = rpmfiFree(psm->fi);
		{
		    char * fstates = fi->fstates;
		    fileAction * actions = fi->actions;
		    rpmte savep;

		    fi->fstates = NULL;
		    fi->actions = NULL;
		    fi = rpmfiFree(fi);

		    savep = rpmtsSetRelocateElement(ts, p);
		    fi = rpmfiNew(ts, p->h, RPMTAG_BASENAMES, 1);
		    (void) rpmtsSetRelocateElement(ts, savep);

		    if (fi != NULL) {	/* XXX can't happen */
			fi->te = p;
			fi->fstates = _free(fi->fstates);
			fi->fstates = fstates;
			fi->actions = _free(fi->actions);
			fi->actions = actions;
			p->fi = fi;
		    }
		}
		psm->fi = rpmfiLink(p->fi, NULL);

/*@-nullstate@*/ /* FIX: psm->fi may be NULL */
		if (rpmpsmStage(psm, PSM_PKGINSTALL)) {
		    ourrc++;
		    lastFailKey = pkgKey;
		}
/*@=nullstate@*/
	    } else {
		ourrc++;
		lastFailKey = pkgKey;
	    }

	    if (gotfd) {
		/*@-noeffectuncon @*/ /* FIX: check rc */
		(void) ts->notify(p->h, RPMCALLBACK_INST_CLOSE_FILE, 0, 0,
			rpmteKey(p), ts->notifyData);
		/*@=noeffectuncon @*/
		/*@-type@*/
		p->fd = NULL;
		/*@=type@*/
	    }

	    p->h = headerFree(p->h);

	    /*@switchbreak@*/ break;
	case TR_REMOVED:
	    rpmMessage(RPMMESS_DEBUG, "========== --- %s\n", rpmteNEVR(p));
	    /*
	     * XXX This has always been a hack, now mostly broken.
	     * If install failed, then we shouldn't erase.
	     */
	    if (rpmteDependsOnKey(p) != lastFailKey) {
		if (rpmpsmStage(psm, PSM_PKGERASE))
		    ourrc++;
	    }
	    /*@switchbreak@*/ break;
	}
	xx = rpmdbSync(rpmtsGetRdb(ts));

/*@-nullstate@*/ /* FIX: psm->fi may be NULL */
	psm = rpmpsmFree(psm);
/*@=nullstate@*/

/*@-type@*/ /* FIX: p is almost opaque */
	p->fi = rpmfiFree(p->fi);
/*@=type@*/

    }
    /*@=branchstate@*/
    pi = rpmtsiFree(pi);

    /*@-nullstate@*/ /* FIX: ts->flList may be NULL */
    if (ourrc)
    	return -1;
    else
	return 0;
    /*@=nullstate@*/
}