/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: kdt-sunos.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 *
 * Kernel Device Tree info for SunOS
 */

#ifndef __kdt_sunos_h
#define __kdt_sunos_h

#if OSMVER >= 5
#if	defined(HAVE_OPENPROM)
#	include <sys/openprom.h>
#endif	/* HAVE_OPENPROM */
#	include <sys/ddi.h>
#	include <sys/sunddi.h>
#	include <sys/ddi_impldefs.h>
#	define DEVI_CHILD(dev)		dev->devi_child
#	define DEVI_SIBLING(dev)	dev->devi_sibling
#	define DEVI_UNIT(dev)		dev->devi_instance
#	define DEVI_NODEID(dev)		dev->devi_nodeid
#	define DEVI_EXISTS(dev)		(dev->devi_addr && dev->devi_ops)
#	define DEVI_PROPS(dev)		dev->devi_sys_prop_ptr
#else
#if	defined(HAVE_OPENPROM)
#	include <sun/openprom.h>
#endif	/* HAVE_OPENPROM */
#	define DEVI_CHILD(dev)		dev->devi_slaves
#	define DEVI_SIBLING(dev)	dev->devi_next
#	define DEVI_UNIT(dev)		dev->devi_unit
#	define DEVI_NODEID(dev)		0 /* No such member */
#	define DEVI_EXISTS(dev)		dev->devi_driver
#	define DEVI_PROPS(dev)		0 /* No such member */
#endif	/* OSMVER == 5 */

/*
 * Various declarations
 */
typedef struct dev_info		KDTdevinfo_t;
#if	defined(HAVE_DDI_PROP)
typedef ddi_prop_t		KDTprop_t;
#endif	/* HAVE_DDI_PROP */

extern char		       *GetSysModel();
extern char		       *KDTgetSysModel();
extern char		       *OBPgetSysModel();
extern char		       *KDTcleanName();

#endif	/* __kdt_sunos_h__ */
