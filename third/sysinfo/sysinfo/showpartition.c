/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Show Partware Info related functions.
 */

#include "defs.h"

extern int			DoPrintUnused;

/*
 * List valid arguments for Partition class. 
 */
extern void ListPartition()
{
    SImsg(SIM_INFO, 
	  "\n\nTo show specific partware use `-class partition -show part1,part2...'.\n\n");
}

/*
 * Pretty Display of a PartInfo entry.
 */
static void DisplayPartInfoPretty(Part, OffSet)
     PartInfo_t		       *Part;
     int			OffSet;
{
    register PartInfo_t	       *sp;
    Desc_t		       *Desc;
    char		      **cpp;
    char		       *cp;

    if (!Part)
	return;

    if (VL_CONFIG) SImsg(SIM_INFO, "\n");
    ShowOffSet(OffSet);
    SImsg(SIM_INFO, "%-15s", Part->BaseName);

    if (Part->MntName || Part->Type || Part->Size)
	SImsg(SIM_INFO, " is a ");

    if (Part->Size)
	SImsg(SIM_INFO, "%s ", GetSizeStr(Part->Size, BYTES));

    if (Part->TypeDesc)
	SImsg(SIM_INFO, "%s ", Part->TypeDesc);
    else if (Part->Type)
	SImsg(SIM_INFO, "%s ", Part->Type);
    else if (Part->Usage == PIU_UNUSED)
	SImsg(SIM_INFO, "UNUSED ");

    SImsg(SIM_INFO, "partition ");

    if (Part->MntName)
	SImsg(SIM_INFO, "in use as %s", Part->MntName);

    SImsg(SIM_INFO, "\n");

#define CHK(a)	( a && *(a) )
    if (VL_DESC || VL_CONFIG) {
	ShowLabel("Name", OffSet);
	SImsg(SIM_INFO, " %s\n", Part->BaseName);
	if (CHK(Part->DevPath)) {
	    ShowLabel("Device Path", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->DevPath);
	}
	if (CHK(Part->DevPathRaw)) {
	    ShowLabel("Device Path Raw", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->DevPathRaw);
	}
	if (Part->UsageStatus) {
	    ShowLabel("Usage Status", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->UsageStatus);
	}
	if (CHK(Part->MntName)) {
	    ShowLabel("Mount/Usage", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->MntName);
	}
	if (CHK(Part->MntOpts)) {
	    ShowLabel("Mount/Usage Options", OffSet);
	    for (cpp = Part->MntOpts; cpp && *cpp; ++cpp)
		SImsg(SIM_INFO, " %s", *cpp);
	    SImsg(SIM_INFO, "\n");
	}
	if (CHK(Part->Type)) {
	    ShowLabel("Type", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->Type);
	}
	if (CHK(Part->TypeDesc)) {
	    ShowLabel("Type Description", OffSet);
	    SImsg(SIM_INFO, " %s\n", Part->TypeDesc);
	}
	if (Part->TypeNum) {
	    ShowLabel("Type Number", OffSet);
	    SImsg(SIM_INFO, " 0x%02X\n", Part->TypeNum);
	}
	if (Part->Size) {
	    ShowLabel("Size", OffSet);
	    SImsg(SIM_INFO, " %s\n", GetSizeStr(Part->Size, BYTES));
	}
	if (Part->AmtUsed) {
	    ShowLabel("Amount In Use", OffSet);
	    SImsg(SIM_INFO, " %s\n", GetSizeStr(Part->AmtUsed, BYTES));
	}
	if (Part->SecSize) {
	    ShowLabel("Sector Size", OffSet);
	    SImsg(SIM_INFO, " %d\n", Part->SecSize);
	}
	/* Assume StartSect is valid only if NumSect is */
	if (Part->StartSect && Part->NumSect) {
	    ShowLabel("Starting Sector", OffSet);
	    SImsg(SIM_INFO, " %ld\n", Part->StartSect);
	}
	if (Part->EndSect && Part->StartSect) {
	    ShowLabel("Ending Sector", OffSet);
	    SImsg(SIM_INFO, " %ld\n", Part->EndSect);
	}
	if (Part->NumSect) {
	    ShowLabel("# of Sectors", OffSet);
	    SImsg(SIM_INFO, " %ld\n", Part->NumSect);
	}
    }
#undef CHK
}

/*
 * Report format Display of a PartInfo entry.
 */
static void DisplayPartInfoReport(Part, OffSet)
     PartInfo_t		       *Part;
     int			OffSet;
{
    static char		       *RptData[17];
    char		       *cp;

    (void) memset(RptData, CNULL, sizeof(RptData));
    RptData[0] = PRTS(Part->DevName);
    RptData[1] = PRTS(Part->BaseName);
    RptData[2] = PRTS(Part->Name);
    RptData[3] = strdup(itoa(Part->Num));
    RptData[4] = PRTS(Part->DevPathRaw);
    RptData[5] = PRTS(Part->Type);
    RptData[6] = PRTS(Part->TypeDesc);
    RptData[7] = strdup(itoa(Part->TypeNum));
    RptData[8] = PRTS(Part->UsageStatus);
    RptData[9] = PRTS(Part->MntName);
    if (ArgvToStr(&cp, Part->MntOpts, 0, ",", 0) == 0)
	RptData[10] = PRTS(cp);
    RptData[11] = strdup(Ltoa(Part->Size));
    RptData[12] = strdup(Ltoa(Part->AmtUsed));
    RptData[13] = strdup(itoa(Part->SecSize));
    RptData[14] = strdup(Ltoa(Part->StartSect));
    RptData[15] = strdup(Ltoa(Part->EndSect));
    RptData[16] = strdup(Ltoa(Part->NumSect));

    Report(CN_PARTITION, R_NAME, PRTS(Part->DevPath), RptData,
	   sizeof(RptData) / sizeof(char *));
}

/*
 * Display Partition info
 */
extern void ShowPartition(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    static PartInfo_t 	       *Root = NULL;
    static MCSIquery_t		Query;
    PartInfo_t		       *Part;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_PARTITION;
    if (mcSysInfo(&Query) != 0) {
	return;
    }
    Root = (PartInfo_t *) Query.Out;

    ClassShowBanner(MyInfo);

    /*
     * Traverse list
     */
    for (Part = Root; Part; Part = Part->Next) {
	if (!Part->BaseName || (Part->Type == PIU_UNUSED && !DoPrintUnused))
	    continue;
	switch (FormatType) {
	case FT_PRETTY:	DisplayPartInfoPretty(Part, 0); break;
	case FT_REPORT:	DisplayPartInfoReport(Part, 0); break;
	}
    }
}
