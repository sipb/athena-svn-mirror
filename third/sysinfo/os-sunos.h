/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.3 $
 */

#ifndef __os_sunos_h__
#define __os_sunos_h__

/*
 * Default Delta for CPUspeed comparison
 */
#define CPU_SPEED_DELTA		2

/*
 * Graphics defines
 */
#define AFB_NAME		"afb"		/* Elite */
#define PGX_NAME		"pgx"		/* m64/PGX */
#define FFB_NAME		"ffb"		/* Creator */
/* Special FFB defines */
#define FFB_DBLNAME		"ffbdbl"	/* Match for sunos.cf */
#define FFB_DBL_BUFFER		0x01		/* Has double buf flag */
#define FFB_Z_BUFFER		0x02		/* Has Z buf flag */

/*
 * List of driver names which provide Serial numbers (DevInfo->Serial)
 * which are known to "bad".  Usually this means they don't contain unique
 * fields.  Such devices are ignored when looking for duplicate entries
 * in the Device tree.
 */
#define IGNORE_SERIAL_LIST	{ "atf", NULL }

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
