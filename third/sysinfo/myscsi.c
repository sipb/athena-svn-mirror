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
 * Common SysInfo SCSI functions
 */

#include "defs.h"
#include "myscsi.h"
#include <netinet/in.h>		/* For ntohs() etc. */

/*
 * Do setup for Scsi*Decode() funcs which need a DiskDrive_t to work on
 */
static DiskDrive_t *DiskSetup(DevInfo, What)
     DevInfo_t		       *DevInfo;
     char		       *What;
{
    DiskDriveData_t	       *DiskDriveData;

    if (!DevInfo || !What)
	return((DiskDrive_t *) NULL);

    /*
     * We may want to add other DevType's as we discover them, but
     * right now we only know how to deal with DiskDrive's.
     */
    if (DevInfo->Type != DT_DISKDRIVE) {
	SImsg(SIM_DBG, "%s: %s unsupported for DevType %d",
	      DevInfo->Name, What, DevInfo->Type);
	return((DiskDrive_t *) NULL);
    }

    /*
     * Find or create the DiskDriveData and DiskDrive we need
     */
    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }
    if (!DiskDriveData->HWdata)
	DiskDriveData->HWdata = NewDiskDrive(NULL);

    return(DiskDriveData->HWdata);
}

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
	  "\t%s: SCSI CAPACITY: Blocks=<%d> BlockSize=<%d>",
	  DevInfo->Name, ntohl(Capacity->Blocks), ntohl(Capacity->BlkSize));

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
	  "\t%s: SCSI INQUIRY: vendor=<%.*s> product=<%.*s>",
	  DevInfo->Name,
	  sizeof(Inq->inq_vid), Inq->inq_vid, 
	  sizeof(Inq->inq_pid), Inq->inq_pid);
    SImsg(SIM_DBG, 
	  "\t%s: SCSI INQUIRY:    rev=<%.*s> serial=<%.*s>",
	  DevInfo->Name,
	  sizeof(Inq->inq_revision), Inq->inq_revision, 
	  sizeof(Inq->inq_serial), Inq->inq_serial);

    /*
     * This is implied by this function being called
     */
    DevInfo->ClassType = CT_SCSI;

    /*
     * Decode results
     */
#define IsOk(s)	( s && s[0] && s[0] != ' ' && isalnum(s[0]) )
    if (IsOk(Inq->inq_vid))
	DevInfo->Vendor = CleanString(Inq->inq_vid, sizeof(Inq->inq_vid), 0);
    if (IsOk(Inq->inq_pid))
	DevInfo->Model = CleanString(Inq->inq_pid, sizeof(Inq->inq_pid), 0);
    if (IsOk(Inq->inq_revision))
	DevInfo->Revision = CleanString(Inq->inq_revision, 
					sizeof(Inq->inq_revision), 0);
    if (IsOk(Inq->inq_serial))
	DevInfo->Serial = CleanString(Inq->inq_serial, 
				      sizeof(Inq->inq_serial), 0);
#undef IsOk
    /*
     * What SCSI version are we?
     */
    cp = NULL;
    switch ((int)Inq->inq_rdf) {
    case RDF_LEVEL0:	cp = "SCSI-1";	break;
    case RDF_SCSI2:	cp = "SCSI-2";	break;
    case RDF_CCS:	cp = "CCS";	break;
    }
    if (cp)
	AddDevDesc(DevInfo, cp, "SCSI Version/Protocol", DA_APPEND);
    else
	SImsg(SIM_UNKN, "Unknown RDF/SCSI Version 0x%x", 
	      Inq->inq_rdf);

    /*
     * ANSI/ECMA/ISO version info
     */
    AddDevDesc(DevInfo, itoa(Inq->inq_ansi), "ANSI Version", DA_APPEND);
    AddDevDesc(DevInfo, itoa(Inq->inq_ecma), "ECMA Version", DA_APPEND);
    AddDevDesc(DevInfo, itoa(Inq->inq_iso), "ISO Version", DA_APPEND);

    /*
     * Check all the flag bits
     */
    if (Inq->inq_rmb)
	AddDevDesc(DevInfo, "Removable Media", "Has", DA_APPEND);
    if (Inq->inq_normaca)
	AddDevDesc(DevInfo, "Setting NACA bit", "Has", DA_APPEND);
    if (Inq->inq_trmiop)
	AddDevDesc(DevInfo, "TERMINATE I/O PROC msg", "Has", DA_APPEND);
    if (Inq->inq_aenc)
	AddDevDesc(DevInfo, "Async Event Notification", "Has", DA_APPEND);
    if (Inq->inq_wbus16)
	AddDevDesc(DevInfo, "Wide SCSI: 16-bit Data Transfers", 
		   "Has", DA_APPEND);
    if (Inq->inq_wbus32)
	AddDevDesc(DevInfo, "Wide SCSI: 32-bit Data Transfers", 
		   "Has", DA_APPEND);
    if (Inq->inq_addr16)
	AddDevDesc(DevInfo, "Wide SCSI: 16-bit Addressing", 
		   "Has", DA_APPEND);
    if (Inq->inq_addr32)
	AddDevDesc(DevInfo, "Wide SCSI: 32-bit Addressing", 
		   "Has", DA_APPEND);
    if (Inq->inq_ackqreqq)
	AddDevDesc(DevInfo, "Data Transfer on Q Cable", "Has", DA_APPEND);
    if (Inq->inq_mchngr)
	AddDevDesc(DevInfo, "Embedded/attached to medium changer", 
		   "Has", DA_APPEND);
    if (Inq->inq_dualp)
	AddDevDesc(DevInfo, "Dual Port Device", "Has", DA_APPEND);
    if (Inq->inq_trandis)
	AddDevDesc(DevInfo, "Transfer Disable Messages", "Has", DA_APPEND);
    if (Inq->inq_sftre)
	AddDevDesc(DevInfo, "Soft Reset option", "Has", DA_APPEND);
    if (Inq->inq_cmdque)
	AddDevDesc(DevInfo, "Command Queuing", "Has", DA_APPEND);
    if (Inq->inq_linked)
	AddDevDesc(DevInfo, "Linked Commands", "Has", DA_APPEND);
    if (Inq->inq_sync)
	AddDevDesc(DevInfo, "Syncronous Data Transfers", "Has", DA_APPEND);
    if (Inq->inq_reladdr)
	AddDevDesc(DevInfo, "Relative Addressing", "Has", DA_APPEND);

    /*
     * Look up device type
     */
    Def = DefGet(DL_SCSI_DTYPE, (char *) NULL, (long) Inq->inq_dtype, 0);
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
	      DevInfo->Name, Inq->inq_dtype);

    return(0);
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
    Cdb.cmd = CMD_INQUIRY;
    Cdb.length = sizeof(ScsiInquiry_t);

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
    Cdb.cmd = CMD_CAPACITY;

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
    ScsiModeGeometry_t	       *Geometry;
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Geometry = (ScsiModeGeometry_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI GEOMETRY: #tracks=<%d>",
	  DevInfo->Name, Geometry->heads);

    if (!(Disk = DiskSetup(DevInfo, "ScsiGeometryDecode")))
	return(-1);

    Disk->Tracks = Geometry->heads;
    Disk->PhyCyl = (int)(Geometry->cyl_ub << 16) + (int)(Geometry->cyl_mb << 8)
	+ (int)(Geometry->cyl_lb);
    Disk->RPM = ntohs(Geometry->rpm);

    return(0);
}

/*
 * Decode a SCSI FORMAT command and add info to DevInfo as we can.
 */
extern int ScsiFormatDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiModeFormat_t	       *Format;
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Format = (ScsiModeFormat_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI FORMAT: #sect=<%d> secsize=<%d>",
	  DevInfo->Name, ntohs(Format->sect_track), 
	  ntohs(Format->data_bytes_sect));

    if (!(Disk = DiskSetup(DevInfo, "ScsiFormatDecode")))
	return(-1);

    Disk->Sect = ntohs(Format->sect_track);
    Disk->SecSize = ntohs(Format->data_bytes_sect);
    Disk->Tracks = ntohs(Format->tracks_per_zone);
    Disk->AltSectPerZone = ntohs(Format->alt_sect_zone);
    Disk->AltTracksPerVol = ntohs(Format->alt_tracks_vol);
    Disk->AltTracksPerZone = ntohs(Format->alt_tracks_zone);
    Disk->IntrLv = ntohs(Format->interleave);
    Disk->TrackSkew = ntohs(Format->track_skew);
    Disk->CylSkew = ntohs(Format->cylinder_skew);

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
    static ScsiModeFormat_t	Format;
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
    Cdb.cmd = CMD_MODE_SENSE;
    Cdb.addr1 = DAD_MODE_FORMAT;
    Cdb.length = 255;	/* Yuck.  It's hardcoded */

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
	(void) memcpy(&Format, LocPtr, Format.mode_page.length);
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
    static ScsiModeGeometry_t	Geometry;
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
    Cdb.cmd = CMD_MODE_SENSE;
    Cdb.addr1 = DAD_MODE_GEOMETRY;
    Cdb.length = 255;	/* Yuck.  It's hardcoded */

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
	(void) memcpy(&Geometry, LocPtr, Geometry.mode_page.length);
	Query->Data = (void *) &Geometry;
	return ScsiGeometryDecode(Query);
    } else
	return(-1);
}

/*
 * Decode a SCSI SPEED command and add info to DevInfo as we can.
 */
extern int ScsiSpeedDecode(Query)
    ScsiQuery_t		       *Query;
{
    ScsiModeSpeed_t	       *Speed;
    DevInfo_t		       *DevInfo;

    if (!Query|| !Query->DevInfo || !Query->Data)
	return(-1);

    Speed = (ScsiModeSpeed_t *) Query->Data;
    DevInfo = Query->DevInfo;

    SImsg(SIM_DBG, 
	  "\t%s: SCSI SPEED: speed=<%d>",
	  DevInfo->Name, ntohs(Speed->speed));

    /*
     * XXX Don't know how useful this will be or what the "speed" values
     * will be, so for now, we just print a debug.
     */

    return(0);
}

/*
 * Get the speed of a SCSI device.
 * This only works on CD-ROM's right now.
 */
extern int ScsiQuerySpeed(Query)
     ScsiQuery_t	       *Query;	
{
    static ScsiCdbG0_t		Cdb;
    static ScsiCmd_t		Cmd;
    static ScsiModeSpeed_t	Speed;
    ScsiModeHeader_t	       *Hdr;
    char		       *LocPtr;

    if (!Query) {
	SImsg(SIM_DBG, "ScsiSpeed: Bad parameters");
	return(-1);
    }

    if (Query->DevInfo && Query->DevInfo->Type != DT_CDROM) {
	SImsg(SIM_DBG, "%s: Not a CDROM (%d).  Skipping ScsiQuerySpeed.",
	      Query->DevInfo->Name, Query->DevInfo->Type);
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.cmd = CMD_MODE_SENSE;
    Cdb.addr1 = CDROM_MODE_SPEED;
    Cdb.length = 255;	/* Yuck.  It's hardcoded */

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
	(void) memset(&Speed, 0, sizeof(Speed));
	(void) memcpy(&Speed, LocPtr, sizeof(ScsiModePage_t));
	(void) memcpy(&Speed, LocPtr, Speed.mode_page.length);
	Query->Data = (void *) &Speed;
	return ScsiSpeedDecode(Query);
    } else
	return(-1);
}

/*
 * Query a device for SCSI information.
 */
extern int ScsiQuery(DevInfo, DevFile, DevFD, OverRide)
    DevInfo_t		       *DevInfo;
    char		       *DevFile;
    int				DevFD;
    int				OverRide;
{
    int				Status = 0;
    int				Type = 0;
    static ScsiQuery_t		Query;
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
     * We expect that after ScsiQueryInquire() DevInfo->Type will
     * be correctly set.
     */
    if (ScsiQueryInquiry(&Query) < 0)
	--Status;

    /*
     * For the rest of these, only try the Query if Type is not
     * set or if the Type is set to an appropriate device type
     */
    Type = Query.DevInfo->Type;
    if (!Type || Type == DT_DISKDRIVE || Type == DT_CDROM) {
	if (ScsiQueryCapacity(&Query) < 0)
	    --Status;
	if (ScsiQueryFormat(&Query) < 0)
	    --Status;
	if (ScsiQueryGeometry(&Query) < 0)
	    --Status;
    }
    if (!Type || Type == DT_CDROM) {
	if (ScsiQuerySpeed(&Query) < 0)
	    --Status;
    }

    /*
     * If we opened the descriptor, then close it now
     */
    if (fd >= 0)
	(void) close(fd);

    /*
     * If all Scsi*() functions returned error, we return error.
     * If at least 1 function succeeded, we return success.
     */
    if (Status == -5)	/* XXX change to be count of ScsiQuery*() above */
	return(-1);
    else
	return(0);
}
