/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Common SysInfo SCSI functions
 */

#include "defs.h"
#include "myscsi.h"

static char *CDLoadTypes[] 	= CD_LOAD_TYPES;
#define MAX_CD_LOAD_TYPES 	sizeof(CDLoadTypes)/sizeof(char *)

/*
 * Decode a SCSI CAPACITY command and add info to DevInfo as we can.
 */
extern int ScsiCapacityDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiCapacity_t	       *Capacity;
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Capacity = (ScsiCapacity_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI CAPACITY: Blocks=<%ld> BlockSize=<%d>",
	  DevInfo->Name, ntohl(Capacity->Blocks), 
	  ntohl(Capacity->BlkSize));

    if (!(Disk = DiskSetup(DevInfo, "ScsiCapacityDecode")))
	return(-1);

    /*
     * Set what we know
     */
    Disk->Size = nsect_to_mbytes( ntohl(Capacity->Blocks),
				  ntohl(Capacity->BlkSize) );
    Disk->SecSize = ntohl(Capacity->BlkSize);

    return(0);
}

/*
 * Decode results from SCSI_INQUIRY_IDENT
 * An Identifier Descriptor is suppose to uniquely identify the device
 * with the purpose of allowing for the detection of multi-pathed devices.
 *
 * This is a SCSI-3 feature which many SCSI devices do not yet implement.
 */
extern int ScsiIdentDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiIdent_t		       *ScsiIdent;
    Ident_t		       *Ident;
    DevInfo_t		       *DevInfo;

    if (!Query || !Query->Data || !Query->DevInfo) {
	SImsg(SIM_DBG, "ScsiIdentDecode: Bad parameters");
	return(-1);
    }

    ScsiIdent = (ScsiIdent_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI IDENT: PageCode=0x%02x CodeSet=0x%x Type=%d IdLen=%d",
	  DevInfo->Name,
	  ScsiIdent->Hdr.PageCode, 
	  ScsiIdent->Descriptors[0].CodeSet, 
	  ScsiIdent->Descriptors[0].IdentType,
	  ScsiIdent->Descriptors[0].IdentLength	  );
    if (ScsiIdent->Descriptors[0].IdentLength > 0)
	SImsg(SIM_DBG, 
	      "\t%s: SCSI IDENT: ID=<%.*s>",
	      DevInfo->Name,
	      ScsiIdent->Descriptors[0].IdentLength,
	      ScsiIdent->Descriptors[0].Identifier);

    /*
     * We only look at the 1st descriptor.
     * Officially this data is only valid if:
     *
     *		PageCode==0x83	(SCSI_INQUIRY_IDENT)
     *		CodeSet > 0 && CodeSet << 0xF
     *		IdentType > 0 && IdentType << 0xF
     *
     * Many older (pre SCSI-3) devices will not understand the
     * Cdb.EVPD and Cdb.Addr1=SCSI_INQUIRY_IDENT bits and will
     * return normal INQUIRY data.
     */
#define InRange(v)	( (v) > 0 && (v) < 0xF )
    if (ScsiIdent->Hdr.PageCode == SCSI_INQUIRY_IDENT &&
	InRange(ScsiIdent->Descriptors[0].CodeSet) &&
	InRange(ScsiIdent->Descriptors[0].IdentType) &&
	ScsiIdent->Descriptors[0].IdentLength > 0) {

	Ident = IdentCreate(NULL);
	Ident->Length = ScsiIdent->Descriptors[0].IdentLength;
	if (ScsiIdent->Descriptors[0].CodeSet == IDCS_ASCII)
	    Ident->Type = IDT_ASCII;
	else if (ScsiIdent->Descriptors[0].CodeSet == IDCS_BINARY)
	    Ident->Type = IDT_BINARY;
	/* Length + CNULL */
	Ident->Identifier = (u_char *) xmalloc(Ident->Length + 1);

	(void) memcpy(Ident->Identifier, ScsiIdent->Descriptors[0].Identifier,
		      Ident->Length);

	/* Terminate with NULL regardless of type */
	Ident->Identifier[Ident->Length] = CNULL;

	DevInfo->Ident = Ident;
    } else {
	SImsg(SIM_DBG, "\t%s: SCSI IDENT: Information Invalid - ignored",
	      DevInfo->Name);
    }

#undef	InRange

    return 0;
}

/*
 * Decode results from SCSI_INQUIRY_SERIAL
 * This value represents the product's serial number as assigned by the
 * vendor.
 *
 * This is a SCSI-3 feature which many SCSI devices do not yet implement.
 */
extern int ScsiSerialDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiSerialNum_t	       *Serial;
    DevInfo_t		       *DevInfo;
    char		       *cp;

    if (!Query || !Query->Data || !Query->DevInfo) {
	SImsg(SIM_DBG, "ScsiSerialDecode: Bad parameters");
	return(-1);
    }

    Serial = (ScsiSerialNum_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI SERIAL: PageCode=0x%02x Length=%d Serial=<%.*s>",
	  DevInfo->Name,
	  Serial->Hdr.PageCode, 
	  Serial->Hdr.Length, 
	  Serial->Hdr.Length, 
	  Serial->Serial);

    /*
     * We only look at the 1st descriptor.
     * Officially this data is only valid if:
     *
     *		PageCode==0x83	(SCSI_INQUIRY_SERIAL)
     *
     * Many older (pre SCSI-3) devices will not understand the
     * Cdb.EVPD and Cdb.Addr1=SCSI_INQUIRY_SERIAL bits and will
     * return normal INQUIRY data.
     */
    if (Serial->Hdr.PageCode == SCSI_INQUIRY_SERIAL &&
	Serial->Hdr.Length > 0) {
	if (cp = CleanString(Serial->Serial, 
			     sizeof(Serial->Hdr.Length), 
			     CSF_ENDNONPR|CSF_ALPHANUM)) {
	    /* Serial must be at least 6 chars long to be valid */
	    if (strlen(cp) >= 6)
		DevInfo->Serial = cp;
	    else
		SImsg(SIM_DBG, "%s: Serial value <%s> too short.  Ignored.",
		      DevInfo->Name, cp);
	
	}
    } else {
	SImsg(SIM_DBG, "\t%s: SCSI SERIAL: Information Invalid - ignored",
	      DevInfo->Name);
    }

    return 0;
}

/*
 * Decode a SCSI INQUIRY command and add info to DevInfo as we can.
 */
extern int ScsiInquiryDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiInquiry_t	       *Inq;
    DevInfo_t		       *DevInfo;
    Define_t		       *Def;
    char		       *DevType = NULL;
    char		       *cp;

    if (!Query || !Query->Data || !Query->DevInfo) {
	SImsg(SIM_DBG, "ScsiInquiryDecode: Bad parameters");
	return(-1);
    }

    Inq = (ScsiInquiry_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI INQUIRY: vendor=<%.*s> product=<%.*s> type=0x%02x",
	  DevInfo->Name,
	  sizeof(Inq->VendorID), Inq->VendorID, 
	  sizeof(Inq->ProductID), Inq->ProductID, Inq->DevType);
    SImsg(SIM_DBG, 
	  "\t%s: SCSI INQUIRY:    rev=<%.*s> serial=<%.*s> vendorspec=<%.*s>",
	  DevInfo->Name,
	  sizeof(Inq->Revision), Inq->Revision, 
	  sizeof(Inq->Serial), Inq->Serial,
	  sizeof(Inq->_vendor3), Inq->_vendor3
	  );

    /*
     * This is implied by this function being called
     */
    DevInfo->ClassType = CT_SCSI;

    /*
     * Decode results
     */
#define IsOk(s)	( s && s[0] && s[0] != ' ' && isalnum(s[0]) )
    if (IsOk(Inq->VendorID))
	DevInfo->Vendor = CleanString(Inq->VendorID, sizeof(Inq->VendorID), 0);
    if (IsOk(Inq->ProductID))
	DevInfo->Model = CleanString(Inq->ProductID, sizeof(Inq->ProductID),0);
    if (IsOk(Inq->Revision))
	DevInfo->Revision = CleanString(Inq->Revision, 
					sizeof(Inq->Revision), 0);
    if (IsOk(Inq->Serial))
	if (cp = CleanString(Inq->Serial, 
			     sizeof(Inq->Serial), 
			     CSF_ENDNONPR|CSF_ALPHANUM)) {
	    /* Serial must be at least 6 chars long to be valid */
	    if (strlen(cp) >= 6)
		DevInfo->Serial = cp;
	    else
		SImsg(SIM_DBG, "%s: Serial value <%s> too short.  Ignored.",
		      DevInfo->Name, cp);
	}
#undef IsOk
    /*
     * What SCSI version are we?
     */
    cp = NULL;
    switch ((int)Inq->RDF) {
    case RDF_LEVEL0:	cp = "SCSI-1";	break;
    case RDF_SCSI2:	cp = "SCSI-2";	break;
    case RDF_CCS:	cp = "CCS";	break;
    }
    if (cp)
	AddDevDesc(DevInfo, cp, "SCSI Version/Protocol", DA_APPEND);
    else
	SImsg(SIM_UNKN, "Unknown RDF/SCSI Version 0x%x", 
	      Inq->RDF);

    /*
     * ANSI/ECMA/ISO version info
     */
    cp = NULL;
    Def = DefGet(DL_SCSI_ANSI, (char *) NULL, (long) Inq->ANSI, 0);
    if (Def && Def->ValStr2)
	cp = Def->ValStr2;
    else {
	SImsg(SIM_DBG, "%s: Unknown %s=0x%02x", 
	      DevInfo->Name, DL_SCSI_ANSI, Inq->ANSI);
	cp = itoa(Inq->ANSI);
    }
    AddDevDesc(DevInfo, cp, "ANSI Version", DA_APPEND);
    AddDevDesc(DevInfo, itoa(Inq->ECMA), "ECMA Version", DA_APPEND);
    AddDevDesc(DevInfo, itoa(Inq->ISO), "ISO Version", DA_APPEND);

    /*
     * Check all the flag bits
     */
    if (Inq->WBus16)
	AddDevDesc(DevInfo, "Wide SCSI: 16-bit Data Transfers", 
		   "Supports", DA_APPEND);
    if (Inq->WBus32)
	AddDevDesc(DevInfo, "Wide SCSI: 32-bit Data Transfers", 
		   "Supports", DA_APPEND);
    if (!Inq->WBus16 && !Inq->WBus32)
	AddDevDesc(DevInfo, "Narrow SCSI: 8-bit Data Transfers", 
		   "Supports", DA_APPEND);
    if (Inq->Addr16)
	AddDevDesc(DevInfo, "Wide SCSI: 16-bit Addressing", 
		   "Supports", DA_APPEND);
    if (Inq->Addr32)
	AddDevDesc(DevInfo, "Wide SCSI: 32-bit Addressing", 
		   "Supports", DA_APPEND);
    if (!Inq->Addr16 && !Inq->Addr32)
	AddDevDesc(DevInfo, "Narrow SCSI: 8-bit Addressing", 
		   "Supports", DA_APPEND);

    if (Inq->ACKQREQQ)
	AddDevDesc(DevInfo, "Data Transfer on Secondary Bus", 
		   "Supports", DA_APPEND);

    if (Inq->Sync)
	AddDevDesc(DevInfo, "Syncronous Data Transfers", "Supports", 
		   DA_APPEND);

    if (Inq->Removable)
	AddDevDesc(DevInfo, "Removable Media", "Supports", DA_APPEND);

    if (Inq->NormACA)
	AddDevDesc(DevInfo, "Normal ACA (NACA)", "Supports", DA_APPEND);
    if (Inq->TrmTsk)
	AddDevDesc(DevInfo, "TERMINATE TASK", "Supports", DA_APPEND);
    if (Inq->AERC)
	AddDevDesc(DevInfo, "Async Event Notification", "Supports", DA_APPEND);
    if (Inq->EncServ)
	AddDevDesc(DevInfo, "Enclosure Services", "Supports", DA_APPEND);
    if (Inq->MultiPort)
	AddDevDesc(DevInfo, "Multi-Ported Device", "Supports", DA_APPEND);
    if (Inq->MChngr)
	AddDevDesc(DevInfo, "Embedded/attached to medium changer", 
		   "Supports", DA_APPEND);
    if (Inq->TranDis)
	AddDevDesc(DevInfo, "Transfer Disable Messages", "Supports", 
		   DA_APPEND);
    if (Inq->CmdQue)
	AddDevDesc(DevInfo, "Command Queuing", "Supports", DA_APPEND);
    if (Inq->Linked)
	AddDevDesc(DevInfo, "Linked Commands", "Supports", DA_APPEND);
    if (Inq->RelAddr)
	AddDevDesc(DevInfo, "Relative Addressing", "Supports", DA_APPEND);

    /*
     * Look up device type
     */
    Def = DefGet(DL_SCSI_DTYPE, (char *) NULL, (long) Inq->DevType, 0);
    if (Def) {
	/*
	 * Set Device Type 
	 */
	if (Def->ValStr1) {
	    DevType_t		*dtPtr;

	    dtPtr = TypeGetByName(Def->ValStr1);
	    if (dtPtr)
		DevInfo->Type = dtPtr->Type;
	    else
		SImsg(SIM_DBG, "%s: SCSIdtype DevType=<%s> is unknown.",
		      DevInfo->Name, Def->ValStr1);
	}
    } else
	SImsg(SIM_DBG, "%s: Unknown SCSIdtype=0x%02x", 
	      DevInfo->Name, Inq->DevType);

    return(0);
}
     
/*
 * Get SCSI Device Identification
 */
extern int ScsiQueryIdent(Query)
     ScsiQuery_t	       *Query;
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiQueryIdent: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_INQUIRY;
    Cdb.EVPD = 1;		/* Special - Enable Vital Product Data */
    Cdb.Addr1 = SCSI_INQUIRY_IDENT;
    Cdb.Length = (Word_t) sizeof(ScsiIdent_t);

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Query->Data = Cmd.Data;
	return ScsiIdentDecode(Query);
    } else
	return(-1);
}

/*
 * Get SCSI Device Serial Number
 */
extern int ScsiQuerySerial(Query)
     ScsiQuery_t	       *Query;
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiQuerySerial: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_INQUIRY;
    Cdb.EVPD = 1;		/* Special - Enable Vital Product Data */
    Cdb.Addr1 = SCSI_INQUIRY_SERIAL;
    Cdb.Length = (Word_t) sizeof(ScsiIdent_t);

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Query->Data = Cmd.Data;
	return ScsiSerialDecode(Query);
    } else
	return(-1);
}

/*
 * Get description info about SCSI devices by sending a SCSI INQUIRY
 * to the device.
 */
extern int ScsiQueryInquiry(Query)
     ScsiQuery_t	       *Query;
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiInquiry: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_INQUIRY;
    Cdb.Length = sizeof(ScsiInquiry_t);

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Query->Data = Cmd.Data;
	return ScsiInquiryDecode(Query);
    } else
	return(-1);
}

/*
 * Get description info about SCSI devices by sending a SCSI CAPACITY
 * to the device.
 */
extern int ScsiQueryCapacity(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG1_t		Cdb;
    static ScsiCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiCapacity: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_CAPACITY;

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Query->Data = Cmd.Data;
	return ScsiCapacityDecode(Query);
    } else
	return(-1);
}

/*
 * Decode a SCSI GEOMETRY command and add info to DevInfo as we can.
 */
extern int ScsiGeometryDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiGeometry_t	       *Geometry;
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;
    u_int			PhyCyl;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Geometry = (ScsiGeometry_t *) Query->Data;
    DevInfo = Query->DevInfo;
    PhyCyl =  word3_to_uint(Geometry->NumCyls);

    SImsg(SIM_DBG, 
	  "\t%s: SCSI GEOMETRY: #tracks=<%d> PhyCyl=<%d> RPM=<%d>",
	  DevInfo->Name, Geometry->Heads, PhyCyl, ntohs(Geometry->RPM));

    if (!(Disk = DiskSetup(DevInfo, "ScsiGeometryDecode")))
	return(-1);

    Disk->Tracks = Geometry->Heads;
    Disk->PhyCyl = PhyCyl;
    Disk->RPM = ntohs(Geometry->RPM);

    return(0);
}

/*
 * Decode a SCSI FORMAT command and add info to DevInfo as we can.
 */
extern int ScsiFormatDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiFormat_t	       *Format;
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Format = (ScsiFormat_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI FORMAT: #sect=<%d> secsize=<%d>",
	  DevInfo->Name, ntohs(Format->SectTrack), 
	  ntohs(Format->DataBytesSect));

    if (!(Disk = DiskSetup(DevInfo, "ScsiFormatDecode")))
	return(-1);

    Disk->Sect = ntohs(Format->SectTrack);
    Disk->SecSize = ntohs(Format->DataBytesSect);
    Disk->Tracks = ntohs(Format->TracksPerZone);
    Disk->AltSectPerZone = ntohs(Format->AltSectZone);
    Disk->AltTracksPerVol = ntohs(Format->AltTracksVol);
    Disk->AltTracksPerZone = ntohs(Format->AltTracksZone);
    Disk->IntrLv = ntohs(Format->Interleave);
    Disk->TrackSkew = ntohs(Format->TrackSkew);
    Disk->CylSkew = ntohs(Format->CylinderSkew);

    return(0);
}

/*
 * Get description info about SCSI devices by sending a SCSI FORMAT
 * to the device.
 */
extern int ScsiQueryFormat(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;
    static ScsiFormat_t		Format;
    ScsiModeHeader_t	       *Hdr;
    char		       *LocPtr;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiFormat: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_MODE_SENSE;
    Cdb.Addr1 = SCSI_DAD_MODE_FORMAT;
    Cdb.Length = 255;	/* Yuck.  It's hardcoded */

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Hdr = (ScsiModeHeader_t *) Cmd.Data;
	LocPtr = SCSI_MODE_PAGE_ADDR(Hdr, char);
	(void) memset(&Format, 0, sizeof(Format));
	(void) memcpy(&Format, LocPtr, sizeof(ScsiModePage_t));
	(void) memcpy(&Format, LocPtr, Format.ModePage.Length);
	Query->Data = (void *) &Format;
	return ScsiFormatDecode(Query);
    } else
	return(-1);
}

/*
 * Get description info about SCSI devices by sending a SCSI GEOMETRY
 * to the device.
 */
extern int ScsiQueryGeometry(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;
    static ScsiGeometry_t	Geometry;
    ScsiModeHeader_t	       *Hdr;
    char		       *LocPtr;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiGeometry: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_MODE_SENSE;
    Cdb.Addr1 = SCSI_DAD_MODE_GEOMETRY;
    Cdb.Length = 255;	/* Yuck.  It's hardcoded */

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Hdr = (ScsiModeHeader_t *) Cmd.Data;
	LocPtr = SCSI_MODE_PAGE_ADDR(Hdr, char);
	(void) memset(&Geometry, 0, sizeof(Geometry));
	(void) memcpy(&Geometry, LocPtr, sizeof(ScsiModePage_t));
	(void) memcpy(&Geometry, LocPtr, Geometry.ModePage.Length);
	Query->Data = (void *) &Geometry;
	return ScsiGeometryDecode(Query);
    } else
	return(-1);
}

/*
 * Decode a SCSI CD Capability command and add info to DevInfo as we can.
 */
extern int ScsiCDCapDecode(Query)
    ScsiQuery_t		       *Query;
{
    static char			Buff[64];
    ScsiCDCap_t		       *Cap;
    DevInfo_t		       *DevInfo;
    Desc_t		      **d;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Cap = (ScsiCDCap_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI CD CAP: speed=<%d>",
	  DevInfo->Name, ntohs(Cap->MaxReadSpeed));

    if (Cap->DVD_ROM_Read || Cap->DVD_R_Read || Cap->DVD_RAM_Read) {
	/* If we can read DVD, then let's set our type now */
	DevInfo->Type = DT_DVD;
	/*
	 * TSchmidt: Need to define something here to change
	 * "Device Type: disk drive" to "Device Type: DVD drive"
	 */
    }

    d = &DevInfo->DescList;

    if (Cap->MaxReadSpeed && ntohs(Cap->MaxReadSpeed)) {
	/*
	 * TSchmidt: If this is a DVD, it should report the DVD read speed
	 * in the description instead of the CD-ROM read speed
	 */
	/* 176 KB is the original (1x) CD-ROM speed */
	(void) snprintf(Buff, sizeof(Buff), "%dX", 
			ntohs(Cap->MaxReadSpeed) / 176);
	DevInfo->ModelDesc = strdup(Buff);
	/*
	 * TSchmidt: If this is a DVD, it should report the DVD read speed
	 * in addition to the CD-ROM read speed
	 */
	AddDesc(d, "CD Speed", DevInfo->ModelDesc, 0);
    }

    /*
     * Media Support
     */
    /* Assume everything can read CD-ROM */
    AddDesc(d, "Reads Media", "CD-ROM", 0);
    if (Cap->CD_R_Read) AddDesc(d, "Reads Media", "CD-R", 0);
    if (Cap->CD_RW_Read) AddDesc(d, "Reads Media", "CD-RW", 0);
    /* Assume all DVD drives can read DVD-ROM */
    if (DevInfo->Type == DT_DVD) AddDesc(d, "Reads Media", "DVD-ROM", 0);
    if (Cap->DVD_R_Read) AddDesc(d, "Reads Media", "DVD-R", 0);
    if (Cap->DVD_RAM_Read) AddDesc(d, "Reads Media", "DVD-RAM", 0);
    if (Cap->Mode2Form1) AddDesc(d, "Reads Media", "Mode-2 Form 1 (XA)", 0);
    if (Cap->Mode2Form2) AddDesc(d, "Reads Media", "Mode-2 Form 2", 0);
    if (Cap->MultiSession) AddDesc(d, "Reads Media", "Multi-session", 0);
    if (Cap->CD_DA_Supported) AddDesc(d, "Reads Media", "Digital Audio", 0);

    if (Cap->CD_R_Write) AddDesc(d, "Writes Media", "CD-R", 0);
    if (Cap->CD_RW_Write) AddDesc(d, "Writes Media", "CD-RW", 0);
    if (Cap->DVD_R_Write) AddDesc(d, "Writes Media", "DVD-R", 0);
    if (Cap->DVD_RAM_Write) AddDesc(d, "Writes Media", "DVD-RAM", 0);

    /*
     * Misc
     */
    if (Cap->TestWrite) AddDesc(d, "Supports", "Emulation/Test Write", 0);
    if (Cap->AudioPlay) AddDesc(d, "Supports", "Audio Play", 0);
    if (Cap->Composite) AddDesc(d, "Supports", "Composite A/V stream", 0);
    if (Cap->DigitalPort1) 
        AddDesc(d, "Supports", "Digital Output on Port 1", 0);
    if (Cap->DigitalPort2) 
        AddDesc(d, "Supports", "Digital Output on Port 2", 0);
    if (Cap->CD_DA_Accurate) 
        AddDesc(d, "Supports", "Digital Audio Stream Accurate", 0);
    if (Cap->RW_Supported) 
        AddDesc(d, "Supports", "R-W Sub Channel Info", 0);
    if (Cap->RW_DeintCorr) 
        AddDesc(d, "Supports", "R-W de-interleaved Sub Channel", 0);
    if (Cap->C2_Pointers) AddDesc(d, "Supports", "C2 Error Pointers", 0);
    if (Cap->ISRC) AddDesc(d, "Supports", "ISRC information", 0);
    if (Cap->UPC) AddDesc(d, "Supports", "UPC Catalog Number", 0);
    if (Cap->ReadBarCode) AddDesc(d, "Supports", "Read Bar Codes", 0);

    if (Cap->LoadingType) {
	if (Cap->LoadingType < MAX_CD_LOAD_TYPES)
	    AddDesc(d, "Loading Type", CDLoadTypes[Cap->LoadingType], 0);
	else
	    AddDesc(d, "Loading Type", itoa(Cap->LoadingType), 0);
    }

    if (Cap->SepChanVol) 
	AddDesc(d, "Supports", "Volume controls each channel seperately", 0);
    if (Cap->SepChanMute) 
	AddDesc(d, "Supports", "Mute controls each channel seperately", 0);
    if (Cap->DiskPresentRep) 
	AddDesc(d, "Supports", "Changer disk present rep", 0);

    if (Cap->NumVolLevels)
	AddDesc(d, "Number Volume Levels", 
		itoa(ntohs(Cap->NumVolLevels)), 0);

    if (Cap->BufferSize)
	AddDesc(d, "Buffer Size", 
		GetSizeStr(ntohs(Cap->BufferSize), KBYTES), 0);

    if (Cap->MaxReadSpeed) 
	AddDesc(d, "Max Read Speed per sec", 
		GetSizeStr(ntohs(Cap->MaxReadSpeed), KBYTES), 0);
    if (Cap->CurReadSpeed) 
	AddDesc(d, "Current Read Speed per sec", 
		GetSizeStr(ntohs(Cap->CurReadSpeed), KBYTES), 0);
    if (Cap->MaxWriteSpeed) 
	AddDesc(d, "Max Write Speed per sec", 
		GetSizeStr(ntohs(Cap->MaxWriteSpeed), KBYTES), 0);
    if (Cap->CurWriteSpeed) 
	AddDesc(d, "Current Write Speed per sec", 
		GetSizeStr(ntohs(Cap->CurWriteSpeed), KBYTES), 0);

    return(0);
}

/*
 * Decode a SCSI Data Compression command and add info to DevInfo as we can.
 */
extern int ScsiDataCompDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiDataComp_t	       *Comp;
    DevInfo_t		       *DevInfo;
    Define_t		       *Def;
    u_int			CompAlg = 0;
    u_int			DeCompAlg = 0;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Comp = (ScsiDataComp_t *) Query->Data;
    DevInfo = Query->DevInfo;
    CompAlg = ntohl(Comp->CompAlg);
    DeCompAlg = ntohl(Comp->DecompAlg);

    SImsg(SIM_DBG, 
	  "\t%s: SCSI DATA COMP: Comp Algo=<0x%02x> Decomp Algo=<0x%02x>",
	  DevInfo->Name, 
	  CompAlg, DeCompAlg);

    /*
     * Look up algorithm type
     */
    Def = DefGet(DL_SCSI_COMP_ALG, (char *) NULL, (long) CompAlg, 0);
    if (Def && Def->ValStr2)
	AddDesc(&DevInfo->DescList, "Compression Algorithm", Def->ValStr2, 0);
    else
	SImsg(SIM_DBG, "%s: Unknown %s=0x%02x", 
	      DevInfo->Name, DL_SCSI_COMP_ALG, CompAlg);

    Def = DefGet(DL_SCSI_COMP_ALG, (char *) NULL, (long) DeCompAlg, 0);
    if (Def && Def->ValStr2)
	AddDesc(&DevInfo->DescList, "Decompression Algorithm", 
		Def->ValStr2, 0);
    else
	SImsg(SIM_DBG, "%s: Unknown %s=0x%02x", 
	      DevInfo->Name, DL_SCSI_COMP_ALG, DeCompAlg);

    return(0);
}

/*
 * Get the CD Capabilities of a SCSI device.
 * This only works on CD/DVD based devices
 */
extern int ScsiQueryCDCap(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;
    static ScsiCDCap_t		Cap;
    ScsiModeHeader_t	       *Hdr;
    char		       *LocPtr;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiQueryCDCap: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_MODE_SENSE;
    Cdb.Addr1 = SCSI_MODE_CD_CAP;
    Cdb.Length = 255;	/* Yuck.  It's hardcoded */

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Hdr = (ScsiModeHeader_t *) Cmd.Data;
	LocPtr = SCSI_MODE_PAGE_ADDR(Hdr, char);
	(void) memset(&Cap, 0, sizeof(Cap));
	(void) memcpy(&Cap, LocPtr, sizeof(ScsiModePage_t));
	(void) memcpy(&Cap, LocPtr, Cap.ModePage.Length);
	Query->Data = (void *) &Cap;
	return ScsiCDCapDecode(Query);
    } else
	return(-1);
}

/*
 * Get the Data Compression info for a Sequential Access device.
 */
extern int ScsiQueryDataComp(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;
    static ScsiDataComp_t	Comp;
    ScsiModeHeader_t	       *Hdr;
    char		       *LocPtr;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiQueryCDCap: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = SCSI_MODE_SENSE;
    Cdb.Addr1 = SCSI_SAD_MODE_DATA_COMP;
    Cdb.Length = 255;	/* Yuck.  It's hardcoded */

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CdbLen = sizeof(Cdb);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (ScsiCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Hdr = (ScsiModeHeader_t *) Cmd.Data;
	LocPtr = SCSI_MODE_PAGE_ADDR(Hdr, char);
	(void) memset(&Comp, 0, sizeof(Comp));
	(void) memcpy(&Comp, LocPtr, sizeof(ScsiModePage_t));
	(void) memcpy(&Comp, LocPtr, Comp.ModePage.Length);
	Query->Data = (void *) &Comp;
	return ScsiDataCompDecode(Query);
    } else
	return(-1);
}

/*
 * Query a SCSI device
 */
extern int ScsiQuery(DevInfo, DevFile, DevFD, OverRide)
    DevInfo_t		       *DevInfo;
    char		       *DevFile;
    int				DevFD;
    int				OverRide;
{
    int				Status = 0;
    int				Type = -1;
    static ScsiQuery_t		Query;
    ScsiInquiry_t	       *Inq;
    int				fd = -1;

    if (!DevInfo || !DevFile) {
	SImsg(SIM_DBG, "ScsiQuery: Bad parameters.");
	return(-1);
    }

    if (DevFD < 0) {
	/*
	 * We use O_RDWR because many OS's require Write perms
	 */
	DevFD = fd = open(DevFile, O_RDWR|O_NDELAY|O_NONBLOCK);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: ScsiQuery: open for read failed: %s",
		  DevFile, SYSERR);
	    return(-1);
	}
    }

    (void) memset(&Query, 0, sizeof(Query));
    Query.DevInfo = DevInfo;
    Query.DevFile = DevFile;
    Query.DevFD = DevFD;
    Query.OverRide = OverRide;

    /* Insure DevInfo->Name is valid */
    if (!Query.DevInfo->Name)
	Query.DevInfo->Name = DevFile;

    /*
     * Do a general inquiry on the device.
     * After this we use the Device Type for further commands.
     */
    if (ScsiQueryInquiry(&Query) == 0) {
	++Status;
	Inq = (ScsiInquiry_t *) Query.Data;
	Type = Inq->DevType;
    }

    if (ScsiQueryIdent(&Query) == 0)
	++Status;
    if (ScsiQuerySerial(&Query) == 0)
	++Status;

    /*
     * Perform device type specific queries.
     */
    if (Type == SCSI_DTYPE_DAD) {
	/* Direct Access Devices (disk drives, etc.) */
	if (ScsiQueryCapacity(&Query) == 0)
	    ++Status;
	if (ScsiQueryFormat(&Query) == 0)
	    ++Status;
	if (ScsiQueryGeometry(&Query) == 0)
	    ++Status;
    }
    if (Type == SCSI_DTYPE_CDROM) {
	/* CD/DVD */
	if (ScsiQueryCDCap(&Query) == 0)
	    ++Status;
    }
    if (Type == SCSI_DTYPE_SEQ) {
	/* Sequential Access devices (tape drives, etc.) */
	if (ScsiQueryDataComp(&Query) == 0)
	    ++Status;
    }

    /*
     * If we opened the descriptor, then close it now
     */
    if (fd >= 0)
	(void) close(fd);

    /*
     * If at least 1 function succeeded, we return success.
     */
    if (Status > 0)
	return 0;
    else
	return -1;
}
