/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * This file contains ATA/IDE/EIDE definetions common to SysInfo.
 */

#ifndef __myata_h__
#define __myata_h__ 

#if	!defined(_BIT_FIELDS_LTOH) && !defined(_BIT_FIELDS_HTOL)
#error "One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined"
#endif	/* _BIT_FIELDS_ */

/*
 * SysInfo ATA definetions
 */
#define ATA_CMD_TIMEOUT		30		/* Timeout per dev (seconds) */
#define ATA_BUF_LEN		8192		/* Max ATA cmd buff size */

/*
 * Parameters for ATA queries - AtaQuery*()
 */
typedef Query_t			AtaQuery_t;	/* Use general Query */
typedef u_char			Word8_t;	/* 8 bit type */
typedef u_short			Word16_t;	/* 16 bit type */

/*
 * ATA Command Data Block
 */
typedef struct {
    u_char			Features;	/* Features */
    u_char			SectCount;	/* Sector Count */
    u_char			SectNum;	/* Sector Number */
    u_char			CylLow;		/* Cylinder Low */
    u_char			CylHigh;	/* Cylinder High */
#if 	defined(_BIT_FIELDS_LTOH)
    u_char		_obsolete2	: 4;
    u_char		Device		: 1;	/* Device to operate on */
    u_char		_obsolete1	: 3;
#elif	defined(_BIT_FIELDS_HTOL)
    u_char		_obsolete1	: 3;
    u_char		Device		: 1;	/* Device to operate on */
    u_char		_obsolete2	: 4;
#endif	/* _BIT_FIELDS_ */
    u_char			Cmd;		/* Command to run */
} AtaCdb_t;
#define ATA_CDB_OFFSET		(sizeof(AtaCdb_t))

/*
 * Parameters for AtaCmd()
 */
typedef struct {
    AtaCdb_t		       *Cdb;		/* ATA Command Data Block */
    char		       *CmdName;	/* String name of Cmd */
    char		       *DevFile;	/* Path to device file */
    int				DevFD;		/* File Descriptor */
    void		       *Data;		/* Return Data */
    size_t			DataLen;	/* Length of Data */
} AtaCmd_t;

/*
 * ATA Commands
 */
#define ATA_CMD_IDENT		0xEC	/* Read AtaIdent_t */
#define ATAPI_CMD_IDENT		0xA1	/* ATAPI Identify Device */

/*
 * AtaIdent_t based on IDENTIFY data as per
 *	ANSI ATA/ATAPI-4 T13 1153D Rev 17
 *
 * It's used by both ATA_CMD_IDENT and ATAPI_CMD_IDENT.  The contents
 * are similiar, but differ a bit depending on which of these commands is
 * used.
 *
 * Total size should be 512 bytes
 */
typedef struct {
    /* Word 0 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	_reserved0	: 1;
    Word16_t	_obsolete00	: 5;
    Word16_t	RemovableDev 	: 1;	/* Removable controller and/or dev */
    Word16_t	RemovableMedia 	: 1;	/* Removable media device */
    Word16_t	DevType		: 5;	/* ATAPI: Device Type==SCSI_DTYPE_* */
    Word16_t	_reserved01	: 2;	
    Word16_t 	ATAtype 	: 1;	/* 0=ATA, 1=ATAPI */
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t 	ATAtype 	: 1;	/* 0=ATA, 1=ATAPI */
    Word16_t	_reserved01	: 2;	
    Word16_t	DevType		: 5;
    Word16_t	RemovableMedia 	: 1;	/* Removable media device */
    Word16_t	RemovableDev 	: 1;	/* Removable controller and/or dev */
    Word16_t	_obsolete00	: 5;
    Word16_t	_reserved0	: 1;
#endif	/* _BIT_FIELDS_ */
    /* Word 1 */
    Word16_t	Cyls;			/* Physical Cylinders */
    Word16_t	RemCyl;			/* # Removable Cylinders */
    Word16_t	Heads;			/* Physical Heads */
    /* Word 4 */
    Word16_t	BytesPerTrack;		/* OBS: Physical bytes / track */
    Word16_t	BytesPerSect;		/* OBS: Physical bytes / sector */
    /* Word 6 */
    Word16_t	Sectors;		/* Physical sectors / track */
    /* Word 7 */
    Word16_t	Vendor0;		/* Vendor Unique */
    Word16_t	Vendor1;		/* Vendor Unique */
    Word16_t	Vendor2;		/* Vendor Unique */
    /* Word 10 */
    char	Serial[20];		/* Serial Number */
    /* Word 20 */
    Word16_t	BuffType;		/* Buffer type */
    Word16_t	BuffSize;		/* 512 byte units */
    /* Word 22 */
    Word16_t	NumEcc;			/* # ECC Bytes avail for rd/wr */
    /* Word 23 */
    char	Revision[8];		/* Revision */
    /* Word 27 */
    char	Model[40];		/* Model */
    /* Word 47 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	MaxMultiSect	: 8;	/* Max # Multiple Sectors */
    Word16_t	Vendor3		: 8;	/* Vendor Unique */
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	Vendor3		: 8;	/* Vendor Unique */
    Word16_t	MaxMultiSect	: 8;	/* Max # Multiple Sectors */
#endif	/* _BIT_FIELDS_ */
    /* Word 48 */
    Word16_t	DblWordIO;		/* Has double word I/O */
    /* Word 49 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	Vendor4		: 8;
    Word16_t	DMAsupported	: 1;	/* DMA is supported */
    Word16_t	LBAsupported	: 1;	/* LBA is supported */
    Word16_t	IORDYcanDisable	: 1;	/* IORDY can be disabled */
    Word16_t	IORDYsupported	: 1;	/* IORDY is supported */
    Word16_t	_reserved49	: 1;
    Word16_t	StandbyTimer	: 1;	/* 0=Vendor Spec, 1:Standard */
    Word16_t	_reserved491	: 2;
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	_reserved49	: 2;
    Word16_t	StandbyTimer	: 1;	/* 0=Vendor Spec, 1:Standard */
    Word16_t	_reserved491	: 1;
    Word16_t	IORDYsupported	: 1;	/* IORDY is supported */
    Word16_t	IORDYcanDisable	: 1;	/* IORDY can be disabled */
    Word16_t	LBAsupported	: 1;	/* LBA is supported */
    Word16_t	DMAsupported	: 1;	/* DMA is supported */
    Word16_t	Vendor4		: 8;
#endif	/* _BIT_FIELDS_ */
    /* Word 50 */
    Word16_t	_reserved50;
#if 	defined(_BIT_FIELDS_LTOH)
    /* Word 51 */
    Word16_t	Vendor5		: 8;	/* Vendor Unique */
    Word16_t	PioMode		: 8;	/* PIO timing mode: 0=slow 1=med 2=fast */
    /* Word 52 */
    Word16_t	Vendor6		: 8;	/* Vendor Unique */
    Word16_t	DmaMode		: 8;	/* DMA timing mode: 0=slow 1=med 2=fast */
    /* Word 53 */
    Word16_t	CurInfoOK	: 1;	/* Info in Words 54-58 is valid */
    Word16_t	EideInfoOK	: 1;	/* Info in Words 64-70 is valid */
    Word16_t	UltraDMAvalid	: 1;	/* Info in Word 88 is valid */
    Word16_t	_reserved53	: 13;
#elif	defined(_BIT_FIELDS_HTOL)
    /* Word 51 */
    Word16_t	PioMode		: 8;	/* PIO timing mode: 0=slow 1=med 2=fast */
    Word16_t	Vendor5		: 8;	/* Vendor Unique */
    /* Word 52 */
    Word16_t	DmaMode		: 8;	/* DMA timing mode: 0=slow 1=med 2=fast */
    Word16_t	Vendor6		: 8;	/* Vendor Unique */
    /* Word 53 */
    Word16_t	_reserved53	: 13;
    Word16_t	UltraDMAvalid	: 1;	/* Info in Word 88 is valid */
    Word16_t	EideInfoOK	: 1;	/* Info in Words 64-70 is valid */
    Word16_t	CurInfoOK	: 1;	/* Info in Words 54-58 is valid */
#endif	/* _BIT_FIELDS_ */
    /* Word 54 */
    Word16_t	CurCyls;		/* Logical # Cyls */
    Word16_t	CurHeads;		/* Logical # Heads */
    Word16_t	CurSectors;		/* Logical # Sects / Track */
    Word16_t	CurCapacity[2];		/* Logical total # sectors on drv */
    /* Word 59 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	SectsPerRWMulti	: 8;	/* # sectors xfer/inter R/W Multi cmd*/
    Word16_t	MultiSectValid	: 1;	/* Multiple sector setting is valid */
    Word16_t	_reserved59	: 7;
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	_reserved59	: 7;
    Word16_t	MultiSectValid	: 1;	/* Multiple sector setting is valid */
    Word16_t	SectsPerRWMulti	: 8;	/* # sectors xfer/inter R/W Multi cmd*/
#endif	/* _BIT_FIELDS_ */
    /* Word 60-61 */
    Word16_t	LBAcapacity[2];		/* Total # sectors */
    /* Word 62 */
    Word16_t	_obsolete62;
#if 	defined(_BIT_FIELDS_LTOH)
    /* Word 63 */
    Word16_t	DmaMWordMode0	: 1;	/* Multi-word DMA mode 0 */
    Word16_t	DmaMWordMode1	: 1;	/* Multi-word DMA mode 1 and below */
    Word16_t	DmaMWordMode2	: 1;	/* Multi-word DMA mode 2 and below */
    Word16_t	_reserved63	: 5;
    Word16_t	DmaMWordAct	: 8;	/* Multi-word DMA mode active */
    /* Word 64 */
    Word16_t	EidePioMode3	: 1;	/* PIO Mode 3 */
    Word16_t	EidePioMode4	: 1;	/* PIO Mode 4 */
    Word16_t	_reservedPIO	: 6;	/* PIO Mode reserved */
    Word16_t	_reserved64	: 8;
#elif	defined(_BIT_FIELDS_HTOL)
    /* Word 63 */
    Word16_t	DmaMWordAct	: 8;	/* Multi-word DMA mode active */
    Word16_t	_reserved63	: 5;
    Word16_t	DmaMWordMode2	: 1;	/* Multi-word DMA mode 2 and below */
    Word16_t	DmaMWordMode1	: 1;	/* Multi-word DMA mode 1 and below */
    Word16_t	DmaMWordMode0	: 1;	/* Multi-word DMA mode 0 */
    /* Word 64 */
    Word16_t	_reserved64	: 8;
    Word16_t	_reservedPIO	: 6;	/* PIO Mode reserved */
    Word16_t	EidePioMode4	: 1;	/* PIO Mode 4 */
    Word16_t	EidePioMode3	: 1;	/* PIO Mode 3 */
#endif	/* _BIT_FIELDS_ */
    /* Word 65 */
    Word16_t	EideDmaMin;	/* Min multiword DMA cycle time (ns) */
    Word16_t	EideDmaTime;	/* Recommended mword DMA cycle time (ns) */
    Word16_t	EidePio;	/* Min PIO cycle time (ns) */
    /* Word68 */
    Word16_t	EidePioIordy;	/* Min PIO cycle time (ns) with IORDY */
    Word16_t	_reserved69;
    Word16_t	_reserved70;
    Word16_t	_reserved71;
    Word16_t	_reserved72;
    Word16_t	_reserved73;
    Word16_t	_reserved74;
    /* Word75 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	MaxQueDepth	: 5;	/* Maximum queue depth */
    Word16_t	_reserved75	: 11;
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	_reserved75	: 11;
    Word16_t	MaxQueDepth	: 5;	/* Maximum queue depth */
#endif	/* _BIT_FIELDS_ */
    Word16_t	_reserved76;
    Word16_t	_reserved77;
    Word16_t	_reserved78;
    Word16_t	_reserved79;
    /* Word80 */			/* ATA version supported: */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	SupAta1   	: 1;	/* ATA-1 */
    Word16_t	SupAta2	  	: 1;	/* ATA-2 */
    Word16_t	SupAta3	  	: 1;	/* ATA-3 */
    Word16_t	SupAta4	  	: 1;	/* ATA-4 */
    Word16_t	SupAta5	  	: 1;	/* ATA-5 */
    Word16_t	SupAta6	  	: 1;	/* ATA-6 */
    Word16_t	SupAta7	  	: 1;	/* ATA-7 */
    Word16_t	SupAta8	  	: 1;	/* ATA-8 */
    Word16_t	SupAta9	  	: 1;	/* ATA-9 */
    Word16_t  	SupAta10  	: 1;	/* ATA-10 */
    Word16_t	SupAta11  	: 1;	/* ATA-11 */
    Word16_t	SupAta12  	: 1;	/* ATA-12 */
    Word16_t	SupAta13  	: 1;	/* ATA-13 */
    Word16_t   	SupAta14  	: 1;	/* ATA-14 */
    Word16_t	_reserved80	: 1;	/* Reserved */
    Word16_t	SupValid  	: 1;	/* is 0x0 or 0xffff if field invalid */
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	SupValid  	: 1;	/* is 0x0 or 0xffff if field invalid */
    Word16_t	_reserved80	: 1;	/* Reserved */
    Word16_t   	SupAta14  	: 1;	/* ATA-14 */
    Word16_t	SupAta13  	: 1;	/* ATA-13 */
    Word16_t	SupAta12  	: 1;	/* ATA-12 */
    Word16_t	SupAta11  	: 1;	/* ATA-11 */
    Word16_t	SupAta10  	: 1;	/* ATA-10 */
    Word16_t	SupAta9	  	: 1;	/* ATA-9 */
    Word16_t	SupAta8	  	: 1;	/* ATA-8 */
    Word16_t	SupAta7	  	: 1;	/* ATA-7 */
    Word16_t	SupAta6	  	: 1;	/* ATA-6 */
    Word16_t	SupAta5	  	: 1;	/* ATA-5 */
    Word16_t	SupAta4	  	: 1;	/* ATA-4 */
    Word16_t	SupAta3	  	: 1;	/* ATA-3 */
    Word16_t	SupAta2	  	: 1;	/* ATA-2 */
    Word16_t	SupAta1   	: 1;	/* ATA-1 */
#endif	/* _BIT_FIELDS_ */
    /* Word81 */
    Word16_t	MinorVers;		/* Minor Version info */
#if 	defined(_BIT_FIELDS_LTOH)
    /* Word82 */
    Word16_t	SMARTfeat	: 1;	/* Supports SMART feature set */
    Word16_t	SecurityFeat	: 1;	/* Supports Security feature set */
    Word16_t	RemovableFeat	: 1;	/* Supports Removeable feature set */
    Word16_t	PwrMgtFeat	: 1;	/* Supports Power Management */
    Word16_t	PACKETsup	: 1;	/* PACKET Command supported */
    Word16_t	WriteCacheSup	: 1;	/* Write cache supported */
    Word16_t	LookAheadSup	: 1;	/* Look-ahead supported */
    Word16_t	ReleaseSup	: 1;	/* Release interrupt supported */
    Word16_t	SERVICEsup	: 1;	/* SERVICE interrupt supported */
    Word16_t	DEVRESETsup	: 1;	/* DEVICE RESET command supported */
    Word16_t	HPAfeat		: 1;	/* Host Protected Area feature set */
    Word16_t	_obsolete821	: 1;
    Word16_t	WRITEBUFsup	: 1;	/* WRITE BUFFER command supported */
    Word16_t	READBUFsup	: 1;	/* READ BUFFER command supported */
    Word16_t	NOPsup		: 1;	/* NOP command supported */
    Word16_t	_obsolete820	: 1;
    /* Word83 */
    Word16_t	DownloadSup	: 1;	/* DOWNLOAD MICROCODE supported */
    Word16_t	DMAqueSup	: 1;	/* R/W DMA QUEUED supported */
    Word16_t	CFAfeat		: 1;	/* CFA feature set */
    Word16_t	APMfeat		: 1;	/* Advanced Pwr Mgt featue set */
    Word16_t	RMSNfeat	: 1;	/* Removable Media Status Notify*/
    Word16_t	_reserved83	: 9;
    Word16_t	Word83b14	: 1;	/* Should be set to 1 */
    Word16_t	Word83b15	: 1;	/* Should be set to 0 */
#elif	defined(_BIT_FIELDS_HTOL)
    /* Word82 */
    Word16_t	_obsolete820	: 1;
    Word16_t	NOPsup		: 1;	/* NOP command supported */
    Word16_t	READBUFsup	: 1;	/* READ BUFFER command supported */
    Word16_t	WRITEBUFsup	: 1;	/* WRITE BUFFER command supported */
    Word16_t	_obsolete821	: 1;
    Word16_t	HPAfeat		: 1;	/* Host Protected Area feature set */
    Word16_t	DEVRESETsup	: 1;	/* DEVICE RESET command supported */
    Word16_t	SERVICEsup	: 1;	/* SERVICE interrupt supported */
    Word16_t	ReleaseSup	: 1;	/* Release interrupt supported */
    Word16_t	LookAheadSup	: 1;	/* Look-ahead supported */
    Word16_t	WriteCacheSup	: 1;	/* Write cache supported */
    Word16_t	PACKETsup	: 1;	/* PACKET Command supported */
    Word16_t	PwrMgtFeat	: 1;	/* Supports Power Management */
    Word16_t	RemovableFeat	: 1;	/* Supports Removeable feature set */
    Word16_t	SecurityFeat	: 1;	/* Supports Security feature set */
    Word16_t	SMARTfeat	: 1;	/* Supports SMART feature set */
    /* Word83 */
    Word16_t	Word83b15	: 1;	/* Should be set to 0 */
    Word16_t	Word83b14	: 1;	/* Should be set to 1 */
    Word16_t	_reserved83	: 9;
    Word16_t	RMSNfeat	: 1;	/* Removable Media Status Notify*/
    Word16_t	APMfeat		: 1;	/* Advanced Pwr Mgt featue set */
    Word16_t	CFAfeat		: 1;	/* CFA feature set */
    Word16_t	DMAqueSup	: 1;	/* R/W DMA QUEUED supported */
    Word16_t	DownloadSup	: 1;	/* DOWNLOAD MICROCODE supported */
#endif	/* _BIT_FIELDS_ */
    Word16_t	CmdSup;			/* Future command set/features */
    Word16_t	CmdEnabled85;		/* Cmds/features enabled */
    Word16_t	CmdEnabled86;		/* Cmds/features enabled */
    Word16_t	CmdEnabled87;		/* Cmds/features enabled */
    /* Word88 */
#if 	defined(_BIT_FIELDS_LTOH)
    Word16_t	UltraDMAmode0	: 1;	/* Ultra DMA mode 0 supported */
    Word16_t	UltraDMAmode1	: 1;	/* Ultra DMA mode 1 supported */
    Word16_t	UltraDMAmode2	: 1;	/* Ultra DMA mode 2 supported */
    Word16_t	UltraDMAmode3	: 1;	/* Ultra DMA mode 3 supported */
    Word16_t	_reserved881	: 4;
    Word16_t	UltraDMAmode0sel: 1;	/* Ultra DMA mode 0 selected */
    Word16_t	UltraDMAmode1sel: 1;	/* Ultra DMA mode 1 selected */
    Word16_t	UltraDMAmode2sel: 1;	/* Ultra DMA mode 2 selected */
    Word16_t	UltraDMAmode3sel: 1;	/* Ultra DMA mode 3 selected */
    Word16_t	_reserved880	: 4;
#elif	defined(_BIT_FIELDS_HTOL)
    Word16_t	_reserved880	: 4;
    Word16_t	UltraDMAmode3sel: 1;	/* Ultra DMA mode 3 selected */
    Word16_t	UltraDMAmode2sel: 1;	/* Ultra DMA mode 2 selected */
    Word16_t	UltraDMAmode1sel: 1;	/* Ultra DMA mode 1 selected */
    Word16_t	UltraDMAmode0sel: 1;	/* Ultra DMA mode 0 selected */
    Word16_t	_reserved881	: 4;
    Word16_t	UltraDMAmode3	: 1;	/* Ultra DMA mode 3 supported */
    Word16_t	UltraDMAmode2	: 1;	/* Ultra DMA mode 2 supported */
    Word16_t	UltraDMAmode1	: 1;	/* Ultra DMA mode 1 supported */
    Word16_t	UltraDMAmode0	: 1;	/* Ultra DMA mode 0 supported */
#endif	/* _BIT_FIELDS_ */
    /* Word89 */
    Word16_t	SecureEraseTime;	/* Time required for security erase */
    /* Word90 */
    Word16_t	EnhSecureEraseTime;	/* Enhanced security erase time */
    /* Word91 */
    Word16_t	CurAPM;			/* Current APM value */
    Word16_t	_reserved[163];
    /* Word255 */
    Word16_t	_reserved255;
} AtaIdent_t;

/*
 * Values for ATAtype
 */
#define ATA_TYPE_ATA		0		/* ATA */
#define ATA_TYPE_ATAPI		1		/* ATAPI */

/*
 * Types for AtaIdent_t.BuffType
 */
#define ATA_BTYPE_SINGLE		1	/* Single port, single 
						   sector */
#define ATA_BTYPE_DUALMULTI		2	/* Dual port, multiple sector 
						   buffer */
#define ATA_BTYPE_DUALMULTICACHE	3	/* Dual port, multiple sector
						   buffer, track cache */

#endif	/* __myata_h__ */
