/*
 * auth.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including auth.p.h at beginning of auth.h file. */

/* Copyright (C) 1990 Transarc Corporation - All rights reserved */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/auth/RCS/auth.p.h,v 2.5 1993/11/08 21:04:04 vasilis Exp $ */

#ifndef __AUTH_AFS_INCL_
#define	__AUTH_AFS_INCL_    1

#include <rx/rxkad.h>			/* to get ticket parameters/contents */

/* super-user pincipal used by servers when talking to other servers */
#define AUTH_SUPERUSER        "afs"

struct ktc_token {
    int32 startTime;
    int32 endTime;
    struct ktc_encryptionKey sessionKey;
    short kvno;  /* XXX UNALIGNED */
    int ticketLen;
    char ticket[MAXKTCTICKETLEN];
};

#if 0
#define	KTC_ERROR	1	/* an unexpected error was encountered */
#define	KTC_TOOBIG	2	/* a buffer was too small for the response */
#define	KTC_INVAL	3	/* an invalid argument was passed in */
#define	KTC_NOENT	4	/* no such entry */
#endif

#endif /* __AUTH_AFS_INCL_ */

/* End of prolog file auth.p.h. */

#define KTC_ERROR                                (11862784L)
#define KTC_TOOBIG                               (11862785L)
#define KTC_INVAL                                (11862786L)
#define KTC_NOENT                                (11862787L)
#define KTC_PIOCTLFAIL                           (11862788L)
#define KTC_NOPIOCTL                             (11862789L)
#define KTC_NOCELL                               (11862790L)
#define KTC_NOCM                                 (11862791L)
extern void initialize_ktc_error_table ();
#define ERROR_TABLE_BASE_ktc (11862784L)

/* for compatibility with older versions... */
#define init_ktc_err_tbl initialize_ktc_error_table
#define ktc_err_base ERROR_TABLE_BASE_ktc
