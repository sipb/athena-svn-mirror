/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * SunOS kstat functions
 */

#include "defs.h"
#if	defined(HAVE_KSTAT)
#include <kstat.h>

/*
 * We will create KstatCtl on our first call - opening a file descriptor -
 * and never destroy (close the fd).
 */
static kstat_ctl_t	       *KstatCtl;

/*
 * Find kstat entry for FindName.
 */
static kstat_t *KstatFind(KstatCtl, FindName)
     kstat_ctl_t	       *KstatCtl;
     char		       *FindName;
{
    static char			Name[KSTAT_STRLEN];
    kstat_t		       *Kstat = NULL;

    /*
     * There's no consistancy between "Name,err" and "Name,error".
     */
    (void) snprintf(Name, sizeof(Name), "%s,err", FindName);
    if (Kstat = kstat_lookup(KstatCtl, NULL, -1, Name))
	return Kstat;

    (void) snprintf(Name, sizeof(Name), "%s,error", FindName);
    Kstat = kstat_lookup(KstatCtl, NULL, -1, Name);

    return Kstat;
}
#endif	/* HAVE_KSTAT */

/*
 * Query for device info on DevInfo by looking in kstat.
 * You find the strangest information in some places.
 */
extern int KstatQuery(DevInfo)
     DevInfo_t		       *DevInfo;
{
#if	defined(HAVE_KSTAT)
    kstat_t		       *Kstat = NULL;
    kstat_named_t	       *Kname;
    char		       *cp;
    char		       *Data;
    size_t			DataSize;
    register int		i;
    register int		len;

    if (!DevInfo || !DevInfo->Name) {
	SImsg(SIM_DBG, "KstatDevInfo: Invalid parameters.");
	return -1;
    }

    /*
     * Only proceed if there's something to be gained.
     */
    if (DevInfo->Vendor && DevInfo->Model && DevInfo->Revision && 
	DevInfo->Serial)
	return 0;

    if (!KstatCtl) {
	KstatCtl = kstat_open();
	if (!KstatCtl) {
	    SImsg(SIM_DBG, "kstat_open failed: %s", SYSERR);
	    return -1;
	}
    }

    if (DevInfo->AltName)
	Kstat = KstatFind(KstatCtl, DevInfo->AltName);
    if (!Kstat && DevInfo->Name)
	Kstat = KstatFind(KstatCtl, DevInfo->Name);

    if (!Kstat) {
	SImsg(SIM_DBG, "%s: kstat lookup of <%s> failed.",
	      DevInfo->Name, 
	      (DevInfo->AltName) ? DevInfo->AltName : DevInfo->Name);
	return -1;
    }

    /*
     * Read in the ks_data segment.
     */
    if (Kstat->ks_ndata > 0)
	if (kstat_read(KstatCtl, Kstat, NULL) == -1) {
	    SImsg(SIM_DBG, "%s: kstat_read failed: %s", DevInfo->Name, SYSERR);
	    return -1;
	}

    Data = (char *) Kstat->ks_data;
    DataSize = sizeof(kstat_named_t);

    /*
     * Iterate over Data in DataSize chunks looking for entries we like.
     */
    for (i = 0; i < Kstat->ks_ndata; ++i) {
	Kname = (kstat_named_t *) Data;

	if (Kname->data_type == KSTAT_DATA_CHAR) {
	    len = sizeof(Kname->value.c);
	    if (!DevInfo->Vendor && Kname->value.c[0] && 
		EQ(Kname->name, "Vendor")) {
		if (DevInfo->Vendor = CleanString(Kname->value.c, len, 0))
		    SImsg(SIM_DBG, "\t%s: KSTAT Vendor=<%s>", 
			  DevInfo->Name, DevInfo->Vendor);
	    } else if (!DevInfo->Model && Kname->value.c[0] && 
		       EQ(Kname->name, "Product")) {
		if (DevInfo->Model = CleanString(Kname->value.c, len, 0))
		    SImsg(SIM_DBG, "\t%s: KSTAT Model=<%s>", 
			  DevInfo->Name, DevInfo->Model);
	    } else if (!DevInfo->Revision && Kname->value.c[0] && 
		       EQ(Kname->name, "Revision")) {
		if (DevInfo->Revision = CleanString(Kname->value.c, len, 0))
		    SImsg(SIM_DBG, "\t%s: KSTAT Revision=<%s>", 
			  DevInfo->Name, DevInfo->Revision);
	    } else if (!DevInfo->Serial && strlen(Kname->value.c) > 6 && 
		       EQ(Kname->name, "Serial No")) {
		if (DevInfo->Serial = CleanString(Kname->value.c, len,
						  CSF_ENDNONPR))
		    SImsg(SIM_DBG, "\t%s: KSTAT Serial=<%s>", 
			  DevInfo->Name, DevInfo->Serial);
	    }
	}

	Data += DataSize;
    }
#endif	/* HAVE_KSTAT */
    return 0;
}
