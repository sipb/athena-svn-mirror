/** \ingroup rpmdep
 * \file lib/rpmlibprov.c
 */

#include "system.h"

#include <rpmlib.h>

#include "rpmds.h"

#include "debug.h"

/**
 */
struct rpmlibProvides_s {
/*@observer@*/ /*@null@*/
    const char * featureName;
/*@observer@*/ /*@null@*/
    const char * featureEVR;
    int featureFlags;
/*@observer@*/ /*@null@*/
    const char * featureDescription;
};

/*@observer@*/ /*@unchecked@*/
static struct rpmlibProvides_s rpmlibProvides[] = {
    { "rpmlib(VersionedDependencies)",	"3.0.3-1",
	(RPMSENSE_RPMLIB|RPMSENSE_EQUAL),
    N_("PreReq:, Provides:, and Obsoletes: dependencies support versions.") },
    { "rpmlib(CompressedFileNames)",	"3.0.4-1",
	(RPMSENSE_RPMLIB|RPMSENSE_EQUAL),
    N_("file name(s) stored as (dirName,baseName,dirIndex) tuple, not as path.")},
    { "rpmlib(PayloadIsBzip2)",		"3.0.5-1",
	(RPMSENSE_RPMLIB|RPMSENSE_EQUAL),
    N_("package payload is compressed using bzip2.") },
    { "rpmlib(PayloadFilesHavePrefix)",	"4.0-1",
	(RPMSENSE_RPMLIB|RPMSENSE_EQUAL),
    N_("package payload file(s) have \"./\" prefix.") },
    { "rpmlib(ExplicitPackageProvide)",	"4.0-1",
	(RPMSENSE_RPMLIB|RPMSENSE_EQUAL),
    N_("package name-version-release is not implicitly provided.") },
    { "rpmlib(HeaderLoadSortsTags)",    "4.0.1-1",
	(                RPMSENSE_EQUAL),
    N_("header tags are always sorted after being loaded.") },
    { "rpmlib(ScriptletInterpreterArgs)",    "4.0.3-1",
	(                RPMSENSE_EQUAL),
    N_("the scriptlet interpreter can use arguments from header.") },
    { "rpmlib(PartialHardlinkSets)",    "4.0.4-1",
	(                RPMSENSE_EQUAL),
    N_("a hardlink file set may be installed without being complete.") },
    { "rpmlib(ConcurrentAccess)",    "4.1-1",
	(                RPMSENSE_EQUAL),
    N_("package scriptlets may access the rpm database while installing.") },
    { NULL,				NULL, 0,	NULL }
};

void rpmShowRpmlibProvides(FILE * fp)
{
    const struct rpmlibProvides_s * rlp;

    for (rlp = rpmlibProvides; rlp->featureName != NULL; rlp++) {
/*@-nullpass@*/ /* FIX: rlp->featureEVR not NULL */
	rpmds pro = rpmdsSingle(RPMTAG_PROVIDENAME, rlp->featureName,
			rlp->featureEVR, rlp->featureFlags);
/*@=nullpass@*/
	const char * DNEVR = rpmdsDNEVR(pro);

	if (pro != NULL && DNEVR != NULL) {
	    fprintf(fp, "    %s\n", DNEVR+2);
	    if (rlp->featureDescription)
		fprintf(fp, "\t%s\n", rlp->featureDescription);
	}
	pro = rpmdsFree(pro);
    }
}

int rpmCheckRpmlibProvides(const rpmds key)
{
    const struct rpmlibProvides_s * rlp;
    int rc = 0;

    for (rlp = rpmlibProvides; rlp->featureName != NULL; rlp++) {
	if (rlp->featureEVR && rlp->featureFlags) {
	    rpmds pro;
	    pro = rpmdsSingle(RPMTAG_PROVIDENAME, rlp->featureName,
			rlp->featureEVR, rlp->featureFlags);
	    rc = rpmdsCompare(pro, key);
	    pro = rpmdsFree(pro);
	}
	if (rc)
	    break;
    }
    return rc;
}

int rpmGetRpmlibProvides(const char *** provNames, int ** provFlags,
                         const char *** provVersions)
{
    const char ** names, ** versions;
    int * flags;
    int n = 0;
    
/*@-boundswrite@*/
    while (rpmlibProvides[n].featureName != NULL)
        n++;
/*@=boundswrite@*/

    names = xcalloc((n+1), sizeof(*names));
    versions = xcalloc((n+1), sizeof(*versions));
    flags = xcalloc((n+1), sizeof(*flags));
    
/*@-boundswrite@*/
    for (n = 0; rpmlibProvides[n].featureName != NULL; n++) {
        names[n] = rpmlibProvides[n].featureName;
        flags[n] = rpmlibProvides[n].featureFlags;
        versions[n] = rpmlibProvides[n].featureEVR;
    }
    
    /*@-branchstate@*/
    if (provNames)
	*provNames = names;
    else
	names = _free(names);
    /*@=branchstate@*/

    /*@-branchstate@*/
    if (provFlags)
	*provFlags = flags;
    else
	flags = _free(flags);
    /*@=branchstate@*/

    /*@-branchstate@*/
    if (provVersions)
	*provVersions = versions;
    else
	versions = _free(versions);
    /*@=branchstate@*/
/*@=boundswrite@*/

    /*@-compmempass@*/ /* FIX: rpmlibProvides[] reachable */
    return n;
    /*@=compmempass@*/
}