/*
 * Copyright (c) 1992-1998 by Michael A. Cooper.
 * All rights reserved.
 *
 * This software is subject to the terms found at 
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * MagniComp License header
 */

#ifndef __mcl_h__
#define __mcl_h__

#include <time.h>
#include "mconfig.h"

#ifndef CNULL
#define CNULL '\0'
#endif

/*
 * Environment variable name for MC License file to use
 */
#define MC_LICENSE_ENV		"MC_LICENSE"

/*
 * Status on finding/not-finding license
 */
#define MCL_STAT_OK		0		/* Everything checks out */
#define MCL_STAT_NOTFOUND	1		/* Couldn't find it */
#define MCL_STAT_EXPIRED	2		/* Expired */
#define MCL_STAT_INVALID	3		/* Corrupt, etc */
#define MCL_STAT_UNKNOWN	4		/* Unknown */

/*
 * Values for mcl_t.Type
 */
#define MCL_FULL		0x01		/* Full license */
#define MCLN_FULL		"full"
#define MCL_DEMO		0x02		/* Demo (Tmp) license */
#define MCLN_DEMO		"demo"

/*
 * MagniComp License type
 */
struct _mcl {
    /* Values returned on success */
    char		       *Key;		/* License Key */
    int				Type;		/* Type of License */
    time_t			Expires;	/* When we expire */
    char		       *HomePage;	/* URL of product */
    char		       *LicenseURL;	/* URL of license */
    int				NumRTU;		/* Number of RTU's */
    int			        OwnerID;	/* ID of Owner */
    char		       *OwnerName;	/* Name of Owner */
    int				Status;		/* One of MCL_STAT_* */
    /* Return value on error */
    char		       *Msg;		/* Messages */
    /* Values used for query */
    char		       *Product;	/* Name of product */
    char		       *ProdVers;	/* Version of product */
    char		       *File;		/* Name of MCL file to use */
    /* Internal usage */
    struct _mcl		       *Next;
};
typedef struct _mcl		mcl_t;

/*
 * MCL filename extension
 */
#define MCL_FILE_EXT		".mcl"

/*
 * Function Declarations
 */
extern char		       *MCLgetInfo();
extern int			MCLcheck();

#endif	/* __mcl_h__ */
