/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * This file contains SCSI definetions common to SysInfo.
 */

#ifndef __myscsi_h__
#define __myscsi_h__ 

#if	!defined(_BIT_FIELDS_LTOH) && !defined(_BIT_FIELDS_HTOL)
 "One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined"
#endif	/* _BIT_FIELDS_ */

/*
 * SysInfo SCSI definetions
 */
#define MySCSI_CMD_TIMEOUT	30		/* Timeout per dev (seconds) */
#define SCSI_BUF_LEN		8192		/* Max SCSI cmd buff size */

/*
 * Types
 */
typedef Query_t			ScsiQuery_t;	/* For Scsi*() funcs */
typedef u_char			Word_t;		/* An 8 bit type */

/*
 * Macros to convert Word_t[X] to proper u_int values
 */
#undef	word3_to_uint
#define	word3_to_uint(a)	((u_int) (\
			(((Word_t *) a)[2]       & 0xFF) | \
			(((Word_t *) a)[1] << 8  & 0xFF00) | \
			(((Word_t *) a)[0] << 16 & 0xFF0000) \
				))

/*
 * Parameters for ScsiCmd()
 */
typedef struct {
    void		       *Cdb;		/* We don't know which type */
    size_t			CdbLen;		/* Length of Cdb */
    char		       *DevFile;
    int				DevFD;
    void		       *Data;		/* Return Data */
} ScsiCmd_t;

/*
 * Standard SCSI control blocks definitions (CDB).
 *
 * These go in or out over the SCSI bus.
 *
 * The first 8 bits of the command block are the same for all
 * defined command groups.  The first byte is an operation which consists
 * of a command code component and a group code component.
 *
 * The group code determines the length of the rest of the command.
 * Group 0 commands are 6 bytes, Group 1 and 2  are 10 bytes, Group 4
 * are 16 bytes, and Group 5 are 12 bytes. Groups 3 is Reserved.
 * Groups 6 and 7 are Vendor Unique.
 *
 */

/*
 * CDB for Group 0 (6 byte) commands
 */
typedef struct {
    Word_t		Cmd;			/* Command (Op Code) */
#if 	defined(_BIT_FIELDS_LTOH)
    Word_t		EVPD 		: 1;	/* Enable Vital Product Data */
    Word_t		CmdDt 		: 1;	/* Command Support Data */
    Word_t		_reserved 	: 6;
#elif	defined(_BIT_FIELDS_HTOL)
    Word_t		_reserved 	: 6;
    Word_t		CmdDt 		: 1;	/* Command Support Data */
    Word_t		EVPD 		: 1;	/* Enable Vital Product Data */
#endif	/* _BIT_FIELDS_ */
    Word_t		Addr1;			/* Logical Block Address */
    Word_t		Addr2;			/* Logical Block Address(LSB)*/
    Word_t		Length;			/* Length of data requested */
    Word_t		Control;		/* Control Byte */
} ScsiCdbG0_t;

/*
 * CDB for Group 1 (10 byte) commands
 */
typedef struct {
    Word_t		Cmd;			/* Command (Op Code) */
#if 	defined(_BIT_FIELDS_LTOH)
    Word_t		EVPD 		: 1;	/* Enable Vital Product Data */
    Word_t		CmdDt 		: 1;	/* Command Support Data */
    Word_t		_reserved 	: 6;
#elif	defined(_BIT_FIELDS_HTOL)
    Word_t		_reserved 	: 6;
    Word_t		CmdDt 		: 1;	/* Command Support Data */
    Word_t		EVPD 		: 1;	/* Enable Vital Product Data */
#endif	/* _BIT_FIELDS_ */
    Word_t		Addr3;
    Word_t		Addr2;
    Word_t		Addr1;
    Word_t		Addr0;
    Word_t		_reserved1;
    Word_t		Count1;
    Word_t		Count0;
    Word_t		Misc;
} ScsiCdbG1_t;

/*
 * SCSI Commands common to all SCSI devices
 */
	/* SCSI GROUP 0 Commands */
#define SCSI_INQUIRY		0x12		/* ScsiInquiry_t */
#define SCSI_INQUIRY_IDENT	0x83		/* ScsiIdent_t */
#define SCSI_INQUIRY_SERIAL	0x80		/* ScsiSerial_t */
#define SCSI_MODE_SENSE		0x1a
	/* SCSI GROUP 1 Commands */
#define SCSI_CAPACITY		0x25		/* ScsiCapacity_t */

/*
 * Mode Sense/Select Header.
 *
 * Mode Sense/Select data consists of a header, followed by zero or more
 * block descriptors, followed by zero or more mode pages.
 *
 */
typedef struct {
    Word_t 	Length;			/* number of bytes following */
    Word_t 	MediumType;		/* device specific */
    Word_t 	DeviceSpecific;		/* device specfic parameters */
    Word_t 	BdescLength;		/* length of block descriptor(s), if any */
} ScsiModeHeader_t;
#define	SCSI_MODE_HEADER_LENGTH	(sizeof (ScsiModeHeader_t))
/*
 * Define a macro to retrieve the first mode page. Could be more
 * general (for multiple mode pages).
 */
#define	SCSI_MODE_PAGE_ADDR(mhdr, type)	\
  ((type *)(((u_long)(mhdr))+SCSI_MODE_HEADER_LENGTH+(mhdr)->BdescLength))

/*
 * Mode page header. Zero or more Mode Pages follow either the block
 * descriptors (if any), or the Mode Header.
 *
 * The 'ps' bit must be zero for mode select operations.
 *
 */
typedef struct {
#if 	defined(_BIT_FIELDS_LTOH)
    Word_t	Code	:6;		/* page code number */
    Word_t	res	:1;		/* reserved */
    Word_t	PSbit	:1;		/* 'Parameter Saveable' bit */
#elif 	defined(_BIT_FIELDS_HTOL)
    Word_t	PSbit	:1;		/* 'Parameter Saveable' bit */
    Word_t	res	:1;		/* reserved */
    Word_t	Code	:6;		/* page code number */
#endif	/* _BIT_FIELDS_LTOH */
    Word_t	Length;			/* length of bytes to follow */
    /*
     * Mode Page specific data follows right after this...
     */
} ScsiModePage_t;

/*
 * SCSI Capacity (SCSI_CAPACITY)
 */
typedef struct {
    u_int	Blocks;			/* Number of blocks */
    u_int	BlkSize;		/* Size of blocks */
} ScsiCapacity_t;

/*
 * INQUIRY command - 0x12 (SCSI_INQUIRY)
 * Defined in SCSI-3 Primary Commands (SPC)
 * Required by all SCSI devices
 */
typedef struct {
#if 	defined(_BIT_FIELDS_LTOH)
    /* Byte 0 */
    Word_t	DevType		: 5;	/* Device Type */
    Word_t	PerQual		: 3;	/* Peripheral Qualifer */
    /* Byte 1 */
    Word_t	_reserved1	: 7;
    Word_t	Removable	: 1;	/* removable media */
    /* Byte 2 */
    Word_t	ANSI		: 3;	/* ANSI version */
    Word_t	ECMA		: 3;	/* ECMA version */
    Word_t	ISO		: 2;	/* ISO/IEC version */
    /* Byte 3 */
    Word_t	RDF		: 4;	/* response data format */
    Word_t	_reserved2	: 1;
    Word_t	NormACA		: 1;	/* setting NACA bit supported */
    Word_t	TrmTsk		: 1;	/* supports TERMINATE I/O PROC msg */
    Word_t	AERC		: 1;	/* async event notification cap. */
    /* Byte 4 */
    Word_t	Length;			/* additional length */
    /* Byte 5 */
    Word_t	_reserved3	: 8;
    /* Byte 6 */
    Word_t	Addr16		: 1;	/* supports 16 bit wide SCSI addr */
    Word_t	Addr32		: 1;	/* supports 32 bit wide SCSI addr */
    Word_t	ACKQREQQ	: 1;	/* data tranfer on Q cable */
    Word_t	MChngr		: 1;	/* embedded/attached to medium chngr */
    Word_t	MultiPort	: 1;	/* Multi Ported device */
    Word_t	_vendor1	: 1;	/* Vendor Specific */
    Word_t	EncServ		: 1;	/* Enclosure Services present */
    Word_t	_reserved4	: 1;
    /* Byte 7 */
    Word_t	_vendor2	: 1;	/* Vendor Specific */
    Word_t	CmdQue		: 1;	/* supports command queueing */
    Word_t	TranDis		: 1;	/* supports transfer disable messages*/
    Word_t	Linked		: 1;	/* supports linked commands */
    Word_t	Sync		: 1;	/* supports synchronous data xfers */
    Word_t	WBus16		: 1;	/* supports 16 bit wide data xfers */
    Word_t	WBus32		: 1;	/* supports 32 bit wide data xfers */
    Word_t	RelAddr		: 1;	/* supports relative addressing */
    /* Bytes 8-15 */
    char	VendorID[8];		/* vendor ID */
    /* Bytes 16-31 */
    char	ProductID[16];		/* product ID */
    /* Bytes 32-35 */
    char	Revision[4];		/* revision level */
    /* 
     * Bytes 36-55 are officially "Vendor-specific"
     * but by convention most devices are a serial # in the first 12 bytes
     */
    char	Serial[12];		/* serial number */
    Word_t	_vendor3[7];		/* Vendor Specific */
    /*
     * Bytes 56-95 are reserved.
     * 96 to 'n' are vendor-specific parameter bytes
     */

#elif	defined(_BIT_FIELDS_HTOL)

    /* Byte 0 */
    Word_t	PerQual		: 3;	/* Peripheral Qualifer */
    Word_t	DevType		: 5;	/* Device Type */
    /* Byte 1 */
    Word_t	Removable	: 1;	/* removable media */
    Word_t	_reserved1	: 7;
    /* Byte 2 */
    Word_t	ISO		: 2;	/* ISO/IEC version */
    Word_t	ECMA		: 3;	/* ECMA version */
    Word_t	ANSI		: 3;	/* ANSI version */
    /* Byte 3 */
    Word_t	AERC		: 1;	/* async event notification cap. */
    Word_t	TrmTsk		: 1;	/* supports TERMINATE I/O PROC msg */
    Word_t	NormACA		: 1;	/* setting NACA bit supported */
    Word_t	_reserved2	: 1;
    Word_t	RDF		: 4;	/* response data format */
    /* Byte 4 */
    Word_t	Length;			/* additional length */
    /* Byte 5 */
    Word_t	_reserved3	: 8;
    /* Byte 6 */
    Word_t	_reserved4	: 1;
    Word_t	EncServ		: 1;	/* Enclosure Services present */
    Word_t	_vendor1	: 1;	/* Vendor Specific */
    Word_t	MultiPort	: 1;	/* Multi Ported device */
    Word_t	MChngr		: 1;	/* embedded/attached to medium chngr */
    Word_t	ACKQREQQ	: 1;	/* data tranfer on Q cable */
    Word_t	Addr32		: 1;	/* supports 32 bit wide SCSI addr */
    Word_t	Addr16		: 1;	/* supports 16 bit wide SCSI addr */
    /* Byte 7 */
    Word_t	RelAddr		: 1;	/* supports relative addressing */
    Word_t	WBus32		: 1;	/* supports 32 bit wide data xfers */
    Word_t	WBus16		: 1;	/* supports 16 bit wide data xfers */
    Word_t	Sync		: 1;	/* supports synchronous data xfers */
    Word_t	Linked		: 1;	/* supports linked commands */
    Word_t	TranDis		: 1;	/* supports transfer disable messages*/
    Word_t	CmdQue		: 1;	/* supports command queueing */
    Word_t	_vendor2	: 1;	/* Vendor Specific */
    /* Bytes 8-15 */
    char	VendorID[8];		/* vendor ID */
    /* Bytes 16-31 */
    char	ProductID[16];		/* product ID */
    /* Bytes 32-35 */
    char	Revision[4];		/* revision level */
    /* 
     * Bytes 36-55 are officially "Vendor-specific"
     * but by convention most devices are a serial # in the first 12 bytes
     */
    char	Serial[12];		/* serial number */
    Word_t	_vendor3[7];		/* Vendor Specific */
    /*
     * Bytes 56-95 are reserved.
     * 96 to 'n' are vendor-specific parameter bytes
     */
#endif	/* _BIT_FIELDS_ */
} ScsiInquiry_t;

/*
 * Response Data Formats for ScsiInquiry_t.RDF
 *
 * RDF_LEVEL0 means that this structure complies with SCSI-1 spec.
 * RDF_CCS means that this structure complies with CCS pseudo-spec.
 * RDF_SCSI2 means that the structure complies with the SCSI-2/3 spec.
 */

#define	RDF_LEVEL0		0x00
#define	RDF_CCS			0x01
#define	RDF_SCSI2		0x02

/*
 * SCSI Device Type values for ScsiInquiry_t.DevType
 */
#define SCSI_DTYPE_DAD		0x00	/* Direct Access (e.g. disk) */
#define SCSI_DTYPE_SEQ		0x01	/* Sequential Access (e.g. tape) */
#define SCSI_DTYPE_PRINTER	0x02
#define SCSI_DTYPE_PROCESSOR	0x03
#define SCSI_DTYPE_WORM		0x04
#define SCSI_DTYPE_CDROM	0x05
#define SCSI_DTYPE_SCANNER	0x06
#define SCSI_DTYPE_OPTICAL	0x07
#define SCSI_DTYPE_CHANGER	0x08
#define SCSI_DTYPE_COM		0x09
#define SCSI_DTYPE_ARRAY	0x0C
#define SCSI_DTYPE_ESI		0x0D

/*
 * Header common to all Inquiry Mode (SCSI_INQUIRY_*) commands.
 */
typedef struct {
#if 	defined(_BIT_FIELDS_LTOH)
    Word_t	DevType		: 5;	/* Device Type */
    Word_t	PerQual		: 3;	/* Peripheral Qualifer */
#elif	defined(_BIT_FIELDS_HTOL)
    Word_t	PerQual		: 3;	/* Peripheral Qualifer */
    Word_t	DevType		: 5;	/* Device Type */
#endif	/* _BIT_FIELDS_ */
    Word_t	PageCode;		/* One of SCSI_INQUIRY_* */
    Word_t	_reserved1;
    Word_t	Length;			/* Page Length (n-3) */
} ScsiInquiryHdr_t;

/*
 * Identification Descriptor used in SCSI_INQUIRY_IDENT by ScsiIdent_t
 */
typedef struct {
#if 	defined(_BIT_FIELDS_LTOH)
    Word_t	CodeSet		: 4;	/* Code Set */
    Word_t	_reserved1	: 4;
    Word_t	IdentType	: 4;	/* Identifier Type */
    Word_t	_reserved2	: 4;
#elif	defined(_BIT_FIELDS_HTOL)
    Word_t	_reserved1	: 4;
    Word_t	CodeSet		: 4;	/* Code Set */
    Word_t	_reserved2	: 4;
    Word_t	IdentType	: 4;	/* Identifier Type */
#endif	/* _BIT_FIELDS_ */
    Word_t	_reserved3;
    Word_t	IdentLength;		/* Length of Identifier (n-3) */
    Word_t	Identifier[251];	/* The Identifier data itself */
} IdentDesc_t;

/*
 * Values for IdentDescriptor_t.CodeSet
 * Identification Descriptor Code Sets.
 * Identifies the type of data found in Identifier.
 */
#define IDCS_BINARY	0x01	/* Binary values */
#define IDCS_ASCII	0x02	/* Printable ASCII */

/*
 * Values for IdentDescriptor_t.IdentType
 * Format and assignment authority for Identifier.
 */
#define IDT_VENDOR8	1	/* Vendor ID in first 8 bytes */
#define IDT_EUI64	2	/* IEEE Extended Unique Identifer 64bit */
#define IDT_FCPH64	3	/* FC-PH 64bit Name_Identifier (8 bytes) */

/*
 * Type for Device Identification form of INQUIRY (SCSI_INQUIRY_IDENT)
 * Defined by SCSI-3 Primary Commands (SPC)
 */
typedef struct {
    ScsiInquiryHdr_t	Hdr;		/* Common Inquiry Header */
    IdentDesc_t		Descriptors[1];	/* Identification Descriptors */
} ScsiIdent_t;

/*
 * Type for Unit Serial Number page form of INQUIRY (SCSI_INQUIRY_SERIAL)
 * Defined in SCSI-3 Primary Commands (SPC)
 */
typedef struct {
    ScsiInquiryHdr_t	Hdr;		/* Common Inquiry Header */
    Word_t		Serial[251];	/* Serial Number value */
} ScsiSerialNum_t;

/*
 * Direct Access device Mode Sense/Mode Select Defined pages
 */
#define	SCSI_DAD_MODE_ERR_RECOV		0x01
#define	SCSI_DAD_MODE_FORMAT		0x03		/* ScsiFormat_t */
#define	SCSI_DAD_MODE_GEOMETRY		0x04		/* ScsiGeometry_t */
#define	SCSI_DAD_MODE_FLEXDISK		0x05		/* ScsiFlexDisk_t */
#define	SCSI_DAD_MODE_VRFY_ERR_RECOV	0x07
#define	SCSI_DAD_MODE_CACHE		0x08
#define	SCSI_DAD_MODE_MEDIA_TYPES	0x0B
#define	SCSI_DAD_MODE_NOTCHPART		0x0C
#define	SCSI_DAD_MODE_POWER_COND	0x0D
#define	SCSI_DAD_MODE_SPEED		0x31		/* ScsiSpeed_t */

/*
 * Format Device Page - Page Code 0x03 (SCSI_DAD_MODE_FORMAT)
 * Applies to SCSI Block Command (SBC)
 */
typedef struct  {
    ScsiModePage_t 	ModePage;	/* common mode page header */
    u_short	TracksPerZone;		/* Tracks/Zone */
    u_short	AltSectZone;		/* Alternate Sectors per Zone */
    u_short 	AltTracksZone;		/* Alternate Tracks per Zone */
    u_short	AltTracksVol;		/* Alternate Tracks/Logical Unit */
    u_short	SectTrack;		/* Sectors Per Track inc Alts */
    u_short 	DataBytesSect;		/* Data Bytes/Physical Sector */
    u_short	Interleave;		/* Interleave */
    u_short	TrackSkew;		/* # Phys Sects b/w tracks in cyl */
    u_short	CylinderSkew;		/* # Phys Sects between cylinders */
#if defined(_BIT_FIELDS_LTOH)
    Word_t	_reserved1	: 4;
    Word_t	SURF		: 1;	/* Progressive Addr of Logical Blks */
    Word_t	RMB		: 1;	/* Supports Removable Media */
    Word_t	HSEC		: 1;	/* Hard Sector formating */
    Word_t	SSEC		: 1;	/* Soft Sector formating */
#elif defined(_BIT_FIELDS_HTOL)
    Word_t	SSEC		: 1;	/* Soft Sector formating */
    Word_t	HSEC		: 1;	/* Hard Sector formating */
    Word_t	RMB		: 1;	/* Supports Removable Media */
    Word_t	SURF		: 1;	/* Progressive Addr of Logical Blks */
    Word_t	_reserved1	: 4;
#endif	/* _BIT_FIELDS_LTOH */
    Word_t	_reserved2[2];
} ScsiFormat_t;

/*
 * Rigid Disk Drive Geometry page 0x04 (SCSI_DAD_MODE_GEOMETRY)
 * Applies to SCSI-3 Block Commands (SBC)
 */
typedef struct {
    ScsiModePage_t 	ModePage;	/* common mode page header */
    Word_t	NumCyls[3];		/* # of Physical Cylinders */
    Word_t	Heads;			/* # of Physical Heads */
    Word_t	PreCompCyls[3];		/* Phys Cyl Write PreComp Starts*/
    Word_t	CurrentCyls[3];		/* Cyl to start reduced current */
    u_short	StepRate;		/* Drive Step Rate (in 100 ns units) */
    Word_t	LandingCyls[3];		/* Landing Zone Cylinder */
#if defined(_BIT_FIELDS_LTOH)
    Word_t	RPL		: 2;	/* Rotational Position Locking */
    Word_t	_reserved1 	: 6;
#elif defined(_BIT_FIELDS_HTOL)
    Word_t	_reserved1 	: 6;
    Word_t 	RPL		: 2;	/* Rotational Position Locking */
#endif	/* _BIT_FIELDS_LTOH */
    Word_t	RotOffset;		/* Rotational Offset */
    Word_t	_reserved2;
    u_short	RPM;			/* Rotations Per Minute */
    Word_t	_reserved3[2];
} ScsiGeometry_t;

/*
 * CD Capabilities page 0x2A (SCSI_MODE_CD_CAP)
 * Applies to SCSI-3 MultiMedia Commands (MMC).
 * Defined by SCSI-3 MMC-1.
 */
#define	SCSI_MODE_CD_CAP		0x2A
typedef struct {
    ScsiModePage_t 	ModePage;	/* common mode page header */
#if defined(_BIT_FIELDS_LTOH)
    /* Byte 2 */
    Word_t	CD_R_Read	: 1;	/* Reads CD-R  media		     */
    Word_t	CD_RW_Read	: 1;	/* Reads CD-RW media		     */
    Word_t	Method2		: 1;	/* Reads fixed packet method2 media  */
    Word_t	DVD_RAM_Read	: 1;	/* Reads DVD-RAM media		     */
    Word_t	DVD_R_Read	: 1;	/* Reads DVD-R media		     */
    Word_t	DVD_ROM_Read	: 1;	/* Reads DVD ROM media		     */
    Word_t	_reserved1	: 2;
    /* Byte 3 */
    Word_t	CD_R_Write	: 1;	/* Supports writing CD-R  media	     */
    Word_t	CD_RW_Write	: 1;	/* Supports writing CD-RW media	     */
    Word_t	TestWrite	: 1;	/* Supports emulation write	     */
    Word_t	_reserved2	: 1;
    Word_t	DVD_RAM_Write	: 1;	/* Supports writing DVD-RAM media    */
    Word_t	DVD_R_Write	: 1;	/* Supports writing DVD-R media	     */
    Word_t	_reserved3	: 2;
    /* Byte 4 */
    Word_t	AudioPlay	: 1;	/* Supports Audio play operation     */
    Word_t	Composite	: 1;	/* Deliveres composite A/V stream    */
    Word_t	DigitalPort1	: 1;	/* Supports digital output on port 1 */
    Word_t	DigitalPort2	: 1;	/* Supports digital output on port 2 */
    Word_t	Mode2Form1	: 1;	/* Reads Mode-2 form 1 media (XA)    */
    Word_t	Mode2Form2	: 1;	/* Reads Mode-2 form 2 media	     */
    Word_t	MultiSession	: 1;	/* Reads multi-session media	     */
    Word_t	_reserved4	: 1;
    /* Byte 5 */
    Word_t	CD_DA_Supported	: 1;	/* Reads audio data with READ CD cmd */
    Word_t	CD_DA_Accurate	: 1;	/* READ CD data stream is accurate   */
    Word_t	RW_Supported	: 1;	/* Reads R-W sub channel information */
    Word_t	RW_DeintCorr	: 1;	/* Reads de-interleved R-W sub chan  */
    Word_t	C2_Pointers	: 1;	/* Supports C2 error pointers	     */
    Word_t	ISRC		: 1;	/* Reads ISRC information	     */
    Word_t	UPC		: 1;	/* Reads media catalog number (UPC)  */
    Word_t	ReadBarCode	: 1;	/* Supports reading bar codes	     */
    /* Byte 6 */
    Word_t	Lock		: 1;	/* PREVENT/ALLOW may lock media	     */
    Word_t	LockState	: 1;	/* Lock state 0=unlocked 1=locked    */
    Word_t	PreventJumper	: 1;	/* State of prev/allow jumper 0=pres */
    Word_t	Eject		: 1;	/* Ejects disc/cartr with STOP LoEj  */
    Word_t	_reserved5	: 1;
    Word_t	LoadingType	: 3;	/* Loading mechanism type	     */
    /* Byte 7 */
    Word_t	SepChanVol	: 1;	/* Vol controls each channel separat */
    Word_t	SepChanMute	: 1;	/* Mute controls each channel separat*/
    Word_t	DiskPresentRep	: 1;	/* Changer supports disk present rep */
    Word_t	SWSlotSel	: 1;	/* Load empty slot in changer	     */
    Word_t	_reserved6	: 4;
    /* Byte 8 */
    u_short	MaxReadSpeed;		/* Max. read speed in KB/s	     */
    /* Byte 10 */
    u_short	NumVolLevels;		/* # of supported volume levels	     */
    /* Byte 12 */
    u_short	BufferSize;		/* Buffer size for the data in KB    */
    /* Byte 14 */
    u_short	CurReadSpeed;		/* Current read speed in KB/s	     */
    /* Byte 16 */
    Word_t	_reserved7	: 8;
    /* Byte 17 */
    Word_t	_reserved8	: 1;
    Word_t	BCK		: 1;	/* Data valid on falling edge of BCK */
    Word_t	RCK		: 1;	/* Set: HIGH high LRCK=left channel  */
    Word_t	LSBF		: 1;	/* Set: LSB first Clear: MSB first   */
    Word_t	Length		: 2;	/* 0=32BCKs 1=16BCKs 2=24BCKs 3=24I2c*/
    Word_t	_reserved9	: 2;
    /* Byte 18 */
    u_short	MaxWriteSpeed;		/* Max. write speed supported in KB/s*/
    /* Byte 20 */
    u_short	CurWriteSpeed;		/* Current write speed in KB/s	     */
#elif	defined(_BIT_FIELDS_HTOL)
    /* Byte 2 */
    Word_t	_reserved1	: 2;
    Word_t	DVD_ROM_Read	: 1;	/* Reads DVD ROM media		     */
    Word_t	DVD_R_Read	: 1;	/* Reads DVD-R media		     */
    Word_t	DVD_RAM_Read	: 1;	/* Reads DVD-RAM media		     */
    Word_t	Method2		: 1;	/* Reads fixed packet method2 media  */
    Word_t	CD_RW_Read	: 1;	/* Reads CD-RW media		     */
    Word_t	CD_R_Read	: 1;	/* Reads CD-R  media		     */
    /* Byte 3 */
    Word_t	_reserved2	: 2;
    Word_t	DVD_R_Write	: 1;	/* Supports writing DVD-R media	     */
    Word_t	DVD_RAM_Write	: 1;	/* Supports writing DVD-RAM media    */
    Word_t	_reserved3	: 1;
    Word_t	TestWrite	: 1;	/* Supports emulation write	     */
    Word_t	CD_RW_Write	: 1;	/* Supports writing CD-RW media	     */
    Word_t	CD_R_Write	: 1;	/* Supports writing CD-R  media	     */
    /* Byte 4 */
    Word_t	_reserved4	: 1;
    Word_t	MultiSession	: 1;	/* Reads multi-session media	     */
    Word_t	Mode2Form2	: 1;	/* Reads Mode-2 form 2 media	     */
    Word_t	Mode2Form1	: 1;	/* Reads Mode-2 form 1 media (XA)    */
    Word_t	DigitalPort2	: 1;	/* Supports digital output on port 2 */
    Word_t	DigitalPort1	: 1;	/* Supports digital output on port 1 */
    Word_t	Composite	: 1;	/* Deliveres composite A/V stream    */
    Word_t	AudioPlay	: 1;	/* Supports Audio play operation     */
    /* Byte 5 */
    Word_t	ReadBarCode	: 1;	/* Supports reading bar codes	     */
    Word_t	UPC		: 1;	/* Reads media catalog number (UPC)  */
    Word_t	ISRC		: 1;	/* Reads ISRC information	     */
    Word_t	C2_Pointers	: 1;	/* Supports C2 error pointers	     */
    Word_t	RW_DeintCorr	: 1;	/* Reads de-interleved R-W sub chan  */
    Word_t	RW_Supported	: 1;	/* Reads R-W sub channel information */
    Word_t	CD_DA_Accurate	: 1;	/* READ CD data stream is accurate   */
    Word_t	CD_DA_Supported	: 1;	/* Reads audio data with READ CD cmd */
    /* Byte 6 */
    Word_t	LoadingType	: 3;	/* Loading mechanism type	     */
    Word_t	_reserved5	: 1;
    Word_t	Eject		: 1;	/* Ejects disc/cartr with STOP LoEj  */
    Word_t	PreventJumper	: 1;	/* State of prev/allow jumper 0=pres */
    Word_t	LockState	: 1;	/* Lock state 0=unlocked 1=locked    */
    Word_t	Lock		: 1;	/* PREVENT/ALLOW may lock media	     */
    /* Byte 7 */
    Word_t	_reserved6	: 4;
    Word_t	SW_SlotSel	: 1;	/* Load empty slot in changer	     */
    Word_t	DiskPresentRep	: 1;	/* Changer supports disk present rep */
    Word_t	SepChanMute	: 1;	/* Mute controls each channel separat*/
    Word_t	SepChanVol	: 1;	/* Vol controls each channel separat */
    /* Byte 8 */
    u_short	MaxReadSpeed;		/* Max. read speed in KB/s	     */
    /* Byte 10 */
    u_short	NumVolLevels;		/* # of supported volume levels	     */
    /* Byte 12 */
    u_short	BufferSize;		/* Buffer size for the data in KB    */
    /* Byte 14 */
    u_short	CurReadSpeed;		/* Current read speed in KB/s	     */
    /* Byte 16 */
    Word_t	_reserved7	: 8;
    /* Byte 17 */
    Word_t	_reserved8	: 2;
    Word_t	Length		: 2;	/* 0=32BCKs 1=16BCKs 2=24BCKs 3=24I2c*/
    Word_t	LSBF		: 1;	/* Set: LSB first Clear: MSB first   */
    Word_t	RCK		: 1;	/* Set: HIGH high LRCK=left channel  */
    Word_t	BCK		: 1;	/* Data valid on falling edge of BCK */
    Word_t	_reserved9	: 1;
    /* Byte 18 */
    u_short	MaxWriteSpeed;		/* Max. write speed supported in KB/s*/
    /* Byte 20 */
    u_short	CurWriteSpeed;		/* Current write speed in KB/s	     */
#endif	/* _BIT_FIELDS_ */
} ScsiCDCap_t;

/*
 * Values for loading_type
 */
#define	LT_CADDY	0
#define	LT_TRAY		1
#define	LT_POP_UP	2
#define	LT_RES3		3
#define	LT_CHANGER_IND	4
#define	LT_CHANGER_CART	5
#define	LT_RES6		6
#define	LT_RES7		7

#define CD_LOAD_TYPES 	{ "Caddy", "Tray", "Pop-Up", "RESERVED3", \
      "Individual-Changer", "Cartridge-Changer", NULL }

/*
 * Flexible Disk Page - Page Code 0x05 (SCSI_DAD_MODE_FLEXDISK)
 * Applies to SCSI Block Command (SBC) devices.
 */
typedef struct {
    ScsiModePage_t	ModePage;	/* Common Mode Page header */
    u_short	TransferRate;		/* Transfer Rate in kbit/s */
    Word_t	NumHeads;		/* Number of Heads */
    Word_t	SectPerTrack;		/* Sectors Per Track */
    u_short	DataPerSector;		/* Data Bytes Per Sector */
    u_short	NumCyls;		/* Number of Cylinders */
    u_short	StartCylWrPre;		/* Starting Cylinder Write Precomp. */
    u_short	StartCylReWr;		/* Starting Cylinder Reduced Write */
    u_short	StepRate;		/* Device Step Rate (# of 100 us) */
    Word_t	StepPulseWidth;		/* Device Step Pulse Width (ms) */
    u_short	HeadSettleDelay; 	/* Head Settle Delay (# of 100 us) */
    Word_t	MotorOnDelay;		/* Motor On Delay (# 1/10 of sec) */
    Word_t	MotorOffDelay;		/* Motor Off Delay (# 1/10 of sec) */
#if defined(_BIT_FIELDS_LTOH)
    Word_t	_reserved1 	: 5;
    Word_t	MO 		: 1;	/* Motor On 0:Pin16 on 1:Pin16 off */
    Word_t	SSN 		: 1;	/* Start Sector Number */
    Word_t	TRDY 		: 1;	/* Medium ready for access */

    Word_t	SPC 		: 4;	/* Step Pulse Per Cylinder */
    Word_t	_reserved2 	: 4;
#elif defined(_BIT_FIELDS_HTOL)
    Word_t	TRDY 		: 1;	/* Medium ready for access */
    Word_t	SSN 		: 1;	/* Start Sector Number */
    Word_t	MO 		: 1;	/* Motor On 0:Pin16 on 1:Pin16 off */
    Word_t	_reserved1 	: 5;

    Word_t	_reserved2 	: 4;
    Word_t	SPC 		: 4;	/* Step Pulse Per Cylinder */
#endif	/* _BIT_FIELDS_ */
    Word_t	WriteComp;		/* Write compensation */
    Word_t	HeadLoadDelay;		/* Head Load Delay */
    Word_t	HeadUnloadDelay;	 /* Head Unload Delay */
#if defined(_BIT_FIELDS_LTOH)
    Word_t	Pin2 		: 4;
    Word_t	Pin34		: 4;
    Word_t	Pin1 		: 4;
    Word_t	Pin4 		: 4;
#elif defined(_BIT_FIELDS_HTOL)
    Word_t	Pin34 		: 4;
    Word_t	Pin2 		: 4;
    Word_t	Pin4 		: 4;
    Word_t	Pin1 		: 4;
#endif	/* _BIT_FIELDS_ */
    u_short	MediumRotate; 		/* Medium Rotation Rate (RPM) */
    Word_t	_reserved3;
    Word_t	_reserved4;
} ScsiFlexDisk_t;

/*
 * Speed Page (SCSI_DAD_MODE_SPEED)
 */
typedef struct {
    ScsiModePage_t 	ModePage;	/* common mode page header */
    Word_t	Speed;			/* drive speed */
    Word_t	_reserved1;
} ScsiSpeed_t;

/*
 * Sequential Access Device information
 */
#define SCSI_SAD_MODE_DATA_COMP	       0x0f		/* Data Compression */

/*
 * Data Compression page 0x0F (SCSI_SAD_MODE_DATA_COMP)
 * Applies to SCSI Sequential Commands (SSC)
 */
typedef struct {
    ScsiModePage_t 	ModePage;	/* common mode page header */
#if defined(_BIT_FIELDS_LTOH)
    Word_t	_reserved1 	: 6;
    Word_t	DCC 		: 1;	/* Data Compression Capable */
    Word_t	DCE 		: 1;	/* Data Comp currently enabled */
    Word_t	_reserved2 	: 5;
    Word_t	RED 		: 2;	/* Report Exception Decomp */
    Word_t	DDE 		: 1;	/* Data Decomp currently enabled */
#elif defined(_BIT_FIELDS_HTOL)
    Word_t	DCE 		: 1;	/* Data Comp currently enabled */
    Word_t	DCC 		: 1;	/* Data Compression Capable */
    Word_t	_reserved1 	: 6;
    Word_t	DDE 		: 1;	/* Data Decomp currently enabled */
    Word_t	RED 		: 2;	/* Report Exception Decomp */
    Word_t	_reserved2 	: 5;
#endif	/* _BIT_FIELDS_ */
    u_int	CompAlg;		/* Compression Algorithm */
    u_int	DecompAlg;		/* Decompression Algorithm */
    Word_t	_reserved3;
    Word_t	_reserved4;
    Word_t	_reserved5;
    Word_t	_reserved6;
} ScsiDataComp_t;

#endif	/* __myscsi_h__ */
