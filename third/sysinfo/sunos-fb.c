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
 * SunOS FrameBuffer functions
 */

#include "defs.h"
#include "sunos-obp.h"

/*
 * Name of frame buffer "indirect" device.
 */
#define FBDEVICE		"fb"

/*
 * Find frame buffer device file for FBname.
 */
extern char *FBfindFile(ProbeData)
     ProbeData_t	       *ProbeData;
{
    char 		       *FBname;
    char 		       *File;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
    static char			FileBuff[MAXPATHLEN];

    if (!ProbeData)
	return((char *) NULL);
    FBname = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

#if	OSMVER == 4
    if ((File = GetCharFile(FBname, NULL)) && FileExists(File))
	return(File);
    if (FileExists("/dev/fb"))
	return("/dev/fb");
#else	/* OSMVER == 5 */

    if (DevDefine && DevDefine->File) {
	(void) snprintf(FileBuff, sizeof(FileBuff), "%s/%s%d", 
		       _PATH_DEV_FBS, DevDefine->File, DevData->DevUnit);
	if (FileExists(FileBuff))
	    return(FileBuff);
    }

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/%s", 
		    _PATH_DEV_FBS, FBname);
    if (FileExists(FileBuff))
	return(FileBuff);

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/%s", 
		    _PATH_DEV, FBname);
    if (FileExists(FileBuff))
	return(FileBuff);

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/%s", 
		    _PATH_DEV_FBS, FBname);
    if (FileExists(FileBuff))
	return(FileBuff);

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/%s", 
		    _PATH_DEV, FBDEVICE);
    if (FileExists(FileBuff))
	return(FileBuff);
#endif	/* OSMVER == 5 */

    SImsg(SIM_DBG, "Could not find device file for <%s>.", FBname);

    return((char *) NULL);
}

/*
 * Check for a monitor and create a device if found.
 */
static DevInfo_t *ProbeMonitor(ProbeData, MonFreq)
     ProbeData_t	       *ProbeData;
     int		       *MonFreq;
{
    DevInfo_t		       *DevInfo = NULL;
    int				GotMonInfo = FALSE;
    static int			MonUnit;
    static char			DevName[32];
    Define_t		       *MonDef;
    char		       *MonIDstr;
    int				MonID = -1;
    DevData_t 		       *FBDevData;
    char		       *FBFile;
    int			        FBFileDesc;
    int				GotEdid = 0;
#if	defined(HAVE_OPENPROM)
    static Query_t		Query;
    OBPprop_t		       *EdidProp = NULL;
#endif	/* HAVE_OPENPROM */
#if	defined(FBIOMONINFO)
    struct mon_info		mon_info;
#endif	/* FBIOMONINFO */

    FBDevData = ProbeData->DevData;
    FBFile = ProbeData->DevFile;
    FBFileDesc = ProbeData->FileDesc;


#if	defined(FBIOMONINFO)
    /*
     * Monitor info
     */
    memset(&mon_info, 0, sizeof(mon_info));
    if (ioctl(FBFileDesc, FBIOMONINFO, &mon_info) == 0)
	GotMonInfo = TRUE;
    else {
	SImsg(SIM_GERR, "%s: Get monitor info (FBIOMONINFO) failed: %s",
	      FBFile, SYSERR);
    }
#endif	/* FBIOMONINFO */

#if	defined(HAVE_OPENPROM)
    /*
     * Ask OBP for the monitor ID.
     */
    if (MonIDstr = OBPfindPropVal("montype", NULL, FBDevData->NodeID, NULL))
	MonID = atoi(MonIDstr);

#if	OSMVER == 5
    /*
     * Look for EDID property data in the Frame Buffer Node entry
     */
    if (EdidProp = OBPfindProp(OBP_EDID, NULL, FBDevData->NodeID, NULL))
	GotEdid = TRUE;
#endif	/* OSMVER == 5 */
#endif	/* HAVE_OPENPROM */

    /*
     * Use FBIOMONINFO info if we got nothing from OBP
     */
    if (GotMonInfo && MonID < 0)
	MonID = mon_info.mon_type;

    /*
     * If we have no data, then return now
     */
    if (!GotMonInfo && !GotEdid && MonID < 0)
	return((DevInfo_t *) NULL);

    /*
     * Commit to making a device
     */
    DevInfo = NewDevInfo(NULL);

    DevInfo->Unit = MonUnit++;
    (void) snprintf(DevName, sizeof(DevName), "monitor%d", DevInfo->Unit);
    DevInfo->Name = strdup(DevName);
    DevInfo->Type = DT_MONITOR;

#if	defined(HAVE_OPENPROM)
    /*
     * Decode and set EDID info
     */
    if (EdidProp) {
	(void) memset(&Query, 0, sizeof(Query));
	Query.DevInfo = DevInfo;
	Query.Data = EdidProp->Raw;
	Query.DataLen = EdidProp->RawLen;

#if	OSMVER == 5
	EdidDecode(&Query);
#endif	/* OSMVER == 5 */
    }
#endif	/* HAVE_OPENPROM */

    if (MonID >= 0) {
	if (MonDef = DefGet("MonitorID", NULL, (long)MonID, 0))
	    DevInfo->Model = MonDef->ValStr2;
	AddDevDesc(DevInfo, itoa(MonID), "Type", DA_APPEND);
    }

#if	defined(FBIOMONINFO)
    if (GotMonInfo) {
	if (MonFreq)
	    *MonFreq = mon_info.vfreq;
	AddDevDesc(DevInfo, FreqStr(mon_info.pixfreq), 
		   "Pixel Frequency", DA_APPEND);
	AddDevDesc(DevInfo, FreqStr(mon_info.hfreq), 
		   "Horizontal Frequency", DA_APPEND);
	AddDevDesc(DevInfo, FreqStr(mon_info.vfreq), 
		   "Vertical Frequency", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.hsync), 
		   "Horizontal Sync (pixels)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.vsync), 
		   "Vertical Sync (scanlines)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.hfporch), 
		   "Horizontal Front Porch (pixels)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.hbporch), 
		   "Horizontal Back Porch (pixels)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.vfporch), 
		   "Vertical Front Porch (pixels)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.vbporch), 
		   "Vertical Back Porch (pixels)", DA_APPEND);
    }
#endif	/* FBIOMONINFO */

    return(DevInfo);
}

/*
 * Set PGX (m64) specific information.
 */
static void FBsetPGX(ProbeData, ShortName, FBgattr)
     /*ARGSUSED*/
     ProbeData_t	       *ProbeData;
     char		       *ShortName;
     struct fbgattr	       *FBgattr;
{
    DevInfo_t		       *DevInfo;

    if (!ProbeData || !ProbeData->UseDevInfo || !FBgattr)
	return;

    DevInfo = ProbeData->UseDevInfo;

    /*
     * From the m64(7D) man page.
     */
    AddDevDesc(DevInfo, GetSizeStr(FBgattr->sattr.dev_specific[0], BYTES),
	       "Total Mappable Memory", DA_APPEND);

    AddDevDesc(DevInfo, itoax(FBgattr->sattr.dev_specific[2]), 
	       "Chip Revision", DA_APPEND);

    AddDevDesc(DevInfo, itoax(FBgattr->sattr.dev_specific[3]), 
	       "DAC Revision", DA_APPEND);

    AddDevDesc(DevInfo, itoax(FBgattr->sattr.dev_specific[4]), 
	       "PROM Revision", DA_APPEND);
}

/*
 * Set GX (cgsix) specific information.
 */
static void FBsetGX(ProbeData, ShortName, FBgattr)
     /*ARGSUSED*/
     ProbeData_t	       *ProbeData;
     char		       *ShortName;
     struct fbgattr	       *FBgattr;
{
#if	defined(FBIOGXINFO)
    struct cg6_info 		cg6_info;
    FrameBuffer_t	       *fb;
    DevInfo_t		       *DevInfo;

    if (!ProbeData || !ProbeData->Opaque)
	return;

    fb = (FrameBuffer_t *) ProbeData->Opaque;
    DevInfo = ProbeData->UseDevInfo;

    memset(&cg6_info, 0, sizeof(cg6_info));
    if (ioctl(ProbeData->FileDesc, FBIOGXINFO, &cg6_info) != 0) {
	SImsg(SIM_GERR, "%s: FBIOGXINFO failed: %s.", 
	      ProbeData->DevFile, SYSERR);
	return;
    }

    AddDevDesc(DevInfo, itoa(cg6_info.slot), "Slot", DA_APPEND);
    AddDevDesc(DevInfo, itoa(cg6_info.boardrev), "Board Revision", DA_APPEND);

    /* 
     * Board Revs 0-4 are Double width
     * Board Revs > 4 are Single width
     */
    if (cg6_info.boardrev <= 4)
	AddDevDesc(DevInfo, NULL, "Double Width Card", DA_APPEND);
    else
	AddDevDesc(DevInfo, NULL, "Single Width Card", DA_APPEND);

    AddDevDesc(DevInfo, 
	       (cg6_info.hdb_capable) ? "Double Buffered" : 
	       "Single Buffered", "Buffering", DA_APPEND);

    if (cg6_info.vmsize)
	fb->VMSize 	= mbytes_to_bytes(cg6_info.vmsize);
#endif	/* FBIOGXINFO */
}

/*
 * Set AFB/FFB specific information.
 */
static void FBsetXFB(ProbeData, ShortName, FBgattr)
     /*ARGSUSED*/
     ProbeData_t	       *ProbeData;
     char		       *ShortName;
     struct fbgattr	       *FBgattr;
{
#if	defined(HAVE_OPENPROM)
    DevDefine_t		       *DevDef = NULL;
    FrameBuffer_t	       *Fb = NULL;
    Define_t		       *Define;
    char		       *Name;
    char 		       *BoardType;
    char 		        DefBoardType[32];
    int				btype;

    if (!ProbeData || !ProbeData->DevDefine || !ProbeData->DevDefine->Name ||
	!ProbeData->DevData || !ShortName)
	return;

    (void) snprintf(DefBoardType, sizeof(DefBoardType), "BoardType%s", 
		    ShortName);

    if (BoardType = OBPfindPropVal("board_type", NULL, 
				   ProbeData->DevData->NodeID, NULL)) {
	btype = atoi(BoardType);
	SImsg(SIM_DBG, "%s board_type=%s/%d/0x%x",
	      DefBoardType, BoardType, btype, btype);

	Define = DefGet(DefBoardType, NULL, (long)btype, 0);
	if (!Define)
	    SImsg(SIM_UNKN, "No %s entry in .cf file for 0x%x",
		  DefBoardType, btype);
	if (Define) {
	    /*
	     * Add what we got
	     */
	    ProbeData->UseDevInfo->Model = Define->ValStr1;
	    if (ProbeData->Opaque)
		Fb = (FrameBuffer_t *) ProbeData->Opaque;
	    else {
		Fb = NewFrameBuffer(NULL);
		ProbeData->Opaque = (void *) Fb;
	    }
	    if (Define->ValStr2)
		Fb->Depth = atoi(Define->ValStr2);
	    if (Define->ValStr3)
		AddDevDesc(ProbeData->UseDevInfo, Define->ValStr3,
			   "Board Orientation", DA_APPEND);
	}
#if	defined(FFB_Z_BUFFER)
	/* Do we have Z Buffering? */
	if (FLAGS_ON(btype, FFB_Z_BUFFER))
	    AddDevDesc(ProbeData->UseDevInfo, NULL, "Z Buffering", DA_INSERT);
#endif	/* FFB_Z_BUFFER */
#if	defined(FFB_DBL_BUFFER)
	/*
	 * Special hardcoded code (sigh!) to determine if this is a Creator
	 * or Creator3D FFB.  Creator3D is double buffered, Creator is Single.
	 * This overrides the OBP Model lookup because the Model info we have
	 * does not appear to be consistant.
	 */
	else if (FLAGS_ON(btype, FFB_DBL_BUFFER)) {
	    AddDevDesc(ProbeData->UseDevInfo, "Double buffered", "Buffering", 
		       DA_INSERT);
	    if (DevDef = DevDefGet(FFB_DBLNAME, 0, 0))
		ProbeData->DevDefine = DevDef;
	}
#endif	/* FFB_TYPE_DOUBLE */
    }
#endif	/* HAVE_OPENPROM */
}

/*
 * Probe a FrameBuffer.
 */
extern DevInfo_t *ProbeFrameBuffer(ProbeData)
     ProbeData_t	       *ProbeData;
{
    char 		       *FBname;
    char 		       *TypeName = NULL;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
    struct fbgattr		fbgattrbuf;
    struct fbgattr	       *FBgattr = NULL;
    DevInfo_t 		       *DevInfo;
    FrameBuffer_t 	       *fb;
    DevDefine_t		       *FBdef = NULL;
    char		       *MonID = NULL;
    char 		       *File;
    char 		       *RealModel = NULL;
    char 		       *cp;
    static char			Buff[BUFSIZ];
    int 			FileDesc;
    int				IsKnown = 0;
    int				MonFreq = 0;
    int				MonHres = 0;
    int				MonVres = 0;
    register int		i;
    register int		l;

    if (!ProbeData)
	return((DevInfo_t *) NULL);
    FBname = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    if (!FBname)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeFrameBuffer(%s)", FBname);

    if (!(File = FBfindFile(ProbeData)))
	return((DevInfo_t *) NULL);

    if ((FileDesc = open(File, O_RDONLY)) < 0) {
	SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", File, SYSERR);
	return((DevInfo_t *) NULL);
    }

    /*
     * Get real fb attributes
     */
    if (ioctl(FileDesc, FBIOGATTR, &fbgattrbuf) == 0)
	FBgattr = &fbgattrbuf;
    else {
	SImsg(SIM_GERR, "%s: FBIOGATTR failed: %s.", File, SYSERR);
	/*
	 * Try to at least get the FB type
	 */
	if (ioctl(FileDesc, FBIOGTYPE, &fbgattrbuf.fbtype) == 0)
	    FBgattr = &fbgattrbuf;
	else
	    SImsg(SIM_GERR, "%s: FBIOGTYPE failed: %s.", File, SYSERR);
    }

    /*
     * We're committed to try
     */
    if (!(fb = NewFrameBuffer(NULL))) {
	SImsg(SIM_GERR, "Cannot create new frame buffer.");
	return((DevInfo_t *) NULL);
    }

    if (!(DevInfo = NewDevInfo(NULL))) {
	SImsg(SIM_GERR, "Cannot create new frame buffer device entry.");
	return((DevInfo_t *) NULL);
    }

    /*
     * Prepare data for probing
     */
    ProbeData->Opaque = (void *) fb;
    ProbeData->DevFile = File;
    ProbeData->FileDesc = FileDesc;
    ProbeData->UseDevInfo = DevInfo;
    if (DevDefine) {
	if (TypeName = strchr(DevDefine->Name, ','))
	    ++TypeName;
	else
	    TypeName = DevDefine->Name;
    }

    /*
     * Call FB specific functions.
     * Sometimes we always call a function and let it decide if it's
     * the right type of FB.
     */
    FBsetGX(ProbeData, TypeName, FBgattr);
    if (EQ(TypeName, FFB_NAME) || EQ(TypeName, AFB_NAME))
	FBsetXFB(ProbeData, TypeName, FBgattr);
    else if (EQ(TypeName, PGX_NAME))
	FBsetPGX(ProbeData, TypeName, FBgattr);

    /*
     * Reset data obtained from FB specific functions.
     */
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;
    DevInfo = ProbeData->UseDevInfo;
    fb = (FrameBuffer_t *) ProbeData->Opaque;

    /*
     * See if there's a monitor attached.
     */
    DevInfo->Slaves = ProbeMonitor(ProbeData, &MonFreq);

    /*
     * We're done doing ioctl()'s
     */
    close(FileDesc);
    ProbeData->FileDesc = -1;

    /*
     * Find out what Model of fb this is.
     */
    if (!DevInfo->Model) {
	if (DevDefine && (!FBgattr || 
			  FLAGS_ON(DevDefine->Flags, DDT_DEFINFO))) {
	    DevInfo->Model = DevDefine->Model;
	} else if (FBgattr) {
	    FBdef = DevDefGet(NULL, DT_FRAMEBUFFER, FBgattr->fbtype.fb_type);
	    if (FBdef) {
		if (FBdef->Model) {
		    (void) snprintf(Buff, sizeof(Buff), "%s", FBdef->Model);
		    if (FBdef->Name)
			(void) snprintf(Buff + (l=strlen(FBdef->Model)), 
					sizeof(Buff)-l, " [%s]",
					FBdef->Name);
		    DevInfo->Model	= strdup(Buff);
		} else
		    DevInfo->Model 	= FBdef->Name;
		IsKnown = TRUE;
	    }
	    if (!IsKnown) {
		SImsg(SIM_UNKN, 
		      "Device `%s' is an unknown type (%d) of frame buffer.",
		      FBname, FBgattr->fbtype.fb_type);
		DevInfo->Model = "UNKNOWN";
	    }
	}
    }

#if	defined(FB_ATTR_NEMUTYPES)
    if (FBgattr) {
	/*
	 * See if this fb emulates other fb's.
	 */
	Buff[0] = CNULL;
	for (i = 0; i < FB_ATTR_NEMUTYPES && FBgattr->emu_types[i] >= 0; ++i)
	    if (FBgattr->emu_types[i] != FBgattr->fbtype.fb_type &&
		(FBdef = DevDefGet(NULL, DT_FRAMEBUFFER, 
				   (long)FBgattr->emu_types[i]))) {
		if (Buff[0])
		    (void) strcat(Buff, ", ");
		(void) strcat(Buff,
			      (FBdef->Name) ? FBdef->Name : 
			      ((FBdef->Model) ? FBdef->Model : ""));
	    }
	if (Buff[0])
	    AddDevDesc(DevInfo, Buff, "Emulates", DA_APPEND);
    }
#endif	/* FB_ATTR_NEMUTYPES */

    /*
     * Put things together
     */
    DevInfo->Name 			= FBname;
    DevInfo->Type 			= DT_FRAMEBUFFER;
    DevInfo->NodeID			= DevData->NodeID;
    DevInfo->DevSpec	 		= (void *) fb;

    if (FBgattr) {
	MonHres = fb->Height		= FBgattr->fbtype.fb_height;
	MonVres = fb->Width 		= FBgattr->fbtype.fb_width;
	fb->Depth 			= FBgattr->fbtype.fb_depth;
	fb->Size 			= FBgattr->fbtype.fb_size;
	fb->CMSize 			= FBgattr->fbtype.fb_cmsize;
    }
#if	defined(HAVE_OPENPROM)
    else {
	if (cp = OBPfindPropVal("awidth", NULL, DevData->NodeID, NULL))
	    MonVres = atoi(cp);
	if (cp = OBPfindPropVal("height", NULL, DevData->NodeID, NULL))
	    MonHres = atoi(cp);
    }
    /*
     * Obtain the refresh frequency for the monitor from OBP.
     */
    if ((cp = OBPfindPropVal("vfreq", NULL, DevData->NodeID, NULL)) ||
	(cp = OBPfindPropVal("v_freq", NULL, DevData->NodeID, NULL)))
	MonFreq = atoi(cp);
#endif	/* HAVE_OPENPROM */

    /*
     * Create a handy resolution entry.
     */
    if (MonVres && MonHres) {
	(void) snprintf(Buff, sizeof(Buff), "%dx%d", MonVres, MonHres);
	if (MonFreq)
	    (void) snprintf(&Buff[l=strlen(Buff)], 
			    sizeof(Buff)-l, "@%d", MonFreq);
	AddDevDesc(DevInfo, Buff, "Current Resolution", DA_INSERT);
    }

    DevInfo->Master 			= MkMasterFromDevData(DevData);

    return(ProbeData->RetDevInfo = DevInfo);
}
