/* tftp.h */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */


/*
 * This file contains the structure definitions for tftp packets.
 */

#define DEFAULT_UID	113
#define DEFAULT_GID	101

#define	DATALEN	512		/* size of data portion of tftp pkt */

struct	tftp_rfc	{
	char	f_fname[1];	/* Remote file name */
	char	f_null1;	/* Zero */
	char	f_mode[1];	/* netascii or image */
	char	f_null2;	/* Zero */
};

struct	tftp_data{
	short unsigned	f_blkno;	/* Block number */
	char		f_blk[DATALEN];	/* Data */
};

struct tftp_error{
	short	f_errcode;	/* Error code */
	char	f_errmsg[1];	/* Error message */
	char	f_null3;	/* Zero */
};

struct	tftp	{
	short		fp_opcode;	/* header */
	union	{	struct	tftp_rfc	f_rfc;
			struct	tftp_data	f_data;
			struct	tftp_error	f_error;
		} fp_data;
};

	/* values for f_opcode */
#define	RRQ	1	/* Read Request */
#define	WRQ	2	/* Write Request */
#define	DATA	3	/* Data block */
#define	DACK	4	/* Data Acknowledge */
#define	ERROR	5	/* Error */
#define ARRQ	6	/* Authenticated Read Request */
#define AWRQ	7	/* Authenticated Write */

#define	TFTPSOCK	69		/* tftp socket number */
#define TFTPSIZ		sizeof(struct tftp)

/* values for error codes in ERROR packets */

#define TEUNDEF	0	/* Not defined, see error message (if any) */
#define TEFNF	1	/* File not found */
#define TEACESS	2	/* Access violation */
#define TEFULL	3	/* Disc full or allocation exceeded */
#define TETFTP	4	/* Illegal TFTP operation */
#define TETID	5	/* Unknown transfer ID */
#define	TEEXIST	6	/* File already exists */
#define TENOUSR	7	/* Bad user id for mail */
#define TEBAUTH 8	/* Bad authentication */
#define	MAXECODE	TEBAUTH

/* Random constants */

#define	READ	RRQ	/* read requested */
#define	WRITE	WRQ	/* write requested */
#define AREAD	ARRQ	/* authenticated read requested */
#define AWRITE	AWRQ	/* authenticated write requested */

#define	NETASCII	0		/* netascii transfer mode */
#define	IMAGE		1		/* image transfer mode */
#define	MAIL		2		/* mail transfer mode */

#define	INPKT		0		/* input packet */
#define	OUTPKT		1		/* output packet */

#define	INETLEN		576		/* max inet packet size */

#define	TRUE		1
#define	FALSE		0
