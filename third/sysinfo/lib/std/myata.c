/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Common SysInfo ATA/IDE/EIDE functions
 */

#include "defs.h"
#if	defined(HAVE_ATA)
#include "myata.h"

/*
 * Decode the results of an ATA IDENTIFY or an ATAPI IDENTIFY command.
 * AtaIdent_t is used for either command.  The fields are all padded
 * to the same location, but some fields have different meanings depending
 * on whether this was an ATA or an ATAPI call.
 */
extern int AtaIdentDecode(Query)
     AtaQuery_t		       *Query;
{
    DevInfo_t		       *DevInfo;
    DiskDrive_t		       *Disk;
    AtaIdent_t		       *Ident;
    Desc_t		      **Desc;
    Define_t		       *Def;
    char		       *cp;
    register u_int		i;

    if (!Query || !Query->DevInfo || !Query->Data)
        return -1;

    Ident = (AtaIdent_t *) Query->Data;
    DevInfo = Query->DevInfo;
    Desc = &DevInfo->DescList;

    cp = NULL;
    switch (Ident->ATAtype) {
      case ATA_TYPE_ATA:	cp = "ATA";	break;
      case ATA_TYPE_ATAPI:	cp = "ATAPI";	break;
      default:
	  SImsg(SIM_DBG, "%s: Undefined ATAtype = %d.", 
		DevInfo->Name, Ident->ATAtype);
    }
    if (cp)
        AddDesc(Desc, "ATA Type", cp, 0);

    SImsg(SIM_DBG, 
	  "\t%s: ATAPI IDENT: Type=%s Model=<%s> Rev=<%s> Serial=<%s>",
	  DevInfo->Name, cp,
	  Ident->Model, Ident->Revision, Ident->Serial);
    
    /*
     * Ident->DevType is only valid for ATAPI
     */
    if (Ident->ATAtype == ATA_TYPE_ATAPI) {
	/*
	 * Look up device type.  The values for DevType are the same as
	 * for SCSI DevType per the ATA/ATAPI standard.
	 */
	Def = DefGet(DL_SCSI_DTYPE, (char *) NULL, (long) Ident->DevType, 0);
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
		  DevInfo->Name, Ident->DevType);
    }

    /*
     * This is implied by this function being called
     */
    DevInfo->ClassType = CT_ATA;

    /*
     * Decode results.  The ATA standard specifies that all string values
     * be returned in reverse byte order "BADC" instead of "ABCD".  We
     * specify CSF_BSWAP to correct for this.
     */
#define IsOk(s)	( s && s[0] )
    if (IsOk(Ident->Model))
	DevInfo->Model = CleanString(Ident->Model, sizeof(Ident->Model), 
				     CSF_BSWAP);
    if (IsOk(Ident->Revision))
	DevInfo->Revision = CleanString(Ident->Revision, 
					sizeof(Ident->Revision), CSF_BSWAP);
    if (IsOk(Ident->Serial))
	if (cp = CleanString(Ident->Serial, 
			     sizeof(Ident->Serial), 
			     CSF_ENDNONPR|CSF_ALPHANUM|CSF_BSWAP)) {
	    /* Serial must be at least 6 chars long to be valid */
	    if (strlen(cp) >= 6)
		DevInfo->Serial = cp;
	    else
		SImsg(SIM_DBG, "%s: Serial value <%s> too short.  Ignored.",
		      DevInfo->Name, cp);
	}
#undef IsOk

    if (Ident->ATAtype == ATA_TYPE_ATA) {
	if (!(Disk = DiskSetup(DevInfo, "AtaIdentDecode")))
	    return -1;

	Disk->DataCyl = Ident->Cyls;
	Disk->Tracks = Ident->Heads;
	Disk->Sect = Ident->Sectors;

	i = Ident->BytesPerSect;
	Disk->SecSize = (i) ? i : 512;

	Disk->Size = nsect_to_mbytes(Disk->DataCyl * Disk->Tracks * Disk->Sect,
				     Disk->SecSize);

	if (Ident->CurInfoOK) {
	    if (Ident->CurCyls) 
		AddDesc(Desc, "Current Cylinders", itoa(Ident->CurCyls), 0);
	    if (Ident->CurHeads) 
		AddDesc(Desc, "Current Heads", itoa(Ident->CurHeads), 0);
	    if (Ident->CurSectors) 
		AddDesc(Desc, "Current Sectors", itoa(Ident->CurSectors), 0);
	}
    }

    if (Ident->SupAta1) AddDesc(Desc, "Supports", "ATA-1", 0);
    if (Ident->SupAta2) AddDesc(Desc, "Supports", "ATA-2", 0);
    if (Ident->SupAta3) AddDesc(Desc, "Supports", "ATA-3", 0);
    if (Ident->SupAta4) AddDesc(Desc, "Supports", "ATA-4", 0);
    if (Ident->SupAta5) AddDesc(Desc, "Supports", "ATA-5", 0);
    if (Ident->SupAta6) AddDesc(Desc, "Supports", "ATA-6", 0);
    if (Ident->SupAta7) AddDesc(Desc, "Supports", "ATA-7", 0);
    if (Ident->SupAta8) AddDesc(Desc, "Supports", "ATA-8", 0);
    if (Ident->SupAta9) AddDesc(Desc, "Supports", "ATA-9", 0);
    if (Ident->SupAta10) AddDesc(Desc, "Supports", "ATA-10", 0);
    if (Ident->SupAta11) AddDesc(Desc, "Supports", "ATA-11", 0);
    if (Ident->SupAta12) AddDesc(Desc, "Supports", "ATA-12", 0);
    if (Ident->SupAta13) AddDesc(Desc, "Supports", "ATA-13", 0);
    if (Ident->SupAta14) AddDesc(Desc, "Supports", "ATA-14", 0);

    /* ATA Minor Version */
    if (Ident->MinorVers && Ident->MinorVers != 0xFFFF) {
        if ((Def = DefGet("ATAvers", NULL, (long)Ident->MinorVers, 0)) &&
	    Def->ValStr2) {
	    AddDesc(Desc, "ATA Version", Def->ValStr2, 0);
	} else
	    SImsg(SIM_DBG, "%s: Undefined ATA MinorVers (0x%x) provided.",
		  DevInfo->Name, Ident->MinorVers);
    }

    if (Ident->BuffType)
        AddDesc(Desc, "Buffer Type", itoa(Ident->BuffType), 0);
    if (Ident->BuffSize)
        AddDesc(Desc, "Buffer Size", 
		GetSizeStr(Ident->BuffSize / 2, KBYTES), 0);
    if (Ident->RemovableMedia)
        AddDesc(Desc, NULL, "Is Removable", 0);

    if (Ident->PACKETsup) 
	AddDesc(Desc, "Supports", "PACKET Command", 0);

    if (Ident->SMARTfeat) 
	AddDesc(Desc, "Supports", "SMART feature set", 0);
    if (Ident->SecurityFeat) 
	AddDesc(Desc, "Supports", "Security feature set", 0);
    if (Ident->RemovableFeat) 
	AddDesc(Desc, "Supports", "Removable feature set", 0);
    if (Ident->PwrMgtFeat) 
	AddDesc(Desc, "Supports", "Power Management feature set", 0);
    if (Ident->APMfeat) 
	AddDesc(Desc, "Supports", "Advanced Power Management", 0);

    if (Ident->IORDYsupported) 
	AddDesc(Desc, "Supports", "IORDY", 0);
    if (Ident->IORDYcanDisable) 
	AddDesc(Desc, "Supports", "IORDY can be disabled", 0);

    if (Ident->LBAsupported)
	AddDesc(Desc, "Supports", "LBA", 0);

    if (Ident->UltraDMAvalid) {
	if (Ident->UltraDMAmode3)
	    AddDesc(Desc, "Ultra DMA Mode", "3", 0);
	else if (Ident->UltraDMAmode2)
	    AddDesc(Desc, "Ultra DMA Mode", "2", 0);
	else if (Ident->UltraDMAmode1)
	    AddDesc(Desc, "Ultra DMA Mode", "1", 0);
	else if (Ident->UltraDMAmode0)
	    AddDesc(Desc, "Ultra DMA Mode", "0", 0);
    }

    if (Ident->DMAsupported) {
	AddDesc(Desc, "Supports", "DMA", 0);
	if (Ident->DmaMode) 
	    AddDesc(Desc, "DMA Mode", itoa(Ident->DmaMode), 0);
	if (Ident->DmaMWordMode2) 
	    AddDesc(Desc, "Supports", "DMA Multiword Mode 2 and below", 0);
	else if (Ident->DmaMWordMode1)
	    AddDesc(Desc, "Supports", "DMA Multiword Mode 1 and below", 0);
	else if (Ident->DmaMWordMode0)
	    AddDesc(Desc, "Supports", "DMA Multiword Mode 0", 0);
    }

    if (Ident->PioMode)
	AddDesc(Desc, "PIO Mode", itoa(Ident->PioMode), 0);
    if (Ident->EideInfoOK) {
	if (Ident->EidePioMode3)
	    AddDesc(Desc, "Supports", "PIO Mode 3", 0);
	if (Ident->EidePioMode4)
	    AddDesc(Desc, "Supports", "PIO Mode 4", 0);
    }

    if (Ident->NumEcc)
	AddDesc(Desc, "No. ECC Bytes", itoa(Ident->NumEcc), 0);

    return 0;
}

/*
 * Perform ATAPI IDENTIFY command.
 */
extern int AtapiQueryIdent(Query)
     AtaQuery_t	       *Query;
{
    static AtaCdb_t		Cdb;
    static AtaCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "AtapiInquiry: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = ATAPI_CMD_IDENT;

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CmdName = "ATAPI IDENTIFY";
    Cmd.DataLen = sizeof(AtaIdent_t);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (AtaCmd(&Cmd) == 0) {
	/*
	 * Decode results.  The structure is common to both
	 * ATA IDENTIFY and ATAPI IDENTIFY so we use the same
	 * decode function.
	 */
	Query->Data = Cmd.Data;
	return AtaIdentDecode(Query);
    } else
	return(-1);
}

/*
 * Perform ATA IDENTIFY command.
 */
extern int AtaQueryIdent(Query)
     AtaQuery_t	       *Query;
{
    static AtaCdb_t		Cdb;
    static AtaCmd_t		Cmd;

    if (!Query) {
	SImsg(SIM_DBG, "AtaInquiry: Bad parameters");
	return(-1);
    }

    /*
     * Initialize
     */
    memset(&Cdb, 0, sizeof(Cdb));
    Cdb.Cmd = ATA_CMD_IDENT;

    memset(&Cmd, 0, sizeof(Cmd));
    Cmd.Cdb = &Cdb;
    Cmd.CmdName = "ATA IDENTIFY";
    Cmd.DataLen = sizeof(AtaIdent_t);
    Cmd.DevFD = Query->DevFD;
    Cmd.DevFile = Query->DevFile;

    if (AtaCmd(&Cmd) == 0) {
	/*
	 * Decode results
	 */
	Query->Data = Cmd.Data;
	return AtaIdentDecode(Query);
    } else
	return(-1);
}

/*
 * Query an ATA device
 */
extern int AtaQuery(DevInfo, DevFile, DevFD, OverRide)
    DevInfo_t		       *DevInfo;
    char		       *DevFile;
    int				DevFD;
    int				OverRide;
{
    int				Status = 0;
    static AtaQuery_t		Query;
    int				fd = -1;

    if (!DevInfo || !DevFile) {
	SImsg(SIM_DBG, "AtaQuery: Bad parameters.");
	return(-1);
    }

    if (DevFD < 0) {
	/*
	 * We use O_RDWR because many OS's require Write perms
	 */
	DevFD = fd = open(DevFile, O_RDWR|O_NDELAY|O_NONBLOCK);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: AtaQuery: open for read failed: %s",
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
     * Do a general ATA inquiry on the device.
     */
    if (AtaQueryIdent(&Query) == 0)
	++Status;

    /*
     * Do an ATAPI inquiry
     */
    if (AtapiQueryIdent(&Query) == 0)
	++Status;

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
#endif	/* HAVE_ATA */
