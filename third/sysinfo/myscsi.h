/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * This file contains SCSI definetions common to SysInfo.
 */

#ifndef __myscsi_h__
#define __myscsi_h__ 

/*
 * SysInfo SCSI definetions
 */
#define MySCSI_CMD_TIMEOUT	30		/* Timeout per dev (seconds) */
#define SCSI_BUF_LEN		8192		/* Max SCSI cmd buff size */

/*
 * Parameters are passed between Scsi*() funcs using ScsiQuery_t
 */
typedef Query_t			ScsiQuery_t;	/* Use General Query */

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
    u_char			cmd;		/* SCSI cmd */
    u_char			tag;		/* LUN + LBA (MSB) */
    u_char			addr1;		/* Logical Block Address */
    u_char			addr2;		/* Logical Block Address(LSB)*/
    u_char			length;		/* Length of data requested */
    u_char			control;	/* Control Byte */
} ScsiCdbG0_t;

/*
 * CDB for Group 1 (10 byte) commands
 */
typedef struct {
    u_char			cmd;		/* SCSI cmd */
    u_char			tag;
    u_char			addr3;
    u_char			addr2;
    u_char			addr1;
    u_char			addr0;
    u_char			reserv1;
    u_char			count1;
    u_char			count0;
    u_char			misc;
} ScsiCdbG1_t;

/*
 * SCSI Commands we need
 */
	/* SCSI GROUP 0 Commands */
#define CMD_INQUIRY		0x12
#define CMD_MODE_SENSE		0x1a
	/* SCSI GROUP 1 Commands */
#define CMD_CAPACITY		0x25

/*
 * SCSI Capacity (CMD_CAPACITY)
 */
typedef struct {
    u_long			Blocks;		/* Number of blocks */
    u_long			BlkSize;	/* Size of blocks */
} ScsiCapacity_t;

/*
 * SCSI Inquiry Data (CMD_INQUIRY)
 *
 * Format of data returned as a result of an INQUIRY command.
 *
 */

#if defined(_BIT_FIELDS_LTOH)
typedef struct {

	/*
	 * byte 0
	 *
	 * Bits 7-5 are the Peripheral Device Qualifier
	 * Bits 4-0 are the Peripheral Device Type
	 *
	 */

	u_char	inq_dtype;

	/* byte 1 */
	u_char	inq_qual	: 7,	/* device type qualifier	*/
		inq_rmb		: 1;	/* removable media		*/

	/* byte 2 */
	u_char	inq_ansi	: 3,	/* ANSI version 		*/
		inq_ecma	: 3,	/* ECMA version 		*/
		inq_iso		: 2;	/* ISO version 			*/

	/* byte 3 */
	u_char	inq_rdf		: 4,	/* response data format 	*/
				: 1,	/* reserved 			*/
		inq_normaca	: 1,	/* setting NACA bit supported */
		inq_trmiop	: 1,	/* TERMINATE I/O PROC msg 	*/
		inq_aenc	: 1;	/* async event notification cap. */

	/* bytes 4-7 */

	u_char	inq_len;		/* additional length 		*/

	u_char			: 8;	/* reserved 			*/

	u_char	inq_addr16	: 1,	/* supports 16 bit wide SCSI addr */
		inq_addr32	: 1,	/* supports 32 bit wide SCSI addr */
		inq_ackqreqq	: 1,	/* data tranfer on Q cable */
		inq_mchngr	: 1,	/* embedded/attached to medium chngr */
		inq_dualp	: 1,	/* dual port device */
		inq_port	: 1,	/* port receiving inquiry cmd */
				: 2;	/* reserved 			*/

	u_char	inq_sftre	: 1,	/* supports Soft Reset option 	*/
		inq_cmdque	: 1,	/* supports command queueing 	*/
		inq_trandis	: 1,	/* supports transfer disable messages */
		inq_linked	: 1,	/* supports linked commands 	*/
		inq_sync	: 1,	/* supports synchronous data xfers */
		inq_wbus16	: 1,	/* supports 16 bit wide data xfers */
		inq_wbus32	: 1,	/* supports 32 bit wide data xfers */
		inq_reladdr	: 1;	/* supports relative addressing */

	/* bytes 8-35 */

	char	inq_vid[8];		/* vendor ID 			*/
	char	inq_pid[16];		/* product ID 			*/
	char	inq_revision[4];	/* revision level 		*/
	char	inq_serial[12];
	/*
	 * Bytes 36-55 are vendor-specific.
	 * Bytes 56-95 are reserved.
	 * 96 to 'n' are vendor-specific parameter bytes
	 */
} ScsiInquiry_t;

#elif defined(_BIT_FIELDS_HTOL)

typedef struct {

	/*
	 * byte 0
	 *
	 * Bits 7-5 are the Peripheral Device Qualifier
	 * Bits 4-0 are the Peripheral Device Type
	 *
	 */

	u_char	inq_dtype;

	/* byte 1 */
	u_char	inq_rmb		: 1,	/* removable media */
		inq_qual	: 7;	/* device type qualifier */

	/* byte 2 */
	u_char	inq_iso		: 2,	/* ISO version */
		inq_ecma	: 3,	/* ECMA version */
		inq_ansi	: 3;	/* ANSI version */

	/* byte 3 */
	u_char	inq_aenc	: 1,	/* async event notification cap. */
		inq_trmiop	: 1,	/* supports TERMINATE I/O PROC msg */
		inq_normaca	: 1,	/* setting NACA bit supported */
				: 1,	/* reserved */
		inq_rdf		: 4;	/* response data format */

	/* bytes 4-7 */

	u_char	inq_len;		/* additional length */

	u_char			: 8;	/* reserved */

	u_char			: 2,	/* reserved 			*/
		inq_port	: 1,	/* port receiving inquiry cmd */
		inq_dualp	: 1,	/* dual port device */
		inq_mchngr	: 1,	/* embedded/attached to medium chngr */
		inq_ackqreqq	: 1,	/* data tranfer on Q cable */
		inq_addr32	: 1,	/* supports 32 bit wide SCSI addr */
		inq_addr16	: 1;	/* supports 16 bit wide SCSI addr */

	u_char	inq_reladdr	: 1,	/* supports relative addressing */
		inq_wbus32	: 1,	/* supports 32 bit wide data xfers */
		inq_wbus16	: 1,	/* supports 16 bit wide data xfers */
		inq_sync	: 1,	/* supports synchronous data xfers */
		inq_linked	: 1,	/* supports linked commands */
		inq_trandis	: 1,	/* supports transfer disable messages */
		inq_cmdque	: 1,	/* supports command queueing */
		inq_sftre	: 1;	/* supports Soft Reset option */

	/* bytes 8-35 */

	char	inq_vid[8];		/* vendor ID */

	char	inq_pid[16];		/* product ID */

	char	inq_revision[4];	/* revision level */

	char	inq_serial[12];		/* serial number */
	/*
	 * Bytes 36-55 are vendor-specific.
	 * Bytes 56-95 are reserved.
	 * 96 to 'n' are vendor-specific parameter bytes
	 */
} ScsiInquiry_t;
#else
error One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined
#endif	/* _BIT_FIELDS_LTOH */

/*
 * Defined Response Data Formats
 *
 * RDF_LEVEL0 means that this structure complies with SCSI-1 spec.
 *
 * RDF_CCS means that this structure complies with CCS pseudo-spec.
 *
 * RDF_SCSI2 means that the structure complies with the SCSI-2/3 spec.
 */

#define	RDF_LEVEL0		0x00
#define	RDF_CCS			0x01
#define	RDF_SCSI2		0x02

/*
 * SCSI Device Types
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
 * SENSE Data
 */

/*
 * Direct Access device Mode Sense/Mode Select Defined pages
 */

#define	DAD_MODE_ERR_RECOV	0x01
#define	DAD_MODE_FORMAT		0x03
#define	DAD_MODE_GEOMETRY	0x04
#define	DAD_MODE_FLEXDISK	0x05
#define	DAD_MODE_VRFY_ERR_RECOV	0x07
#define	DAD_MODE_CACHE		0x08
#define	DAD_MODE_MEDIA_TYPES	0x0B
#define	DAD_MODE_NOTCHPART	0x0C
#define	DAD_MODE_POWER_COND	0x0D

/*
 * Mode Sense/Select Header.
 *
 * Mode Sense/Select data consists of a header, followed by zero or more
 * block descriptors, followed by zero or more mode pages.
 *
 */

typedef struct {
	u_char length;		/* number of bytes following */
	u_char medium_type;	/* device specific */
	u_char device_specific;	/* device specfic parameters */
	u_char bdesc_length;	/* length of block descriptor(s), if any */
} ScsiModeHeader_t;
#define	SCSI_MODE_HEADER_LENGTH	(sizeof (ScsiModeHeader_t))
/*
 * Define a macro to retrieve the first mode page. Could be more
 * general (for multiple mode pages).
 */
#define	SCSI_MODE_PAGE_ADDR(mhdr, type)	\
  ((type *)(((u_long)(mhdr))+SCSI_MODE_HEADER_LENGTH+(mhdr)->bdesc_length))


/*
 * Mode page header. Zero or more Mode Pages follow either the block
 * descriptors (if any), or the Mode Header.
 *
 * The 'ps' bit must be zero for mode select operations.
 *
 */

typedef struct {
#if defined(_BIT_FIELDS_LTOH)
	u_char	code	:6,	/* page code number */
			:1,	/* reserved */
		ps	:1;	/* 'Parameter Saveable' bit */
#elif defined(_BIT_FIELDS_HTOL)
	u_char	ps	:1,	/* 'Parameter Saveable' bit */
			:1,	/* reserved */
		code	:6;	/* page code number */
#else
error One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined
#endif	/* _BIT_FIELDS_LTOH */
	u_char	length;		/* length of bytes to follow */
	/*
	 * Mode Page specific data follows right after this...
	 */
} ScsiModePage_t;

/*
 * Page 0x3 - Direct Access Device Format Parameters
 */

typedef struct  {
	ScsiModePage_t mode_page;	/* common mode page header */
	u_short	tracks_per_zone;	/* Handling of Defects Fields */
	u_short	alt_sect_zone;
	u_short alt_tracks_zone;
	u_short	alt_tracks_vol;
	u_short	sect_track;		/* Track Format Field */
	u_short data_bytes_sect;	/* Sector Format Fields */
	u_short	interleave;
	u_short	track_skew;
	u_short	cylinder_skew;
#if defined(_BIT_FIELDS_LTOH)
	u_char			: 3,
		_reserved_ins	: 1,	/* see <scsi/impl/mode.h> */
			surf	: 1,
			rmb	: 1,
			hsec	: 1,
			ssec	: 1;	/* Drive Type Field */
#elif defined(_BIT_FIELDS_HTOL)
	u_char		ssec	: 1,	/* Drive Type Field */
			hsec	: 1,
			rmb	: 1,
			surf	: 1,
		_reserved_ins	: 1,	/* see <scsi/impl/mode.h> */
				: 3;
#else
error One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined
#endif	/* _BIT_FIELDS_LTOH */
	u_char	reserved[2];
} ScsiModeFormat_t;

/*
 * Page 0x4 - Rigid Disk Drive Geometry Parameters
 */

typedef struct {
	ScsiModePage_t mode_page;	/* common mode page header */
	u_char	cyl_ub;			/* number of cylinders */
	u_char	cyl_mb;
	u_char	cyl_lb;
	u_char	heads;			/* number of heads */
	u_char	precomp_cyl_ub;		/* cylinder to start precomp */
	u_char	precomp_cyl_mb;
	u_char	precomp_cyl_lb;
	u_char	current_cyl_ub;		/* cyl to start reduced current */
	u_char	current_cyl_mb;
	u_char	current_cyl_lb;
	u_short	step_rate;		/* drive step rate */
	u_char	landing_cyl_ub;		/* landing zone cylinder */
	u_char	landing_cyl_mb;
	u_char	landing_cyl_lb;
#if defined(_BIT_FIELDS_LTOH)
	u_char		rpl	: 2,	/* rotational position locking */
				: 6;
#elif defined(_BIT_FIELDS_HTOL)
	u_char			: 6,
			rpl	: 2;	/* rotational position locking */
#else
error One of _BIT_FIELDS_LTOH or _BIT_FIELDS_HTOL must be defined
#endif	/* _BIT_FIELDS_LTOH */
	u_char	rotational_offset;	/* rotational offset */
	u_char	reserved;
	u_short	rpm;			/* rotations per minute */
	u_char	reserved2[2];
} ScsiModeGeometry_t;

/*
 * Page 0x31: CD-ROM speed page
 */

#define	CDROM_MODE_SPEED	0x31

typedef struct {
	ScsiModePage_t mode_page;	/* common mode page header */
	u_char	speed;			/* drive speed */
	u_char	reserved;
} ScsiModeSpeed_t;

#endif	/* __myscsi_h__ */
