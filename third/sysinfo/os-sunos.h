/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-sunos.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __os_sunos_h__
#define __os_sunos_h__

#if	defined(HAVE_OPENPROM)

/*
 * OBP device file
 */
#define _PATH_OPENPROM		"/dev/openprom"

/*
 * OBP keywords
 */
#define OBP_BANNERNAME		"banner-name"
#define OBP_BOARD		"board#"
#define OBP_BOARDTYPE		"board-type"
#define OBP_CLOCKFREQ		"clock-frequency"
#define OBP_CPUID		"cpu-id"
#define OBP_DCACHESIZE		"dcache-size"
#define OBP_DEVICEID		"device-id"
#define OBP_DEVTYPE		"device_type"
#define OBP_ECACHESIZE		"ecache-size"
#define OBP_FHC			"fhc"
#define OBP_KEYBOARD		"keyboard"
#define OBP_MEMUNIT		"mem-unit"
#define OBP_MODEL		"model"
#define OBP_NAME		"name"
#define OBP_SIZE		"size"
#define OBP_SYSBOARD		"sysboard"

#endif	/* HAVE_OPENPROM */

/*
 * Cpu Type information
 */
#if	OSMVER == 5
typedef short			cputype_t;
#else
typedef int			cputype_t;
#endif	/* OSMVER == 5 */
extern cputype_t		CpuType;

/*
 * Functions
 */
extern int			IsString();
extern char		       *DecodeVal();
extern char		       *ExpandKey();

#endif	/* __os_sunos_h__ */
