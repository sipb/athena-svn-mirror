/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-hpux.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __os_hpux_h__
#define __os_hpux_h__

/*
 * HP HP-UX
 */

#include <sys/param.h>

#define MAN_SHORT		"HP"
#define MAN_LONG		"Hewlett-Packard Company"

#if OSMVER == 10
#define NAMELIST		"/stand/vmunix"
#else
#define NAMELIST		"/hp-ux"
#endif

#define VERSION_SYM		"_release_version"
#define PHYSMEM_SYM		"physmem"
#
#define IS_POSIX_SOURCE		1
#define NEED_KVM		1
#define HAVE_NLIST		1
#define RE_TYPE			RE_REGCOMP

/*
 * Run Test Programs using sh(1) since HPUX (9.x)
 * does not put a #! in the header of /bin/hp9000s* programs.
 */
#define RUN_TEST_CMD		{ "/bin/sh", "-c", NULL }

/*
 * Constants
 */
#define HP_KA_HP200		"hp9000s200"
#define HP_KA_HP300		"hp9000s300"
#define HP_KA_HP400		"hp9000s400"
#define HP_KA_HP500		"hp9000s500"
#define HP_KA_HP700		"hp9000s700"
#define HP_KA_HP800		"hp9000s800"
#define HP_AA_PARISC		"PA-RISC"
#define HP_AA_MC68K		"MC680x0"

/*
 * Compatibility
 */
#define setreuid(r,e)		setresuid(r,e,-1)
#define GETPAGESIZE()		sysconf(_SC_PAGE_SIZE)

#endif	/* __os_hpux_h__ */
