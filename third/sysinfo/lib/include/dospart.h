/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __dospart_h__
#define __dospart_h__ 

/*
 * Query type
 */
typedef struct {
    DevInfo_t		       *DevInfo;	/* DevInfo to add info to */
    char		       *CtlDev;		/* Device to read info from */
    char		       *BasePath;	/* Base path of partitions */
    int				Flags;		/* Query flags */
} DosPartQuery_t;

/*
 * This stuff is from <sys/disklabel.h> in FreeBSD 2.2.6
 */
#ifdef PC98
#define DOS_BB_SECTOR		0	/* DOS Boot Block relative sector */
#define DOS_LABEL_SECTOR	1	/* 0: 256b/s, 1: 512b/s */
#define	DOS_PART_OFF		0	/* Partition offset */
#define DOS_NPARTS		16	/* # of DOS Partitions */

typedef struct {
    u_char	dp_mid;
    u_char	dp_sid;
    u_char	dp_dum1;
    u_char	dp_dum2;
    u_char	dp_ipl_sct;
    u_char	dp_ipl_head;
    u_short	dp_ipl_cyl;
    u_char	dp_ssect;		/* starting sector */
    u_char	dp_shd;			/* starting head */
    u_short	dp_scyl;		/* starting cylinder */
    u_char	dp_esect;		/* end sector */
    u_char	dp_ehd;			/* end head */
    u_short	dp_ecyl;		/* end cylinder */
    u_char	dp_name[16];
} DosPart_t;

#else /* IBMPC */
#define DOS_BB_SECTOR		0	/* DOS boot block relative sector */
#define DOS_PART_SIZE		446
#define DOS_NPARTS		4	/* Max # primary partitions */
#define DOS_NPARTS_EXTENDED	60	/* Max # extended partitions */

typedef struct {
    u_char	dp_flags;		/* bootstrap flags */
    u_char	dp_shd;			/* starting head */
    u_char	dp_ssect;		/* starting sector */
    u_char	dp_scyl;		/* starting cylinder */
    u_char	dp_type;		/* partititon type */
    u_char	dp_ehd;			/* end head */
    u_char	dp_esect;		/* end sector */
    u_char	dp_ecyl;		/* end cylinder */
    u_long	dp_start;		/* absolute starting sector number */
    u_long	dp_size;		/* partition size in sectors */
} DosPart_t;
#endif

/*
 * Partition Types
 */
typedef struct {
    u_char		Type;		/* Value == dp_typ */
    char	       *Desc;		/* Description */
} DosPartType_t;

#define DOS_TYPE_EXTENDED	0x05	/* Extended Partitin */
#define DOS_TYPE_WIN98_EXTENDED	0x0f	/* Win98 Extended Partitin */
#define DOS_TYPE_LINUX_EXTENDED	0x85	/* Linux Extended Partitin */
#define IS_EXTENDED(i) \
	((i) == DOS_TYPE_EXTENDED || (i) == DOS_TYPE_WIN98_EXTENDED || \
	 (i) == DOS_TYPE_LINUX_EXTENDED )

#define DOS_ACTIVE		0x80	/* Is Active (DosPart_t.dp_flags) */
#define DOS_BOOT_MAGIC		0xAA55	/* Boot Magic signature */

#define MIN_SEC_SIZE		512
#define MAX_SEC_SIZE		2048

#define DOS_EXTENDED_OFFSET	446	/* Offset from start of Extended 
					   Partition to where DosPart_t
					   starts */

/*
 * Master Boot Record
 */
typedef struct {
    u_char		Padding[2];
    u_char		BootInst[DOS_PART_SIZE];
    DosPart_t		Parts[DOS_NPARTS];
    u_short 		Signature;
    /* Overflow allows us to read in large MBR's */
    u_char		Overflow[MAX_SEC_SIZE-MIN_SEC_SIZE];
} MBR_t;

#endif /* __dospart_h__ */
