/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-sppux.h,v 1.1.1.1 1996-10-07 20:16:56 ghudson Exp $
 */

#ifndef __os_sppux_h__
#define __os_sppux_h__

/*
 * Convex SPP-UX
 */

#include <sys/param.h>

#define MAN_LONG		"Convex Computer Corporation"
#define MAN_SHORT		"Convex"
#define MODEL_NAME		"Exemplar SPP"
#define HAVE_NLIST		1
#define IS_POSIX_SOURCE		1
#define NEED_KVM		1
#define RE_TYPE			RE_REGCOMP

#define KMEMFILE		"/dev/uxmem"
#define NAMELIST		"/os/mach"

#ifdef __convex_spp1
#define KARCH_TYPE		"SPP1"
#endif	/* __convex_spp1 */

/*
 * Constants
 */
#define HP_AA_PARISC		"PA-RISC"
#define HP_AA_MC68K		"MC680x0"

/*
 * Compatibility
 */
#define setreuid(r,e)		setresuid(r,e,-1)
#define GETPAGESIZE()		sysconf(_SC_PAGE_SIZE)

#endif	/* __os_sppux_h__ */
