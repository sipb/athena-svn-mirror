/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Show Software Info related functions.
 */

#include "defs.h"

static void 			DisplaySoftInfoNode();
extern int			SwFiles;

/*
 * List valid arguments for Software class. 
 */
extern void ListSoftInfo()
{
    SImsg(SIM_INFO, 
"\n\nTo show specific software use `-class software -show pkg1,pkg2...'.\n\n");
}

/*
 * Display Software Info
 */
extern void ShowSoftInfo(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    static SoftInfo_t 	       *Root = NULL;
    static MCSIquery_t		Query;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_SOFTINFO;
    if (mcSysInfo(&Query) != 0) {
	return;
    }
    Root = (SoftInfo_t *) Query.Out;

    if (!Root || !(Root->Slaves || Root->Next))
	return;

    ClassShowBanner(MyInfo);

    DisplaySoftInfoNode(Root, 0);
}

/*
 * Pretty Display of a SoftInfo entry.
 */
static void DisplaySoftInfoPretty(SoftInfo, OffSet)
     SoftInfo_t		       *SoftInfo;
     int			OffSet;
{
    register SoftInfo_t	       *sp;
    register SoftFileList_t    *SoftFile;
    Desc_t		       *Desc;
    char		       *cp;

    if (!SoftInfo->Name)
	return;

    if (VL_CONFIG) SImsg(SIM_INFO, "\n");
    ShowOffSet(OffSet);
    SImsg(SIM_INFO, "%-15s   ", SoftInfo->Name);

    /* Only include version here if it's a Product to keep output clean */
    if (SoftInfo->EntryTypeNum == MC_SET_PROD && SoftInfo->Version)
	SImsg(SIM_INFO, " %s", SoftInfo->Version);
    if (SoftInfo->Desc)
	SImsg(SIM_INFO, " %s", SoftInfo->Desc);
    else if (SoftInfo->DescVerbose) {
	/* Only print the first line */
	if (cp = strchr(SoftInfo->DescVerbose, '\n'))
	    SImsg(SIM_INFO, " %.*s", 
		  cp-SoftInfo->DescVerbose, SoftInfo->DescVerbose);
	else
	    SImsg(SIM_INFO, " %s", SoftInfo->DescVerbose);
    }
    SImsg(SIM_INFO, "\n");

#define CHK(a)	( a && *(a) )
    if (VL_DESC || VL_CONFIG) {
	ShowLabel("Name", OffSet);
	SImsg(SIM_INFO, " %s\n", SoftInfo->Name);
	if (CHK(SoftInfo->Version)) {
	    ShowLabel("Version", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Version);
	}
	if (CHK(SoftInfo->Revision)) {
	    ShowLabel("Revision", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Revision);
	}
	if (CHK(SoftInfo->Category)) {
	    ShowLabel("Category", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Category);
	}
	if (CHK(SoftInfo->SubCategory)) {
	    ShowLabel("Sub Category", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->SubCategory);
	}
	if (CHK(SoftInfo->OSname)) {
	    ShowLabel("OS Name", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->OSname);
	}
	if (CHK(SoftInfo->OSversion)) {
	    ShowLabel("OS Version", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->OSversion);
	}
	if (CHK(SoftInfo->Arch)) {
	    ShowLabel("Architecture", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Arch);
	}
	if (CHK(SoftInfo->ISArch)) {
	    ShowLabel("Instruct Set Arch", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->ISArch);
	}
	if (CHK(SoftInfo->BuildDate)) {
	    ShowLabel("Date Built", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->BuildDate);
	}
	if (CHK(SoftInfo->InstDate)) {
	    ShowLabel("Date Installed", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->InstDate);
	}
	if (CHK(SoftInfo->ProdStamp)) {
	    ShowLabel("Production Stamp", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->ProdStamp);
	}
	if (CHK(SoftInfo->BaseDir)) {
	    ShowLabel("Base Directory", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->BaseDir);
	}
	if (CHK(SoftInfo->VendorName)) {
	    ShowLabel("Vendor Name", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->VendorName);
	}
	if (CHK(SoftInfo->VendorEmail)) {
	    ShowLabel("Vendor Email", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->VendorEmail);
	}
	if (CHK(SoftInfo->VendorPhone)) {
	    ShowLabel("Vendor Phone", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->VendorPhone);
	}
	if (CHK(SoftInfo->VendorStock)) {
	    ShowLabel("Vendor Stock #", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->VendorStock);
	}
	for (Desc = SoftInfo->DescList; Desc; Desc = Desc->Next) {
	    ShowLabel( (Desc->Label) ? Desc->Label : "Description", OffSet);
	    SImsg(SIM_INFO, " %s\n", Desc->Desc);
	}
	if (SoftInfo->Master && SoftInfo->Master->Name) {
	    ShowLabel("Part of Product", OffSet);
	    SImsg(SIM_INFO, " %s", SoftInfo->Master->Name);
	    if (SoftInfo->Master->Version)
		SImsg(SIM_INFO, " %s", SoftInfo->Master->Version);
	    SImsg(SIM_INFO, "\n");
	}
	if (SoftInfo->Slaves) {
	    ShowLabel("Product Components", OffSet);
	    for (sp = SoftInfo->Slaves; sp; sp = sp->Next)
		SImsg(SIM_INFO, " %s", sp->Name);
	    SImsg(SIM_INFO, "\n");
	}
	if (SoftInfo->DiskUsage) {
	    ShowLabel("Disk Usage", OffSet);
	    SImsg(SIM_INFO, " %s\n", GetSizeStr(SoftInfo->DiskUsage, 1));
	}
	if (CHK(SoftInfo->License)) {
	    ShowLabel("License", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->License);
	}
	if (CHK(SoftInfo->Copyright)) {
	    ShowLabel("Copyright", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Copyright);
	}
	if (CHK(SoftInfo->URL)) {
	    ShowLabel("Product URL", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->URL);
	}
	if (CHK(SoftInfo->Desc)) {
	    ShowLabel("Description", OffSet);
	    SImsg(SIM_INFO, " %s\n", SoftInfo->Desc);
	}
	if (CHK(SoftInfo->DescVerbose)) {
	    ShowLabel("Verbose Description", OffSet);
	    if (strchr(SoftInfo->DescVerbose, '\n'))
		SImsg(SIM_INFO, " \\\n%s\n", SoftInfo->DescVerbose);
	    else
		SImsg(SIM_INFO, " %s\n", SoftInfo->DescVerbose);
	}
	if (SoftInfo->FileList) {
	    ShowLabel("Files", OffSet);
	    if (SwFiles) {
		SImsg(SIM_INFO, "\n");
		for (SoftFile = SoftInfo->FileList; SoftFile; 
		     SoftFile = SoftFile->Next) {
		    if (!SoftFile->FileData)
			continue;
		    ShowOffSet(OffSet + 8);
		    SImsg(SIM_INFO, "%s", SoftFile->FileData->Path);
		    if (SoftFile->FileData->LinkTo)
			SImsg(SIM_INFO, " -> %s", 
			      SoftFile->FileData->LinkTo);
		    if (SoftFile->FileData->FileSize)
			SImsg(SIM_INFO, "  SIZE: %s",
			      GetSizeStr(SoftFile->FileData->FileSize, 1));
		    if (SoftFile->FileData->MD5)
			SImsg(SIM_INFO, ", MD5: %s", SoftFile->FileData->MD5);
		    if (SoftFile->FileData->CheckSum)
			SImsg(SIM_INFO, ", CHECKSUM: %s", 
			      SoftFile->FileData->CheckSum);
		    SImsg(SIM_INFO, "\n");
		}
	    } else {
		SImsg(SIM_INFO, 
		      " Specify `+swfiles' to see list of files\n");
	    }
	}
    }
#undef CHK
}

/*
 * Replace all actual newline chars with the string \n.
 * If anything is replaced in String, memory will be alloc'ed.
 * If nothing is replaced, String will be returned.
 */
static char *EscapeNewlines(String)
     char		       *String;
{
    int				Found = 0;
    register char	       *cp;
    char		       *bp;
    char		       *Buffer;
    size_t			Len;

    if (!String)
	return (char *) NULL;

    for (cp = String, Found = 0; cp && *cp; ++cp)
	if (*cp == '\n')
	    ++Found;

    if (!Found)
	/* 
	 * There's no need to scan String since we found nothing.
	 */
	return String;

    Len = cp - String + Found + 1;

    bp = Buffer = (char *) xcalloc(1, Len);

    for (cp = String; cp && *cp; ++cp) {
	if (*cp == '\n') {
	    *bp++ = '\\';
	    *bp++ = 'n';
	} else
	    *bp++ = *cp;
    }
    *bp = CNULL;

    return Buffer;
}

/*
 * Report format Display of a SoftInfo entry.
 */
static void DisplaySoftInfoReport(Soft, OffSet)
     SoftInfo_t		       *Soft;
     int			OffSet;
{
    static char		       *RptData[26];
    Desc_t		       *Desc;
    SoftFileList_t	       *FileList;
    SoftFileData_t	       *File;
    char		       *cp;

    if (!Soft || !Soft->Name)
	return;

    (void) memset(RptData, CNULL, sizeof(RptData));
    RptData[0] = PRTS(Soft->EntryType);
    RptData[1] = PRTS(Soft->Name);
    RptData[2] = PRTS(Soft->Version);
    RptData[3] = PRTS(Soft->Revision);
    RptData[4] = PRTS(Soft->Desc);
    RptData[5] = PRTS(Soft->URL);
    RptData[6] = PRTS(Soft->License);
    RptData[7] = PRTS(Soft->Category);
    RptData[8] = PRTS(Soft->SubCategory);
    RptData[9] = PRTS(Soft->OSname);
    RptData[10] = PRTS(Soft->OSversion);
    RptData[11] = PRTS(Soft->Arch);
    RptData[12] = PRTS(Soft->ISArch);
    RptData[13] = PRTS(Soft->InstDate);
    RptData[14] = PRTS(Soft->BuildDate);
    RptData[15] = PRTS(Soft->ProdStamp);
    RptData[16] = PRTS(Soft->BaseDir);
    if (Soft->DiskUsage)
	RptData[17] = strdup(Ltoa(Soft->DiskUsage));
    RptData[18] = PRTS(Soft->VendorName);
    RptData[19] = PRTS(Soft->VendorEmail);
    RptData[20] = PRTS(Soft->VendorPhone);
    RptData[21] = PRTS(Soft->VendorStock);
    if (Soft->Master) {
	RptData[22] = PRTS(Soft->Master->Name);
	RptData[23] = PRTS(Soft->Master->Version);
	RptData[24] = PRTS(Soft->Master->Revision);
    }
    RptData[25] = PRTS(EscapeNewlines(Soft->DescVerbose));
    RptData[26] = PRTS(EscapeNewlines(Soft->Copyright));

    Report(CN_SOFTINFO, R_NAME, PRTS(Soft->Name), RptData,
	   sizeof(RptData) / sizeof(char *));

    /*
     * Report description info
     */
    (void) memset(RptData, CNULL, sizeof(RptData));
    for (Desc = Soft->DescList; Desc; Desc = Desc->Next) {
	RptData[0] = PRTS(Desc->Label);
	RptData[1] = PRTS(Desc->Desc);
	Report(CN_SOFTINFO, R_DESC, PRTS(Soft->Name), RptData, 2);
    }

    if (SwFiles) {
	/*
	 * Report all the file information
	 */
	(void) memset(RptData, CNULL, sizeof(RptData));
	for (FileList = Soft->FileList; FileList; 
	     FileList = FileList->Next) {
	    RptData[3] = RptData[5] = "";
	    if (File = FileList->FileData) {
		RptData[0] = SoftInfoFileType(File->Type);
		RptData[1] = PRTS(File->Path);
		RptData[2] = PRTS(File->LinkTo);
		if (File->FileSize && (cp = Ltoa(File->FileSize)))
		    RptData[3] = strdup(cp);
		RptData[4] = PRTS(File->MD5);
		RptData[5] = PRTS(File->CheckSum);
		if (File->PkgNames && 
		    (ArgvToStr(&cp, File->PkgNames, 0, ",", 0) == 0))
		    RptData[6] = cp;
		Report(CN_SOFTINFO, R_FILE, PRTS(Soft->Name), RptData, 6);
	    }
	}
    }
}

/*
 * Display a SoftInfo entry by calling the appropriate type of
 * format function.
 */
static void DisplaySoftInfo(SoftInfo, OffSet)
     SoftInfo_t		       *SoftInfo;
     int			OffSet;
{
    switch (FormatType) {
    case FT_PRETTY:	DisplaySoftInfoPretty(SoftInfo, OffSet); break;
    case FT_REPORT:	DisplaySoftInfoReport(SoftInfo, OffSet); break;
    }
}

/*
 * Display info about SoftInfo node and traverse tree.
 * --RECURSE--
 */
static void DisplaySoftInfoNode(SoftInfo, OffSet)
     SoftInfo_t		       *SoftInfo;
     int			OffSet;
{
    if (SoftInfo)
	DisplaySoftInfo(SoftInfo, OffSet);

    if (SoftInfo && SoftInfo->Slaves)
	DisplaySoftInfoNode(SoftInfo->Slaves, 
			(SoftInfo->Name) ? OffSet + OffSetAmt : 0);

    if (SoftInfo && SoftInfo->Next)
	DisplaySoftInfoNode(SoftInfo->Next, (SoftInfo->Name) ? OffSet : 0);
}
