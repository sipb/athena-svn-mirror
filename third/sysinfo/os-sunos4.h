/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-sunos4.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __os_sunos4_h__
#define __os_sunos4_h__

/*
 * SunOS 4.x
 */

#include <stdio.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <machine/cpu.h>
#include <mon/idprom.h>
#include <sun/dkio.h>
#include <sun/dklabel.h>
#include <sun/fbio.h>

/*
 * What manufacturer is this?
 */
#if defined(TAD_SPBK_ARCH)
#	define MAN_SHORT	"Tadpole"
#	define MAN_LONG		"Tadpole Technology Inc"
#	define TADPOLE
#else
#if defined(CPU_TYPE_SERIES4) || defined(CPU_TYPE_SERIES5) || \
    	   defined(CPU_TYPE_SERIES5E) || defined(CPU_TYPE_SERIES6)
#	define MAN_SHORT	"Solbourne"
#	define MAN_LONG		"Solbourne Computer Corporation"
#	define SOLBOURNE
#else
#	define MAN_SHORT	"Sun"
#	define MAN_LONG		"Sun Microsystems Incorporated"
#	define HAVE_SUNROMVEC
#endif
#endif

#if defined(mc68020)
#	define ARCH_TYPE 	"sun3"
#	define HAVE_MAINBUS
#endif
#if defined(sparc)
#	define ARCH_TYPE 	"sun4"
#	if !defined(TADPOLE) && !defined(SOLBOURNE) && !defined(SUN4E_ARCH)
#		define HAVE_IPI
#	endif
#	if !defined(TADPOLE)
#		define HAVE_MAINBUS
#	endif
#endif
#if defined(i386)
#	define ARCH_TYPE 	"sun386"
#	define HAVE_MAINBUS
#endif
#if defined(OPENPROMS)
#	define HAVE_OPENPROM
#endif
#if !defined(SOLBOURNE)
#	define HAVE_IDPROM
#endif
#define HAVE_ANONINFO
#define HAVE_DEVICE_SUPPORT
#define HAVE_GETHOSTID
#define HAVE_GETMODEL_FIRST
#define HAVE_GETROM_FIRST
#define HAVE_GETSERIAL_FIRST
#define HAVE_KVM
#define HAVE_NIT
#define HAVE_NLIST
#define HAVE_PHYSMEM
#define HAVE_UNAME
#define HAVE_WAITPID
#define HAVE_SYSCONF
#define NEED_SOCKIO
#if !defined(SECSIZE)
#	define SECSIZE	512		/* Size of a disk sector */
#endif
#define KERNSTR_END	'\n'
#define RE_TYPE			RE_COMP

/*
 * This is to get around an error on Sun386i's in <sun386/cpu.h>
 */
#if defined(SUN386_ARCH) && !defined(I386_ARCH)
#define I386_ARCH SUN386_ARCH
#endif

#include <sys/buf.h>

#if 	defined(TADPOLE)
#	include <taddev/ide_drvr_def.h>
#else	/* !TADPOLE */
#	if 	defined(HAVE_IPI)
#		include <sundev/ipvar.h>
#	endif 	/* HAVE_IPI */
#	if	defined(XYLOGICS_IPI)
#		include <sundev/xlreg.h>
#		include <sundev/xlextensions.h>
#	endif	/* XYLOGICS_IPI */
#	include <sundev/xdreg.h>
#	include <sundev/xyreg.h>
#if	defined(NEED_SUNDEV_SCSI_H)	/* Avoid conflicts with stdef.h */
#	include <sundev/scsi.h>
#endif	/* NEED_SUNDEV_SCSI_H */
#	if 	defined(i386)
#		include <sundev/sdreg.h>
#	endif
#endif	/* TADPOLE */

#if defined(HAVE_MAINBUS)
#	include <sundev/mbvar.h>
#endif

#if defined(HAVE_OPENPROM)
#	include <sundev/openpromio.h>
#endif

/*
 * For Sun 3/80 running SunOS 4.0.3
 *
 * NOTE: This code is not supported and may cease to work at any time.
 */
#if defined(SUN3X_ARCH)

typedef	int			pid_t;
#define	KIOCLAYOUT		_IOR(k, 20, int)
#define	KB_VT220I		0x82	/* International vt220 Emulation */
#undef	HAVE_WAITPID
#define	HAVE_WAIT4
#undef	HAVE_SUNROMVEC		/* Sun 3/80 ROM version crashes system */

#endif	/* SUN3X_ARCH */

/*
 * We're done define HAVE_* features
 */
#include "os-sunos.h"

/*
 * Declarations
 */
char			       *strchr();

/*
 * Types
 */
#define WAITARG_T		int
typedef struct dk_conf		DKconf;
typedef struct dk_geom		DKgeom;
typedef struct dk_info		DKinfo;
typedef struct dk_label		DKlabel;
typedef struct dk_type		DKtype;

#endif	/* __os_sunos4_h__ */
