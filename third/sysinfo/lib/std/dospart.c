/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * DOS Partition functions
 */

#include "defs.h"
#include "dospart.h"

/*
 * Private declares
 */
static int		GetExtended();
static int		GetParts();

/*
 * Read the MBR from File
 * Return < 0 on error.
 * Return 0 on success.
 * Return 1 if no partition.
 */
static int DosPartGetMBR(Mbr, MbrOffSet, File)
     MBR_t		       *Mbr;
     off_t			MbrOffSet;
     char		       *File;
{
    int				Amt;
    int				SecSize;
    int				fd;

    /*
     * Open it ourself to get the right open flags.
     */
    fd = open(File, O_RDONLY|O_NDELAY|O_NONBLOCK);
    if (fd < 0) {
        SImsg(SIM_GERR, "%s: Open for read failed: %s", File, SYSERR);
        return(-1);
    }

    if (lseek(fd, MbrOffSet, SEEK_SET) < 0) {
	SImsg(SIM_GERR, "%s: seek failed: %s", File, SYSERR);
	(void) close(fd);
	return -1;
    }

    for (SecSize = MIN_SEC_SIZE; SecSize <= MAX_SEC_SIZE; SecSize *= 2) {
	Amt = read(fd, (void *) Mbr->BootInst, SecSize);
	if (Amt == SecSize) {
	    /* Success */
	    (void) close(fd);
	    if (Mbr->Signature != DOS_BOOT_MAGIC) {
		SImsg(SIM_DBG, "%s: No DOS Partition found.", File);
		return(1);
	    }
	    return(0);
	}
    }

    SImsg(SIM_GERR, "%s: read MBR failed.", File);
    (void) close(fd);

    return(-1);
}

/*
 * Return description of what Dos Partition Type is
 */
extern char *DosPartType(Type)
     u_char			Type;
{
    Define_t		       *Def;

    if (Def = DefGet(DL_DOSPARTTYPES, NULL, (long)Type, 0))
	/* Return the short name else the long name */
	return (Def->ValStr2) ? Def->ValStr2 : Def->ValStr1 ;

    SImsg(SIM_DBG, "DOS Partition Type 0x%2X (%d) is unknown.", Type, Type);

    return((char *) NULL);
}

/*
 * Set type information in PartInfo using Type
 */
static int DosPartSetType(PartInfo, Type)
     PartInfo_t		       *PartInfo;
     u_char			Type;
{
    Define_t		       *Def;
    static char			Buff[64];

    PartInfo->TypeNum = Type;

    if (Def = DefGet(DL_DOSPARTTYPES, NULL, (long)Type, 0)) {
	if (Def->ValStr2 && !EQ(Def->ValStr2, "unused"))
	    PartInfo->Type = Def->ValStr2;
	if (Def->ValStr1 && !EQ(Def->ValStr1, "unused"))
	    PartInfo->TypeDesc = Def->ValStr1;
	if (!PartInfo->Type && !PartInfo->TypeDesc && Type) {
	    (void) snprintf(Buff, sizeof(Buff), "Unknown Type 0x%02X (%d)",
			    Type, Type);
	    PartInfo->TypeDesc = strdup(Buff);
	}
	return 0;
    } else {
	SImsg(SIM_DBG, "DOS Partition Type 0x%2X (%d) is unknown.", 
	      Type, Type);
	return -1;
    }
}

/*
 * Get Extended Partition information by reading a DOS Partition table
 * from DevFile at location StartSect.
 */
static int GetExtended(StartSect, DiskDrive, DevFile, PartNum, PartInfo, Level)
     Large_t			StartSect;
     DiskDrive_t	       *DiskDrive;
     char		       *DevFile;
     int			PartNum;
     PartInfo_t		       *PartInfo;
     int			Level;
{
    static int			fd;
    static char		        Buffer[8192];
    static DosPart_t	       *DosParts;
    static MBR_t		Mbr;
    int				SecSize;

    fd = open(DevFile, O_RDONLY|O_NDELAY|O_NONBLOCK);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open for read failed: %s", DevFile, SYSERR);
	return -1;
    }

    SecSize = (DiskDrive->SecSize) ? DiskDrive->SecSize : 512;

    if (SecSize >= sizeof(Buffer)) {
	SImsg(SIM_GERR, "%s: Buffer size (%d) to small for secsize (%d).",
	      DevFile, sizeof(Buffer), SecSize);
	return -1;
    }

    /*
     * An Extended Partition contains a standard DOS Partition table at
     * the beginning of it's own partition.  So we need to locate the
     * start of the Extended Partition, seek there, then read the 
     * DOS Partition table.
     */
    if (myllseek(fd, (Offset_t)StartSect * SecSize, SEEK_SET) < 0) {
	SImsg(SIM_GERR, "%s: seek failed: %s", DevFile, SYSERR);
	(void) close(fd);
	return -1;
    }

    if (SecSize != read(fd, Buffer, SecSize)) {
	SImsg(SIM_GERR, "%s: read extended partition %d failed: %s", 
	      DevFile, PartNum, SYSERR);
	(void) close(fd);
	return -1;
    }

    (void) close(fd);

    /*
     * The standard partition is located at this "special" DOS standard
     * offset location.
     */
    DosParts = (DosPart_t *) (Buffer + DOS_EXTENDED_OFFSET);

    /*
     * Now do standard Partition extraction
     */
    (void) GetParts(DosParts, StartSect, DiskDrive, DevFile, PartNum, Level);

    return 0;
}

/*
 * Get standard DOS partition information from DosParts (a list) and add
 * the results to DiskDrive.  If an Extended Partition is found, we call
 * the GetExtended() function which in turn calls us recursively with a
 * new DosParts list.
 *
 * --RECURSE--
 */
static int GetParts(DosParts, Offset, DiskDrive, DevFile, PartNum, Level)
     DosPart_t		       *DosParts;
     Large_t			Offset;		/* Offset from start of disk */
     DiskDrive_t	       *DiskDrive;
     char		       *DevFile;
     int			PartNum;	/* Start Part #'s with this */
     int			Level;
{
    DiskPart_t		       *NewPart;
    static PartInfo_t	       *PartInfo;
    static DiskPart_t	       *Last;
    static Large_t		ExtStartSect;
    static Large_t		StartSect;
    static char			Path[128];
    static char			Slice[32];
    static int			i;
    int				Num;
    char		       *cp;
    char		      **cpp;

    Last = NULL;
    ExtStartSect = 0;

    for (i = 0; i < DOS_NPARTS; ++i) {
	StartSect = (Large_t) (DosParts[i].dp_start + Offset);

	if (IS_EXTENDED(DosParts[i].dp_type)) {
	    if (!ExtStartSect)
		/* Remember this for when we're ready */
		ExtStartSect = StartSect;
	    if (Level > 0)
		/* 
		 * We've already been called at least once so we're in 
		 * an Extended Partition for which we do not create new
		 * PartInfo entries to adhere to the rules of normal
		 * DOS Partition numbers.
		 */
		continue;
	}

	if (DosParts[i].dp_size == 0 && DosParts[i].dp_type == 0)
	    continue;

	Num = PartNum + i;
	(void) snprintf(Slice, sizeof(Slice), "s%d", Num);
	(void) snprintf(Path, sizeof(Path), "%s%s", DevFile, Slice);

	PartInfo = PartInfoCreate(NULL);
	PartInfo->Num = Num;
	PartInfo->DevPath = strdup(Path);
	PartInfo->Name = strdup(Slice);
	PartInfo->SecSize = DiskDrive->SecSize;

	if (i == 0)
	    PartInfo->Title = "DOS";

	/*
	 * We use Type and Usage backwards here because Usage gets fully
	 * displayed on output and Type gets truncated.
	 */
	if (DosParts[i].dp_flags == DOS_ACTIVE) {
	    cpp = (char **) xcalloc(2, sizeof(char *));
	    cpp[0] = strdup("ACTIVE");
	    PartInfo->MntOpts = cpp;
	}

	(void) DosPartSetType(PartInfo, DosParts[i].dp_type);
	PartInfo->TypeNum = DosParts[i].dp_type;

	PartInfo->StartSect = StartSect;
	PartInfo->NumSect = (Large_t) DosParts[i].dp_size;

	if (IS_EXTENDED(DosParts[i].dp_type))
	    PartInfo->Usage = PIU_UNKNOWN;

	/*
	 * Add it to the list
	 */
	NewPart = NewDiskPart(NULL);
	NewPart->PartInfo = PartInfo;
	/*
	 * If DiskDrive exists, find the end of the list and set Last
	 */
	if (!Last && DiskDrive->DiskPart)
	    for (Last = DiskDrive->DiskPart; Last && Last->Next; 
		 Last = Last->Next);
	if (Last) {
	    Last->Next = NewPart;
	    Last = NewPart;
	} else
	    DiskDrive->DiskPart = Last = NewPart;
    }

    if (ExtStartSect)
	/*
	 * We previously found an Extended Partition (there can only be one).
	 * Now we go read it.
	 */
	GetExtended(ExtStartSect, DiskDrive, DevFile, 
		    /* Level > 0 means we're already in an Ext Part */
		    (Level == 0) ? PartNum + i : PartNum + 1, 
		    PartInfo, Level+1);

    return 0;
}

/*
 * Read all the DOS Parts for DevInfo
 * DevFile should be the name of the control device to read.
 */
extern int DosPartGet(Query)
     DosPartQuery_t	       *Query;
{
    DevInfo_t		       *DevInfo;
    char		       *CtlDev;
    char		       *BaseFile;
    int				Status;
    static MBR_t		Mbr;
    DiskDriveData_t	       *DiskDriveData;
    DiskDrive_t		       *Disk;
    PartInfo_t		       *PartInfo;

    if (!Query)
	return -1;

    DevInfo = Query->DevInfo;
    CtlDev = Query->CtlDev;
    BaseFile = Query->BasePath;

    if (!DevInfo || !DevInfo->Name || !CtlDev)
	return(-1);

    if (!BaseFile)
	BaseFile = CtlDev;

    Status = DosPartGetMBR(&Mbr, (off_t)DOS_BB_SECTOR, CtlDev);
    if (Status < 0)
	return(-1);
    else if (Status > 0)
	return(0);

    if (BaseFile != CtlDev)
	DevAddFile(DevInfo, strdup(BaseFile));
    DevAddFile(DevInfo, strdup(CtlDev));

    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }

    if (DiskDriveData->HWdata)
	Disk = DiskDriveData->HWdata;
    else {
	Disk = NewDiskDrive(NULL);
	DiskDriveData->HWdata = Disk;
    }

    /* DOS Partition numbers start with 1 */
    GetParts(&Mbr.Parts[0], (Large_t)0, Disk, BaseFile, 1, 0);

    return(0);
}
