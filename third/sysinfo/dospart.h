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

#ifndef __dospart_h__
#define __dospart_h__ 

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
    /*#define DOSMID_386BSD		(0x14|0x80) /* 386bsd|bootable */
    u_char	dp_sid;
    /*#define DOSSID_386BSD		(0x44|0x80) /* 386bsd|active */	
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
#define DOS_NPARTS		4

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

#define DOS_ACTIVE		0x80	/* Is Active */
#define DOS_BOOT_MAGIC		0xAA55	/* Boot Magic signature */

#define MIN_SEC_SIZE		512
#define MAX_SEC_SIZE		2048

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
