/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Common SysInfo EDID functions
 */

#include "defs.h"
#include "edid.h"

static u_char EdidHeader[] = EDID_HEADER;

/*
 * Established Timings I, II, III in decreasing order (bit 7->0)
 */
static char	*EstTimingI[] = {
    "720x400@70", "720x400@88", "640x480@60", "640x480@67", 
    "640x480@72", "640x480@75", "800x600@56", "800x600@60", NULL
};
static char	*EstTimingII[] = {
    "800x600@72", "800x600@75", "832x624@75", "1024x768@87",
    "1024x768@60", "1024x768@70", "1024x768@75", "1280x1024@75", NULL
};
static char	*EstTimingIII[] = {
    "1152x870@75", NULL
};

/*
 * Is Header an EDID Header?
 * Return TRUE if yes or FALSE if not.
 */
static int EdidIsHeader(Header)
     u_char		       *Header;
{
    register int		i;
    int				Max;

    if (!Header)
	return(FALSE);

    Max = sizeof(EdidHeader) / sizeof(u_char);
    for (i = 0; i < Max; ++i)
	if (Header[i] != EdidHeader[i])
	    return(FALSE);

    return(TRUE);
}

/*
 * Get the Manufacturer ID from Edid.
 */
static char *EdidGetMan(Edid)
     EDIDv1_t		       *Edid;
{
    static char			Buff[4];

    if (!Edid)
	return((char *) NULL);

    if (Edid->ManIDc1 == 0 && Edid->ManIDc2 == 0 && Edid->ManIDc3 == 0) {
	SImsg(SIM_DBG, "EDID: GetMan: No Manufacturer ID present.");
	return((char *) NULL);
    }

    (void) snprintf(Buff, sizeof(Buff), "%c%c%c",
		    'A' + Edid->ManIDc1 - 1, 'A' + Edid->ManIDc2 - 1, 
		    'A' + Edid->ManIDc3 - 1);

    return(Buff);
}

/*
 * Get the Product ID from Edid.
 */
static char *EdidGetProductID(Edid)
     EDIDv1_t		       *Edid;
{
    static char			Buff[32];

    if (!Edid)
	return((char *) NULL);

    if (Edid->ProductIdCode[0] == 0 && Edid->ProductIdCode[1] == 0) {
	SImsg(SIM_DBG, "EDID: GetProductID: No Product ID present.");
	return((char *) NULL);
    }

    /*
     * Stored LSB
     */
    (void) snprintf(Buff, sizeof(Buff), "%02X%02X",
		    Edid->ProductIdCode[1], Edid->ProductIdCode[0]);

    return(Buff);
}

/*
 * Get the Serial from Edid.
 */
static char *EdidGetSerial(Edid)
     EDIDv1_t		       *Edid;
{
    static char			Buff[32];

    if (!Edid)
	return((char *) NULL);

    if (Edid->Serial[0] == 0 && Edid->Serial[1] == 0 &&
	Edid->Serial[2] == 0 && Edid->Serial[3] == 0) {
	SImsg(SIM_DBG, "EDID: GetSerial: No Serial ID present.");
	return((char *) NULL);
    }

    if (Edid->Serial[0] == 0x1 && Edid->Serial[1] == 0x1 &&
	Edid->Serial[2] == 0x1 && Edid->Serial[3] == 0x1) {
	SImsg(SIM_DBG, "EDID: GetSerial: Serial ID unused.");
	return((char *) NULL);
    }

    (void) snprintf(Buff, sizeof(Buff), "%02X%02X%02X%02X",
		    Edid->Serial[0], Edid->Serial[1],
		    Edid->Serial[2], Edid->Serial[3]);

    return(Buff);
}

/*
 * Set Monitor Range info into DevInfo
 */
static int EdidSetRange(DevInfo, Range)
     DevInfo_t		       *DevInfo;
     MonRange_t		       *Range;
{
    if (!DevInfo || !Range)
	return(-1);

    if (Range->MinVertical)
	AddDevDesc(DevInfo, itoa(Range->MinVertical), 
		   "RG: Minimum Vertical Rate (Hz)", DA_APPEND);
    if (Range->MaxVertical)
	AddDevDesc(DevInfo, itoa(Range->MaxVertical), 
		   "RG: Maximum Vertical Rate (Hz)", DA_APPEND);
    if (Range->MinHorizontal)
	AddDevDesc(DevInfo, itoa(Range->MinHorizontal), 
		   "RG: Minimum Horizontal Rate (KHz)", DA_APPEND);
    if (Range->MaxHorizontal)
	AddDevDesc(DevInfo, itoa(Range->MaxHorizontal), 
		   "RG: Maximum Horizontal Rate (KHz)", DA_APPEND);
    if (Range->MaxPixelClock)
	AddDevDesc(DevInfo, itoa(Range->MaxPixelClock * 10), 
		   "RG: Maximum Pixel Clock (MHz)", DA_APPEND);

    return(0);
}

/*
 * Set specific Established Timing Info
 */
static int EdidSetEstTiming(DevInfo, Timing, EstTimings)
     DevInfo_t		       *DevInfo;
     u_char			Timing;
     char		      **EstTimings;
{
    register int		i;
    register int		e;
    u_char			mask;

    for (i = 1, e = 0, mask = 1; i < 8 && EstTimings[e]; ++i, ++e) {
	if (Timing & mask)
	    DevAddRes(DevInfo, EstTimings[e]);
	mask <<= 1;
    }

    return(0);
}

/*
 * Set Established Timing Info
 */
static int EdidSetEstTimings(DevInfo, Timings, NumTimings)
     DevInfo_t		       *DevInfo;
     u_char		       *Timings;
     size_t			NumTimings;
{
    if (NumTimings >= 1)
	EdidSetEstTiming(DevInfo, Timings[0], EstTimingI);
    if (NumTimings >= 2)
	EdidSetEstTiming(DevInfo, Timings[1], EstTimingII);
    if (NumTimings >= 3)
	EdidSetEstTiming(DevInfo, Timings[2], EstTimingIII);

    return(0);
}

/*
 * Set Standard Timing Info
 */
static int EdidSetStdTimings(DevInfo, Timings, NumTimings)
     DevInfo_t		       *DevInfo;
     StdTiming_t	       *Timings;
     size_t			NumTimings;
{
    static char			Buff[32];
    register int		t;
    register StdTiming_t       *Ptr;
    int				HorPixels;
    int				VerPixels;
    float			Multi;

    for (t = 0, Ptr = Timings; t < NumTimings; ++t, ++Ptr) {
	if (Ptr->HorActivePixels == 0 ||
	    Ptr->HorActivePixels == 0x1)    /* Unused? */
	    continue;
	/*
	 * Get the Aspect Ratio and use it as a multiplier
	 */
	switch (Ptr->Aspect) {
	case 0:	Multi = (float)1 / (float)1;		break;
	case 1:	Multi = (float)3 / (float)4;		break;
	case 2:	Multi = (float)4 / (float)5;		break;
	case 3:	Multi = (float)9 / (float)16;		break;
	default:
	    SImsg(SIM_DBG, "%s: Unknown Aspect Ratio type: %d", 
		  DevInfo->Name, Ptr->Aspect);
	    continue;
	}
	/*
	 * Magic formulas as specified by VESA EDID standard.
	 */
	HorPixels = ((Ptr->HorActivePixels - 1) * 8) + 256;
	VerPixels = (int)( (float)HorPixels * (float)Multi );
	(void) snprintf(Buff, sizeof(Buff), "%dx%d@%d",
			HorPixels, VerPixels, Ptr->RefreshRate + 60);

	DevAddRes(DevInfo, strdup(Buff));
    }

    return(0);
}

/*
 * Set Detailed Timing Info
 */
static int EdidSetDetail(DevInfo, Details, NumDetails)
     DevInfo_t		       *DevInfo;
     Detail_t		       *Details;
     size_t			NumDetails;
{
    register Detail_t	       *Ptr;
    register char	       *cp;
    register int		Blk;
    char		       *String = NULL;
    char		       *Model = NULL;
    char		       *Serial = NULL;

    if (!DevInfo || !Details)
	return(-1);

    for (Blk = 0, Ptr = Details; Blk < NumDetails; ++Blk, ++Ptr) {
	if (Ptr->Flag1 != 0) {
	    SImsg(SIM_DBG, "%s: EDID: Detail Block %d has Flag1=0x%x",
		  DevInfo->Name, Blk, Ptr->Flag1);
	    /*
	     * This must be a Detailed Timing Descriptor
	     * Skip for now.
	     */
	    continue;
	} else {
	    switch (Ptr->DataType) {
	    case MDD_NAME:
		cp = CleanString(Ptr->Data, sizeof(Ptr->Data), 0);
		if (cp && strlen(cp))
		    DevInfo->Model = Model = cp;
		break;
	    case MDD_SERIAL:
		cp = CleanString(Ptr->Data, sizeof(Ptr->Data), 0);
		if (cp && strlen(cp))
		    DevInfo->Serial = Serial = cp;
		break;
	    case MDD_STRING:
		/* Not sure what's stored here so we'll print out on debug */
		cp = CleanString(Ptr->Data, sizeof(Ptr->Data), 0);
		if (cp && strlen(cp))
		    String = cp;
		break;
	    case MDD_RANGE:
		EdidSetRange(DevInfo, (MonRange_t *) Ptr->Data);
		break;
	    case MDD_STDTIMING:
		EdidSetStdTimings(DevInfo, (StdTiming_t *) Ptr->Data, 6);
		break;
	    default:
		if (Ptr->DataType > 0x00 && Ptr->DataType <= 0x0F)
		    SImsg(SIM_DBG, 
			  "%s: EDID: Ignoring manufacturer DataType=0x%X",
			  DevInfo->Name, Ptr->DataType);
		else
		    SImsg(SIM_DBG, "%s: EDID: Ignoring Detail DataType=0x%X",
			  DevInfo->Name, Ptr->DataType);
	    }
	}
    }

    SImsg(SIM_DBG, "%s: EDID: Model=<%s> Serial=<%s> String=<%s>",
	  DevInfo->Name, PRTS(Model), PRTS(Serial), PRTS(String));

    return(0);
}

/*
 * Decode EDID Version 1 
 */
static int EdidV1Decode(Query)
     Query_t		       *Query;
{
    static EDIDv1_t		Edid;
    static char			Buff[256];
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Define;
    char		       *DevName = NULL;
    char		       *ManID = NULL;
    char		       *ProductID = NULL;
    register char	       *cp;
    Monitor_t		       *Mon = NULL;

    if (!Query || Query->DataLen != EDID_V1_SIZE || !Query->Data || 
	!Query->DevInfo)
	return(-1);

    /*
     * Setup what vars we're going to use
     */
    DevInfo = Query->DevInfo;
    if (DevInfo && DevInfo->Name)
	DevName = DevInfo->Name;
    else
	DevName = "unknown";
    if (DevInfo->DevSpec)
	Mon = (Monitor_t *) DevInfo->DevSpec;
    else {
	Mon = NewMonitor(NULL);
	DevInfo->DevSpec = (void *) Mon;
    }

    (void) memcpy(&Edid, Query->Data, EDID_V1_SIZE);

    /*
     * It's possible to have EDID data that's not really EDID.
     */
    if (!EdidIsHeader(Edid.Header)) {
	SImsg(SIM_DBG, "%s: EDID present, but header is not EDID.", DevName);
	return(-1);
    }

    SImsg(SIM_DBG, "%s: EDID present: Version %u Revision %u ExtFlag=%d",
	  DevName, Edid.EDIDversion, Edid.EDIDrevision, Edid.ExtFlag); 

    (void) snprintf(Buff, sizeof(Buff), "%u", Edid.EDIDversion);
    AddDevDesc(DevInfo, Buff, "EDID Version", DA_APPEND);
    (void) snprintf(Buff, sizeof(Buff), "%u", Edid.EDIDrevision);
    AddDevDesc(DevInfo, Buff, "EDID Revision", DA_APPEND);

    /*
     * Get the Manufacturer and Product ID codes
     */
    ManID = EdidGetMan(&Edid);
    ProductID = EdidGetProductID(&Edid);
    SImsg(SIM_DBG, "%s: EDID ManID=<%s> ProductID=<%s>",
	  DevName, PRTS(ManID), PRTS(ProductID));
    /*
     * See if we can look up translated names for ManID and ProductID
     */
    if (ManID) {
	AddDevDesc(DevInfo, ManID, "EDID Manufacturer ID", DA_APPEND);
	if(Define = DefGet("EISAvendors", ManID, 0, 0))
	    DevInfo->Vendor = Define->ValStr1;
	else
	    SImsg(SIM_DBG, "%s: `%s' is not in `EISAvendors' in config/*.cf",
		  DevName, ManID);
    }
    if (ProductID)
	AddDevDesc(DevInfo, ProductID, "EDID Product ID", DA_APPEND);
    if (ProductID && ManID) {
	(void) snprintf(Buff, sizeof(Buff), "%s%s", ManID, ProductID);
	if (Define = DefGet("EISAdevices", Buff, 0, 0))
	    DevInfo->Model = Define->ValStr1;
	else
	    SImsg(SIM_DBG, "%s: `%s' is not in `EISAdevices' in config/*.cf",
		  DevName, Buff);
    }

    if (!DevInfo->Vendor)
	DevInfo->Vendor = strdup(ManID);
    if (!DevInfo->Model)
	DevInfo->Model = strdup(ProductID);

    if (cp = EdidGetSerial(&Edid))
	DevInfo->Serial = strdup(cp);

    (void) EdidSetEstTimings(DevInfo, Edid.EstTimings,
			     sizeof(Edid.EstTimings)/sizeof(u_char));
    (void) EdidSetStdTimings(DevInfo, Edid.StdTimings,
			     sizeof(Edid.StdTimings)/sizeof(StdTiming_t));
    (void) EdidSetDetail(DevInfo, Edid.Details, 
			 sizeof(Edid.Details)/sizeof(Detail_t));

    /*
     * Get Manufacture date (MM/YYYY)
     */
    if (Edid.ManWeek && Edid.ManYear) {
	(void) snprintf(Buff, sizeof(Buff), "%d/%d",
			(int)(Edid.ManWeek / 4.33), Edid.ManYear + 1990);
	AddDevDesc(DevInfo, Buff, "Manufacture Date", DA_APPEND);
    }

    if (Edid.HorImageSize && Edid.VerImageSize) {
	Mon->MaxHorSize = Edid.HorImageSize;
	Mon->MaxVerSize = Edid.VerImageSize;
    }

    /*
     * Determine Class Type
     */
    switch (Edid.DisplayType) {
    case 0:	DevInfo->ClassType = CT_MONO;		break;
    case 1:	DevInfo->ClassType = CT_RGBCOLOR;	break;
    case 2:	DevInfo->ClassType = CT_NONRGBCOLOR;	break;
    default:
	SImsg(SIM_DBG, "%s: %d is an unknown DisplayType value from EDID.",
	      DevInfo->Name, Edid.DisplayType);
    }

    AddDevDesc(DevInfo, (Edid.SigType) ? "Digital" : "Analog",
	       "Signal Input Type", DA_APPEND);

    if (Edid.DpmsStandBy)
	AddDevDesc(DevInfo, "Stand-By", "DPMS Support", DA_APPEND);
    if (Edid.DpmsSuspend)
	AddDevDesc(DevInfo, "Suspend", "DPMS Support", DA_APPEND);
    if (Edid.DpmsActiveOff)
	AddDevDesc(DevInfo, "Active-Off", "DPMS Support", DA_APPEND);

    if (Edid.StdColorSpace)
	AddDevDesc(DevInfo, "Uses standard default color space", 
		   NULL, DA_APPEND);

    if (Edid.HasPrefTiming)
	AddDevDesc(DevInfo, "Preferred Timing in 1st Detail Block", 
		   "Has", DA_APPEND);

    if (Edid.HasGtf)
	AddDevDesc(DevInfo, "GTF Standard Timings", "Has", DA_APPEND);

    return(0);
}

/*
 * Decode an EDID structure and set Query->DevInfo with what we find.
 */
extern int EdidDecode(Query)
     Query_t		       *Query;
{
    if (!Query)
	return(-1);

    /*
     * We only support V1 (128-byte) EDID right now
     */
    if (Query->DataLen == EDID_V1_SIZE)
	return EdidV1Decode(Query);
    else {
	SImsg(SIM_DBG, "%s: EdidDecode: Unknown EDID Structure size: %d",
	      (Query->DevInfo && Query->DevInfo->Name) ? 
	      Query->DevInfo->Name : "unknown", Query->DataLen);
	return(-1);
    }
}
